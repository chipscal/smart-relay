using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Linq;
using MQTTnet;
using MQTTnet.Extensions.ManagedClient;
using System.Text;
using System.Text.RegularExpressions;
using MQTTnet.Client;
using MQTTnet.Packets;
using System.Security.Cryptography.X509Certificates;
using System.Diagnostics;

namespace Clab.Smart.Relay.App;

public class MqttSettings
{
    public string       ClientId                        { get; set; }
    public string       Address                         { get; set; }
    public int          Port                            { get; set; }
    public string       Username                        { get; set; }
    public string       Password                        { get; set; }
    public string       CaFile                          { get; set; }
    public string       CertificatePfx                  { get; set; }
}

public class MQTTNetCertificateProvider : IMqttClientCertificatesProvider
{
    public string PFXfilePath   { get; set; }

    public MQTTNetCertificateProvider(string pfxfilePath)
    {
        PFXfilePath = pfxfilePath;
    }
    
    public X509CertificateCollection GetCertificates()
    {
        var certfile = X509Certificate.CreateFromCertFile(PFXfilePath);
        return new X509CertificateCollection{certfile};
    }
}
    

public class MQTTClient
{

    public delegate Task MQTTMessageCallback(string topic, ArraySegment<byte> payload);

    public event Func<MqttClientConnectedEventArgs, Task> ConnectedAsync
    {
        add { _mqttClient.ConnectedAsync += value; }
        remove { _mqttClient.ConnectedAsync -= value; }
    }

    public event Func<MqttClientDisconnectedEventArgs, Task> DisconnectedAsync
    {
        add { _mqttClient.DisconnectedAsync += value; }
        remove { _mqttClient.DisconnectedAsync -= value; }
    }

    public event Func<ConnectingFailedEventArgs, Task> ConnectingFailedAsync
    {
        add { _mqttClient.ConnectingFailedAsync += value; }
        remove { _mqttClient.ConnectingFailedAsync -= value; }
    }

    private MQTTNetCertificateProvider _mqttCertProvider;

    IManagedMqttClient _mqttClient;

    private TopicTree<List<MQTTMessageCallback>> _mqttSubscriptions = 
            new TopicTree<List<MQTTMessageCallback>>();
    
    private SemaphoreSlim _mutex = new SemaphoreSlim(1, 1);

    public MQTTClient(MqttSettings settings)
    {
        // Creates a new client
        MqttClientOptionsBuilder builder = new MqttClientOptionsBuilder();

        if (!String.IsNullOrEmpty(settings.ClientId))
        {
            builder.WithClientId(settings.ClientId);
        }

        if (!String.IsNullOrEmpty(settings.Address))
        {
            builder.WithTcpServer(settings.Address, settings.Port);
        }

        if (!String.IsNullOrEmpty(settings.Username) && !String.IsNullOrEmpty(settings.Password))
        {
            builder.WithCredentials(settings.Username, settings.Password);
        }

        if(!String.IsNullOrEmpty(settings.CaFile) && !String.IsNullOrEmpty(settings.CertificatePfx))
        {
            var cafile = X509Certificate.CreateFromCertFile(settings.CaFile);
            var cafileConverted = new X509Certificate2(cafile);
            _mqttCertProvider = new MQTTNetCertificateProvider(settings.CertificatePfx);
            builder.WithTlsOptions(new MqttClientTlsOptions()
                        {
                            IgnoreCertificateRevocationErrors = true,
                            IgnoreCertificateChainErrors = true,
                            UseTls = true,
                            TrustChain = [cafileConverted],
                            ClientCertificatesProvider = _mqttCertProvider,
                            AllowUntrustedCertificates = true, // Only for Development
                            // CertificateValidationHandler = context => true // Per accettare manualmente certificati validi
                        });
        }
        
        builder.WithProtocolVersion(MQTTnet.Formatter.MqttProtocolVersion.V500);                
        
        // Create client options objects
        ManagedMqttClientOptions options = new ManagedMqttClientOptionsBuilder()
                                .WithAutoReconnectDelay(TimeSpan.FromSeconds(60))
                                .WithClientOptions(builder.Build())
                                .Build();

        // Creates the client object
        _mqttClient = new MqttFactory().CreateManagedMqttClient();
    
        // Set up handlers
        _mqttClient.ApplicationMessageReceivedAsync += OnMessageReceivedAsync;

        // Starts a connection with the Broker
        _mqttClient.StartAsync(options).GetAwaiter().GetResult();
    }

    public async Task CloseAsync()
    {
        await _mqttClient.StopAsync();
    }

    private async Task OnMessageReceivedAsync(MqttApplicationMessageReceivedEventArgs obj)
    {
        foreach (var callbackList in _mqttSubscriptions.GetTreeMatches(obj.ApplicationMessage.Topic))
        {
            foreach (var callback in callbackList) 
            {
                var task = Task.Run(async () => {
                    await callback(obj.ApplicationMessage.Topic, obj.ApplicationMessage.PayloadSegment);
                });
            }
        }
    }

    public async Task SubscribeAsync(string topic, MQTTMessageCallback callback)
    {
        await _mutex.WaitAsync();
        try
        {
            TopicTreeNode<List<MQTTMessageCallback>> node = _mqttSubscriptions.TryTreeFind(topic);
            if (node != null)
            {
                node.Value.Add(callback);
            }
            else
            {
                var beforeMqttTopics = _mqttSubscriptions.Keys.ToArray();

                _mqttSubscriptions.TryTreeAdd(
                        new TopicTreeNode<List<MQTTMessageCallback>>(
                                topic, new List<MQTTMessageCallback> { callback }));

                var afterMqttTopics = _mqttSubscriptions.Keys.ToArray();

                foreach (var entry in beforeMqttTopics)
                {
                    if (afterMqttTopics.Where(t => t == entry).Count() == 0) //topic removed from root as is more specific
                    {
                        await _mqttClient.UnsubscribeAsync(topic);
                    }
                }
                
                foreach (var entry in afterMqttTopics)
                {
                    if (beforeMqttTopics.Where(t => t == entry).Count() == 0) //new most generic topic
                    {
                        await _mqttClient.SubscribeAsync(topic);
                    }
                }
            }
        }
        finally
        {
            _mutex.Release();
        }
    }

    public async Task UnSubscribeAsync(string topic, MQTTMessageCallback callback)
    {
        await _mutex.WaitAsync();
        try
        {
            TopicTreeNode<List<MQTTMessageCallback>> node = _mqttSubscriptions.TryTreeFind(topic);
            if (node == null)
                throw new InvalidOperationException($"Topic <<{topic}>> not subscribed!");

            if (node.Value.Remove(callback))
            {
                if (node.Value.Count() == 0) // no more subscription on this topic
                {
                    var beforeMqttTopics = _mqttSubscriptions.Keys.ToArray();

                    _mqttSubscriptions.TryTreeRemove(topic);

                    var afterMqttTopics = _mqttSubscriptions.Keys.ToArray();

                    if (beforeMqttTopics.Where(t => t == topic).Count() != 0)
                    {
                        await _mqttClient.UnsubscribeAsync(topic);
                    }

                    foreach (var entry in afterMqttTopics)
                    {
                        if (beforeMqttTopics.Where(t => t == entry).Count() == 0) //new most generic topic
                        {
                            await _mqttClient.SubscribeAsync(topic);
                        }
                    }
                }
            }
        }
        finally
        {
            _mutex.Release();
        }
    }

    public async Task EnqueueMessageAsync(string topic, string payload)
    {
        var toPublish = new MqttApplicationMessageBuilder()
                                .WithTopic(topic)
                                .WithPayload(payload)
                                .Build();
        
        await _mqttClient.EnqueueAsync(toPublish);
    }

    public async Task EnqueueMessageAsync(string topic, byte[] payload)
    {
        var toPublish = new MqttApplicationMessageBuilder()
                                .WithTopic(topic)
                                .WithPayload(payload)
                                .Build();
        
        await _mqttClient.EnqueueAsync(toPublish);
    }

    public async Task EnqueueMessageAsync(string topic, ArraySegment<byte> payload)
    {
        var toPublish = new MqttApplicationMessageBuilder()
                                .WithTopic(topic)
                                .WithPayload(payload)
                                .Build();
        
        await _mqttClient.EnqueueAsync(toPublish);
    }
}
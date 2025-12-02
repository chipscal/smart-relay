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

    private Dictionary<string, List<MQTTMessageCallback>> _callbacks = 
            new Dictionary<string, List<MQTTMessageCallback>>();
    
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
        var topicPath = obj.ApplicationMessage.Topic.Split("/");
        foreach(var subscriptionList in _callbacks)
        {
            var subTopicPath = subscriptionList.Key.Split("/");

            int start = 0;
            if (subTopicPath[0].ToLowerInvariant() == "$share") //shared subscription support
                start = 2;
                
            if (subTopicPath.Last() != "#" && subTopicPath.Length - start != topicPath.Length)
                continue; 


            bool topicMatched = true;
            for (int k = start; k < subTopicPath.Length; k++)
            {
                if (k == subTopicPath.Length - 1 && subTopicPath[k] == "#")
                    break;

                if (subTopicPath[k] != "+" && subTopicPath[k].ToUpperInvariant() != topicPath[k - start].ToUpperInvariant())
                {
                    topicMatched = false;
                    break;
                }   
            }

            //topic matches subscriptions
            if (topicMatched)
                foreach (var callback in subscriptionList.Value) 
                {
                    Debug.WriteLine($"{subscriptionList.Key}:{callback.ToString()}");
                    var task = Task.Run(async () => {
                        await callback(obj.ApplicationMessage.Topic, obj.ApplicationMessage.PayloadSegment);
                    });
                }
        }
    }

    public async Task SubscribeAsync(string topic, MQTTMessageCallback callback)
    {
        bool toSub = false;
        lock(this)
        {
            if (!_callbacks.ContainsKey(topic))
            {
                _callbacks[topic] = new List<MQTTMessageCallback>();
                toSub = true;
            }

            _callbacks[topic].Add(callback);
        }
        if (toSub)
            await _mqttClient.SubscribeAsync(topic);

        //TODO: avoid replicated messages 
        // find most generic topic if exists and fuse callbacks
        // unsubscribe to less generic one
    }

    public async Task UnSubscribeAsync(string topic, MQTTMessageCallback callback)
    {
        bool toUnSub = false;
        lock(this)
        {
            if (!_callbacks.ContainsKey(topic))
                throw new ArgumentOutOfRangeException($"<{topic}> not subscribed!");

            
            _callbacks[topic].Remove(callback);
            if (_callbacks[topic].Count == 0)
                toUnSub = true;
        }
        if (toUnSub)
            await _mqttClient.UnsubscribeAsync(topic);
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
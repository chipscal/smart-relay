using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Clab.Smart.Relay.App;



public class Device(MQTTClient mqttClient)
{
    
    public const string TelemTopicFormat        = "/dev/{0}/telem";
    public const string PropValueTopicFormat    = "/dev/{0}/prop/{1}/value";
    public const string CmdAckTopicFormat       = "/dev/{0}/cmd/{1}/{2}";
    public const string PropDesiredTopicFormat  = "/dev/{0}/prop/{1}/desired";
    public const string CmdExecTopicFormat      = "/dev/{0}/cmd/{1}/exec";


    public string   DeviceUID       { get; set; }
    public string   ModelName       { get; set; }

    public Version  HardwareRev     { get; set; }
    public Version  SoftwareRev     { get; set; }

    public IEnumerable<DeviceProperty>              Telemetry       { get; set; }
    public IEnumerable<DeviceSettableProperty>      Properties      { get; set; }

    private  MQTTClient _mqttClient = mqttClient;

    public async Task EnableSelfRefresh()
    {
        var telemTopic = string.Format(TelemTopicFormat, DeviceUID);
        await _mqttClient.SubscribeAsync(telemTopic, RefreshFromTelemetryHandle);

        var propTopic = string.Format(PropValueTopicFormat, DeviceUID, "+");
        await _mqttClient.SubscribeAsync(propTopic, CreateRefreshPropertyHandle);

    }

    private async Task CreateRefreshPropertyHandle(string topic, ArraySegment<byte> payload)
    {
        throw new NotImplementedException();
    }

    private async Task RefreshFromTelemetryHandle(string topic, ArraySegment<byte> payload)
    {
        RefreshFromTelemetry(payload);
    }

    public void RefreshFromTelemetry(ArraySegment<byte> payload)
    {
        throw new NotImplementedException();
    }

    public async Task RestartAsync()
    {
        var topic = string.Format(CmdExecTopicFormat, DeviceUID, "restart");
        
        var ts = (int)(DateTime.UtcNow - DateTime.UnixEpoch).TotalSeconds;
        await _mqttClient.EnqueueMessageAsync(topic, GenerateMQTTCommandPayload(ts, null));
    }

    public async Task HardRestartAsync()
    {
        var topic = string.Format(CmdExecTopicFormat, DeviceUID, "restart-hard");
        
        var ts = (int)(DateTime.UtcNow - DateTime.UnixEpoch).TotalSeconds;
        await _mqttClient.EnqueueMessageAsync(topic, GenerateMQTTCommandPayload(ts, null));
    }

    public async Task<bool> RefreshLatchAsync()
    {
        return await AcknowledgedCommand("refresh", null, CancellationToken.None);
    }

    
    public async Task<bool> AcknowledgedCommand(string command, ArraySegment<byte> payload, CancellationToken cancellationToken)
    {
        if (command == null)
            return false;

        var ts = (int)(DateTime.UtcNow - DateTime.UnixEpoch).TotalSeconds;
        int res = -2;

        MQTTClient.MQTTMessageCallback ackLambda = async (string topic, ArraySegment<byte> payload) =>
        {
            var decoded = Convert.FromBase64String(Encoding.UTF8.GetString(payload));
            
            var buffer = new byte[sizeof(uint)];
            Array.Copy(decoded, 0, buffer, 0, sizeof(uint));
            if (!BitConverter.IsLittleEndian)
                Array.Reverse(buffer, 0, sizeof(uint));
            
            var recvTs = BitConverter.ToUInt32(buffer);


            if (recvTs == ts)
            {
                if (topic.ToLowerInvariant().EndsWith("/ack"))
                    res = 0;
                else
                    res = -1;
            }
        };

        var ackTopic = string.Format(CmdAckTopicFormat, DeviceUID, command, "#");
        await _mqttClient.SubscribeAsync(ackTopic, ackLambda);

        var topic = string.Format(CmdExecTopicFormat, DeviceUID, "restart");
        
        await _mqttClient.EnqueueMessageAsync(topic, GenerateMQTTCommandPayload(ts, payload));

        while (!cancellationToken.IsCancellationRequested && res <= -2)
        {
            await Task.Delay(1000);
        }

        await _mqttClient.UnSubscribeAsync(ackTopic, ackLambda);

        return res == 0;
    }
    
    private static string GenerateMQTTCommandPayload(int ts, ReadOnlySpan<byte> payload)
    {
        var stream = new MemoryStream();

        
        if (!BitConverter.IsLittleEndian)
            ts = BinaryPrimitives.ReverseEndianness(ts);
        stream.Write(BitConverter.GetBytes(ts));

        stream.Write(payload);

        return Convert.ToBase64String(stream.ToArray());
    }
}

public class DeviceProperty(Device device)
{
    public DeviceTags   Tag         { get; set; }
    public string       Value       { get; set; }
    public DateTime     LastUpdate  { get; set; }

    protected Device    _device = device;


    public async Task<bool> Query()
    {
        var payload = Encoding.UTF8.GetBytes(Tag.ToAlias());
        return await _device.AcknowledgedCommand("query", payload, CancellationToken.None);
    }
}

public class DeviceSettableProperty(Device device, MQTTClient mqttClient) : DeviceProperty(device)
{
    public string       Desired     { get; private set; }
    public DateTime     LastSync    { get; set; }

    private  MQTTClient _mqttClient = mqttClient;

    public async Task TrySet(string desired)
    {
        
    }

}
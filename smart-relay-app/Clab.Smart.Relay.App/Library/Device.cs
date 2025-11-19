using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Linq;

namespace Clab.Smart.Relay.App;



public class Device(MQTTClient mqttClient)
{
    
    public const string TelemTopicFormat        = "/dev/{0}/telem";
    public const string PropValueTopicFormat    = "/dev/{0}/prop/{1}/value";
    public const string CmdAckTopicFormat       = "/dev/{0}/cmd/{1}/ack";
    public const string CmdNAckTopicFormat      = "/dev/{0}/cmd/{1}/nack";
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
    }

    private async Task RefreshFromTelemetryHandle(string topic, ArraySegment<byte> payload)
    {
        throw new NotImplementedException();
    }

    public void RefreshFromTelemetry(ArraySegment<byte> payload)
    {
        throw new NotImplementedException();
    }

    public async Task RestartAsync()
    {
        var topic = string.Format(CmdExecTopicFormat, DeviceUID, "restart");
        
        await _mqttClient.EnqueueMessageAsync(topic, GenerateMQTTCommandPayload(null));
    }

    public async Task HardRestartAsync()
    {
        var topic = string.Format(CmdExecTopicFormat, DeviceUID, "restart-hard");
        
        await _mqttClient.EnqueueMessageAsync(topic, GenerateMQTTCommandPayload(null));
    }

    public async Task RefreshLatchAsync()
    {
        var topic = string.Format(CmdExecTopicFormat, DeviceUID, "refresh");
        
        await _mqttClient.EnqueueMessageAsync(topic, GenerateMQTTCommandPayload(null));
    }

    private static string GenerateMQTTCommandPayload(ReadOnlySpan<byte> payload)
    {
        var stream = new MemoryStream();

        var ts = (int)(DateTime.UtcNow - DateTime.UnixEpoch).TotalSeconds;
        if (!BitConverter.IsLittleEndian)
            ts = BinaryPrimitives.ReverseEndianness(ts);
        stream.Write(BitConverter.GetBytes(ts));

        stream.Write(payload);

        return Convert.ToBase64String(stream.ToArray());
    }
}

public class DeviceProperty
{
    public DeviceTags   Tag         { get; set; }
    public string       Value       { get; set; }
    public DateTime     LastUpdate  { get; set; }
}

public class DeviceSettableProperty(MQTTClient mqttClient) : DeviceProperty
{
    public string       Desired     { get; set; }
    public DateTime     LastSync    { get; set; }

    private  MQTTClient _mqttClient = mqttClient;

    

}
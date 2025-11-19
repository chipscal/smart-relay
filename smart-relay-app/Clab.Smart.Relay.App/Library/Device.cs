using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Linq;

namespace Clab.Smart.Relay.App;



public class Device(MQTTClient mqttClient)
{
    
    public const string TelemTopic          = "/dev/+/telem";
    public const string PropValueTopic      = "/dev/+/prop/+/value";
    public const string CmdAckTopic         = "/dev/+/cmd/+/ack";
    public const string CmdNAckTopic        = "/dev/+/cmd/+/nack";

    public const string PropDesiredFormat   = "/dev/{0}/prop/{1}/desired";
    public const string CmdExecFormat       = "/dev/{0}/cmd/{1}/exec";


    public string   DeviceUID       { get; set; }
    public string   ModelName       { get; set; }

    public Version  HardwareRev     { get; set; }
    public Version  SoftwareRev     { get; set; }

    public IEnumerable<DeviceProperty>              Telemetry       { get; set; }
    public IEnumerable<DeviceSettableProperty>      Properties      { get; set; }

    private  MQTTClient _mqttClient = mqttClient;

    public async Task RestartAsync()
    {
        var topic = string.Format(CmdExecFormat, DeviceUID, "restart");
        
        await _mqttClient.EnqueueMessageAsync(topic, GenerateMQTTCommandPayload(null));
    }

    public async Task HardRestartAsync()
    {
        var topic = string.Format(CmdExecFormat, DeviceUID, "restart-hard");
        
        await _mqttClient.EnqueueMessageAsync(topic, GenerateMQTTCommandPayload(null));
    }

    public async Task RefreshLatchAsync()
    {
        var topic = string.Format(CmdExecFormat, DeviceUID, "refresh");
        
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
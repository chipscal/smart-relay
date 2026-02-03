using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace Clab.Smart.Relay.App;



public class Device(string deviceUID, MQTTClient mqttClient)
{
    
    public const string BaseTopicFormat         = "/dev/{0}";
    public const string TelemTopicFormat        = "/dev/{0}/telem";
    public const string PropValueTopicFormat    = "/dev/{0}/prop/{1}/value";
    public const string CmdAckTopicFormat       = "/dev/{0}/cmd/{1}/{2}";
    public const string PropDesiredTopicFormat  = "/dev/{0}/prop/{1}/desired";
    public const string CmdExecTopicFormat      = "/dev/{0}/cmd/{1}/exec";


    public string   DeviceUID           { get; private set; } = deviceUID;
    public string   ModelName           { get; private set; }

    public Version  HardwareRev         { get; private set; }
    public Version  SoftwareRev         { get; private set; }

    public int      CurrentInputCount           { get; private set; }
    public int      VoltageInputCount           { get; private set; }
    public int      PulseInputCount             { get; private set; }
    public int      TemperatureInputCount       { get; private set; }
    public int      DigitalInputCount           { get; private set; }
    public int      RelayOutputCount            { get; private set; }
    public int      LatchOutputCount            { get; private set; }

    public Dictionary<DeviceTags, DeviceProperty>              Telemetry       { get; set; } = new Dictionary<DeviceTags, DeviceProperty>();
    public Dictionary<DeviceTags, DeviceSettableProperty>      Properties      { get; set; } = new Dictionary<DeviceTags, DeviceSettableProperty>();

    public event EventHandler OnTelemetryUpdate;

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
        const string topicMatch = @"prop/(\w+)/value$";

        var propNameRegex = new Regex(topicMatch);
        var match = propNameRegex.Match(topic);
        if (!match.Success) 
        {
            Debug.WriteLine("Received malformed topic string???");
            return;
        }

        var tag = DevicePropertyUtils.FromAlias(match.Groups[1].Value);


        lock(this)
        {
            DeviceSettableProperty toUpsert = Properties.GetValueOrDefault(tag);
            if (toUpsert == null)
            {
                toUpsert = new DeviceSettableProperty(this, _mqttClient);
                Properties[tag] = toUpsert;
            }

            toUpsert.Tag = tag;
            toUpsert.Value = Encoding.UTF8.GetString(payload);
            toUpsert.LastUpdate = DateTime.UtcNow;
        }
    }

    private async Task RefreshFromTelemetryHandle(string topic, ArraySegment<byte> payload)
    {
        RefreshFromTelemetry(payload);

        OnTelemetryUpdate(this, EventArgs.Empty);
    }

    public void RefreshFromTelemetry(ArraySegment<byte> payload)
    {
        // HREV|SREV|n_curr[0:3],n_volt[4:7]|n_pulse[0:3],n_temperature[4:7]
        // n_latch|n_relay|n_digital|reserved[0:7]
        // Latch0|Latch1|Latch2|Latch3 (bit mask)
        // Relay0|Relay1|Relay2|Relay3 (bit mask)
        // Digital0|Digital1|Digital2|Digital3 (bit mask)
        // Current0_LSB|Current0_MSB|...|CurrentN_LSB|CurrentN_MSB
        // Voltage0_LSB|Voltage0_MSB|...|VoltageN_LSB|VoltageN_MSB
        // Pulse0_LSB|Pulse0_MSB|...|PulseN_LSB|PulseN_MSB
        // Temperature0_LSB|Temperature0_MSB|...|TemperatureN_LSB|TemperatureN_MSB
        Debug.WriteLine("Received telemetry!");

        int offset = 0;
        var decoded = Convert.FromBase64String(Encoding.UTF8.GetString(payload));

        var buffer = new byte[sizeof(uint)];
        Array.Copy(decoded, offset, buffer, 0, sizeof(uint));
        offset += sizeof(uint);
        if (!BitConverter.IsLittleEndian)
            Array.Reverse(buffer, 0, sizeof(uint));
        
        var ts = BitConverter.ToUInt32(buffer);
        var publishDate = DateTime.UnixEpoch.AddSeconds(ts);

        // 1st DWORD
        byte hRev               = decoded[offset];
        byte sRev               = decoded[offset + 1];
        byte numCurrent         = (byte)((decoded[offset + 2] & 0xF0) >> 4);
        byte numVoltage         = (byte)(decoded[offset + 2] & 0x0F);
        byte numPulse           = (byte)((decoded[offset + 3] & 0xF0) >> 4);
        byte numTemperature     = (byte)(decoded[offset + 3] & 0x0F);

        // 2nd DWORD
        byte numLatch           = decoded[offset + 4];
        byte numRelay           = decoded[offset + 5];
        byte numDigital         = decoded[offset + 6];
        byte reserved           = decoded[offset + 7];
        if (reserved != 0)
        {
            Debug.WriteLine("Malformed telemetry!!!");
            return;
        }
        offset += 8;

        HardwareRev = new Version(hRev, 0);
        SoftwareRev = new Version(sRev, 0);
        CurrentInputCount = numCurrent;
        VoltageInputCount = numVoltage;
        PulseInputCount = numPulse;
        TemperatureInputCount = numTemperature;

        LatchOutputCount = numLatch;
        RelayOutputCount = numLatch;
        DigitalInputCount = numDigital;

        // Latch
        Array.Copy(decoded, offset, buffer, 0, sizeof(uint));
        offset += sizeof(uint);
        if (!BitConverter.IsLittleEndian)
            Array.Reverse(buffer, 0, sizeof(uint));
        
        
        var latchMask = BitConverter.ToUInt32(buffer);
        for (int k = 0; k < numLatch; k++)
        {
            var tag = DeviceTags.LATCH1 + k;

            DeviceProperty toUpsert = null;
            lock (Telemetry)
            {
                toUpsert = Telemetry.GetValueOrDefault(tag);
                if (toUpsert == null)
                {
                    toUpsert = new DeviceProperty(this);
                    Telemetry[tag] = toUpsert;
                }
            }

            toUpsert.Tag = tag;
            toUpsert.Value = (latchMask & (1 << k)) > 0 ? "on" : "off";
            toUpsert.LastUpdate = publishDate;


            lock (Properties)
            {
                tag = DeviceTags.LATCH_DELAYS1 + k;
                var propToUpsert = Properties.GetValueOrDefault(tag);
                if (propToUpsert == null)
                {
                    propToUpsert = new DeviceSettableProperty(this, _mqttClient);
                    propToUpsert.Tag = tag;
                    Properties[tag] = propToUpsert;
                }
            
                tag = DeviceTags.LATCH_OVERRIDE1 + k;
                propToUpsert = Properties.GetValueOrDefault(tag);
                if (propToUpsert == null)
                {
                    propToUpsert = new DeviceSettableProperty(this, _mqttClient);
                    propToUpsert.Tag = tag;
                    Properties[tag] = propToUpsert;
                }
            }

        }

        // Relay
        Array.Copy(decoded, offset, buffer, 0, sizeof(uint));
        offset += sizeof(uint);
        if (!BitConverter.IsLittleEndian)
            Array.Reverse(buffer, 0, sizeof(uint));
        
        
        var relayMask = BitConverter.ToUInt32(buffer);
        for (int k = 0; k < numRelay; k++)
        {
            var tag = DeviceTags.RELAY1 + k;

            DeviceProperty toUpsert = null;
            lock (Telemetry)
            {
                toUpsert = Telemetry.GetValueOrDefault(tag);
                if (toUpsert == null)
                {
                    toUpsert = new DeviceProperty(this);
                    Telemetry[tag] = toUpsert;
                }
            }

            toUpsert.Tag = tag;
            toUpsert.Value = (relayMask & (1 << k)) > 0 ? "on" : "off";
            toUpsert.LastUpdate = publishDate;

            lock (Properties)
            {
                tag = DeviceTags.RELAY_DELAYS1 + k;
                var propToUpsert = Properties.GetValueOrDefault(tag);
                if (propToUpsert == null)
                {
                    propToUpsert = new DeviceSettableProperty(this, _mqttClient);
                    propToUpsert.Tag = tag;
                    Properties[tag] = propToUpsert;
                }

                tag = DeviceTags.RELAY_OVERRIDE1 + k;
                propToUpsert = Properties.GetValueOrDefault(tag);
                if (propToUpsert == null)
                {
                    propToUpsert = new DeviceSettableProperty(this, _mqttClient);
                    propToUpsert.Tag = tag;
                    Properties[tag] = propToUpsert;
                }
            }
        }

        // Digital
        Array.Copy(decoded, offset, buffer, 0, sizeof(uint));
        offset += sizeof(uint);
        if (!BitConverter.IsLittleEndian)
            Array.Reverse(buffer, 0, sizeof(uint));
        
        
        var digitalMask = BitConverter.ToUInt32(buffer);
        for (int k = 0; k < numDigital; k++)
        {
            var tag = DeviceTags.DIGITAL_INPUT1 + k;

            DeviceProperty toUpsert = null;
            lock (Telemetry)
            {
                toUpsert = Telemetry.GetValueOrDefault(tag);
                if (toUpsert == null)
                {
                    toUpsert = new DeviceProperty(this);
                    Telemetry[tag] = toUpsert;
                }
            }

            toUpsert.Tag = tag;
            toUpsert.Value = (digitalMask & (1 << k)) > 0 ? "on" : "off";
            toUpsert.LastUpdate = publishDate;
        }

        // Current sensors
        for (int k = 0; k < numCurrent; k++)
        {
            var tag = DeviceTags.A_CURRENT_INPUT1 + k;

            DeviceProperty toUpsert = null;
            lock (Telemetry)
            {
                toUpsert = Telemetry.GetValueOrDefault(tag);
                if (toUpsert == null)
                {
                    toUpsert = new DeviceProperty(this);
                    Telemetry[tag] = toUpsert;
                }
            }
            Array.Copy(decoded, offset, buffer, 0, sizeof(UInt16));
            offset += sizeof(UInt16);
            if (!BitConverter.IsLittleEndian)
                Array.Reverse(buffer, 0, sizeof(UInt16));

            var value = BitConverter.ToUInt16(buffer, 0);

            toUpsert.Tag = tag;
            toUpsert.Value = (value / 1000.0).ToString("0.00"); //mA
            toUpsert.LastUpdate = publishDate;
        }

        // Voltage sensors
        for (int k = 0; k < numVoltage; k++)
        {
            var tag = DeviceTags.A_VOLTAGE_INPUT1 + k;

            DeviceProperty toUpsert = null;
            lock (Telemetry)
            {
                toUpsert = Telemetry.GetValueOrDefault(tag);
                if (toUpsert == null)
                {
                    toUpsert = new DeviceProperty(this);
                    Telemetry[tag] = toUpsert;
                }
            }
            Array.Copy(decoded, offset, buffer, 0, sizeof(UInt16));
            offset += sizeof(UInt16);
            if (!BitConverter.IsLittleEndian)
                Array.Reverse(buffer, 0, sizeof(UInt16));

            var value = BitConverter.ToUInt16(buffer, 0);

            toUpsert.Tag = tag;
            toUpsert.Value = (value / 1000.0).ToString("0.00"); //V
            toUpsert.LastUpdate = publishDate;
        }

        // Pulse sensors
        for (int k = 0; k < numPulse; k++)
        {
            var tag = DeviceTags.PULSE_INPUT1 + k;

            DeviceProperty toUpsert = null;
            lock (Telemetry)
            {
                toUpsert = Telemetry.GetValueOrDefault(tag);
                if (toUpsert == null)
                {
                    toUpsert = new DeviceProperty(this);
                    Telemetry[tag] = toUpsert;
                }
            }

            Array.Copy(decoded, offset, buffer, 0, sizeof(UInt16));
            offset += sizeof(UInt16);
            if (!BitConverter.IsLittleEndian)
                Array.Reverse(buffer, 0, sizeof(UInt16));

            var value = BitConverter.ToUInt16(buffer, 0);

            toUpsert.Tag = tag;
            toUpsert.Value = value.ToString();
            toUpsert.LastUpdate = publishDate;

            lock (Properties)
            {
                tag = DeviceTags.PULSE_FILTER1 + k;
                var propToUpsert = Properties.GetValueOrDefault(tag);
                if (propToUpsert == null)
                {
                    propToUpsert = new DeviceSettableProperty(this, _mqttClient);
                    propToUpsert.Tag = tag;
                    Properties[tag] = propToUpsert;
                }
            }
        }

        for (int k = 0; k < numTemperature; k++)
        {
            var tag = DeviceTags.TEMPERATURE_INPUT1 + k;

            DeviceProperty toUpsert = null;
            lock (Telemetry)
            {
                toUpsert = Telemetry.GetValueOrDefault(tag);
                if (toUpsert == null)
                {
                    toUpsert = new DeviceProperty(this);
                    Telemetry[tag] = toUpsert;
                }
            }

            Array.Copy(decoded, offset, buffer, 0, sizeof(UInt16));
            offset += sizeof(UInt16);
            if (!BitConverter.IsLittleEndian)
                Array.Reverse(buffer, 0, sizeof(UInt16));

            var value = BitConverter.ToUInt16(buffer, 0);

            toUpsert.Tag = tag;
            toUpsert.Value = (value / 100.0).ToString("0.00"); //K
            toUpsert.LastUpdate = publishDate;
        }

        ModelName = Encoding.UTF8.GetString(decoded, offset, decoded.Length - offset);
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

    public async Task<bool> RefreshLatchAsync(CancellationToken cancellationToken = default)
    {
        return await AcknowledgedCommandAsync("refresh", null, cancellationToken);
    }

    
    public async Task<bool> AcknowledgedCommandAsync(string command, ArraySegment<byte> payload, CancellationToken cancellationToken = default)
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
    
    internal static string GenerateMQTTCommandPayload(int ts, ReadOnlySpan<byte> payload)
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


    public async Task<bool> QueryAsync(CancellationToken cancellationToken = default)
    {
        var payload = Encoding.UTF8.GetBytes(Tag.ToAlias());
        return await _device.AcknowledgedCommandAsync("query", payload, cancellationToken);
    }
}

public class DeviceSettableProperty(Device device, MQTTClient mqttClient) : DeviceProperty(device)
{
    public string       Desired     { get; private set; }
    public DateTime     LastSync    { get; set; }

    private  MQTTClient _mqttClient = mqttClient;

    /// <summary>
    /// Tries to set desired property by publishing the desired value over mqtt
    /// </summary>
    /// <param name="desired">A string rapresenting desired value</param>
    /// <returns></returns>
    public async Task TrySetAsync(string desired)
    {
        var topic = string.Format(Device.PropDesiredTopicFormat, _device.DeviceUID, Tag.ToAlias());
        
        await _mqttClient.EnqueueMessageAsync(topic, Encoding.UTF8.GetBytes(desired));

        lock (this)
        {
            Desired = desired;
            LastSync = DateTime.UtcNow;
        }

    }

}
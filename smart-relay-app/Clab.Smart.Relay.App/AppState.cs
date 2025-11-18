using System;
using System.Threading.Tasks;

namespace Clab.Smart.Relay.App;

public class AppState
{
    
    public const string TelemTopic          = "/dev/+/telem";
    public const string PropDesiredTopic    = "/dev/+/prop/+/desired";
    public const string PropValueTopic      = "/dev/+/prop/+/value";
    public const string CmdExecTopic        = "/dev/+/cmd/+/exec";
    public const string CmdAckTopic         = "/dev/+/cmd/+/ack";
    public const string CmdNAckTopic        = "/dev/+/cmd/+/nack";


    private MQTTClient  _mqttClient = null;

    public AppState()
    {
    }

    public async Task<bool> MqttConnect(string address, int port, string username, string password)
    {
        if (_mqttClient == null)
        {
            _mqttClient = new MQTTClient(new MqttSettings
                    {
                        Address = address,
                        Port = port,
                        Username = username,
                        Password = password
                    });

            await _mqttClient.SubscribeAsync(TelemTopic,      TelemetryCallback);
            await _mqttClient.SubscribeAsync(PropValueTopic,  PropertyValueCallback);
            await _mqttClient.SubscribeAsync(CmdAckTopic,     CommandAckNackCallback);
            await _mqttClient.SubscribeAsync(CmdNAckTopic,    CommandAckNackCallback);
            
            return true;
        }

        return false;
    }

    public async Task<bool> MqttDisconnect()
    {
        if (_mqttClient != null)
        {
            await _mqttClient.UnSubscribeAsync(TelemTopic,      TelemetryCallback);
            await _mqttClient.UnSubscribeAsync(PropValueTopic,  PropertyValueCallback);
            await _mqttClient.UnSubscribeAsync(CmdAckTopic,     CommandAckNackCallback);
            await _mqttClient.UnSubscribeAsync(CmdNAckTopic,    CommandAckNackCallback);

            await _mqttClient.CloseAsync();

            return true;
        }

        return false;
    }



    private async Task CommandAckNackCallback(string topic, ArraySegment<byte> payload)
    {
        throw new NotImplementedException();
    }

    private async Task PropertyValueCallback(string topic, ArraySegment<byte> payload)
    {
        throw new NotImplementedException();
    }

    private async Task TelemetryCallback(string topic, ArraySegment<byte> payload)
    {
        throw new NotImplementedException();
    }
}
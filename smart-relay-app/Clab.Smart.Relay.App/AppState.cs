using System;
using System.Threading.Tasks;

namespace Clab.Smart.Relay.App;

public class AppState
{

    public IEnumerable<Device>      KnownDevices    {get; set;}


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

            // await _mqttClient.SubscribeAsync(Device.TelemTopic,      TelemetryCallback);
            // await _mqttClient.SubscribeAsync(Device.PropValueTopic,  PropertyValueCallback);
            // await _mqttClient.SubscribeAsync(Device.CmdAckTopic,     CommandAckNackCallback);
            // await _mqttClient.SubscribeAsync(Device.CmdNAckTopic,    CommandAckNackCallback);
            
            return true;
        }

        return false;
    }

    public async Task<bool> MqttDisconnect()
    {
        if (_mqttClient != null)
        {
            // await _mqttClient.UnSubscribeAsync(Device.TelemTopic,      TelemetryCallback);
            // await _mqttClient.UnSubscribeAsync(Device.PropValueTopic,  PropertyValueCallback);
            // await _mqttClient.UnSubscribeAsync(Device.CmdAckTopic,     CommandAckNackCallback);
            // await _mqttClient.UnSubscribeAsync(Device.CmdNAckTopic,    CommandAckNackCallback);

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
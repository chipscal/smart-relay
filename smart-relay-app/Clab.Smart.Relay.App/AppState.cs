using System;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Clab.Smart.Relay.App;

public class AppState
{

    public Dictionary<string, Device>      KnownDevices    {get; set;}


    private MQTTClient  _mqttClient = null;

    public AppState()
    {
        KnownDevices = new Dictionary<string, Device>();
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

            await _mqttClient.SubscribeAsync(string.Format(Device.BaseTopicFormat, "#"), DiscoveredDeviceCallback);
            
            return true;
        }

        return false;
    }


    public async Task<bool> MqttDisconnect()
    {
        if (_mqttClient != null)
        {
            await _mqttClient.UnSubscribeAsync(string.Format(Device.BaseTopicFormat, "#"), DiscoveredDeviceCallback);

            await _mqttClient.CloseAsync();

            return true;
        }

        return false;
    }

    private async Task DiscoveredDeviceCallback(string topic, ArraySegment<byte> payload)
    {
        const string topicMatch = @"^/dev/(\w+)/";

        var propNameRegex = new Regex(topicMatch);
        var match = propNameRegex.Match(topic);
        if (match.Success) 
        {
            var deviceUID = match.Groups[1].Value;
            bool toEnable = false;

            lock (KnownDevices)
            {
                if (!KnownDevices.ContainsKey(deviceUID))
                {
                    var device = new Device(deviceUID, _mqttClient);
                    KnownDevices[deviceUID] = device;
                    toEnable = true;
                }
            }
            
            if (toEnable)
                await KnownDevices[deviceUID].EnableSelfRefresh();
        }
    }

}
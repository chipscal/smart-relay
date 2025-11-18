using System;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Net;
using System.Diagnostics;

namespace Clab.Smart.Relay.App;

public class DiscoveryServiceClient : IDisposable
{
    public const int DiscoveryServicePort = 46789 ;

    private UdpClient UdpClient {get;}

    public DiscoveryServiceClient()
    {
        UdpClient = new UdpClient(DiscoveryServicePort);
        UdpClient.EnableBroadcast = true;
    }

    public void Dispose()
    {
        UdpClient.Close();
        UdpClient.Dispose();
    }

    public async Task<IEnumerable<BrokerInfo>> FindBrokers(int waitMillis, CancellationToken cancellationToken = default)
    {

        const string brokerRegex = @"^HELLO!\r?\n([^\r\n]*)\r?\n(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})\r?\n(\d+)\r?\n$";

        var foundBrokers = new List<BrokerInfo>();

        foreach (var networkInterface in NetworkInterface.GetAllNetworkInterfaces())
        {
            if (networkInterface.OperationalStatus == OperationalStatus.Up)
            {
                foreach (var ip in networkInterface.GetIPProperties().UnicastAddresses)
                {
                    if (ip.Address.AddressFamily == AddressFamily.InterNetwork) // IPv4
                    {
                        IPAddress brodcastAddress = GetBroadcastAddress(ip.Address, ip.IPv4Mask);

                        // Send server HELLO!
                        byte[] sendBytes = Encoding.UTF8.GetBytes("HELLO!");
                        await UdpClient.SendAsync(sendBytes, new IPEndPoint(brodcastAddress, DiscoveryServicePort), cancellationToken: cancellationToken);
                        // receive responses
                        
                        var taskCts = new CancellationTokenSource();
                        var taskToken = taskCts.Token;

                        var task = Task.Run(async () =>
                        {
                            while (!taskToken.IsCancellationRequested)
                            {
                                try
                                {
                                    var datagram = await UdpClient.ReceiveAsync(cancellationToken: taskToken);
                                    string receivedText = Encoding.UTF8.GetString(datagram.Buffer);
                                    Debug.WriteLine($"[{datagram.RemoteEndPoint}] {receivedText}");

                                    var match = Regex.Match(receivedText, brokerRegex);
                                    if (match!.Success)
                                    {
                                        var address = new byte[] 
                                        {  
                                            byte.Parse(match.Groups[2].Value),
                                            byte.Parse(match.Groups[3].Value),
                                            byte.Parse(match.Groups[4].Value),
                                            byte.Parse(match.Groups[5].Value)
                                        };

                                        foundBrokers.Add(new BrokerInfo
                                        {
                                            Name = match.Groups[1].Value,
                                            Host = new IPAddress(address),
                                            Port = int.Parse(match.Groups[6].Value)
                                        });
                                    }


                                }
                                catch
                                {
                                    // socket closed or cancelled operation
                                    Debug.WriteLine("Operation aborted!");
                                    return;
                                }                               
                            }

                        });
                       
                        _ = Task.Run(async () =>
                        {
                            await Task.Delay(waitMillis, cancellationToken: cancellationToken);
                            taskCts.Cancel();
                        });
                        
                        await task.WaitAsync(CancellationToken.None);         

                    }
                }
            }
        }

        return foundBrokers;       
    }

    private IPAddress GetBroadcastAddress(IPAddress address, IPAddress mask)
    {
        var addressBytes = address.GetAddressBytes();
        var maskBytes = mask.GetAddressBytes();
        var broadcastBytes = new byte[addressBytes.Length];

        for (int i = 0; i < broadcastBytes.Length; i++)
        {
            broadcastBytes[i] = (byte)(addressBytes[i] | ~maskBytes[i]);
        }

        return new IPAddress(broadcastBytes);

    }
}

public class BrokerInfo
{
    public string       Name {get; set;}
    public IPAddress    Host {get; set;}
    public int          Port {get; set;}
}
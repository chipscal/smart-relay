param (
    [Parameter(Mandatory=$true)]$broadcastAddress = "255.255.255.255",
    [Parameter(Mandatory=$true)]$port = 46789
)

# Create a UDP client
$udpClient = New-Object System.Net.Sockets.UdpClient
$udpClient.EnableBroadcast = $true

# Define the message to send
$message = "HELLO!"
$bytes = [System.Text.Encoding]::UTF8.GetBytes($message)

# Send the broadcast packet
$udpClient.Send($bytes, $bytes.Length, $broadcastAddress, $port)

Write-Host "Broadcast message sent: $Message"

$ipep = new-object net.ipendpoint([net.ipaddress]::any, 0)
$receive = $udpClient.receive([ref]$ipep)

Write-Output ([text.encoding]::ascii.getstring($receive))

# Close the UDP client
$udpClient.Close()
using System.Collections.ObjectModel;
using System.Threading.Tasks;

namespace Clab.Smart.Relay.App;

public partial class DevicesPage : ContentPage
{

    public class DeviceInfo
    {
        public string   Name            { get; set; }
        public string   ModelName       { get; set; }
        public string   ModelInitials   { get; set; }
        public Color    IsActive        { get; set; }
    }

    public ObservableCollection<DeviceInfo> Items { get; set; }

    public DevicesPage()
    {
        InitializeComponent();

        Items = new ObservableCollection<DeviceInfo>
        {
            new DeviceInfo
            {
                Name = "RA1237917341",
                ModelName = "SMART R1",
                ModelInitials = "R1",
                IsActive = Colors.Green
            },
            new DeviceInfo
            {
                Name = "RA1237917331",
                ModelName = "SMART R1",
                ModelInitials = "R1",
                IsActive = Colors.Blue,
            },
            new DeviceInfo
            {
                Name = "RA1237917343",
                ModelName = "SMART R1",
                ModelInitials = "R1",
                IsActive = Colors.Green
            },
            new DeviceInfo
            {
                Name = "RA1237917342",
                ModelName = "SMART R1",
                ModelInitials = "R1",
                IsActive = Colors.Blue
            },
        };

		BindingContext = this;
    }
    
    private async void OnConnectClicked(object? sender, EventArgs e)
	{
		await Shell.Current.GoToAsync("//MainPage");
	}
}

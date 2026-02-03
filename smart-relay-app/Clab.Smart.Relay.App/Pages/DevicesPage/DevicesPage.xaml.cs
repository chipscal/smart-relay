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

	private AppState		AppState { get; }


    public DevicesPage(AppState appState)
    {
        InitializeComponent();
        Items = new ObservableCollection<DeviceInfo>();

        AppState = appState;

		BindingContext = this;
    }

    private void BeginRefreshDevices(object sender, EventArgs e)
    {
        MainThread.BeginInvokeOnMainThread(() =>
        {
            Items.Clear();

            foreach (var device in AppState.KnownDevices)
            {
                Items.Add(new DeviceInfo
                {
                    Name = device.Key,
                    IsActive = Colors.Green,
                    ModelName = device.Value.ModelName,
                    ModelInitials = Utils.GetInitials(device.Value.ModelName)
                });
            }
        });
    }


    protected override async void OnAppearing()
    {
        base.OnAppearing();

		AppState.OnNewDeviceFound += BeginRefreshDevices;
    }


    protected override void OnDisappearing()
    {
        base.OnDisappearing();

		AppState.OnNewDeviceFound -= BeginRefreshDevices;
    }
    
    private async void OnConnectClicked(object? sender, EventArgs e)
	{
		await Shell.Current.GoToAsync($"//DevicePage?deviceUID={((DeviceInfo)DeviceCollectionView.SelectedItem).Name}");
	}
}

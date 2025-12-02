using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;

namespace Clab.Smart.Relay.App;

public partial class ScanPage : ContentPage
{

	public ObservableCollection<BrokerInfo> Items { get; set; }

	IDispatcherTimer 		_timer;

	private AppState		AppState { get; }
	

	public BrokerInfo SelectedBroker
	{
		get;
		set;
	}

	public ScanPage(AppState appState)
	{
		InitializeComponent();

		AppState = appState;

		_timer = Dispatcher.CreateTimer();
        _timer.Interval = TimeSpan.FromSeconds(15);
		_timer.Tick += async (s, e) => await ScanBrokers();

		Items = new ObservableCollection<BrokerInfo>();
		BindingContext = this;
	}

	protected override async void OnAppearing()
    {
        base.OnAppearing();

		await ScanBrokers();
       	_timer.Start();
    }

    protected override void OnDisappearing()
    {
        base.OnDisappearing();

		_timer.Stop();
    }

	private async Task ScanBrokers()
    {
		var oldSelectedBroker =  (BrokerInfo)BrokerCollectionView.SelectedItem;

        using (var discoveryClient = new DiscoveryServiceClient())
		{
			var brokers = await discoveryClient.FindBrokers(600, CancellationToken.None); 
			Items.Clear();
			foreach(var broker in brokers.OrderBy(b => b.Name))
				Items.Add(broker);
		}

		if (oldSelectedBroker != null)
        {
			var newSelectedBroker = Items.Where(b => b.Name == oldSelectedBroker.Name).SingleOrDefault();
			if (newSelectedBroker != null)
            {
                BrokerCollectionView.SelectedItem = newSelectedBroker;
            }
        }

    }

	private async void OnConnectClicked(object? sender, EventArgs e)
	{
		var selectedBroker = (BrokerInfo)BrokerCollectionView.SelectedItem;

		if (selectedBroker != null)
        {
			var connected = await AppState.MqttConnect(selectedBroker.Host.ToString(), selectedBroker.Port, 
					UsernameEntry.Text, PasswordEntry.Text);

			if (connected)
            {
				await Shell.Current.GoToAsync("//DevicesPage");
                
            }
            
        }
	}
}

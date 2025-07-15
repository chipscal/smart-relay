using System.Collections.ObjectModel;

namespace Clab.Smart.Relay.App;

public partial class ScanPage : ContentPage
{

	public class BrokerInfo
	{
		public string Name { get; set; }
		public string Address { get; set; }
		public int Port { get; set; }
	}

	public ObservableCollection<BrokerInfo> Items { get; set; }


	public BrokerInfo SelectedBroker
	{
		get;
		set;
	}

	public ScanPage()
	{
		InitializeComponent();

		Items = new ObservableCollection<BrokerInfo>
		{
			new BrokerInfo
			{
				Name = "prova",
				Address = "192.168.1.1",
				Port = 21354
			},
			new BrokerInfo
			{
				Name = "prova2",
				Address = "192.168.1.3",
				Port = 21354
			},
			new BrokerInfo
			{
				Name = "prova3",
				Address = "192.168.1.4",
				Port = 21354
			}
		};

		BindingContext = this;
	}

	private async void OnConnectClicked(object? sender, EventArgs e)
	{
		await Shell.Current.GoToAsync("//MainPage");
	}
}

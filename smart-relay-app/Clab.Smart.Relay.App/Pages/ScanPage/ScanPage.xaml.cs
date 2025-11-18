using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;

namespace Clab.Smart.Relay.App;

public partial class ScanPage : ContentPage
{

	public ObservableCollection<BrokerInfo> Items { get; set; }
	

	// public static readonly BindableProperty ItemsProperty =
    //     BindableProperty.Create(
    //         nameof(Items),
    //         typeof(ObservableCollection<BrokerInfo>),
    //         typeof(PropertyListView),
    //         default(ObservableCollection<BrokerInfo>),
    //         propertyChanged: OnItemsChanged);

    // public ObservableCollection<BrokerInfo> Items
    // {
    //     get => (ObservableCollection<BrokerInfo>)GetValue(ItemsProperty);
    //     set => SetValue(ItemsProperty, value);
    // }

    // private static void OnItemsChanged(BindableObject bindable, object oldValue, object newValue)
    // {
    //     if (bindable is ScanPage control && newValue is ObservableCollection<BrokerInfo> newCollection)
    //     {
    //         control.BrokerCollectionView.ItemsSource = newCollection;
    //     }
    // }


	public BrokerInfo SelectedBroker
	{
		get;
		set;
	}

	public ScanPage()
	{
		InitializeComponent();

		Items = new ObservableCollection<BrokerInfo>();
		BindingContext = this;
	}

	protected override async void OnAppearing()
    {
        base.OnAppearing();

       	using (var discoveryClient = new DiscoveryServiceClient())
		{
			var brokers = await discoveryClient.FindBrokers(600, CancellationToken.None); 
			Items.Clear();
			foreach(var broker in brokers.OrderBy(b => b.Name))
				Items.Add(broker);
			// Items = new ObservableCollection<BrokerInfo>(brokers);
		}
    }

	private async void OnConnectClicked(object? sender, EventArgs e)
	{
		await Shell.Current.GoToAsync("//MainPage");
	}
}

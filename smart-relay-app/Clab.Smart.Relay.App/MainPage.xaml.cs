using System.Threading.Tasks;

namespace Clab.Smart.Relay.App;

public partial class MainPage : ContentPage
{
	public MainPage()
	{
		InitializeComponent();
	}

	private void OnConnectClicked(object? sender, EventArgs e)
	{

	}
	
	private async void OnScanClicked(object? sender, EventArgs e)
	{
		await Shell.Current.GoToAsync("//ScanPage");
	}
}

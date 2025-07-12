using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;

namespace Clab.Smart.Relay.App;


public partial class DeviceStatusView : ContentView
{

    public static readonly BindableProperty DeviceStatusProperty =
        BindableProperty.Create(
            nameof(DeviceStatus),
            typeof(ObservableCollection<DevicePropertyInfo>),
            typeof(DeviceStatusView),
            default(ObservableCollection<DevicePropertyInfo>),
            propertyChanged: OnDeviceStatusChanged);

    public ObservableCollection<DevicePropertyInfo> DeviceStatus
    {
        get => (ObservableCollection<DevicePropertyInfo>)GetValue(DeviceStatusProperty);
        set => SetValue(DeviceStatusProperty, value);
    }

    private static void OnDeviceStatusChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is DeviceStatusView control && newValue is ObservableCollection<DevicePropertyInfo> newCollection)
        {
            control.DeviceStatusCollectionView.ItemsSource = newCollection;
        }
    }


    public DeviceStatusView()
    {
        InitializeComponent();

        BindingContext = this;
    }

}

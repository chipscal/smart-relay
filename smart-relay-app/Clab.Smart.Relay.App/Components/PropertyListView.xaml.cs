using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;

namespace Clab.Smart.Relay.App;

public partial class PropertyListView : ContentView
{

    
    public PropertyListView()
    {
        InitializeComponent();

        BindingContext = this;

        DeviceProperties = new ObservableCollection<DevicePropertyInfo>();
    }

    public static readonly BindableProperty DevicePropertiesProperty =
        BindableProperty.Create(
            nameof(DeviceProperties),
            typeof(ObservableCollection<DevicePropertyInfo>),
            typeof(PropertyListView),
            default(ObservableCollection<DevicePropertyInfo>),
            propertyChanged: OnDeviceStatusChanged);

    public ObservableCollection<DevicePropertyInfo> DeviceProperties
    {
        get => (ObservableCollection<DevicePropertyInfo>)GetValue(DevicePropertiesProperty);
        set => SetValue(DevicePropertiesProperty, value);
    }

    private static void OnDeviceStatusChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is PropertyListView control && newValue is ObservableCollection<DevicePropertyInfo> newCollection)
        {
            control.DevicePropertiesListInstance.ItemsSource = newCollection;
        }
    }

    public static readonly BindableProperty OnAddPropertyClickedProperty =
        BindableProperty.Create(
            nameof(OnAddPropertyClicked),
            typeof(Func<Task>),
            typeof(PropertyListView),
            default(Func<Task>));

    public Func<Task> OnAddPropertyClicked
    {
        get => (Func<Task>)GetValue(OnAddPropertyClickedProperty);
        set => SetValue(OnAddPropertyClickedProperty, value);
    }

    private async void AddPropClicked(object sender, EventArgs e)
    {
        if (OnAddPropertyClicked != null)
            await OnAddPropertyClicked.Invoke();
    }

}

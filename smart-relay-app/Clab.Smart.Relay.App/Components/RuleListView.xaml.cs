using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;

namespace Clab.Smart.Relay.App;

public partial class RuleListView : ContentView
{


    public static readonly BindableProperty DevicePropertiesProperty =
        BindableProperty.Create(
            nameof(DeviceRules),
            typeof(ObservableCollection<DeviceRuleInfo>),
            typeof(RuleListView),
            default(ObservableCollection<DeviceRuleInfo>),
            propertyChanged: OnDeviceStatusChanged);

    public ObservableCollection<DeviceRuleInfo> DeviceRules
    {
        get => (ObservableCollection<DeviceRuleInfo>)GetValue(DevicePropertiesProperty);
        set => SetValue(DevicePropertiesProperty, value);
    }

    private static void OnDeviceStatusChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is RuleListView control && newValue is ObservableCollection<DeviceRuleInfo> newCollection)
        {
            control.DeviceRulesListInstance.ItemsSource = newCollection;
        }
    }


    public RuleListView()
    {
        InitializeComponent();

        BindingContext = this;

        DeviceRules = new ObservableCollection<DeviceRuleInfo>();
    }

    public static readonly BindableProperty OnAddRuleClickedProperty =
        BindableProperty.Create(
            nameof(OnAddRuleClicked),
            typeof(Func<Task>),
            typeof(RuleListView),
            default(Func<Task>));

    public Func<Task> OnAddRuleClicked
    {
        get => (Func<Task>)GetValue(OnAddRuleClickedProperty);
        set => SetValue(OnAddRuleClickedProperty, value);
    }

    private async void AddRuleClicked(object sender, EventArgs e)
    {
        if (OnAddRuleClicked != null)
            await OnAddRuleClicked.Invoke();
    }

}

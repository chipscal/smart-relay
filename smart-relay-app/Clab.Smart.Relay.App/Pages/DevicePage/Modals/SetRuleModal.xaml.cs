using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;
using Microsoft.Maui.Controls;

namespace Clab.Smart.Relay.App;


public partial class SetRuleModal : ContentPage, INotifyPropertyChanged
{

    private string _deviceUID;
    public string DeviceUID
    {
        get => _deviceUID;
        set
        {
            _deviceUID = value;
            OnPropertyChanged();
        }
    }

    private DeviceTags _action;
    public DeviceTags Action
    {
        get => _action;
        set
        {
            _action = value;
            OnPropertyChanged();
        }
    }


    public static readonly BindableProperty ConditionsProperty =
        BindableProperty.Create(
            nameof(Conditions),
            typeof(ObservableCollection<DeviceUnaryConditionInfo>),
            typeof(RuleListView),
            default(ObservableCollection<DeviceUnaryConditionInfo>),
            propertyChanged: OnConditionsChanged);

    public ObservableCollection<DeviceUnaryConditionInfo> Conditions
    {
        get => (ObservableCollection<DeviceUnaryConditionInfo>)GetValue(ConditionsProperty);
        set => SetValue(ConditionsProperty, value);
    }

    private static void OnConditionsChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is SetRuleModal control && newValue is ObservableCollection<DeviceUnaryConditionInfo> newCollection)
        {
            control.ConditionsCollection.ItemsSource = newCollection;
        }
    }

    public ObservableCollection<DeviceTags> AllOutputTags { get; } = new ObservableCollection<DeviceTags>(
            Enum.GetValues<DeviceTags>().Where(t => t.IsOutputProperty()));

    public ObservableCollection<DeviceTags> AllInputTags { get; } = new ObservableCollection<DeviceTags>(
            Enum.GetValues<DeviceTags>().Where(t => t.IsInputProperty()));

    public SetRuleModal()
    {
        InitializeComponent();

        BindingContext = this;
    }



    private async void OnCancelClicked(object? sender, EventArgs e)
    {
        await Navigation.PopModalAsync();
    }

    private async void OnSaveClicked(object? sender, EventArgs e)
    {

    }

    private async void OnAddConditionClicked(object? sender, EventArgs e)
    {
        var taskCompletionSource = new TaskCompletionSource<DeviceUnaryConditionInfo>();

        await Navigation.PushModalAsync(new SetUnaryConditionModal(taskCompletionSource)
        {

        });

        try
        {
            var result = await taskCompletionSource.Task;
            if (result != null) //always good
            {
                // do something
            }
        }
        catch
        {
            //operation cancelled!
        }
    }

    private async void OnDeleteConditionClicked(object? sender, EventArgs e)
    {
    
    }
}

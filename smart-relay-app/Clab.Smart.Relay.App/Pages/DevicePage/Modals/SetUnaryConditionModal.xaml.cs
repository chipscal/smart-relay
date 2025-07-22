using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;
using Microsoft.Maui.Controls;

namespace Clab.Smart.Relay.App;


public partial class SetUnaryConditionModal : ContentPage, INotifyPropertyChanged
{

    private int? _index;
    public int? Index
    {
        get => _index;
        set
        {
            _index = value;
            OnPropertyChanged();
        }
    }

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

    private DeviceTags _port;
    public DeviceTags Port
    {
        get => _port;
        set
        {
            _port = value;
            OnPropertyChanged();
        }
    }

    private DeviceUnaryOperator _operator;
    public DeviceUnaryOperator Operator
    {
        get => _operator;
        set
        {
            _operator = value;
            OnPropertyChanged();
        }
    }


    private double _value;
    public double Value
    {
        get => _value;
        set
        {
            _value = value;
            OnPropertyChanged();
        }
    }

    private string _target;
    public string Target
    {
        get => _target;
        set
        {
            _target = value;
            OnPropertyChanged();
        }
    }


    public ObservableCollection<DeviceTags> AllInputTags { get; } = new ObservableCollection<DeviceTags>(
            Enum.GetValues<DeviceTags>().Where(t => t.IsInputProperty()));

    public ObservableCollection<DeviceUnaryOperator> AllOperatorTags { get; } = new ObservableCollection<DeviceUnaryOperator>(
            Enum.GetValues<DeviceUnaryOperator>());


    private readonly TaskCompletionSource<DeviceUnaryConditionInfo> _taskCompletionSource;

    public SetUnaryConditionModal(TaskCompletionSource<DeviceUnaryConditionInfo> taskCompletionSource)
    {
        InitializeComponent();
        _taskCompletionSource = taskCompletionSource;

        BindingContext = this;
    }



    private async void OnCancelClicked(object? sender, EventArgs e)
    {
        _taskCompletionSource.TrySetCanceled();
        await Navigation.PopModalAsync();
    }

    private async void OnSaveClicked(object? sender, EventArgs e)
    {
        _taskCompletionSource.TrySetResult(new DeviceUnaryConditionInfo
        {
            Index = this.Index,
            Operator = this.Operator,
            Port = this.Port,
            Target = this.Target,
            Value = this.Value
        });

        await Navigation.PopModalAsync();
    }

}

using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;
using Microsoft.Maui.Controls;

namespace Clab.Smart.Relay.App;


public partial class SetProgramModal : ContentPage, INotifyPropertyChanged
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

    private DateTime _minDate = DateTime.UnixEpoch;
    public DateTime MinDate
    {
        get => _minDate;
        set
        {
            _minDate = value;
            OnPropertyChanged();
        }
    }

    private DateTime _maxDate = DateTime.MaxValue;
    public DateTime MaxDate
    {
        get => _maxDate;
        set
        {
            _maxDate = value;
            OnPropertyChanged();
        }
    }

    private DateTime _startDate = DateTime.Today;
    public DateTime StartDate
    {
        get => _startDate;
        set
        {
            _startDate = value;
            OnPropertyChanged();
        }
    }

    private DateTime _endDate = DateTime.Today;
    public DateTime EndDate
    {
        get => _endDate;
        set
        {
            _endDate = value;
            OnPropertyChanged();
        }
    }

    private TimeSpan _startTime = DateTime.Today.TimeOfDay;
    public TimeSpan StartTime
    {
        get => _startTime;
        set
        {
            _startTime = value;
            OnPropertyChanged();
        }
    }


    private uint? _duration;
    public uint? Duration
    {
        get => _duration;
        set
        {
            _duration = value;
            OnPropertyChanged();
        }
    }

    private uint? _idle;
    public uint? Idle
    {
        get => _idle;
        set
        {
            _idle = value;
            OnPropertyChanged();
        }
    }

    public static readonly BindableProperty ActiveOutputsProperty =
        BindableProperty.Create(
            nameof(ActiveOutputs),
            typeof(ObservableCollection<DeviceTags>),
            typeof(PropertyListView),
            default(ObservableCollection<DeviceTags>),
            propertyChanged: OnDeviceStatusChanged);

    public ObservableCollection<DeviceTags> ActiveOutputs
    {
        get => (ObservableCollection<DeviceTags>)GetValue(ActiveOutputsProperty);
        set => SetValue(ActiveOutputsProperty, value);
    }

    private static void OnDeviceStatusChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is SetProgramModal control && newValue is ObservableCollection<DeviceTags> newCollection)
        {
            control.ActiveOutputsCollection.ItemsSource = newCollection;
        }
    }

    
    private DeviceTags? _selectedOutput;
    public DeviceTags? SelectedOutput
    {
        get => _selectedOutput;
        set
        {
            _selectedOutput = value;
            OnPropertyChanged();
        }
    }


    public ObservableCollection<DeviceTags> AllOutputTags { get; } = new ObservableCollection<DeviceTags>(
            Enum.GetValues<DeviceTags>().Where(t => t.IsOutputProperty()));


    public SetProgramModal()
    {
        InitializeComponent();

        BindingContext = this;
    }

    private async void OnAddOutputClicked(object? sender, EventArgs e)
    {
        var taskCompletionSource = new TaskCompletionSource<DeviceTags>();

        await Navigation.PushModalAsync(new AddOutputModal(taskCompletionSource)
        {
            DeviceUID = this.DeviceUID,
        });

        try
        {
            var result = await taskCompletionSource.Task;
            if (!ActiveOutputs.Contains(result))
                ActiveOutputs.Add(result);

            ReorderActiveOutputs();
        }
        catch
        {
            //operation cancelled!
        }
	}

    private void ReorderActiveOutputs()
    {
        var temp = ActiveOutputs.OrderDescending().ToList();
        ActiveOutputs.Clear();
        foreach (var output in temp)
            ActiveOutputs.Add(output);
    }

    private async void OnDeleteOutputClicked(object? sender, EventArgs e)
    {
        if (SelectedOutput != null)
        {
            ActiveOutputs.Remove(SelectedOutput.Value);
            SelectedOutput = null;
        }
	}

    private async void OnCancelClicked(object? sender, EventArgs e)
    {
        await Navigation.PopModalAsync();
    }
    
    private async void OnSaveClicked(object? sender, EventArgs e)
	{
		
	}

}

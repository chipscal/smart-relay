using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;
using Microsoft.Maui.Controls;

namespace Clab.Smart.Relay.App;


public partial class AddOutputModal : ContentPage, INotifyPropertyChanged
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

    private DeviceTags _tag;
    public DeviceTags Tag
    {
        get => _tag;
        set
        {
            _tag = value;
            OnPropertyChanged();
        }
    }

    public ObservableCollection<DeviceTags> AllOutputTags { get; } = new ObservableCollection<DeviceTags>(
            Enum.GetValues<DeviceTags>().Where(t => t.IsOutputProperty()));

    private readonly TaskCompletionSource<DeviceTags> _taskCompletionSource;


    public AddOutputModal(TaskCompletionSource<DeviceTags> taskCompletionSource)
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
        _taskCompletionSource.TrySetResult(Tag);
        await Navigation.PopModalAsync();
	}

}

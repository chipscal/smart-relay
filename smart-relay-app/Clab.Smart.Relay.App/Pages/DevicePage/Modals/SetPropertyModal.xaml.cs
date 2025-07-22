using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;
using Microsoft.Maui.Controls;

namespace Clab.Smart.Relay.App;


public partial class SetPropertyModal : ContentPage, INotifyPropertyChanged
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


    private string _value;
    public string Value
    {
        get => _value;
        set
        {
            _value = value;
            OnPropertyChanged();
        }
    }

    public ObservableCollection<DeviceTags> AllTags { get; } = new ObservableCollection<DeviceTags>(
            Enum.GetValues<DeviceTags>().Where(t => t.IsSettableProperty()));

    public SetPropertyModal()
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

}

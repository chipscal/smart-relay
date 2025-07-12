using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;

namespace Clab.Smart.Relay.App;

public partial class ProgramListView : ContentView
{

    public ProgramListView()
    {
        InitializeComponent();

        BindingContext = this;
    }

    public static readonly BindableProperty DeviceProgramsProperty =
        BindableProperty.Create(
            nameof(DevicePrograms),
            typeof(ObservableCollection<DeviceProgramInfo>),
            typeof(ProgramListView),
            default(ObservableCollection<DeviceProgramInfo>),
            propertyChanged: OnDeviceStatusChanged);

    public ObservableCollection<DeviceProgramInfo> DevicePrograms
    {
        get => (ObservableCollection<DeviceProgramInfo>)GetValue(DeviceProgramsProperty);
        set => SetValue(DeviceProgramsProperty, value);
    }

    private static void OnDeviceStatusChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is ProgramListView control && newValue is ObservableCollection<DeviceProgramInfo> newCollection)
        {
            control.ProgramListInstance.ItemsSource = newCollection;
        }
    }

    public static readonly BindableProperty OnAddProgramClickedProperty =
        BindableProperty.Create(
            nameof(OnAddProgramClicked),
            typeof(Func<Task>),
            typeof(ProgramListView),
            default(Func<Task>));

    public Func<Task> OnAddProgramClicked
    {
        get => (Func<Task>)GetValue(OnAddProgramClickedProperty);
        set => SetValue(OnAddProgramClickedProperty, value);
    }


    private async void AddProgramClicked(object sender, EventArgs e)
    {
        if (OnAddProgramClicked != null)
            await OnAddProgramClicked.Invoke();
    }

}

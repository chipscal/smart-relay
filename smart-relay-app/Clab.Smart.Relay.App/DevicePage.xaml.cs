using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Clab.Smart.Relay.App;

public partial class DevicePage : ContentPage
{

    public class DevicePropertyInfo
    {
        public string Name { get; set; }
        public string Value { get; set; }
    }


    public ObservableCollection<DevicePropertyInfo> DeviceStatus { get; set; }


    public DevicePage()
    {
        InitializeComponent();

        DeviceStatus = new ObservableCollection<DevicePropertyInfo>
        {
            new DevicePropertyInfo
            {
                Name = "H.Rev",
                Value = "1"
            },
            new DevicePropertyInfo
            {
                Name = "S.Rev",
                Value = "1.1"
            },
            new DevicePropertyInfo
            {
                Name = "Latch",
                Value = "00000000000000000000000000000000"
            },
            new DevicePropertyInfo
            {
                Name = "Relay",
                Value = "00000000000000000000000000000001"
            },
            new DevicePropertyInfo
            {
                Name = "Digital",
                Value = "00000000000000000000000000000000"
            },
            new DevicePropertyInfo
            {
                Name = "Voltage[1]",
                Value = "0.00 mV"
            },
            new DevicePropertyInfo
            {
                Name = "Pulse[1]",
                Value = "12400"
            },
            new DevicePropertyInfo
            {
                Name = "Temperature[1]",
                Value = "22.32 °C"
            },
        };


        BindingContext = this;
    }

    protected override void OnSizeAllocated(double width, double height)
    {
        base.OnSizeAllocated(width, height);

        //respond to size change below...
        Debug.WriteLine("size changed2");
        // DeviceStatusCollectionView.InvalidateMeasureNonVirtual(Microsoft.Maui.Controls.Internals.InvalidationTrigger.SizeRequestChanged);
        DeviceStatus = DeviceStatus;
    }
    
}

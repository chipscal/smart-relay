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

    public class DeviceRuleInfo
    {
        public int Index { get; set; }
        public IEnumerable<string> UnaryRules { get; set; }
        public string Action { get; set; }

        public string CombinedRule => UnaryRules.Aggregate((s1, s2) => s1 + "\n" + s2);
    }

    public class DeviceProgramInfo
    {
        public int Index { get; set; }
        public DateTime Start { get; set; }
        public DateTime End { get; set; }
        public TimeSpan Duration { get; set; }
        public TimeSpan Idle { get; set; }

    }

    public ObservableCollection<DevicePropertyInfo> DeviceStatus { get; set; }
    public ObservableCollection<DevicePropertyInfo> DeviceProperties { get; set; }
    public ObservableCollection<DeviceRuleInfo> DeviceRules { get; set; }
    public ObservableCollection<DeviceProgramInfo> DevicePrograms { get; set; }


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

        DeviceProperties = new ObservableCollection<DevicePropertyInfo>
        {
            new DevicePropertyInfo
            {
                Name = "R[1]",
                Value = "<2000, 3000> ms"
            },
            new DevicePropertyInfo
            {
                Name = "P[1]",
                Value = "3000 ms"
            }
        };

        DeviceRules = new ObservableCollection<DeviceRuleInfo>
        {
            new DeviceRuleInfo
            {
                Index = 1,
                UnaryRules = new string[]
                {
                    "d[0]=1,RA1237917341",
                    "v[0]=1.22,RA1237917341"
                },
                Action = "r0"
            },
            new DeviceRuleInfo
            {
                Index = 2,
                UnaryRules = new string[]
                {
                    "d[0]=1,RA1237917343",
                    "v[0]>1.5,RA1237917343"
                },
                Action = "r0"
            }
        };

        DevicePrograms = new ObservableCollection<DeviceProgramInfo>
        {
            new DeviceProgramInfo
            {
                Index = 1,
                Start = DateTime.UtcNow.AddDays(-20),
                End = DateTime.UtcNow.AddDays(60),
                Duration = TimeSpan.FromHours(2),
                Idle = TimeSpan.FromHours(5)
            }
        };

        BindingContext = this;
    }

    private void AddPropClicked(object sender, EventArgs e)
    {
    }

    private void AddRuleClicked(object sender, EventArgs e)
    {
    }
    
    private void AddProgramClicked(object sender, EventArgs e)
    {
    }

    // protected override void OnSizeAllocated(double width, double height)
    // {
    //     base.OnSizeAllocated(width, height);

    //     //respond to size change below...
    //     Debug.WriteLine("size changed2");
    //     DeviceStatusCollectionView.InvalidateMeasureNonVirtual(Microsoft.Maui.Controls.Internals.InvalidationTrigger.SizeRequestChanged);
    //     DeviceStatus = DeviceStatus;
    // }

}

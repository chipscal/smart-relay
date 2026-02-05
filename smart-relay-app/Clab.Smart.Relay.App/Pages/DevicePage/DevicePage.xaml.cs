using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;
using Microsoft.Maui.Controls;

namespace Clab.Smart.Relay.App;


[QueryProperty(nameof(DeviceUID), "deviceUID")]
public partial class DevicePage : ContentPage
{

    private string _deviceUID = null;
    public string DeviceUID 
    {
        get => _deviceUID; 
        set
        {
            if (_deviceUID != null && AppState.KnownDevices.TryGetValue(_deviceUID, out Device oldDevice))
            {
                oldDevice.OnTelemetryUpdate -= BeginRefreshDeviceStatusAndProperties;
                oldDevice.OnPropertyChanged -= BeginRefreshDeviceStatusAndProperties;
            }

            _deviceUID = value;
            

            MainThread.BeginInvokeOnMainThread(() =>
            {
                UpdateDeviceStatusAndProperties(); 

                if (AppState.KnownDevices.TryGetValue(_deviceUID, out Device device))
                {
                    device.OnTelemetryUpdate += BeginRefreshDeviceStatusAndProperties;
                    device.OnPropertyChanged += BeginRefreshDeviceStatusAndProperties;
                }

                MainThread.InvokeOnMainThreadAsync(ScanAllProperties);
            });
        }
    }

    private DeviceHeaderView DeviceHeader;
    private DeviceStatusView DeviceStatus;
    private PropertyListView PropertyList;
    private RuleListView     RuleList;
    private ProgramListView  ProgramList;

	private AppState		AppState { get; }

    private double? _lastWidth = null;

    public DevicePage(AppState appState)
    {
        InitializeComponent();

        AppState = appState;

        DeviceHeader = new DeviceHeaderView();
        DeviceStatus = new DeviceStatusView();
        PropertyList = new PropertyListView();
        RuleList = new RuleListView();
        ProgramList = new ProgramListView();

        ProgramList = new ProgramListView();
        ProgramList.DevicePrograms = new ObservableCollection<DeviceProgramInfo>
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
        ProgramList.OnAddProgramClicked = async () =>
        {
            await Navigation.PushModalAsync(new SetProgramModal
            {
                ActiveOutputs = new ObservableCollection<DeviceTags>
                {
                    DeviceTags.RELAY1,
                    DeviceTags.LATCH1
                }
            });
        };

        PropertyList = new PropertyListView();
        PropertyList.DeviceProperties = new ObservableCollection<DevicePropertyInfo>
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
        PropertyList.OnAddPropertyClicked = async () =>
        {
            await Navigation.PushModalAsync(new SetPropertyModal
            {
                DeviceUID = "RA1237917341",
            });
        };

        RuleList = new RuleListView();
        RuleList.DeviceRules = new ObservableCollection<DeviceRuleInfo>
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

        RuleList.OnAddRuleClicked = async () =>
        {
            await Navigation.PushModalAsync(new SetRuleModal
            {
                DeviceUID = "RA1237917341",
                Conditions = new ObservableCollection<DeviceUnaryConditionInfo>
                {
                    new DeviceUnaryConditionInfo
                    {
                        Index = 1,
                        Port = DeviceTags.DIGITAL_INPUT1,
                        Operator = DeviceUnaryOperator.EQ,
                        Target = "RA1237917342",
                        Value = 1
                    },
                    new DeviceUnaryConditionInfo
                    {
                        Index = 2,
                        Port = DeviceTags.DIGITAL_INPUT2,
                        Operator = DeviceUnaryOperator.EQ,
                        Target = "RA1237917342",
                        Value = 1
                    }
                }
            });
        };

        if (DeviceInfo.Current.Idiom == DeviceIdiom.Phone)
            BuildMobile();
        else
            BuildDesktop();

        BindingContext = this;
    }

    private void BeginRefreshDeviceStatusAndProperties(object sender, EventArgs e)
    {
        MainThread.BeginInvokeOnMainThread(() =>
        {
            UpdateDeviceStatusAndProperties(); 
        });
    }

    private void UpdateDeviceStatusAndProperties()
    {
        if (AppState.KnownDevices.TryGetValue(DeviceUID, out Device device))
        {
            DeviceHeader.Title = DeviceUID;
            DeviceHeader.Model = device.ModelName;

            DeviceStatus.DeviceStatus.Clear();
            foreach (var info in device.Telemetry.Values
                    .OrderBy(t => t.Tag)
                    .Select(t => new DevicePropertyInfo { Name = t.Tag.ToString(), Value = t.Value }))
            {
                DeviceStatus.DeviceStatus.Add(info);
            }

            PropertyList.DeviceProperties.Clear();
            foreach (var prop in device.Properties.Values
                    .Where(p => !p.Tag.IsControlProperty())
                    .OrderBy(p => p.Tag)
                    .Select(p => new DevicePropertyInfo { Name = p.Tag.ToString(), Value = ConvertPropertyToHuman(p.Tag, p.Value)}))
            {
                PropertyList.DeviceProperties.Add(prop);
            }

            RuleList.DeviceRules.Clear();
            foreach (var prop in device.Properties.Values
                    .Where(p => p.Tag >= DeviceTags.RULE1 && p.Tag <= DeviceTags.RULE32)
                    .OrderBy(p => p.Tag)
                    .Select(p =>
                    {

                        var deviceRule = new DeviceRule();
                        deviceRule.ParseFrom(Encoding.UTF8.GetString(Convert.FromBase64String(p.Value)));

                        return new DeviceRuleInfo
                        {
                            Index = p.Tag - DeviceTags.RULE1 + 1,
                            Action = deviceRule.Action.ToString(),
                            UnaryRules = deviceRule.Rules.Select(r => r.Serialize())
                        };
                    }))
            {
                RuleList.DeviceRules.Add(prop);
            }

        }
    }

    private async Task ScanAllProperties()
    {
        if (AppState.KnownDevices.TryGetValue(DeviceUID, out Device device))
        {
            foreach (var prop in device.Properties.Values)
            {
                await prop.QueryAsync();
            }

            for (int k = 0; k < 32; k++)
            {
                var tag = DeviceTags.PROG1 + k;
                if (!device.Properties.ContainsKey(tag))
                {
                    await device.TryQueryProperty(tag);
                }
                
                tag = DeviceTags.RULE1 + k;
                if (!device.Properties.ContainsKey(tag))
                {
                    await device.TryQueryProperty(tag);
                }
            }
        }
    }

    /// <summary>
    /// Converts base64 property to human readable string
    /// </summary>
    /// <param name="tag">of the property</param>
    /// <param name="value">to convert</param>
    /// <returns>human readable string according to tag and value</returns>
    /// <remarks>For localization AppState can be accessed here!</remarks>
    private string ConvertPropertyToHuman(DeviceTags tag, string value)
    {
        var decodedBuffer = Convert.FromBase64String(value);

        if (tag >= DeviceTags.PULSE_FILTER1 && tag <= DeviceTags.PULSE_FILTER32)
        {
            if (decodedBuffer.Length == sizeof(UInt16))
            {

                if (!BitConverter.IsLittleEndian)
                    Array.Reverse(decodedBuffer, 0, sizeof(UInt16));

                var millis = BitConverter.ToUInt16(decodedBuffer, 0);

                return $"{millis} ms";
            }
        }

        if (tag >= DeviceTags.RELAY_DELAYS1 && tag <= DeviceTags.RELAY_DELAYS32)
        {
            if (decodedBuffer.Length == sizeof(UInt16))
            {
                int start = decodedBuffer[0];
                int stop = decodedBuffer[1];

                return $"<start: {start} s, stop: {stop} s>";
            }
        }

        return Encoding.UTF8.GetString(decodedBuffer);
    }

    private void OnPageSizeChanged(object sender, EventArgs e)
    {

        MainThread.BeginInvokeOnMainThread(() =>
        {
            if (_lastWidth == null || 
            (_lastWidth >= 600 && Width < 600) ||
            (_lastWidth < 600 && Width >= 600))
            {
                _lastWidth = Width;

                DeviceHeader = new DeviceHeaderView();
                DeviceStatus = new DeviceStatusView();
                PropertyList = new PropertyListView();
                RuleList = new RuleListView();
                ProgramList = new ProgramListView();

                if (DeviceInfo.Current.Idiom == DeviceIdiom.Phone)
                    BuildMobile();
                else if (Width < 600)
                    BuildMobile();
                else
                    BuildDesktop();

                UpdateDeviceStatusAndProperties();
            }

        });
    }

    private void BuildMobile()
    {
        var stack = new VerticalStackLayout();

        stack.Children.Add(DeviceHeader);
        stack.Children.Add(DeviceStatus);
        stack.Children.Add(PropertyList);
        stack.Children.Add(RuleList);
        stack.Children.Add(ProgramList);


        this.Content = new ScrollView { Content = stack };        
    }

    private void BuildDesktop()
    {
        var mainGrid = new Grid
        {
            RowDefinitions =
            {
                new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
            },
            ColumnDefinitions =
            {
                new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
                new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) }
            },
            Margin = 10,
            RowSpacing = 10,
            ColumnSpacing = 10
        };


        var left = new ScrollView
        {
            Content = new VerticalStackLayout
            {
                Children =
                {
                    DeviceHeader,
                    DeviceStatus,
                    PropertyList,

                },
                Margin = 10
            }
        };

        mainGrid.AddWithSpan(left, 0, 0);

        var right = new ScrollView
        {
            Content = new VerticalStackLayout
            {
                Children =
                {
                    RuleList,
                    ProgramList,    
                },
                Margin = 10
            }
        };

        mainGrid.AddWithSpan(right, 0, 1);

        this.Content = mainGrid;
    }

}

using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;
using Microsoft.Maui.Controls;

namespace Clab.Smart.Relay.App;


[QueryProperty(nameof(DeviceUID), "deviceUID")]
public partial class DevicePage : ContentPage
{

    public string DeviceUID {get; set;}

    private readonly DeviceHeaderView DeviceHeader;
    private readonly DeviceStatusView DeviceStatus;
    private readonly PropertyListView PropertyList;
    private readonly RuleListView     RuleList;
    private readonly ProgramListView  ProgramList;

    public DevicePage()
    {
        InitializeComponent();

        DeviceHeader = new DeviceHeaderView
        {
            Title = "RA1237917341",
            Model = "SMART R1"
        };

        DeviceStatus = new DeviceStatusView();
        DeviceStatus.DeviceStatus = new ObservableCollection<DevicePropertyInfo>
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
                Value = "None"
            },
            new DevicePropertyInfo
            {
                Name = "Relay",
                Value = "r0"
            },
            new DevicePropertyInfo
            {
                Name = "Digital",
                Value = "d0,d1"
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

    protected override async void OnAppearing()
    {
        base.OnAppearing();
        Debug.WriteLine(DeviceUID);
    }


    protected override void OnDisappearing()
    {
        base.OnDisappearing();

    }

    private void BuildMobile()
    {
        var stack = new VerticalStackLayout
        {
            
        };

        stack.Children.Add(DeviceHeader);
        stack.Children.Add(DeviceStatus);
        stack.Children.Add(PropertyList);
        stack.Children.Add(RuleList);
        stack.Children.Add(ProgramList);


        PageRoot.Content = stack;        
    }

    private void BuildDesktop()
    {
        var grid = new Grid
        {
            RowDefinitions =
            {
                new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
                new RowDefinition { Height = new GridLength(2, GridUnitType.Star) },
                new RowDefinition { Height = new GridLength(3, GridUnitType.Star) }

            },
            ColumnDefinitions =
            {
                new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
                new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
                new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
            },
            Margin = 10,
            RowSpacing = 10,
            ColumnSpacing = 10
        };


        grid.AddWithSpan(DeviceHeader, 0, 0, columnSpan: 3);
        grid.AddWithSpan(DeviceStatus, 1, 0, columnSpan: 3);
        grid.AddWithSpan(PropertyList, 2, 0, columnSpan: 1);
        grid.AddWithSpan(RuleList, 2, 1, columnSpan: 1);
        grid.AddWithSpan(ProgramList, 2, 2, columnSpan: 1);

        PageRoot.Content = grid;
    }

}

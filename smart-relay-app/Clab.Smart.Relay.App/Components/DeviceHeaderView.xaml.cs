using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Clab.Smart.Relay.App.Common;

namespace Clab.Smart.Relay.App;

public partial class DeviceHeaderView : ContentView
{
    
    public static readonly BindableProperty TitleProperty =
        BindableProperty.Create(
            nameof(Title),
            typeof(string),
            typeof(DeviceHeaderView),
            default(string),
            propertyChanged: OnTitleChanged);

    public string Title
    {
        get => (string)GetValue(TitleProperty);
        set => SetValue(TitleProperty, value);
    }

    private static void OnTitleChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is DeviceHeaderView control && newValue is string newText)
        {
            control.TitleLabel.Text = newText;
        }
    }

    public static readonly BindableProperty ModelProperty =
        BindableProperty.Create(
            nameof(Model),
            typeof(string),
            typeof(DeviceHeaderView),
            default(string),
            propertyChanged: OnModelChanged);

    public string Model
    {
        get => (string)GetValue(ModelProperty);
        set => SetValue(ModelProperty, value);
    }

    private static void OnModelChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is DeviceHeaderView control && newValue is string newText)
        {
            control.ModelLabel.Text = newText;
        }
    }

    public DeviceHeaderView()
    {
        InitializeComponent();
    }

}

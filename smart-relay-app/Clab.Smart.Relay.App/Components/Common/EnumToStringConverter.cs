using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Globalization;
using System.Threading.Tasks;

namespace Clab.Smart.Relay.App.Common;


public class EnumToStringConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        return value?.ToString();
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        return Enum.Parse(targetType, value.ToString());
    }
}
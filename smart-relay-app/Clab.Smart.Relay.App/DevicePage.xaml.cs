using System.Collections.ObjectModel;
using System.Threading.Tasks;

namespace Clab.Smart.Relay.App;

public partial class DevicePage : ContentPage
{

    

    public DevicePage()
    {
        InitializeComponent();

        
		BindingContext = this;
    }
    
}

using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Clab.Smart.Relay.App.Common;

public class DeviceProgramInfo
{
    public int Index { get; set; }
    public DateTime Start { get; set; }
    public DateTime End { get; set; }
    public TimeSpan Duration { get; set; }
    public TimeSpan Idle { get; set; }

}

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

public class DeviceUnaryConditionInfo : DeviceUnaryCondition
{
    public int? Index { get; set; }
}
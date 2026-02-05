using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text.RegularExpressions;

namespace Clab.Smart.Relay.App;

/// @brief I/O port types.
public enum DevicePortType 
{
    EMPTY       = 0x0,
    RELAY       = 'r',
    LATCH       = 'l',
    // LED         = 'e',
    CURRENT     = 'c',
    VOLTAGE     = 'v',
    PULSE       = 'p',
    DIGITAL     = 'd',
    TEMPERATURE = 't'
};

public enum DeviceUnaryOperator
{
    NOPE        = 0x0,
    EQ          = '=',
    NEQ         = '!',
    GT          = '>',
    GTE         = 'g',
    LT          = '<',
    LTE         = 'l',
    CHANGE      = '?'
}

public class DeviceUnaryCondition
{
    public DeviceTags?              Port        { get; set; }
    public DeviceUnaryOperator      Operator    { get; set; }
    public double                   Value       { get; set; }
    public string                   Target      { get; set; }

    /// <summary>
    /// Parse condition from serialized string rapresentation
    /// </summary>
    /// <param name="input">"<port_type>[<port_index>]<operator><value>,<target0:T-1>" (e.g. "d[0]=1,XXXXXXXXXXXXXXXX")</param>
    /// <returns></returns>
    public bool ParseFrom(string input)
    {
        const string condMatch = @"^(?<port_type>[rlcvpdt]+)\[(?<port_index>\d+)\](?<operator>[=!>g<l?]+)(?<value>[^,]+),(?<target>.+)$";

        var regex = new Regex(condMatch);
        var match = regex.Match(input);

        if (match.Success)
        {
            if (int.TryParse(match.Groups["port_index"].Value, out int portIndex) && 
                    Utils.TryParseCharEnum<DevicePortType>(match.Groups["port_type"].Value, out DevicePortType portType))
            {
                Port = portType switch
                {
                    DevicePortType.RELAY => DeviceTags.RELAY1 + portIndex,
                    DevicePortType.LATCH => DeviceTags.LATCH1 + portIndex,
                    DevicePortType.CURRENT => DeviceTags.A_CURRENT_INPUT1 + portIndex,
                    DevicePortType.VOLTAGE => DeviceTags.A_VOLTAGE_INPUT1 + portIndex,
                    DevicePortType.PULSE => DeviceTags.PULSE_INPUT1 + portIndex,
                    DevicePortType.DIGITAL => DeviceTags.DIGITAL_INPUT1 + portIndex, 
                    DevicePortType.TEMPERATURE => DeviceTags.TEMPERATURE_INPUT1 + portIndex,
                    _ => null
                };
            }
            else
            {
                return false;
            }

            if (Utils.TryParseCharEnum<DeviceUnaryOperator>(match.Groups["operator"].Value, out DeviceUnaryOperator unaryOperator))
            {
                Operator = unaryOperator;
            }
            else
            {
                return false;
            }


            if (double.TryParse(match.Groups["value"].Value, out double ruleValue))
            {
                Value = ruleValue;
            }
            else
            {
                return false;
            }

            Target = match.Groups["target"].Value;

            return true;
        }

        return false;
    }

    /// <summary>
    /// Generate a string rapresentation of the object.
    /// </summary>
    /// <returns>"<port_type>[<port_index>]<operator><value>,<target0:T-1>" (e.g. "d[0]=1,XXXXXXXXXXXXXXXX")</returns>
    public string Serialize()
    {
        if (Port == null || Operator == DeviceUnaryOperator.NOPE)
            return "";

        var portType = DevicePortType.EMPTY;
        int portIndex = 0;

        if (Port.Value >= DeviceTags.RELAY1 && Port.Value <= DeviceTags.RELAY32)
        {
            portType = DevicePortType.RELAY;
            portIndex = Port.Value - DeviceTags.RELAY1;
        }
        else if (Port.Value >= DeviceTags.LATCH1 && Port.Value <= DeviceTags.LATCH32)
        {
            portType = DevicePortType.LATCH;
            portIndex = Port.Value - DeviceTags.LATCH1;
        }
        else if (Port.Value >= DeviceTags.A_CURRENT_INPUT1 && Port.Value <= DeviceTags.A_CURRENT_INPUT32)
        {
            portType = DevicePortType.CURRENT;
            portIndex = Port.Value - DeviceTags.A_CURRENT_INPUT1;
        }
        else if (Port.Value >= DeviceTags.A_VOLTAGE_INPUT1 && Port.Value <= DeviceTags.A_VOLTAGE_INPUT32)
        {
            portType = DevicePortType.VOLTAGE;
            portIndex = Port.Value - DeviceTags.A_VOLTAGE_INPUT1;
        }
        else if (Port.Value >= DeviceTags.PULSE_INPUT1 && Port.Value <= DeviceTags.PULSE_INPUT32)
        {
            portType = DevicePortType.PULSE;
            portIndex = Port.Value - DeviceTags.PULSE_INPUT1;
        }
        else if (Port.Value >= DeviceTags.DIGITAL_INPUT1 && Port.Value <= DeviceTags.DIGITAL_INPUT32)
        {
            portType = DevicePortType.DIGITAL;
            portIndex = Port.Value - DeviceTags.DIGITAL_INPUT1;
        }
        else if (Port.Value >= DeviceTags.TEMPERATURE_INPUT1 && Port.Value <= DeviceTags.TEMPERATURE_INPUT32)
        {
            portType = DevicePortType.TEMPERATURE;
            portIndex = Port.Value - DeviceTags.TEMPERATURE_INPUT1;
        }

        var valueString = Value.ToString(CultureInfo.InvariantCulture);

        return $"{(char)portType}[{portIndex}]{(char)Operator}{valueString},{Target}";
    }
}

public class DeviceRule
{
    public IEnumerable<DeviceUnaryCondition>    Rules   { get; set; }
    public DeviceTags?                          Action  { get ;set; }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="input">"{<unary_rule(0)>;<unary_rule(1)>;...<unary_rule(N-1)>}<port_type><port_index>" (e.g. "{d[0]=1,XXXXXXXXXXXXXXX1;v[0]=1.22,XXXXXXXXXXXXXXX2}r0")</param>
    /// <returns></returns>
    public bool ParseFrom(string input)
    {

        if (!string.IsNullOrWhiteSpace(input) && input.Length >= 2)
        {
            if (input[0] != '{')
                return false;

            var rulesEndIndex = input.IndexOf('}');
            if (rulesEndIndex < 0)
                return false;

            var ruleList = new List<DeviceUnaryCondition>();
            foreach (var entry in input.Substring(1, rulesEndIndex - 1).Split(';'))
            {
                var rule = new DeviceUnaryCondition();
                if (!rule.ParseFrom(entry))
                {
                    return false;
                }
                ruleList.Add(rule);
            }        

            Rules = ruleList;

            if (input.Length > rulesEndIndex + 2 && int.TryParse(input.Substring(rulesEndIndex + 2), out int portIndex) && 
                    Utils.TryParseCharEnum<DevicePortType>(input[rulesEndIndex + 1], out DevicePortType portType))
            {
                Action = portType switch
                {
                    DevicePortType.RELAY => DeviceTags.RELAY1 + portIndex,
                    DevicePortType.LATCH => DeviceTags.LATCH1 + portIndex,
                    DevicePortType.CURRENT => DeviceTags.A_CURRENT_INPUT11 + portIndex,
                    DevicePortType.VOLTAGE => DeviceTags.A_VOLTAGE_INPUT1 + portIndex,
                    DevicePortType.PULSE => DeviceTags.PULSE_INPUT1 + portIndex,
                    DevicePortType.DIGITAL => DeviceTags.DIGITAL_INPUT11 + portIndex, 
                    DevicePortType.TEMPERATURE => DeviceTags.TEMPERATURE_INPUT11 + portIndex,
                    _ => null
                };
            }
        }



        return false;   
    }
}
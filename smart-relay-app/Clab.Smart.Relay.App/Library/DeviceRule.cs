using System;
using System.Collections.Generic;
using System.Linq;

namespace Clab.Smart.Relay.App;


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
    public DeviceTags               Port        { get; set; }
    public DeviceUnaryOperator      Operator    { get; set; }
    public double                   Value       { get; set; }
    public string                   Target      { get; set; }
}
using System;
using System.Collections.Generic;
using System.Linq;

namespace Clab.Smart.Relay.App;

public enum DeviceTags
{
    #region SERVICES
    DEVICE_MODEL,
    BATTERY_LEVEL,
    POWER_AC_STATUS,
    POWER_DC_STATUS,
    POWER_SOLAR_STATUS,
    CONNECTION_STATUS,
    SOFTWARE_REV,
    HARDWARE_REV,
    #endregion
    #region BUS
    #endregion
    #region INPUTS
    DIGITAL_INPUT1,
    DIGITAL_INPUT2,
    DIGITAL_INPUT3,
    DIGITAL_INPUT4,
    DIGITAL_INPUT5,
    DIGITAL_INPUT6,
    DIGITAL_INPUT7,
    DIGITAL_INPUT8,
    DIGITAL_INPUT9,
    DIGITAL_INPUT10,
    DIGITAL_INPUT11,
    DIGITAL_INPUT12,
    DIGITAL_INPUT13,
    DIGITAL_INPUT14,
    DIGITAL_INPUT15,
    DIGITAL_INPUT16,
    DIGITAL_INPUT17,
    DIGITAL_INPUT18,
    DIGITAL_INPUT19,
    DIGITAL_INPUT20,
    DIGITAL_INPUT21,
    DIGITAL_INPUT22,
    DIGITAL_INPUT23,
    DIGITAL_INPUT24,
    DIGITAL_INPUT25,
    DIGITAL_INPUT26,
    DIGITAL_INPUT27,
    DIGITAL_INPUT28,
    DIGITAL_INPUT29,
    DIGITAL_INPUT30,
    DIGITAL_INPUT31,
    DIGITAL_INPUT32,
    PULSE_INPUT1,
    PULSE_INPUT2,
    PULSE_INPUT3,
    PULSE_INPUT4,
    PULSE_INPUT5,
    PULSE_INPUT6,
    PULSE_INPUT7,
    PULSE_INPUT8,
    PULSE_INPUT9,
    PULSE_INPUT10,
    PULSE_INPUT11,
    PULSE_INPUT12,
    PULSE_INPUT13,
    PULSE_INPUT14,
    PULSE_INPUT15,
    PULSE_INPUT16,
    PULSE_INPUT17,
    PULSE_INPUT18,
    PULSE_INPUT19,
    PULSE_INPUT20,
    PULSE_INPUT21,
    PULSE_INPUT22,
    PULSE_INPUT23,
    PULSE_INPUT24,
    PULSE_INPUT25,
    PULSE_INPUT26,
    PULSE_INPUT27,
    PULSE_INPUT28,
    PULSE_INPUT29,
    PULSE_INPUT30,
    PULSE_INPUT31,
    PULSE_INPUT32,
    A_CURRENT_INPUT1,
    A_CURRENT_INPUT2,
    A_CURRENT_INPUT3,
    A_CURRENT_INPUT4,
    A_CURRENT_INPUT5,
    A_CURRENT_INPUT6,
    A_CURRENT_INPUT7,
    A_CURRENT_INPUT8,
    A_CURRENT_INPUT9,
    A_CURRENT_INPUT10,
    A_CURRENT_INPUT11,
    A_CURRENT_INPUT12,
    A_CURRENT_INPUT13,
    A_CURRENT_INPUT14,
    A_CURRENT_INPUT15,
    A_CURRENT_INPUT16,
    A_CURRENT_INPUT17,
    A_CURRENT_INPUT18,
    A_CURRENT_INPUT19,
    A_CURRENT_INPUT20,
    A_CURRENT_INPUT21,
    A_CURRENT_INPUT22,
    A_CURRENT_INPUT23,
    A_CURRENT_INPUT24,
    A_CURRENT_INPUT25,
    A_CURRENT_INPUT26,
    A_CURRENT_INPUT27,
    A_CURRENT_INPUT28,
    A_CURRENT_INPUT29,
    A_CURRENT_INPUT30,
    A_CURRENT_INPUT31,
    A_CURRENT_INPUT32,
    A_VOLTAGE_INPUT1,
    A_VOLTAGE_INPUT2,
    A_VOLTAGE_INPUT3,
    A_VOLTAGE_INPUT4,
    A_VOLTAGE_INPUT5,
    A_VOLTAGE_INPUT6,
    A_VOLTAGE_INPUT7,
    A_VOLTAGE_INPUT8,
    A_VOLTAGE_INPUT9,
    A_VOLTAGE_INPUT10,
    A_VOLTAGE_INPUT11,
    A_VOLTAGE_INPUT12,
    A_VOLTAGE_INPUT13,
    A_VOLTAGE_INPUT14,
    A_VOLTAGE_INPUT15,
    A_VOLTAGE_INPUT16,
    A_VOLTAGE_INPUT17,
    A_VOLTAGE_INPUT18,
    A_VOLTAGE_INPUT19,
    A_VOLTAGE_INPUT20,
    A_VOLTAGE_INPUT21,
    A_VOLTAGE_INPUT22,
    A_VOLTAGE_INPUT23,
    A_VOLTAGE_INPUT24,
    A_VOLTAGE_INPUT25,
    A_VOLTAGE_INPUT26,
    A_VOLTAGE_INPUT27,
    A_VOLTAGE_INPUT28,
    A_VOLTAGE_INPUT29,
    A_VOLTAGE_INPUT30,
    A_VOLTAGE_INPUT31,
    A_VOLTAGE_INPUT32,
    TEMPERATURE_INPUT1,
    TEMPERATURE_INPUT2,
    TEMPERATURE_INPUT3,
    TEMPERATURE_INPUT4,
    TEMPERATURE_INPUT5,
    TEMPERATURE_INPUT6,
    TEMPERATURE_INPUT7,
    TEMPERATURE_INPUT8,
    TEMPERATURE_INPUT9,
    TEMPERATURE_INPUT10,
    TEMPERATURE_INPUT11,
    TEMPERATURE_INPUT12,
    TEMPERATURE_INPUT13,
    TEMPERATURE_INPUT14,
    TEMPERATURE_INPUT15,
    TEMPERATURE_INPUT16,
    TEMPERATURE_INPUT17,
    TEMPERATURE_INPUT18,
    TEMPERATURE_INPUT19,
    TEMPERATURE_INPUT20,
    TEMPERATURE_INPUT21,
    TEMPERATURE_INPUT22,
    TEMPERATURE_INPUT23,
    TEMPERATURE_INPUT24,
    TEMPERATURE_INPUT25,
    TEMPERATURE_INPUT26,
    TEMPERATURE_INPUT27,
    TEMPERATURE_INPUT28,
    TEMPERATURE_INPUT29,
    TEMPERATURE_INPUT30,
    TEMPERATURE_INPUT31,
    TEMPERATURE_INPUT32,
    #endregion
    #region OUTPUTS
    RELAY1,
    RELAY2,
    RELAY3,
    RELAY4,
    RELAY5,
    RELAY6,
    RELAY7,
    RELAY8,
    RELAY9,
    RELAY10,
    RELAY11,
    RELAY12,
    RELAY13,
    RELAY14,
    RELAY15,
    RELAY16,
    RELAY17,
    RELAY18,
    RELAY19,
    RELAY20,
    RELAY21,
    RELAY22,
    RELAY23,
    RELAY24,
    RELAY25,
    RELAY26,
    RELAY27,
    RELAY28,
    RELAY29,
    RELAY30,
    RELAY31,
    RELAY32,
    LATCH1,
    LATCH2,
    LATCH3,
    LATCH4,
    LATCH5,
    LATCH6,
    LATCH7,
    LATCH8,
    LATCH9,
    LATCH10,
    LATCH11,
    LATCH12,
    LATCH13,
    LATCH14,
    LATCH15,
    LATCH16,
    LATCH17,
    LATCH18,
    LATCH19,
    LATCH20,
    LATCH21,
    LATCH22,
    LATCH23,
    LATCH24,
    LATCH25,
    LATCH26,
    LATCH27,
    LATCH28,
    LATCH29,
    LATCH30,
    LATCH31,
    LATCH32,
    #endregion
    #region CONTROLS
    PROG1,
    PROG2,
    PROG3,
    PROG4,
    PROG5,
    PROG6,
    PROG7,
    PROG8,
    PROG9,
    PROG10,
    PROG11,
    PROG12,
    PROG13,
    PROG14,
    PROG15,
    PROG16,
    PROG17,
    PROG18,
    PROG19,
    PROG20,
    PROG21,
    PROG22,
    PROG23,
    PROG24,
    PROG25,
    PROG26,
    PROG27,
    PROG28,
    PROG29,
    PROG30,
    PROG31,
    PROG32,
    RULE1,
    RULE2,
    RULE3,
    RULE4,
    RULE5,
    RULE6,
    RULE7,
    RULE8,
    RULE9,
    RULE10,
    RULE11,
    RULE12,
    RULE13,
    RULE14,
    RULE15,
    RULE16,
    RULE17,
    RULE18,
    RULE19,
    RULE20,
    RULE21,
    RULE22,
    RULE23,
    RULE24,
    RULE25,
    RULE26,
    RULE27,
    RULE28,
    RULE29,
    RULE30,
    RULE31,
    RULE32,
    #endregion
    #region PARAMS
    PULSE_FILTER1,
    PULSE_FILTER2,
    PULSE_FILTER3,
    PULSE_FILTER4,
    PULSE_FILTER5,
    PULSE_FILTER6,
    PULSE_FILTER7,
    PULSE_FILTER8,
    PULSE_FILTER9,
    PULSE_FILTER10,
    PULSE_FILTER11,
    PULSE_FILTER12,
    PULSE_FILTER13,
    PULSE_FILTER14,
    PULSE_FILTER15,
    PULSE_FILTER16,
    PULSE_FILTER17,
    PULSE_FILTER18,
    PULSE_FILTER19,
    PULSE_FILTER20,
    PULSE_FILTER21,
    PULSE_FILTER22,
    PULSE_FILTER23,
    PULSE_FILTER24,
    PULSE_FILTER25,
    PULSE_FILTER26,
    PULSE_FILTER27,
    PULSE_FILTER28,
    PULSE_FILTER29,
    PULSE_FILTER30,
    PULSE_FILTER31,
    PULSE_FILTER32,
    RELAY_DELAYS1,
    RELAY_DELAYS2,
    RELAY_DELAYS3,
    RELAY_DELAYS4,
    RELAY_DELAYS5,
    RELAY_DELAYS6,
    RELAY_DELAYS7,
    RELAY_DELAYS8,
    RELAY_DELAYS9,
    RELAY_DELAYS10,
    RELAY_DELAYS11,
    RELAY_DELAYS12,
    RELAY_DELAYS13,
    RELAY_DELAYS14,
    RELAY_DELAYS15,
    RELAY_DELAYS16,
    RELAY_DELAYS17,
    RELAY_DELAYS18,
    RELAY_DELAYS19,
    RELAY_DELAYS20,
    RELAY_DELAYS21,
    RELAY_DELAYS22,
    RELAY_DELAYS23,
    RELAY_DELAYS24,
    RELAY_DELAYS25,
    RELAY_DELAYS26,
    RELAY_DELAYS27,
    RELAY_DELAYS28,
    RELAY_DELAYS29,
    RELAY_DELAYS30,
    RELAY_DELAYS31,
    RELAY_DELAYS32,
    LATCH_DELAYS1,
    LATCH_DELAYS2,
    LATCH_DELAYS3,
    LATCH_DELAYS4,
    LATCH_DELAYS5,
    LATCH_DELAYS6,
    LATCH_DELAYS7,
    LATCH_DELAYS8,
    LATCH_DELAYS9,
    LATCH_DELAYS10,
    LATCH_DELAYS11,
    LATCH_DELAYS12,
    LATCH_DELAYS13,
    LATCH_DELAYS14,
    LATCH_DELAYS15,
    LATCH_DELAYS16,
    LATCH_DELAYS17,
    LATCH_DELAYS18,
    LATCH_DELAYS19,
    LATCH_DELAYS20,
    LATCH_DELAYS21,
    LATCH_DELAYS22,
    LATCH_DELAYS23,
    LATCH_DELAYS24,
    LATCH_DELAYS25,
    LATCH_DELAYS26,
    LATCH_DELAYS27,
    LATCH_DELAYS28,
    LATCH_DELAYS29,
    LATCH_DELAYS30,
    LATCH_DELAYS31,
    LATCH_DELAYS32,
    #endregion
    #region OVERRIDE
    RELAY_OVERRIDE1,
    RELAY_OVERRIDE2,
    RELAY_OVERRIDE3,
    RELAY_OVERRIDE4,
    RELAY_OVERRIDE5,
    RELAY_OVERRIDE6,
    RELAY_OVERRIDE7,
    RELAY_OVERRIDE8,
    RELAY_OVERRIDE9,
    RELAY_OVERRIDE10,
    RELAY_OVERRIDE11,
    RELAY_OVERRIDE12,
    RELAY_OVERRIDE13,
    RELAY_OVERRIDE14,
    RELAY_OVERRIDE15,
    RELAY_OVERRIDE16,
    RELAY_OVERRIDE17,
    RELAY_OVERRIDE18,
    RELAY_OVERRIDE19,
    RELAY_OVERRIDE20,
    RELAY_OVERRIDE21,
    RELAY_OVERRIDE22,
    RELAY_OVERRIDE23,
    RELAY_OVERRIDE24,
    RELAY_OVERRIDE25,
    RELAY_OVERRIDE26,
    RELAY_OVERRIDE27,
    RELAY_OVERRIDE28,
    RELAY_OVERRIDE29,
    RELAY_OVERRIDE30,
    RELAY_OVERRIDE31,
    RELAY_OVERRIDE32,
    LATCH_OVERRIDE1,
    LATCH_OVERRIDE2,
    LATCH_OVERRIDE3,
    LATCH_OVERRIDE4,
    LATCH_OVERRIDE5,
    LATCH_OVERRIDE6,
    LATCH_OVERRIDE7,
    LATCH_OVERRIDE8,
    LATCH_OVERRIDE9,
    LATCH_OVERRIDE10,
    LATCH_OVERRIDE11,
    LATCH_OVERRIDE12,
    LATCH_OVERRIDE13,
    LATCH_OVERRIDE14,
    LATCH_OVERRIDE15,
    LATCH_OVERRIDE16,
    LATCH_OVERRIDE17,
    LATCH_OVERRIDE18,
    LATCH_OVERRIDE19,
    LATCH_OVERRIDE20,
    LATCH_OVERRIDE21,
    LATCH_OVERRIDE22,
    LATCH_OVERRIDE23,
    LATCH_OVERRIDE24,
    LATCH_OVERRIDE25,
    LATCH_OVERRIDE26,
    LATCH_OVERRIDE27,
    LATCH_OVERRIDE28,
    LATCH_OVERRIDE29,
    LATCH_OVERRIDE30,
    LATCH_OVERRIDE31,
    LATCH_OVERRIDE32,
    #endregion
}

public static class DevicePropertyUtils
{
    /// <summary>
    /// Returns tag alias string.
    /// </summary>
    /// <param name="tag">to convert</param>
    /// <returns>alias string associated to the tag</returns>
    /// <exception cref="ArgumentOutOfRangeException"></exception>
    public static string ToAlias(this DeviceTags tag)
    {
        return tag switch
        {
            DeviceTags.DEVICE_MODEL => "model",
            DeviceTags.BATTERY_LEVEL => "ba",
            DeviceTags.POWER_AC_STATUS => "pac",
            DeviceTags.POWER_DC_STATUS => "pdc",
            DeviceTags.POWER_SOLAR_STATUS => "psr",
            DeviceTags.CONNECTION_STATUS => "conn",
            DeviceTags.SOFTWARE_REV => "srev",
            DeviceTags.HARDWARE_REV => "hrev",
            DeviceTags.DIGITAL_INPUT1 => "d0",
            DeviceTags.DIGITAL_INPUT2 => "d1",
            DeviceTags.DIGITAL_INPUT3 => "d2",
            DeviceTags.DIGITAL_INPUT4 => "d3",
            DeviceTags.DIGITAL_INPUT5 => "d4",
            DeviceTags.DIGITAL_INPUT6 => "d5",
            DeviceTags.DIGITAL_INPUT7 => "d6",
            DeviceTags.DIGITAL_INPUT8 => "d7",
            DeviceTags.DIGITAL_INPUT9 => "d8",
            DeviceTags.DIGITAL_INPUT10 => "d9",
            DeviceTags.DIGITAL_INPUT11 => "d10",
            DeviceTags.DIGITAL_INPUT12 => "d11",
            DeviceTags.DIGITAL_INPUT13 => "d12",
            DeviceTags.DIGITAL_INPUT14 => "d13",
            DeviceTags.DIGITAL_INPUT15 => "d14",
            DeviceTags.DIGITAL_INPUT16 => "d15",
            DeviceTags.DIGITAL_INPUT17 => "d16",
            DeviceTags.DIGITAL_INPUT18 => "d17",
            DeviceTags.DIGITAL_INPUT19 => "d18",
            DeviceTags.DIGITAL_INPUT20 => "d19",
            DeviceTags.DIGITAL_INPUT21 => "d20",
            DeviceTags.DIGITAL_INPUT22 => "d21",
            DeviceTags.DIGITAL_INPUT23 => "d22",
            DeviceTags.DIGITAL_INPUT24 => "d23",
            DeviceTags.DIGITAL_INPUT25 => "d24",
            DeviceTags.DIGITAL_INPUT26 => "d25",
            DeviceTags.DIGITAL_INPUT27 => "d26",
            DeviceTags.DIGITAL_INPUT28 => "d27",
            DeviceTags.DIGITAL_INPUT29 => "d28",
            DeviceTags.DIGITAL_INPUT30 => "d29",
            DeviceTags.DIGITAL_INPUT31 => "d30",
            DeviceTags.DIGITAL_INPUT32 => "d31",
            DeviceTags.PULSE_INPUT1 => "p0",
            DeviceTags.PULSE_INPUT2 => "p1",
            DeviceTags.PULSE_INPUT3 => "p2",
            DeviceTags.PULSE_INPUT4 => "p3",
            DeviceTags.PULSE_INPUT5 => "p4",
            DeviceTags.PULSE_INPUT6 => "p5",
            DeviceTags.PULSE_INPUT7 => "p6",
            DeviceTags.PULSE_INPUT8 => "p7",
            DeviceTags.PULSE_INPUT9 => "p8",
            DeviceTags.PULSE_INPUT10 => "p9",
            DeviceTags.PULSE_INPUT11 => "p10",
            DeviceTags.PULSE_INPUT12 => "p11",
            DeviceTags.PULSE_INPUT13 => "p12",
            DeviceTags.PULSE_INPUT14 => "p13",
            DeviceTags.PULSE_INPUT15 => "p14",
            DeviceTags.PULSE_INPUT16 => "p15",
            DeviceTags.PULSE_INPUT17 => "p16",
            DeviceTags.PULSE_INPUT18 => "p17",
            DeviceTags.PULSE_INPUT19 => "p18",
            DeviceTags.PULSE_INPUT20 => "p19",
            DeviceTags.PULSE_INPUT21 => "p20",
            DeviceTags.PULSE_INPUT22 => "p21",
            DeviceTags.PULSE_INPUT23 => "p22",
            DeviceTags.PULSE_INPUT24 => "p23",
            DeviceTags.PULSE_INPUT25 => "p24",
            DeviceTags.PULSE_INPUT26 => "p25",
            DeviceTags.PULSE_INPUT27 => "p26",
            DeviceTags.PULSE_INPUT28 => "p27",
            DeviceTags.PULSE_INPUT29 => "p28",
            DeviceTags.PULSE_INPUT30 => "p29",
            DeviceTags.PULSE_INPUT31 => "p30",
            DeviceTags.PULSE_INPUT32 => "p31",
            DeviceTags.A_CURRENT_INPUT1 => "c0",
            DeviceTags.A_CURRENT_INPUT2 => "c1",
            DeviceTags.A_CURRENT_INPUT3 => "c2",
            DeviceTags.A_CURRENT_INPUT4 => "c3",
            DeviceTags.A_CURRENT_INPUT5 => "c4",
            DeviceTags.A_CURRENT_INPUT6 => "c5",
            DeviceTags.A_CURRENT_INPUT7 => "c6",
            DeviceTags.A_CURRENT_INPUT8 => "c7",
            DeviceTags.A_CURRENT_INPUT9 => "c8",
            DeviceTags.A_CURRENT_INPUT10 => "c9",
            DeviceTags.A_CURRENT_INPUT11 => "c10",
            DeviceTags.A_CURRENT_INPUT12 => "c11",
            DeviceTags.A_CURRENT_INPUT13 => "c12",
            DeviceTags.A_CURRENT_INPUT14 => "c13",
            DeviceTags.A_CURRENT_INPUT15 => "c14",
            DeviceTags.A_CURRENT_INPUT16 => "c15",
            DeviceTags.A_CURRENT_INPUT17 => "c16",
            DeviceTags.A_CURRENT_INPUT18 => "c17",
            DeviceTags.A_CURRENT_INPUT19 => "c18",
            DeviceTags.A_CURRENT_INPUT20 => "c19",
            DeviceTags.A_CURRENT_INPUT21 => "c20",
            DeviceTags.A_CURRENT_INPUT22 => "c21",
            DeviceTags.A_CURRENT_INPUT23 => "c22",
            DeviceTags.A_CURRENT_INPUT24 => "c23",
            DeviceTags.A_CURRENT_INPUT25 => "c24",
            DeviceTags.A_CURRENT_INPUT26 => "c25",
            DeviceTags.A_CURRENT_INPUT27 => "c26",
            DeviceTags.A_CURRENT_INPUT28 => "c27",
            DeviceTags.A_CURRENT_INPUT29 => "c28",
            DeviceTags.A_CURRENT_INPUT30 => "c29",
            DeviceTags.A_CURRENT_INPUT31 => "c30",
            DeviceTags.A_CURRENT_INPUT32 => "c31",
            DeviceTags.A_VOLTAGE_INPUT1 => "v0",
            DeviceTags.A_VOLTAGE_INPUT2 => "v1",
            DeviceTags.A_VOLTAGE_INPUT3 => "v2",
            DeviceTags.A_VOLTAGE_INPUT4 => "v3",
            DeviceTags.A_VOLTAGE_INPUT5 => "v4",
            DeviceTags.A_VOLTAGE_INPUT6 => "v5",
            DeviceTags.A_VOLTAGE_INPUT7 => "v6",
            DeviceTags.A_VOLTAGE_INPUT8 => "v7",
            DeviceTags.A_VOLTAGE_INPUT9 => "v8",
            DeviceTags.A_VOLTAGE_INPUT10 => "v9",
            DeviceTags.A_VOLTAGE_INPUT11 => "v10",
            DeviceTags.A_VOLTAGE_INPUT12 => "v11",
            DeviceTags.A_VOLTAGE_INPUT13 => "v12",
            DeviceTags.A_VOLTAGE_INPUT14 => "v13",
            DeviceTags.A_VOLTAGE_INPUT15 => "v14",
            DeviceTags.A_VOLTAGE_INPUT16 => "v15",
            DeviceTags.A_VOLTAGE_INPUT17 => "v16",
            DeviceTags.A_VOLTAGE_INPUT18 => "v17",
            DeviceTags.A_VOLTAGE_INPUT19 => "v18",
            DeviceTags.A_VOLTAGE_INPUT20 => "v19",
            DeviceTags.A_VOLTAGE_INPUT21 => "v20",
            DeviceTags.A_VOLTAGE_INPUT22 => "v21",
            DeviceTags.A_VOLTAGE_INPUT23 => "v22",
            DeviceTags.A_VOLTAGE_INPUT24 => "v23",
            DeviceTags.A_VOLTAGE_INPUT25 => "v24",
            DeviceTags.A_VOLTAGE_INPUT26 => "v25",
            DeviceTags.A_VOLTAGE_INPUT27 => "v26",
            DeviceTags.A_VOLTAGE_INPUT28 => "v27",
            DeviceTags.A_VOLTAGE_INPUT29 => "v28",
            DeviceTags.A_VOLTAGE_INPUT30 => "v29",
            DeviceTags.A_VOLTAGE_INPUT31 => "v30",
            DeviceTags.A_VOLTAGE_INPUT32 => "v31",
            DeviceTags.TEMPERATURE_INPUT1 => "t0",
            DeviceTags.TEMPERATURE_INPUT2 => "t1",
            DeviceTags.TEMPERATURE_INPUT3 => "t2",
            DeviceTags.TEMPERATURE_INPUT4 => "t3",
            DeviceTags.TEMPERATURE_INPUT5 => "t4",
            DeviceTags.TEMPERATURE_INPUT6 => "t5",
            DeviceTags.TEMPERATURE_INPUT7 => "t6",
            DeviceTags.TEMPERATURE_INPUT8 => "t7",
            DeviceTags.TEMPERATURE_INPUT9 => "t8",
            DeviceTags.TEMPERATURE_INPUT10 => "t9",
            DeviceTags.TEMPERATURE_INPUT11 => "t10",
            DeviceTags.TEMPERATURE_INPUT12 => "t11",
            DeviceTags.TEMPERATURE_INPUT13 => "t12",
            DeviceTags.TEMPERATURE_INPUT14 => "t13",
            DeviceTags.TEMPERATURE_INPUT15 => "t14",
            DeviceTags.TEMPERATURE_INPUT16 => "t15",
            DeviceTags.TEMPERATURE_INPUT17 => "t16",
            DeviceTags.TEMPERATURE_INPUT18 => "t17",
            DeviceTags.TEMPERATURE_INPUT19 => "t18",
            DeviceTags.TEMPERATURE_INPUT20 => "t19",
            DeviceTags.TEMPERATURE_INPUT21 => "t20",
            DeviceTags.TEMPERATURE_INPUT22 => "t21",
            DeviceTags.TEMPERATURE_INPUT23 => "t22",
            DeviceTags.TEMPERATURE_INPUT24 => "t23",
            DeviceTags.TEMPERATURE_INPUT25 => "t24",
            DeviceTags.TEMPERATURE_INPUT26 => "t25",
            DeviceTags.TEMPERATURE_INPUT27 => "t26",
            DeviceTags.TEMPERATURE_INPUT28 => "t27",
            DeviceTags.TEMPERATURE_INPUT29 => "t28",
            DeviceTags.TEMPERATURE_INPUT30 => "t29",
            DeviceTags.TEMPERATURE_INPUT31 => "t30",
            DeviceTags.TEMPERATURE_INPUT32 => "t31",
            DeviceTags.RELAY1 => "r0",
            DeviceTags.RELAY2 => "r1",
            DeviceTags.RELAY3 => "r2",
            DeviceTags.RELAY4 => "r3",
            DeviceTags.RELAY5 => "r4",
            DeviceTags.RELAY6 => "r5",
            DeviceTags.RELAY7 => "r6",
            DeviceTags.RELAY8 => "r7",
            DeviceTags.RELAY9 => "r8",
            DeviceTags.RELAY10 => "r9",
            DeviceTags.RELAY11 => "r10",
            DeviceTags.RELAY12 => "r11",
            DeviceTags.RELAY13 => "r12",
            DeviceTags.RELAY14 => "r13",
            DeviceTags.RELAY15 => "r14",
            DeviceTags.RELAY16 => "r15",
            DeviceTags.RELAY17 => "r16",
            DeviceTags.RELAY18 => "r17",
            DeviceTags.RELAY19 => "r18",
            DeviceTags.RELAY20 => "r19",
            DeviceTags.RELAY21 => "r20",
            DeviceTags.RELAY22 => "r21",
            DeviceTags.RELAY23 => "r22",
            DeviceTags.RELAY24 => "r23",
            DeviceTags.RELAY25 => "r24",
            DeviceTags.RELAY26 => "r25",
            DeviceTags.RELAY27 => "r26",
            DeviceTags.RELAY28 => "r27",
            DeviceTags.RELAY29 => "r28",
            DeviceTags.RELAY30 => "r29",
            DeviceTags.RELAY31 => "r30",
            DeviceTags.RELAY32 => "r31",
            DeviceTags.LATCH1 => "l0",
            DeviceTags.LATCH2 => "l1",
            DeviceTags.LATCH3 => "l2",
            DeviceTags.LATCH4 => "l3",
            DeviceTags.LATCH5 => "l4",
            DeviceTags.LATCH6 => "l5",
            DeviceTags.LATCH7 => "l6",
            DeviceTags.LATCH8 => "l7",
            DeviceTags.LATCH9 => "l8",
            DeviceTags.LATCH10 => "l9",
            DeviceTags.LATCH11 => "l10",
            DeviceTags.LATCH12 => "l11",
            DeviceTags.LATCH13 => "l12",
            DeviceTags.LATCH14 => "l13",
            DeviceTags.LATCH15 => "l14",
            DeviceTags.LATCH16 => "l15",
            DeviceTags.LATCH17 => "l16",
            DeviceTags.LATCH18 => "l17",
            DeviceTags.LATCH19 => "l18",
            DeviceTags.LATCH20 => "l19",
            DeviceTags.LATCH21 => "l20",
            DeviceTags.LATCH22 => "l21",
            DeviceTags.LATCH23 => "l22",
            DeviceTags.LATCH24 => "l23",
            DeviceTags.LATCH25 => "l24",
            DeviceTags.LATCH26 => "l25",
            DeviceTags.LATCH27 => "l26",
            DeviceTags.LATCH28 => "l27",
            DeviceTags.LATCH29 => "l28",
            DeviceTags.LATCH30 => "l29",
            DeviceTags.LATCH31 => "l30",
            DeviceTags.LATCH32 => "l31",
            DeviceTags.PROG1 => "prog0",
            DeviceTags.PROG2 => "prog1",
            DeviceTags.PROG3 => "prog2",
            DeviceTags.PROG4 => "prog3",
            DeviceTags.PROG5 => "prog4",
            DeviceTags.PROG6 => "prog5",
            DeviceTags.PROG7 => "prog6",
            DeviceTags.PROG8 => "prog7",
            DeviceTags.PROG9 => "prog8",
            DeviceTags.PROG10 => "prog9",
            DeviceTags.PROG11 => "prog10",
            DeviceTags.PROG12 => "prog11",
            DeviceTags.PROG13 => "prog12",
            DeviceTags.PROG14 => "prog13",
            DeviceTags.PROG15 => "prog14",
            DeviceTags.PROG16 => "prog15",
            DeviceTags.PROG17 => "prog16",
            DeviceTags.PROG18 => "prog17",
            DeviceTags.PROG19 => "prog18",
            DeviceTags.PROG20 => "prog19",
            DeviceTags.PROG21 => "prog20",
            DeviceTags.PROG22 => "prog21",
            DeviceTags.PROG23 => "prog22",
            DeviceTags.PROG24 => "prog23",
            DeviceTags.PROG25 => "prog24",
            DeviceTags.PROG26 => "prog25",
            DeviceTags.PROG27 => "prog26",
            DeviceTags.PROG28 => "prog27",
            DeviceTags.PROG29 => "prog28",
            DeviceTags.PROG30 => "prog29",
            DeviceTags.PROG31 => "prog30",
            DeviceTags.PROG32 => "prog31",
            DeviceTags.RULE1 => "rule0",
            DeviceTags.RULE2 => "rule1",
            DeviceTags.RULE3 => "rule2",
            DeviceTags.RULE4 => "rule3",
            DeviceTags.RULE5 => "rule4",
            DeviceTags.RULE6 => "rule5",
            DeviceTags.RULE7 => "rule6",
            DeviceTags.RULE8 => "rule7",
            DeviceTags.RULE9 => "rule8",
            DeviceTags.RULE10 => "rule9",
            DeviceTags.RULE11 => "rule10",
            DeviceTags.RULE12 => "rule11",
            DeviceTags.RULE13 => "rule12",
            DeviceTags.RULE14 => "rule13",
            DeviceTags.RULE15 => "rule14",
            DeviceTags.RULE16 => "rule15",
            DeviceTags.RULE17 => "rule16",
            DeviceTags.RULE18 => "rule17",
            DeviceTags.RULE19 => "rule18",
            DeviceTags.RULE20 => "rule19",
            DeviceTags.RULE21 => "rule20",
            DeviceTags.RULE22 => "rule21",
            DeviceTags.RULE23 => "rule22",
            DeviceTags.RULE24 => "rule23",
            DeviceTags.RULE25 => "rule24",
            DeviceTags.RULE26 => "rule25",
            DeviceTags.RULE27 => "rule26",
            DeviceTags.RULE28 => "rule27",
            DeviceTags.RULE29 => "rule28",
            DeviceTags.RULE30 => "rule29",
            DeviceTags.RULE31 => "rule30",
            DeviceTags.RULE32 => "rule31",
            DeviceTags.PULSE_FILTER1 => "pf0",
            DeviceTags.PULSE_FILTER2 => "pf1",
            DeviceTags.PULSE_FILTER3 => "pf2",
            DeviceTags.PULSE_FILTER4 => "pf3",
            DeviceTags.PULSE_FILTER5 => "pf4",
            DeviceTags.PULSE_FILTER6 => "pf5",
            DeviceTags.PULSE_FILTER7 => "pf6",
            DeviceTags.PULSE_FILTER8 => "pf7",
            DeviceTags.PULSE_FILTER9 => "pf8",
            DeviceTags.PULSE_FILTER10 => "pf9",
            DeviceTags.PULSE_FILTER11 => "pf10",
            DeviceTags.PULSE_FILTER12 => "pf11",
            DeviceTags.PULSE_FILTER13 => "pf12",
            DeviceTags.PULSE_FILTER14 => "pf13",
            DeviceTags.PULSE_FILTER15 => "pf14",
            DeviceTags.PULSE_FILTER16 => "pf15",
            DeviceTags.PULSE_FILTER17 => "pf16",
            DeviceTags.PULSE_FILTER18 => "pf17",
            DeviceTags.PULSE_FILTER19 => "pf18",
            DeviceTags.PULSE_FILTER20 => "pf19",
            DeviceTags.PULSE_FILTER21 => "pf20",
            DeviceTags.PULSE_FILTER22 => "pf21",
            DeviceTags.PULSE_FILTER23 => "pf22",
            DeviceTags.PULSE_FILTER24 => "pf23",
            DeviceTags.PULSE_FILTER25 => "pf24",
            DeviceTags.PULSE_FILTER26 => "pf25",
            DeviceTags.PULSE_FILTER27 => "pf26",
            DeviceTags.PULSE_FILTER28 => "pf27",
            DeviceTags.PULSE_FILTER29 => "pf28",
            DeviceTags.PULSE_FILTER30 => "pf29",
            DeviceTags.PULSE_FILTER31 => "pf30",
            DeviceTags.PULSE_FILTER32 => "pf31",
            DeviceTags.RELAY_DELAYS1 => "rd0",
            DeviceTags.RELAY_DELAYS2 => "rd1",
            DeviceTags.RELAY_DELAYS3 => "rd2",
            DeviceTags.RELAY_DELAYS4 => "rd3",
            DeviceTags.RELAY_DELAYS5 => "rd4",
            DeviceTags.RELAY_DELAYS6 => "rd5",
            DeviceTags.RELAY_DELAYS7 => "rd6",
            DeviceTags.RELAY_DELAYS8 => "rd7",
            DeviceTags.RELAY_DELAYS9 => "rd8",
            DeviceTags.RELAY_DELAYS10 => "rd9",
            DeviceTags.RELAY_DELAYS11 => "rd10",
            DeviceTags.RELAY_DELAYS12 => "rd11",
            DeviceTags.RELAY_DELAYS13 => "rd12",
            DeviceTags.RELAY_DELAYS14 => "rd13",
            DeviceTags.RELAY_DELAYS15 => "rd14",
            DeviceTags.RELAY_DELAYS16 => "rd15",
            DeviceTags.RELAY_DELAYS17 => "rd16",
            DeviceTags.RELAY_DELAYS18 => "rd17",
            DeviceTags.RELAY_DELAYS19 => "rd18",
            DeviceTags.RELAY_DELAYS20 => "rd19",
            DeviceTags.RELAY_DELAYS21 => "rd20",
            DeviceTags.RELAY_DELAYS22 => "rd21",
            DeviceTags.RELAY_DELAYS23 => "rd22",
            DeviceTags.RELAY_DELAYS24 => "rd23",
            DeviceTags.RELAY_DELAYS25 => "rd24",
            DeviceTags.RELAY_DELAYS26 => "rd25",
            DeviceTags.RELAY_DELAYS27 => "rd26",
            DeviceTags.RELAY_DELAYS28 => "rd27",
            DeviceTags.RELAY_DELAYS29 => "rd28",
            DeviceTags.RELAY_DELAYS30 => "rd29",
            DeviceTags.RELAY_DELAYS31 => "rd30",
            DeviceTags.RELAY_DELAYS32 => "rd31",
            DeviceTags.LATCH_DELAYS1 => "ld0",
            DeviceTags.LATCH_DELAYS2 => "ld1",
            DeviceTags.LATCH_DELAYS3 => "ld2",
            DeviceTags.LATCH_DELAYS4 => "ld3",
            DeviceTags.LATCH_DELAYS5 => "ld4",
            DeviceTags.LATCH_DELAYS6 => "ld5",
            DeviceTags.LATCH_DELAYS7 => "ld6",
            DeviceTags.LATCH_DELAYS8 => "ld7",
            DeviceTags.LATCH_DELAYS9 => "ld8",
            DeviceTags.LATCH_DELAYS10 => "ld9",
            DeviceTags.LATCH_DELAYS11 => "ld10",
            DeviceTags.LATCH_DELAYS12 => "ld11",
            DeviceTags.LATCH_DELAYS13 => "ld12",
            DeviceTags.LATCH_DELAYS14 => "ld13",
            DeviceTags.LATCH_DELAYS15 => "ld14",
            DeviceTags.LATCH_DELAYS16 => "ld15",
            DeviceTags.LATCH_DELAYS17 => "ld16",
            DeviceTags.LATCH_DELAYS18 => "ld17",
            DeviceTags.LATCH_DELAYS19 => "ld18",
            DeviceTags.LATCH_DELAYS20 => "ld19",
            DeviceTags.LATCH_DELAYS21 => "ld20",
            DeviceTags.LATCH_DELAYS22 => "ld21",
            DeviceTags.LATCH_DELAYS23 => "ld22",
            DeviceTags.LATCH_DELAYS24 => "ld23",
            DeviceTags.LATCH_DELAYS25 => "ld24",
            DeviceTags.LATCH_DELAYS26 => "ld25",
            DeviceTags.LATCH_DELAYS27 => "ld26",
            DeviceTags.LATCH_DELAYS28 => "ld27",
            DeviceTags.LATCH_DELAYS29 => "ld28",
            DeviceTags.LATCH_DELAYS30 => "ld29",
            DeviceTags.LATCH_DELAYS31 => "ld30",
            DeviceTags.LATCH_DELAYS32 => "ld31",
            DeviceTags.RELAY_OVERRIDE1 => "ro0",
            DeviceTags.RELAY_OVERRIDE2 => "ro1",
            DeviceTags.RELAY_OVERRIDE3 => "ro2",
            DeviceTags.RELAY_OVERRIDE4 => "ro3",
            DeviceTags.RELAY_OVERRIDE5 => "ro4",
            DeviceTags.RELAY_OVERRIDE6 => "ro5",
            DeviceTags.RELAY_OVERRIDE7 => "ro6",
            DeviceTags.RELAY_OVERRIDE8 => "ro7",
            DeviceTags.RELAY_OVERRIDE9 => "ro8",
            DeviceTags.RELAY_OVERRIDE10 => "ro9",
            DeviceTags.RELAY_OVERRIDE11 => "ro10",
            DeviceTags.RELAY_OVERRIDE12 => "ro11",
            DeviceTags.RELAY_OVERRIDE13 => "ro12",
            DeviceTags.RELAY_OVERRIDE14 => "ro13",
            DeviceTags.RELAY_OVERRIDE15 => "ro14",
            DeviceTags.RELAY_OVERRIDE16 => "ro15",
            DeviceTags.RELAY_OVERRIDE17 => "ro16",
            DeviceTags.RELAY_OVERRIDE18 => "ro17",
            DeviceTags.RELAY_OVERRIDE19 => "ro18",
            DeviceTags.RELAY_OVERRIDE20 => "ro19",
            DeviceTags.RELAY_OVERRIDE21 => "ro20",
            DeviceTags.RELAY_OVERRIDE22 => "ro21",
            DeviceTags.RELAY_OVERRIDE23 => "ro22",
            DeviceTags.RELAY_OVERRIDE24 => "ro23",
            DeviceTags.RELAY_OVERRIDE25 => "ro24",
            DeviceTags.RELAY_OVERRIDE26 => "ro25",
            DeviceTags.RELAY_OVERRIDE27 => "ro26",
            DeviceTags.RELAY_OVERRIDE28 => "ro27",
            DeviceTags.RELAY_OVERRIDE29 => "ro28",
            DeviceTags.RELAY_OVERRIDE30 => "ro29",
            DeviceTags.RELAY_OVERRIDE31 => "ro30",
            DeviceTags.RELAY_OVERRIDE32 => "ro31",
            DeviceTags.LATCH_OVERRIDE1 => "lo0",
            DeviceTags.LATCH_OVERRIDE2 => "lo1",
            DeviceTags.LATCH_OVERRIDE3 => "lo2",
            DeviceTags.LATCH_OVERRIDE4 => "lo3",
            DeviceTags.LATCH_OVERRIDE5 => "lo4",
            DeviceTags.LATCH_OVERRIDE6 => "lo5",
            DeviceTags.LATCH_OVERRIDE7 => "lo6",
            DeviceTags.LATCH_OVERRIDE8 => "lo7",
            DeviceTags.LATCH_OVERRIDE9 => "lo8",
            DeviceTags.LATCH_OVERRIDE10 => "lo9",
            DeviceTags.LATCH_OVERRIDE11 => "lo10",
            DeviceTags.LATCH_OVERRIDE12 => "lo11",
            DeviceTags.LATCH_OVERRIDE13 => "lo12",
            DeviceTags.LATCH_OVERRIDE14 => "lo13",
            DeviceTags.LATCH_OVERRIDE15 => "lo14",
            DeviceTags.LATCH_OVERRIDE16 => "lo15",
            DeviceTags.LATCH_OVERRIDE17 => "lo16",
            DeviceTags.LATCH_OVERRIDE18 => "lo17",
            DeviceTags.LATCH_OVERRIDE19 => "lo18",
            DeviceTags.LATCH_OVERRIDE20 => "lo19",
            DeviceTags.LATCH_OVERRIDE21 => "lo20",
            DeviceTags.LATCH_OVERRIDE22 => "lo21",
            DeviceTags.LATCH_OVERRIDE23 => "lo22",
            DeviceTags.LATCH_OVERRIDE24 => "lo23",
            DeviceTags.LATCH_OVERRIDE25 => "lo24",
            DeviceTags.LATCH_OVERRIDE26 => "lo25",
            DeviceTags.LATCH_OVERRIDE27 => "lo26",
            DeviceTags.LATCH_OVERRIDE28 => "lo27",
            DeviceTags.LATCH_OVERRIDE29 => "lo28",
            DeviceTags.LATCH_OVERRIDE30 => "lo29",
            DeviceTags.LATCH_OVERRIDE31 => "lo30",
            DeviceTags.LATCH_OVERRIDE32 => "lo31",
            _ => throw new ArgumentOutOfRangeException(tag.ToString())
        };
    }

    public static bool IsSettableProperty(this DeviceTags tag)
    {
        return tag switch
        {
            DeviceTags.PULSE_FILTER1 or
            DeviceTags.PULSE_FILTER2 or
            DeviceTags.PULSE_FILTER3 or
            DeviceTags.PULSE_FILTER4 or
            DeviceTags.PULSE_FILTER5 or
            DeviceTags.PULSE_FILTER6 or
            DeviceTags.PULSE_FILTER7 or
            DeviceTags.PULSE_FILTER8 or
            DeviceTags.PULSE_FILTER9 or
            DeviceTags.PULSE_FILTER10 or
            DeviceTags.PULSE_FILTER11 or
            DeviceTags.PULSE_FILTER12 or
            DeviceTags.PULSE_FILTER13 or
            DeviceTags.PULSE_FILTER14 or
            DeviceTags.PULSE_FILTER15 or
            DeviceTags.PULSE_FILTER16 or
            DeviceTags.PULSE_FILTER17 or
            DeviceTags.PULSE_FILTER18 or
            DeviceTags.PULSE_FILTER19 or
            DeviceTags.PULSE_FILTER20 or
            DeviceTags.PULSE_FILTER21 or
            DeviceTags.PULSE_FILTER22 or
            DeviceTags.PULSE_FILTER23 or
            DeviceTags.PULSE_FILTER24 or
            DeviceTags.PULSE_FILTER25 or
            DeviceTags.PULSE_FILTER26 or
            DeviceTags.PULSE_FILTER27 or
            DeviceTags.PULSE_FILTER28 or
            DeviceTags.PULSE_FILTER29 or
            DeviceTags.PULSE_FILTER30 or
            DeviceTags.PULSE_FILTER31 or
            DeviceTags.PULSE_FILTER32 or
            DeviceTags.RELAY_DELAYS1 or
            DeviceTags.RELAY_DELAYS2 or
            DeviceTags.RELAY_DELAYS3 or
            DeviceTags.RELAY_DELAYS4 or
            DeviceTags.RELAY_DELAYS5 or
            DeviceTags.RELAY_DELAYS6 or
            DeviceTags.RELAY_DELAYS7 or
            DeviceTags.RELAY_DELAYS8 or
            DeviceTags.RELAY_DELAYS9 or
            DeviceTags.RELAY_DELAYS10 or
            DeviceTags.RELAY_DELAYS11 or
            DeviceTags.RELAY_DELAYS12 or
            DeviceTags.RELAY_DELAYS13 or
            DeviceTags.RELAY_DELAYS14 or
            DeviceTags.RELAY_DELAYS15 or
            DeviceTags.RELAY_DELAYS16 or
            DeviceTags.RELAY_DELAYS17 or
            DeviceTags.RELAY_DELAYS18 or
            DeviceTags.RELAY_DELAYS19 or
            DeviceTags.RELAY_DELAYS20 or
            DeviceTags.RELAY_DELAYS21 or
            DeviceTags.RELAY_DELAYS22 or
            DeviceTags.RELAY_DELAYS23 or
            DeviceTags.RELAY_DELAYS24 or
            DeviceTags.RELAY_DELAYS25 or
            DeviceTags.RELAY_DELAYS26 or
            DeviceTags.RELAY_DELAYS27 or
            DeviceTags.RELAY_DELAYS28 or
            DeviceTags.RELAY_DELAYS29 or
            DeviceTags.RELAY_DELAYS30 or
            DeviceTags.RELAY_DELAYS31 or
            DeviceTags.RELAY_DELAYS32 or
            DeviceTags.LATCH_DELAYS1 or
            DeviceTags.LATCH_DELAYS2 or
            DeviceTags.LATCH_DELAYS3 or
            DeviceTags.LATCH_DELAYS4 or
            DeviceTags.LATCH_DELAYS5 or
            DeviceTags.LATCH_DELAYS6 or
            DeviceTags.LATCH_DELAYS7 or
            DeviceTags.LATCH_DELAYS8 or
            DeviceTags.LATCH_DELAYS9 or
            DeviceTags.LATCH_DELAYS10 or
            DeviceTags.LATCH_DELAYS11 or
            DeviceTags.LATCH_DELAYS12 or
            DeviceTags.LATCH_DELAYS13 or
            DeviceTags.LATCH_DELAYS14 or
            DeviceTags.LATCH_DELAYS15 or
            DeviceTags.LATCH_DELAYS16 or
            DeviceTags.LATCH_DELAYS17 or
            DeviceTags.LATCH_DELAYS18 or
            DeviceTags.LATCH_DELAYS19 or
            DeviceTags.LATCH_DELAYS20 or
            DeviceTags.LATCH_DELAYS21 or
            DeviceTags.LATCH_DELAYS22 or
            DeviceTags.LATCH_DELAYS23 or
            DeviceTags.LATCH_DELAYS24 or
            DeviceTags.LATCH_DELAYS25 or
            DeviceTags.LATCH_DELAYS26 or
            DeviceTags.LATCH_DELAYS27 or
            DeviceTags.LATCH_DELAYS28 or
            DeviceTags.LATCH_DELAYS29 or
            DeviceTags.LATCH_DELAYS30 or
            DeviceTags.LATCH_DELAYS31 or
            DeviceTags.LATCH_DELAYS32 or
            DeviceTags.RELAY_OVERRIDE1 or
            DeviceTags.RELAY_OVERRIDE2 or
            DeviceTags.RELAY_OVERRIDE3 or
            DeviceTags.RELAY_OVERRIDE4 or
            DeviceTags.RELAY_OVERRIDE5 or
            DeviceTags.RELAY_OVERRIDE6 or
            DeviceTags.RELAY_OVERRIDE7 or
            DeviceTags.RELAY_OVERRIDE8 or
            DeviceTags.RELAY_OVERRIDE9 or
            DeviceTags.RELAY_OVERRIDE10 or
            DeviceTags.RELAY_OVERRIDE11 or
            DeviceTags.RELAY_OVERRIDE12 or
            DeviceTags.RELAY_OVERRIDE13 or
            DeviceTags.RELAY_OVERRIDE14 or
            DeviceTags.RELAY_OVERRIDE15 or
            DeviceTags.RELAY_OVERRIDE16 or
            DeviceTags.RELAY_OVERRIDE17 or
            DeviceTags.RELAY_OVERRIDE18 or
            DeviceTags.RELAY_OVERRIDE19 or
            DeviceTags.RELAY_OVERRIDE20 or
            DeviceTags.RELAY_OVERRIDE21 or
            DeviceTags.RELAY_OVERRIDE22 or
            DeviceTags.RELAY_OVERRIDE23 or
            DeviceTags.RELAY_OVERRIDE24 or
            DeviceTags.RELAY_OVERRIDE25 or
            DeviceTags.RELAY_OVERRIDE26 or
            DeviceTags.RELAY_OVERRIDE27 or
            DeviceTags.RELAY_OVERRIDE28 or
            DeviceTags.RELAY_OVERRIDE29 or
            DeviceTags.RELAY_OVERRIDE30 or
            DeviceTags.RELAY_OVERRIDE31 or
            DeviceTags.RELAY_OVERRIDE32 or
            DeviceTags.LATCH_OVERRIDE1 or
            DeviceTags.LATCH_OVERRIDE2 or
            DeviceTags.LATCH_OVERRIDE3 or
            DeviceTags.LATCH_OVERRIDE4 or
            DeviceTags.LATCH_OVERRIDE5 or
            DeviceTags.LATCH_OVERRIDE6 or
            DeviceTags.LATCH_OVERRIDE7 or
            DeviceTags.LATCH_OVERRIDE8 or
            DeviceTags.LATCH_OVERRIDE9 or
            DeviceTags.LATCH_OVERRIDE10 or
            DeviceTags.LATCH_OVERRIDE11 or
            DeviceTags.LATCH_OVERRIDE12 or
            DeviceTags.LATCH_OVERRIDE13 or
            DeviceTags.LATCH_OVERRIDE14 or
            DeviceTags.LATCH_OVERRIDE15 or
            DeviceTags.LATCH_OVERRIDE16 or
            DeviceTags.LATCH_OVERRIDE17 or
            DeviceTags.LATCH_OVERRIDE18 or
            DeviceTags.LATCH_OVERRIDE19 or
            DeviceTags.LATCH_OVERRIDE20 or
            DeviceTags.LATCH_OVERRIDE21 or
            DeviceTags.LATCH_OVERRIDE22 or
            DeviceTags.LATCH_OVERRIDE23 or
            DeviceTags.LATCH_OVERRIDE24 or
            DeviceTags.LATCH_OVERRIDE25 or
            DeviceTags.LATCH_OVERRIDE26 or
            DeviceTags.LATCH_OVERRIDE27 or
            DeviceTags.LATCH_OVERRIDE28 or
            DeviceTags.LATCH_OVERRIDE29 or
            DeviceTags.LATCH_OVERRIDE30 or
            DeviceTags.LATCH_OVERRIDE31 or
            DeviceTags.LATCH_OVERRIDE32 => true,
            _ => false
        };
    }

    public static bool IsOutputProperty(this DeviceTags tag)
    {
        return tag switch
        {
            DeviceTags.RELAY1 or
            DeviceTags.RELAY2 or
            DeviceTags.RELAY3 or
            DeviceTags.RELAY4 or
            DeviceTags.RELAY5 or
            DeviceTags.RELAY6 or
            DeviceTags.RELAY7 or
            DeviceTags.RELAY8 or
            DeviceTags.RELAY9 or
            DeviceTags.RELAY10 or
            DeviceTags.RELAY11 or
            DeviceTags.RELAY12 or
            DeviceTags.RELAY13 or
            DeviceTags.RELAY14 or
            DeviceTags.RELAY15 or
            DeviceTags.RELAY16 or
            DeviceTags.RELAY17 or
            DeviceTags.RELAY18 or
            DeviceTags.RELAY19 or
            DeviceTags.RELAY20 or
            DeviceTags.RELAY21 or
            DeviceTags.RELAY22 or
            DeviceTags.RELAY23 or
            DeviceTags.RELAY24 or
            DeviceTags.RELAY25 or
            DeviceTags.RELAY26 or
            DeviceTags.RELAY27 or
            DeviceTags.RELAY28 or
            DeviceTags.RELAY29 or
            DeviceTags.RELAY30 or
            DeviceTags.RELAY31 or
            DeviceTags.RELAY32 or
            DeviceTags.LATCH1 or
            DeviceTags.LATCH2 or
            DeviceTags.LATCH3 or
            DeviceTags.LATCH4 or
            DeviceTags.LATCH5 or
            DeviceTags.LATCH6 or
            DeviceTags.LATCH7 or
            DeviceTags.LATCH8 or
            DeviceTags.LATCH9 or
            DeviceTags.LATCH10 or
            DeviceTags.LATCH11 or
            DeviceTags.LATCH12 or
            DeviceTags.LATCH13 or
            DeviceTags.LATCH14 or
            DeviceTags.LATCH15 or
            DeviceTags.LATCH16 or
            DeviceTags.LATCH17 or
            DeviceTags.LATCH18 or
            DeviceTags.LATCH19 or
            DeviceTags.LATCH20 or
            DeviceTags.LATCH21 or
            DeviceTags.LATCH22 or
            DeviceTags.LATCH23 or
            DeviceTags.LATCH24 or
            DeviceTags.LATCH25 or
            DeviceTags.LATCH26 or
            DeviceTags.LATCH27 or
            DeviceTags.LATCH28 or
            DeviceTags.LATCH29 or
            DeviceTags.LATCH30 or
            DeviceTags.LATCH31 or
            DeviceTags.LATCH32 => true,
            _ => false
        };
    }

    public static bool IsInputProperty(this DeviceTags tag)
    {
        return tag switch
        {
            DeviceTags.DIGITAL_INPUT1 or
            DeviceTags.DIGITAL_INPUT2 or
            DeviceTags.DIGITAL_INPUT3 or
            DeviceTags.DIGITAL_INPUT4 or
            DeviceTags.DIGITAL_INPUT5 or
            DeviceTags.DIGITAL_INPUT6 or
            DeviceTags.DIGITAL_INPUT7 or
            DeviceTags.DIGITAL_INPUT8 or
            DeviceTags.DIGITAL_INPUT9 or
            DeviceTags.DIGITAL_INPUT10 or
            DeviceTags.DIGITAL_INPUT11 or
            DeviceTags.DIGITAL_INPUT12 or
            DeviceTags.DIGITAL_INPUT13 or
            DeviceTags.DIGITAL_INPUT14 or
            DeviceTags.DIGITAL_INPUT15 or
            DeviceTags.DIGITAL_INPUT16 or
            DeviceTags.DIGITAL_INPUT17 or
            DeviceTags.DIGITAL_INPUT18 or
            DeviceTags.DIGITAL_INPUT19 or
            DeviceTags.DIGITAL_INPUT20 or
            DeviceTags.DIGITAL_INPUT21 or
            DeviceTags.DIGITAL_INPUT22 or
            DeviceTags.DIGITAL_INPUT23 or
            DeviceTags.DIGITAL_INPUT24 or
            DeviceTags.DIGITAL_INPUT25 or
            DeviceTags.DIGITAL_INPUT26 or
            DeviceTags.DIGITAL_INPUT27 or
            DeviceTags.DIGITAL_INPUT28 or
            DeviceTags.DIGITAL_INPUT29 or
            DeviceTags.DIGITAL_INPUT30 or
            DeviceTags.DIGITAL_INPUT31 or
            DeviceTags.DIGITAL_INPUT32 or
            DeviceTags.PULSE_INPUT1 or
            DeviceTags.PULSE_INPUT2 or
            DeviceTags.PULSE_INPUT3 or
            DeviceTags.PULSE_INPUT4 or
            DeviceTags.PULSE_INPUT5 or
            DeviceTags.PULSE_INPUT6 or
            DeviceTags.PULSE_INPUT7 or
            DeviceTags.PULSE_INPUT8 or
            DeviceTags.PULSE_INPUT9 or
            DeviceTags.PULSE_INPUT10 or
            DeviceTags.PULSE_INPUT11 or
            DeviceTags.PULSE_INPUT12 or
            DeviceTags.PULSE_INPUT13 or
            DeviceTags.PULSE_INPUT14 or
            DeviceTags.PULSE_INPUT15 or
            DeviceTags.PULSE_INPUT16 or
            DeviceTags.PULSE_INPUT17 or
            DeviceTags.PULSE_INPUT18 or
            DeviceTags.PULSE_INPUT19 or
            DeviceTags.PULSE_INPUT20 or
            DeviceTags.PULSE_INPUT21 or
            DeviceTags.PULSE_INPUT22 or
            DeviceTags.PULSE_INPUT23 or
            DeviceTags.PULSE_INPUT24 or
            DeviceTags.PULSE_INPUT25 or
            DeviceTags.PULSE_INPUT26 or
            DeviceTags.PULSE_INPUT27 or
            DeviceTags.PULSE_INPUT28 or
            DeviceTags.PULSE_INPUT29 or
            DeviceTags.PULSE_INPUT30 or
            DeviceTags.PULSE_INPUT31 or
            DeviceTags.PULSE_INPUT32 or
            DeviceTags.A_CURRENT_INPUT1 or
            DeviceTags.A_CURRENT_INPUT2 or
            DeviceTags.A_CURRENT_INPUT3 or
            DeviceTags.A_CURRENT_INPUT4 or
            DeviceTags.A_CURRENT_INPUT5 or
            DeviceTags.A_CURRENT_INPUT6 or
            DeviceTags.A_CURRENT_INPUT7 or
            DeviceTags.A_CURRENT_INPUT8 or
            DeviceTags.A_CURRENT_INPUT9 or
            DeviceTags.A_CURRENT_INPUT10 or
            DeviceTags.A_CURRENT_INPUT11 or
            DeviceTags.A_CURRENT_INPUT12 or
            DeviceTags.A_CURRENT_INPUT13 or
            DeviceTags.A_CURRENT_INPUT14 or
            DeviceTags.A_CURRENT_INPUT15 or
            DeviceTags.A_CURRENT_INPUT16 or
            DeviceTags.A_CURRENT_INPUT17 or
            DeviceTags.A_CURRENT_INPUT18 or
            DeviceTags.A_CURRENT_INPUT19 or
            DeviceTags.A_CURRENT_INPUT20 or
            DeviceTags.A_CURRENT_INPUT21 or
            DeviceTags.A_CURRENT_INPUT22 or
            DeviceTags.A_CURRENT_INPUT23 or
            DeviceTags.A_CURRENT_INPUT24 or
            DeviceTags.A_CURRENT_INPUT25 or
            DeviceTags.A_CURRENT_INPUT26 or
            DeviceTags.A_CURRENT_INPUT27 or
            DeviceTags.A_CURRENT_INPUT28 or
            DeviceTags.A_CURRENT_INPUT29 or
            DeviceTags.A_CURRENT_INPUT30 or
            DeviceTags.A_CURRENT_INPUT31 or
            DeviceTags.A_CURRENT_INPUT32 or
            DeviceTags.A_VOLTAGE_INPUT1 or
            DeviceTags.A_VOLTAGE_INPUT2 or
            DeviceTags.A_VOLTAGE_INPUT3 or
            DeviceTags.A_VOLTAGE_INPUT4 or
            DeviceTags.A_VOLTAGE_INPUT5 or
            DeviceTags.A_VOLTAGE_INPUT6 or
            DeviceTags.A_VOLTAGE_INPUT7 or
            DeviceTags.A_VOLTAGE_INPUT8 or
            DeviceTags.A_VOLTAGE_INPUT9 or
            DeviceTags.A_VOLTAGE_INPUT10 or
            DeviceTags.A_VOLTAGE_INPUT11 or
            DeviceTags.A_VOLTAGE_INPUT12 or
            DeviceTags.A_VOLTAGE_INPUT13 or
            DeviceTags.A_VOLTAGE_INPUT14 or
            DeviceTags.A_VOLTAGE_INPUT15 or
            DeviceTags.A_VOLTAGE_INPUT16 or
            DeviceTags.A_VOLTAGE_INPUT17 or
            DeviceTags.A_VOLTAGE_INPUT18 or
            DeviceTags.A_VOLTAGE_INPUT19 or
            DeviceTags.A_VOLTAGE_INPUT20 or
            DeviceTags.A_VOLTAGE_INPUT21 or
            DeviceTags.A_VOLTAGE_INPUT22 or
            DeviceTags.A_VOLTAGE_INPUT23 or
            DeviceTags.A_VOLTAGE_INPUT24 or
            DeviceTags.A_VOLTAGE_INPUT25 or
            DeviceTags.A_VOLTAGE_INPUT26 or
            DeviceTags.A_VOLTAGE_INPUT27 or
            DeviceTags.A_VOLTAGE_INPUT28 or
            DeviceTags.A_VOLTAGE_INPUT29 or
            DeviceTags.A_VOLTAGE_INPUT30 or
            DeviceTags.A_VOLTAGE_INPUT31 or
            DeviceTags.A_VOLTAGE_INPUT32 => true,
            _ => false
        };
    }

    /// <summary>
    /// Returns tag associated to given alias string
    /// </summary>
    /// <param name="alias">to convert</param>
    /// <returns>Device tag if alias matches a unique tag</returns>
    /// <exception cref="ArgumentNullException"></exception>
    /// <exception cref="InvalidOperationException"></exception>
    public static DeviceTags FromAlias(string alias)
    {
        //Note: can be slow if intensively used but permits to have a single map from device tags to aliases...
        return Enum.GetValues<DeviceTags>().Where(t => t.ToAlias() == alias).Single();
    }
}

#pragma once

// Board configuration example

// ----------------------------- You can conveniently define pin names here:
// #define SYS_LED         2
// #define V12_ENABLE      25
// ...

// ------------------------------ Put here I2C bus definition here:
// #define I2C0_SCL                    22
// #define I2C0_SDA                    21
// #define I2C0_MASTER_NUM             0                         
// #define I2C0_MASTER_FREQ_HZ         400000       

// ------------------------------ Put useful values here:
// #define ADC_V_SCALE_VOLTAGE     ((10000.0f + 52300.0f ) / 10000.0f) //Ohm

// ------------------------------ Define ADC implementation here:

// ---------------- I2C ADC example:
// It is possible to use external ADC connected to I2C enabling this
// #define IOT_BOARD_USES_I2C_ADC
// It enables ADS101X driver
// #define IOT_BOARD_USES_ADS101X_ADC
// It enables ADS101X first address
// #define IOT_BOARD_USES_ADS101X_ADC0
// If defined RDY pin of ADS101X can be used
// #define IOT_BOARD_ADS101X_ADC0_RDY_GPIO    39 
// #define ADC_BIT_RESOLUTION      12
// #define ADC_VREF_DEFAULT        1100U
// #define ADC_N_SAMPLE_DEFAULT    1
// #define ADC_MIN_MEASURE_IGNORE  0
// #define ADC_C_SENSE_RESISTOR    120.0f //Ohm


// --------------- Internal ADC example:
// #define IOT_BOARD_USES_INTERNAL_ADC
// #define ADC_BIT_RESOLUTION      12
// #define ADC_VREF_DEFAULT        1100U
// #define ADC_N_SAMPLE_DEFAULT    100
// #define ADC_MIN_MEASURE_IGNORE  3
// #define ADC_C_SENSE_RESISTOR    120.0f //Ohm


// ------------------------------- Define board I/O here:
// #define IOT_BOARD_N_LED                          1U
// #define IOT_BOARD_N_CURRENT                      0U
// #define IOT_BOARD_N_VOLTAGE                      1U
// #define IOT_BOARD_N_PULSE                        2U
// #define IOT_BOARD_N_DIGITAL                      2U
// #define IOT_BOARD_N_LATCH                        2U
// #define IOT_BOARD_N_RELAY                        1U
// #define IOT_BOARD_N_TEMPERATURE                  1U


// ------------------------------ Declare board I/O here:
// #define IOT_BOARD_LATCH_MACRO                    { std::make_tuple(L0_F, L0_R), std::make_tuple(L1_F, L1_R) }
// #define IOT_BOARD_RELAY_MACRO                    { R0 }
// #define IOT_BOARD_LED_MACRO                      { SYS_LED }
// #define IOT_BOARD_PULSE_MACRO                    { P0, P1 }
// #define IOT_BOARD_PULSE_IRQ_MACRO                { __MAKE_PULSE_ENTRY(P0), __MAKE_PULSE_ENTRY(P1) }
// #define IOT_BOARD_PULSE_DISABLE_DEFAULTS_MILLIS  { 3000U, 3000U }
// #define IOT_BOARD_DIGITAL_MACRO                  { P0, P1 }
// declare to store last pulse count on flash
// #define IOT_BOARD_PULSE_RESTORE
// #define IOT_BOARD_STROBE_DURATION                100U
// #define IOT_BOARD_N_TEMPERATURE_USES_LM75D                  


// ---------------- I2C ADC example:
// #define IOT_BOARD_CURRENT_I2C_ADDR_MACRO         { C0_ADC_ADDR }
// #define IOT_BOARD_CURRENT_I2C_CH_MACRO           { C0_ADC_CH }
// #define IOT_BOARD_CURRENT_I2C_ATTEN_MACRO        { C0_ADC_ATTEN } //see adc_atten_t
// #define IOT_BOARD_VOLTAGE_I2C_ADDR_MACRO         { V0_ADC_ADDR }
// #define IOT_BOARD_VOLTAGE_I2C_CH_MACRO           { V0_ADC_CH }
// #define IOT_BOARD_VOLTAGE_I2C_SCALES_MACRO       { ADC_V_SCALE_VOLTAGE }
// #define IOT_BOARD_VOLTAGE_I2C_ATTEN_MACRO        { V0_ADC_ATTEN } //see adc_atten_t

// ---------------- Internal ADC example:
// #define IOT_BOARD_CURRENT_MACRO                  { C0 }
// #define IOT_BOARD_VOLTAGE_MACRO                  { V0 }
// #define IOT_BOARD_VOLTAGE_SCALES_MACRO           { ADC_V_SCALE_VOLTAGE }
// #define IOT_BOARD_VOLTAGE_ATTEN_MACRO            { 3 } //see adc_atten_t




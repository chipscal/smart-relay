#pragma once

// Smart Relay WiFi R1
#define MODEL_NAME      "Smart R1"

// ----------------------------- You can conveniently define pin names here:
#define SYS_LED         2
#define DIN1            7
#define CIN1            48
#define CIN2            47
#define AIN1            1
#define R1              38

// ------------------------------ Put here I2C bus definition here:
#define I2C0_SCL                    9
#define I2C0_SDA                    8
#define I2C0_MASTER_NUM             0                         
#define I2C0_MASTER_FREQ_HZ         400000       

// ------------------------------ Put useful values here:
#define ADC_V_SCALE_VOLTAGE     ((10000.0f + 10000.0f ) / 10000.0f) //Ohm
#define TEMPERATURE_ADDR        0b1001111

// ------------------------------ Define ADC implementation here:
#define IOT_BOARD_USES_INTERNAL_ADC
#define ADC_BIT_RESOLUTION      12
#define ADC_VREF_DEFAULT        1100U
#define ADC_N_SAMPLE_DEFAULT    100
#define ADC_MIN_MEASURE_IGNORE  0
#define ADC_C_SENSE_RESISTOR    0


// ------------------------------- Define board I/O here:
#define IOT_BOARD_N_LED                          1U
#define IOT_BOARD_N_CURRENT                      0U
#define IOT_BOARD_N_VOLTAGE                      1U
#define IOT_BOARD_N_PULSE                        1U
#define IOT_BOARD_N_DIGITAL                      3U
#define IOT_BOARD_N_LATCH                        0U
#define IOT_BOARD_N_RELAY                        1U
#define IOT_BOARD_N_TEMPERATURE                  1U


// ------------------------------ Declare board I/O here:
#define IOT_BOARD_RELAY_MACRO                    { R1 }
#define IOT_BOARD_LED_MACRO                      { SYS_LED }
#define IOT_BOARD_DIGITAL_MACRO                  { DIN1, CIN1, CIN2 }
#define IOT_BOARD_DIGITAL_INVERTING_MACRO        { __INVERTING_DIGITAL_VALUE, __INVERTING_DIGITAL_VALUE, __INVERTING_DIGITAL_VALUE }
#define IOT_BOARD_PULSE_MACRO                    { DIN1 }
#define IOT_BOARD_PULSE_IRQ_MACRO                { __MAKE_PULSE_ENTRY(DIN1) }
#define IOT_BOARD_PULSE_DISABLE_DEFAULTS_MILLIS  { 3000U }
#define IOT_BOARD_PULSE_RESTORE
#define IOT_BOARD_STROBE_DURATION                0U
#define IOT_BOARD_VOLTAGE_MACRO                  { AIN1 }
#define IOT_BOARD_VOLTAGE_SCALES_MACRO           { ADC_V_SCALE_VOLTAGE }
#define IOT_BOARD_VOLTAGE_ATTEN_MACRO            { 3 } //see adc_atten_t
#define IOT_BOARD_USES_I2C_TEMPERATURE
#define IOT_BOARD_TEMPERATURE_USES_LM75D                  
#define IOT_BOARD_TEMPERATURE_I2C_ADDR_MACRO     { TEMPERATURE_ADDR }

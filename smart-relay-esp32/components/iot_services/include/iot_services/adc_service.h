#pragma once

#include "esp_check.h"
#include "iot_services/pinout.h"


#ifdef IOT_BOARD_USES_INTERNAL_ADC

#include "driver/gpio.h"
#include "esp_adc/adc_cali.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Initialize ADCs (define ADC_USE_ADC2 to enable also ADC2). 
/// @note this function is NOT thread safe and must me called only one time during initialization!
/// @param  
esp_err_t adc_service_init(void);

/// @brief Deinitilize ADC and clean up resources. 
/// @note this function is NOT thread safe and must me called only one time during deinitialization!
/// @param  
esp_err_t adc_service_deinit(void);

/// @brief Calibrates the ADC channel to compensate intrinsic differences of the default VREF of the modules.
/// @note this function is thread safe.
/// @param[in] gpio pin used - it will be transformed to the actual ADC channel,
/// @param[out] out_handle handle to the calibration struct to be created, 
/// @return ESP_OK if ok.
esp_err_t adc_service_calibration_init(gpio_num_t gpio, adc_atten_t atten, adc_cali_handle_t *out_handle);

/// @brief Cleans up allocated calibration resources.
/// @note this function is thread safe.
/// @param handle to the calibration struct. 
esp_err_t adc_service_calibration_deinit(adc_cali_handle_t handle);

/// @brief Analog raw measure of the corresponding gpio.
/// @note check module pinout for possible gpios to use;
/// @note this function is thread safe. 
/// @param[in] gpio pin used - it will be transformed to the actual ADC channel,
/// @param n_samples number of consecutive samples (>= 1) taken to compute average measure,
/// @return average uncalibrated analog measure of the corresponding gpio.
/// @note the returned value will be an integer according to the bit resolution used (defined with ADC_BIT_RESOLUTION)
/// considering the default ADC_ATTEN_DB_11 attenuation.
int adc_service_measure_raw(gpio_num_t gpio, int n_samples);

/// @brief Analog voltage measure of the corresponding gpio.
/// @note check module pinout for possible gpios to use;
/// @note this function is thread safe. 
/// @param gpio pin used - it will be transformed to the actual ADC channel,
/// @param handle to the calibration struct,
/// @param n_samples number of consecutive samples (>= 1) taken to compute average measure,
/// @return avaraged voltage (mV) measure of the corresponding gpio.
int adc_service_measure_voltage(gpio_num_t gpio, adc_cali_handle_t handle, int n_samples);

#ifdef __cplusplus
}
#endif

#endif
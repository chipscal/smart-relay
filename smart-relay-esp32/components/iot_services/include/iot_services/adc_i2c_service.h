#pragma once

#include "esp_check.h"
#include "iot_services/pinout.h"

#include <array>

namespace clab::iot_services
{
    /// @brief ADC devices definition.
    typedef enum : uint8_t {
        ADC0          = __ADC0,
        ADC1          = __ADC1,
        ADC2          = __ADC2,
        ADC3          = __ADC3,
    } adc_i2c_device_t;

    /// @brief ADC channels definition.
    typedef enum : uint8_t {
        ADC_CH0          = __ADC_CH0,
        ADC_CH1          = __ADC_CH1,
        ADC_CH2          = __ADC_CH2,
        ADC_CH3          = __ADC_CH3,
    } adc_i2c_channel_t;

    /// @brief ADC attenuations definition.
    typedef enum : uint8_t {
        ADC_ATTEN0       = __ADC_ATTEN0,
        ADC_ATTEN1       = __ADC_ATTEN1,
        ADC_ATTEN2       = __ADC_ATTEN2,
        ADC_ATTEN3       = __ADC_ATTEN3,
        ADC_ATTEN4       = __ADC_ATTEN4,
        ADC_ATTEN5       = __ADC_ATTEN5,
    } adc_i2c_atten_t;

    struct adc_i2c_input_t {
        
        /// @brief I2C device.
        adc_i2c_device_t    device;

        /// @brief I2C channel.
        adc_i2c_channel_t   channel;

        /// @brief Attenuation value.
        adc_i2c_atten_t     atten;

        /// @brief Allignment byte.
        uint8_t             reserved;
    };


    /// @brief Initialize ADCs I2C service. 
    /// @note this function is NOT thread safe and must me called only one time during initialization!
    /// @param  
    esp_err_t adc_i2c_service_init(void);

    /// @brief Deinitilize ADC I2C service and clean up resources. 
    /// @note this function is NOT thread safe and must me called only one time during deinitialization!
    /// @param  
    esp_err_t adc_i2c_service_deinit(void);

    /// @brief Analog raw measure of the corresponding ADC input.
    /// @note this function is thread safe. 
    /// @param[in] input ADC input,
    /// @param n_samples number of consecutive samples (>= 1) taken to compute average measure,
    /// @param[out] output where to store average uncalibrated analog measure of the corresponding ADC input 
    /// @return ESP_OK on success.
    esp_err_t adc_i2c_service_measure_raw(adc_i2c_input_t &input, int n_samples, int &output);

    /// @brief Analog voltage measure of the corresponding ADC input.
    /// @note this function is thread safe. 
    /// @param[in] input ADC input,
    /// @param n_samples number of consecutive samples (>= 1) taken to compute average measure,
    /// @param[out] output where to store average analog measure (mV) of the corresponding ADC input 
    /// @return ESP_OK on success.
    esp_err_t adc_i2c_service_measure_voltage(adc_i2c_input_t &input, int n_samples, float &output);



    /// @brief Helper function to define ADC I2C input array at compile time.
    /// @tparam N size of the array
    /// @param idc_addresses initialization list of length N of I2C addresses,
    /// @param idc_channels initialization list of length N of I2C channels,
    /// @param attenuations initialization list of length N of ADC attenuations,
    /// @return an std::array of size N and type adc_i2c_input_t properly initializated.
    template <std::size_t N>
    constexpr std::array<adc_i2c_input_t, N> adc_i2c_array_create(const std::array<uint8_t, N> &idc_addresses, 
            const std::array<uint8_t, N> &idc_channels, const std::array<uint8_t, N> &attenuations) {
        
        std::array<adc_i2c_input_t, N> to_ret;
        for (size_t k = 0; k < N; k++) {
            to_ret[k].device = static_cast<adc_i2c_device_t>(idc_addresses[k]);
            to_ret[k].channel = static_cast<adc_i2c_channel_t>(idc_channels[k]);
            to_ret[k].atten = static_cast<adc_i2c_atten_t>(attenuations[k]);
            to_ret[k].reserved = 0U;
        }

        return to_ret;
    }
}


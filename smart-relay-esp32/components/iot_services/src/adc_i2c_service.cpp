#include "iot_services/adc_i2c_service.h"
#include "iot_services/iot_services.h"
#include "iot_services/i2c_bus_service.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

static const char *TAG = "iot_services::adc_i2c";

#ifdef IOT_BOARD_USES_I2C_ADC

#ifdef IOT_BOARD_USES_ADS101X_ADC
    #define __ADS101X_ADDR0 0b1001000
    #define __ADS101X_ADDR1 0b1001001
    #define __ADS101X_ADDR2 0b1001010
    #define __ADS101X_ADDR3 0b1001011

    #define __ADS101X_DEFAULT_TIMEOUT       1000U

    #define __ADS101X_CONVERSION_REG        0b00000000
    #define __ADS101X_CONFIG_REG            0b00000001
    #define __ADS101X_LOW_THRESHOLD_REG     0b00000010
    #define __ADS101X_HIGH_THRESHOLD_REG    0b00000011

    #define __ADS101X_CONFIG_MSB_WRITE_DO_SINGLE_SHOT    0b10000000
    #define __ADS101X_CONFIG_MSB_WRITE_MUX_CH0           0b01000000
    #define __ADS101X_CONFIG_MSB_WRITE_MUX_CH1           0b01010000
    #define __ADS101X_CONFIG_MSB_WRITE_MUX_CH2           0b01100000
    #define __ADS101X_CONFIG_MSB_WRITE_MUX_CH3           0b01110000
    #define __ADS101X_CONFIG_MSB_WRITE_FSR_6V1           0b00000000
    #define __ADS101X_CONFIG_MSB_WRITE_FSR_4V1           0b00000010
    #define __ADS101X_CONFIG_MSB_WRITE_FSR_2V0           0b00000100
    #define __ADS101X_CONFIG_MSB_WRITE_FSR_1V0           0b00000110
    #define __ADS101X_CONFIG_MSB_WRITE_FSR_0V5           0b00001000
    #define __ADS101X_CONFIG_MSB_WRITE_FSR_0V2           0b00001010
    #define __ADS101X_CONFIG_MSB_WRITE_MODE_CONTINUOS    0b00000000
    #define __ADS101X_CONFIG_MSB_WRITE_MODE_SINGLE       0b00000001

    #define __ADS101X_CONFIG_LSB_WRITE_DR_128            0b00000000
    #define __ADS101X_CONFIG_LSB_WRITE_DR_250            0b00100000
    #define __ADS101X_CONFIG_LSB_WRITE_DR_490            0b01000000
    #define __ADS101X_CONFIG_LSB_WRITE_DR_920            0b01100000
    #define __ADS101X_CONFIG_LSB_WRITE_DR_1600           0b10000000
    #define __ADS101X_CONFIG_LSB_WRITE_DR_2400           0b10100000
    #define __ADS101X_CONFIG_LSB_WRITE_DR_3300           0b11000000
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_TRADITIONAL  0b00000000
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_WINDOW       0b00010000
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_ACTIVE_LOW   0b00000000
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_ACTIVE_HIGH  0b00001000
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_NON_LATCHING 0b00000000
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_LATCHING     0b00000100
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_ASSERT_1S    0b00000000
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_ASSERT_2S    0b00000001
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_ASSERT_4S    0b00000010
    #define __ADS101X_CONFIG_LSB_WRITE_COMP_DISABLED     0b00000011


    #define __ADS101X_CONFIG_MSB_OS_CONVERSION_PENDING_MASK       0b10000000
    #define __ADS101X_CONFIG_VALUES_MSB_CONVERSION_MASK           0b11111111
    #define __ADS101X_CONFIG_VALUES_LSB_CONVERSION_MASK           0b11110000

#endif

namespace clab::iot_services {

    SemaphoreHandle_t   adc_i2c_mutex = NULL;


    #ifdef IOT_BOARD_USES_ADS101X_ADC
        int16_t adc_i2c_ads101x_read_value(uint8_t msb, uint8_t lsb);

        constexpr uint8_t adc_i2c_default_config_msb { 
            __ADS101X_CONFIG_MSB_WRITE_FSR_6V1 |
            __ADS101X_CONFIG_MSB_WRITE_MODE_SINGLE
        };

        constexpr uint8_t adc_i2c_default_config_lsb { 
            __ADS101X_CONFIG_LSB_WRITE_DR_3300 |
            __ADS101X_CONFIG_LSB_WRITE_COMP_DISABLED |
            __ADS101X_CONFIG_LSB_WRITE_COMP_ACTIVE_HIGH
        };

        constexpr uint16_t adc_i2c_default_config { (adc_i2c_default_config_msb << 8) | adc_i2c_default_config_lsb};

        //Note: it depends from Data Rate selected
        constexpr uint32_t adc_i2c_default_wait_micros { 1000000 / 3300 };

        uint8_t adc_i2c_device_addresses[4] = {
            __ADS101X_ADDR0,
            __ADS101X_ADDR1,
            __ADS101X_ADDR2,
            __ADS101X_ADDR3,
        };

        uint8_t adc_i2c_ch_to_conf_msb_channels[4] = {
            __ADS101X_CONFIG_MSB_WRITE_MUX_CH0,
            __ADS101X_CONFIG_MSB_WRITE_MUX_CH1,
            __ADS101X_CONFIG_MSB_WRITE_MUX_CH2,
            __ADS101X_CONFIG_MSB_WRITE_MUX_CH3,
        };

        const uint8_t adc_i2c_atten_to_conf_msb_fsr[8] = {
            __ADS101X_CONFIG_MSB_WRITE_FSR_6V1,
            __ADS101X_CONFIG_MSB_WRITE_FSR_4V1,
            __ADS101X_CONFIG_MSB_WRITE_FSR_2V0,
            __ADS101X_CONFIG_MSB_WRITE_FSR_1V0,
            __ADS101X_CONFIG_MSB_WRITE_FSR_0V5,
            __ADS101X_CONFIG_MSB_WRITE_FSR_0V2,
            __ADS101X_CONFIG_MSB_WRITE_FSR_0V2,
            __ADS101X_CONFIG_MSB_WRITE_FSR_0V2,
        };

        const float adc_i2c_atten_to_device_fsr[8] = {
            6.144f,
            4.096f,
            2.048f,
            1.024f,
            0.512f,
            0.256f,
            0.256f,
            0.256f,
        };



    #endif
    
    esp_err_t adc_i2c_service_init(void) {
        
        esp_err_t result;
        if (adc_i2c_mutex == NULL)
            adc_i2c_mutex = xSemaphoreCreateMutex(); //In tests may have been already allocated
        ESP_LOGI(TAG, "ADC I2C service mutex created...");

        #ifdef IOT_BOARD_USES_ADS101X_ADC0
            ESP_LOGI(TAG, "ADC mode: ads101x");

            result = i2c_bus_register_write_2bytes(adc_i2c_device_addresses[ADC0], __ADS101X_CONFIG_REG, 
                    adc_i2c_default_config_msb | __ADS101X_CONFIG_MSB_WRITE_MUX_CH0,
                    adc_i2c_default_config_lsb, __ADS101X_DEFAULT_TIMEOUT);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during ADC0 configuration!");
                return result;
            }

            #ifdef IOT_BOARD_ADS101X_ADC0_RDY_GPIO
                //TODO: setup interrupt routine
            #endif
        #endif

        #ifdef IOT_BOARD_USES_ADS101X_ADC1
            result = i2c_bus_register_write_2bytes(adc_i2c_device_addresses[ADC1], __ADS101X_CONFIG_REG,  
                    adc_i2c_default_config_msb | __ADS101X_CONFIG_MSB_WRITE_MUX_CH0,
                    adc_i2c_default_config_lsb, __ADS101X_DEFAULT_TIMEOUT);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during ADC0 configuration!");
                return result;
            }

            #ifdef IOT_BOARD_ADS101X_ADC1_RDY_GPIO
                //TODO: setup interrupt routine
            #endif
        #endif

        return ESP_OK;
    }
    
    esp_err_t adc_i2c_service_deinit(void) {
        #if defined(IOT_BOARD_USES_ADS101X_ADC0) && defined(IOT_BOARD_ADS101X_ADC0_RDY_GPIO)
            //TODO: detach interrupt routine
        #endif

        #if defined(IOT_BOARD_USES_ADS101X_ADC1) && defined(IOT_BOARD_ADS101X_ADC1_RDY_GPIO)
            //TODO: detach interrupt routine
        #endif

        return ESP_OK;
    }

    
    esp_err_t adc_i2c_service_measure_raw(adc_i2c_input_t &input, int n_samples, int &output) {
        esp_err_t result;

        xSemaphoreTake(adc_i2c_mutex,  portMAX_DELAY);

        output = 0;
        for (int k = 0; k < n_samples; k++) {
            result = i2c_bus_register_write_2bytes(adc_i2c_device_addresses[input.device], __ADS101X_CONFIG_REG, 
                    adc_i2c_default_config_msb | adc_i2c_atten_to_conf_msb_fsr[input.atten] | 
                    adc_i2c_ch_to_conf_msb_channels[input.channel] | __ADS101X_CONFIG_MSB_WRITE_DO_SINGLE_SHOT,
                    adc_i2c_default_config_lsb, __ADS101X_DEFAULT_TIMEOUT);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during ADC%d configuration!", input.device);
                xSemaphoreGive(adc_i2c_mutex);
                return result;
            }

            ets_delay_us(adc_i2c_default_wait_micros);

            uint8_t buffer[2];
            result = i2c_bus_register_read(adc_i2c_device_addresses[input.device], __ADS101X_CONVERSION_REG, buffer, sizeof(buffer), 
                    __ADS101X_DEFAULT_TIMEOUT);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during ADC%d conversion reading!", input.device);
                xSemaphoreGive(adc_i2c_mutex);
                return result;
            }

            output += adc_i2c_ads101x_read_value(buffer[0], buffer[1]);
        }

        output /= n_samples;
        
        xSemaphoreGive(adc_i2c_mutex);
        return ESP_OK;
    }

    
    esp_err_t adc_i2c_service_measure_voltage(adc_i2c_input_t &input, int n_samples, float &output) {
        
        //Note: also negative values possible
        float internal_scale = adc_i2c_atten_to_device_fsr[input.atten] / 2048.0f;
        
        int raw = 0;
        esp_err_t result = adc_i2c_service_measure_raw(input, n_samples, raw);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during ADC%d raw measurement!", input.device);
            return result;
        }

        output = raw * internal_scale * 1000;
        return ESP_OK;
    }

    #ifdef IOT_BOARD_USES_ADS101X_ADC
        int16_t adc_i2c_ads101x_read_value(uint8_t msb, uint8_t lsb) {
            int16_t to_ret = (msb << 8 ) | 
                    (lsb & __ADS101X_CONFIG_VALUES_LSB_CONVERSION_MASK);
            return to_ret >> 4;
        }
    #endif

}
#endif
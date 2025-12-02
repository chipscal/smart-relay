#include "iot_services/iot_services.h"
#include "iot_services/io_service.h"
#include "iot_services/storage_service.h"


#if defined(IOT_BOARD_USES_I2C_ADC)
    #include "iot_services/adc_i2c_service.h"

    //dummy definition to use in dummy_array
    #define adc_cali_handle_t uint8_t
#else
    #include "iot_services/adc_service.h"
#endif

#if defined(IOT_BOARD_USES_I2C_TEMPERATURE)
    #include "iot_services/i2c_bus_service.h"
#endif


#include <atomic>
#include <array>
#include <map>
#include <algorithm>
#include <string>
#include <cstring>
#include <cmath>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "mbedtls/base64.h"


static const char *TAG = "iot_services::io_service";

#define ESP_INTR_FLAG_DEFAULT 0

#if defined(IOT_BOARD_TEMPERATURE_USES_LM75D)
    #define __LM75D_TEMP_REGISTER     0b00000000
    #define __LM75D_CONF_REGISTER     0b00000001
    #define __LM75D_HYST_REGISTER     0b00000010
    #define __LM75D_OVER_REGISTER     0b00000011

    #define __LM75D_DEFAULT_TIMEOUT         1000U
    #define __LM75D_WAIT_MEASURE_MILLIS     50U

    #define __LM75D_NORMAL_BEHAVIOUR  0b00000000
    #define __LM75D_SHUTDOWN          0b00000001

    #define __LM75D_VALUES_MSB_CONVERSION_MASK           0b11111111
    #define __LM75D_VALUES_LSB_CONVERSION_MASK           0b11100000
    #define __LM75D_VALUES_SCALE_FACTOR                  0.125f;


#endif


namespace clab::iot_services {   

    uint32_t    last_cmd_timestamp = 0;

    void        (*telem_callback)(const uint8_t *payload, int payload_size) = nullptr;

    SemaphoreHandle_t               io_mutex = NULL;


    #ifdef IOT_BOARD_LATCH_MACRO
        std::array<std::tuple<uint8_t, uint8_t>, IOT_BOARD_N_LATCH>    latch_out       = IOT_BOARD_LATCH_MACRO;
        std::array<bool, IOT_BOARD_N_LATCH>                            latch_status    = filled_array<bool, IOT_BOARD_N_LATCH>(false);
    #else
        dummy_array<std::tuple<uint8_t, uint8_t>>   latch_out;
        dummy_array<bool>                           latch_status;
    #endif

    #ifdef IOT_BOARD_RELAY_MACRO
        std::array<uint8_t, IOT_BOARD_N_RELAY>         relay_out       = IOT_BOARD_RELAY_MACRO;
        std::array<bool, IOT_BOARD_N_RELAY>            relay_status    = filled_array<bool, IOT_BOARD_N_RELAY>(false);
    #else
        dummy_array<uint8_t>                        relay_out;
        dummy_array<bool>                           relay_status;
    #endif
    constexpr unsigned int      adc_resolution  { ADC_BIT_RESOLUTION };

    #if defined(IOT_BOARD_USES_I2C_ADC) && defined(IOT_BOARD_CURRENT_I2C_ADDR_MACRO)
        std::array<adc_i2c_input_t, IOT_BOARD_N_VOLTAGE>       a_current_in      = adc_i2c_array_create<IOT_BOARD_N_CURRENT>(
                IOT_BOARD_CURRENT_I2C_ADDR_MACRO, IOT_BOARD_CURRENT_I2C_ADDR_MACRO, IOT_BOARD_CURRENT_I2C_ATTEN_MACRO);

    #elif defined(IOT_BOARD_CURRENT_MACRO)
        std::array<uint8_t, IOT_BOARD_N_CURRENT>               a_current_in        = IOT_BOARD_CURRENT_MACRO; 
        std::array<adc_cali_handle_t, IOT_BOARD_N_CURRENT>     a_current_calib     = filled_array<adc_cali_handle_t, IOT_BOARD_N_CURRENT>(NULL); 
    #else
        dummy_array<uint8_t>                            a_current_in;
        dummy_array<adc_cali_handle_t>                  a_current_calib;
    #endif

    #if defined(IOT_BOARD_USES_I2C_ADC) && defined(IOT_BOARD_VOLTAGE_I2C_ADDR_MACRO)
        std::array<adc_i2c_input_t, IOT_BOARD_N_VOLTAGE>       a_volt_in           = adc_i2c_array_create<IOT_BOARD_N_VOLTAGE>(
                IOT_BOARD_VOLTAGE_I2C_ADDR_MACRO, IOT_BOARD_VOLTAGE_I2C_ADDR_MACRO, IOT_BOARD_VOLTAGE_I2C_ATTEN_MACRO);

        std::array<float, IOT_BOARD_N_VOLTAGE>                 a_volt_scales       = IOT_BOARD_VOLTAGE_I2C_SCALES_MACRO;  

    #elif defined(IOT_BOARD_VOLTAGE_MACRO)
        std::array<uint8_t, IOT_BOARD_N_VOLTAGE>               a_volt_in           = IOT_BOARD_VOLTAGE_MACRO;  
        std::array<float, IOT_BOARD_N_VOLTAGE>                 a_volt_scales       = IOT_BOARD_VOLTAGE_SCALES_MACRO;  
        std::array<float, IOT_BOARD_N_VOLTAGE>                 a_volt_atten        = IOT_BOARD_VOLTAGE_ATTEN_MACRO;  
        std::array<adc_cali_handle_t, IOT_BOARD_N_VOLTAGE>     a_volt_calib        = filled_array<adc_cali_handle_t, IOT_BOARD_N_VOLTAGE>(NULL); 

    #else
        dummy_array<uint8_t>                            a_volt_in;
        dummy_array<float>                              a_volt_scales; 
        dummy_array<float>                              a_volt_atten; 
        dummy_array<adc_cali_handle_t>                  a_volt_calib;
    #endif

    #ifdef IOT_BOARD_LED_MACRO
        std::array<uint8_t, IOT_BOARD_N_LED>           led_out         = IOT_BOARD_LED_MACRO;
        std::array<bool, IOT_BOARD_N_LED>              led_status      = filled_array<bool, IOT_BOARD_N_LED>(false);
    #else
        dummy_array<uint8_t>                            led_out;
        dummy_array<bool>                               led_status;
    #endif

    #ifdef IOT_BOARD_PULSE_MACRO
        std::array<uint8_t, IOT_BOARD_N_PULSE>          pulse_in           = IOT_BOARD_PULSE_MACRO; 
        // Use a map to collect IRQ counts
        std::map<const uint8_t, uint16_t>               pulse_disable;//   = IOT_BOARD_PULSE_DISABLE_DEFAULTS_MILLIS; 
        // Use a map to collect IRQ counts
        std::map<const uint8_t, std::atomic<uint16_t>>  irq_counts;//      = IOT_BOARD_PULSE_IRQ_MACRO;
        // Use a map to collect IRQ counts
        std::map<const uint8_t, std::atomic<int64_t>>   last_irq;//        = IOT_BOARD_PULSE_IRQ_MACRO;
    #else
        dummy_array<uint8_t>                            pulse_in;
        dummy_array<uint8_t>                            pulse_disable;
        std::map<const uint8_t, std::atomic<uint16_t>>  irq_counts;
        std::map<const uint8_t, std::atomic<int64_t>>   last_irq;
    #endif

    #if defined(IOT_BOARD_USES_I2C_ADC) && defined(IOT_BOARD_DIGITAL_I2C_ADDR_MACRO)
        std::array<digital_i2c_input_t, IOT_BOARD_N_DIGITAL>       digital_in           = digital_i2c_array_create<IOT_BOARD_N_DIGITAL>(
                IOT_BOARD_DIGITAL_I2C_ADDR_MACRO, IOT_BOARD_DIGITAL_I2C_ADDR_MACRO, IOT_BOARD_DIGITAL_I2C_ATTEN_MACRO);
    #elif defined(IOT_BOARD_DIGITAL_MACRO)
        std::array<uint8_t, IOT_BOARD_N_DIGITAL>               digital_in           = IOT_BOARD_DIGITAL_MACRO;  
        std::array<bool, IOT_BOARD_N_DIGITAL>                  digital_invert       = IOT_BOARD_DIGITAL_INVERTING_MACRO;
    #else
        dummy_array<uint8_t>                            digital_in;
    #endif

    #if defined(IOT_BOARD_USES_I2C_TEMPERATURE) && defined(IOT_BOARD_TEMPERATURE_I2C_ADDR_MACRO)
        std::array<uint8_t, IOT_BOARD_N_TEMPERATURE>       temp_in              = IOT_BOARD_TEMPERATURE_I2C_ADDR_MACRO;
    #elif defined(IOT_BOARD_TEMPERATURE_MACRO)
        std::array<uint8_t, IOT_BOARD_N_TEMPERATURE>       temp_in              = IOT_BOARD_TEMPERATURE_MACRO;  
    #else
        dummy_array<uint8_t>                            temp_in;
    #endif

    

    uint16_t        esp32_io_pulse_get(uint8_t pulse);
    esp_err_t       esp32_io_pulse_save(uint8_t pulse, uint16_t value);
    esp_err_t       esp32_io_pulse_init();
    esp_err_t       esp32_io_pulse_deinit();
    uint16_t        esp32_io_pulse_restore(uint8_t pulse);
    esp_err_t       esp32_io_pulse_filter_restore();
    esp_err_t       esp32_io_relay_cmd(uint8_t relay, bool status);
    esp_err_t       esp32_io_latch_cmd(uint8_t latch, bool status);
    void            esp32_io_telem_emit(const uint8_t *payload, int payload_size);
    bool            esp32_io_digital_get(uint8_t input);
    float           esp32_io_analog_current_get(uint8_t input, float resistor);
    float           esp32_io_analog_voltage_get(uint8_t input, float scale);
    float           esp32_io_temperature_get(uint8_t input);

    esp_err_t io_init() {
        ESP_LOGI(TAG, "Starting up I/O interfaces...");
        esp_err_t result;

        if (io_mutex == NULL) {
            io_mutex = xSemaphoreCreateMutex(); //In tests may have been already allocated
            ESP_LOGI(TAG, "I/O service mutex created...");
        }

        gpio_config_t config = {
            .pin_bit_mask = 0ULL,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = gpio_pullup_t::GPIO_PULLUP_DISABLE,
            .pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE,
            .intr_type = gpio_int_type_t::GPIO_INTR_DISABLE,
        };


        for (int k = 0; k < led_out.size(); k++) {
            config.pin_bit_mask |= (1ULL << led_out[k]);
            led_status[k] = false;
        }

        for (int k = 0; k < latch_out.size(); k++) {
            config.pin_bit_mask |= (1ULL << std::get<0>(latch_out[k]));
            config.pin_bit_mask |= (1ULL << std::get<1>(latch_out[k]));
            
            latch_status[k] = false;
        }

        for (int k = 0; k < relay_out.size(); k++) {
            config.pin_bit_mask |= (1ULL << relay_out[k]);
            relay_status[k] = false;
        }

        result = gpio_config(&config);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during gpios configuration!");
            return result;
        }

        ESP_LOGI(TAG, "Gpio configured...");
        
        uint8_t     last_report[io_buffer_report_size];
        size_t      read_bytes = 0;
        result = storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_report", ((char *)&last_report), io_buffer_report_size, &read_bytes);
        if (result == ESP_OK && read_bytes == io_buffer_report_size) {
            ESP_LOGI(TAG, "Found <<<last_report>>>, trying to deserialize");

            uint8_t last_latch_mask = last_report[4];
            for (int k = 0; k < latch_out.size(); k++) {
                latch_status[k] = (last_latch_mask & (1 << k)) > 0;
                ESP_LOGI(TAG, "L[%d] last status: %s", k, latch_status[k] ? "true" : "false");
            }

            uint8_t last_relay_mask = last_report[8];
            for (int k = 0; k < relay_out.size(); k++) {
                relay_status[k] = (last_relay_mask & (1 << k)) > 0;
                ESP_LOGI(TAG, "R[%d] last status: %s", k, relay_status[k] ? "true" : "false");
            }

            //Note: no need to recover extensions outputs as they mantain status during restart!
        }
        // cleaning up
        result = storage_db_remove(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_report");
        if (result == ESP_OK) {
            ESP_LOGW(TAG, "Unable to remove <<<last_report>>>!");
        }


        for (int k = 0; k < latch_out.size(); k++) {
            if (esp32_io_latch_cmd(k, latch_status[k]) < 0) {
                ESP_LOGE(TAG, "Error occurred during latch initialization");
                abort();
            }
        }
        ESP_LOGI(TAG, "Latching up...");

        for (int k = 0; k < relay_out.size(); k++) {
            if (esp32_io_relay_cmd(k, relay_status[k]) < 0) {
                ESP_LOGE(TAG, "Error occurred durnig relay initialization");
                abort();
            }
        }

        gpio_config_t digital_config = {
            .pin_bit_mask = 0,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = gpio_pullup_t::GPIO_PULLUP_DISABLE,
            .pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE,
            .intr_type = gpio_int_type_t::GPIO_INTR_NEGEDGE,
        };
        for (int k = 0; k < digital_in.size(); k++) {
            digital_config.pin_bit_mask |= (1ULL << digital_in[k]);
        }


        result = gpio_config(&digital_config);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during digital input configuration!");
            return result;
        }
        ESP_LOGI(TAG, "Digital up...");


        result = esp32_io_pulse_init();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during pulse input configuration!");
            return result;
        }


        #ifdef IOT_BOARD_USES_INTERNAL_ADC
            for (int k = 0; k < a_current_in.size(); k++) {
                result = adc_service_calibration_init(static_cast<gpio_num_t>(a_current_in[k]), ADC_ATTEN_DB_12, 
                        &(a_current_calib[k]));
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "C[%d] Error during calibration: %d", k, result);
                    return result;
                }
            }

            for (int k = 0; k < a_volt_in.size(); k++) {
                result = adc_service_calibration_init(static_cast<gpio_num_t>(a_volt_in[k]), static_cast<adc_atten_t>(a_volt_atten[k]),
                        &(a_volt_calib[k]));
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "V[%d] Error during calibration: %d", k, result);
                    return result;
                }
            }

            ESP_LOGI(TAG, "Analog inputs calibrated...");
        #endif

        #ifdef IOT_BOARD_TEMPERATURE_USES_LM75D
            result = i2c_bus_register_write_byte(TEMPERATURE_ADDR, __LM75D_CONF_REGISTER, 
                        __LM75D_NORMAL_BEHAVIOUR | __LM75D_SHUTDOWN, __LM75D_DEFAULT_TIMEOUT);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during temperature sensor configuration!");
                return result;
            }
        #endif

        return ESP_OK;
    }

    esp_err_t io_deinit() {
        ESP_LOGI(TAG, "Closing up I/O interfaces...");

        esp_err_t result = esp32_io_pulse_deinit();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during pulse input configuration!");
            return result;
        }
        ESP_LOGI(TAG, "Pulse interface closed...");


        #ifdef IOT_BOARD_USES_INTERNAL_ADC
            for (int k = 0; k < a_current_in.size(); k++) {
                adc_service_calibration_deinit(a_current_calib[k]);
            }

            for (int k = 0; k < a_volt_in.size(); k++) {
                adc_service_calibration_deinit(a_volt_calib[k]);
            }

            ESP_LOGI(TAG, "Analog inputs calibration destroyed...");
        #endif

        return ESP_OK;
    }

    esp_err_t io_led_cmd(uint8_t led, bool status) {
        if (led > led_out.size()) {
            ESP_LOGE(TAG, "Led[%d] not defined!", led);
            return ESP_ERR_INVALID_ARG;
        }

        xSemaphoreTake(io_mutex, portMAX_DELAY);

        led_status[led] = status;
        gpio_set_level(static_cast<gpio_num_t>(led_out[led]), status ? 1 : 0);

        xSemaphoreGive(io_mutex);

        ESP_LOGI(TAG, "Led[%d]: %s", led, status ? "on" : "off");

        return ESP_OK;
    }
    
    bool io_led_status(uint8_t led) {
        if (led > led_out.size()) {
            ESP_LOGE(TAG, "Led[%d] not defined!", led);
            return false;
        }
        xSemaphoreTake(io_mutex, portMAX_DELAY);
        bool status = led_status[led];
        xSemaphoreGive(io_mutex);

        return status;
    }

    esp_err_t io_relay_cmd(uint8_t relay, bool status) {
        return esp32_io_relay_cmd(relay, status);
    }

    bool io_relay_status(uint8_t relay) {
        if (relay > relay_out.size()) {
            ESP_LOGE(TAG, "Relay[%d] not defined!", relay);
            return false;
        }
        xSemaphoreTake(io_mutex, portMAX_DELAY);
        bool status = relay_status[relay];
        xSemaphoreGive(io_mutex);

        return status;
    }

    esp_err_t io_latch_cmd(uint8_t latch, bool status) {
        return esp32_io_latch_cmd(latch, status);
    }

    bool io_latch_status(uint8_t latch) {
        if (latch > latch_out.size()) {
            ESP_LOGE(TAG, "Latch[%d] not defined!", latch);
            return false;
        }
        xSemaphoreTake(io_mutex, portMAX_DELAY);
        bool status = latch_status[latch];
        xSemaphoreGive(io_mutex);

        return status;
    }


    esp_err_t io_latch_refresh() {
        ESP_LOGI(TAG, "Starting latch status refresh...");

        esp_err_t result;
        for (size_t k = 0; k < latch_out.size(); k++) {
            result = esp32_io_latch_cmd(k, latch_status[k]);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Latch[%d] error occurred: %d", k, result);
                return result;
            }
        }
        
        ESP_LOGI(TAG, "Latch output refreshed");

        return ESP_OK;
    }

    bool io_digital_input_status(uint8_t input) {
        return esp32_io_digital_get(input);
    }

    uint16_t io_pulse_status(uint8_t input) {
        
        xSemaphoreTake(io_mutex, portMAX_DELAY);
        uint16_t value = esp32_io_pulse_get(input);
        xSemaphoreGive(io_mutex);

        return value;
    }

    float io_voltage_input_status(uint8_t input) {
        return esp32_io_analog_voltage_get(input, a_volt_scales[input]); // mV
    }

    float io_current_input_status(uint8_t input) {
        return esp32_io_analog_current_get(input, ADC_C_SENSE_RESISTOR); //mA
    }



    esp_err_t io_telem_report(bool save_pulses) {
        uint8_t telem_buf[io_buffer_report_size];

        esp_err_t result = io_buffer_report(telem_buf, io_buffer_report_size, save_pulses);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Report generation error: %d", result);
            return result;
        }

        esp32_io_telem_emit(telem_buf, io_buffer_report_size);
        return ESP_OK;
    }

    esp_err_t io_buffer_report(uint8_t *buffer, int buffer_size, bool save_pulses) {
        if (buffer_size < io_buffer_report_size) {
            ESP_LOGE(TAG, "Buffer is too small!");
            return ESP_ERR_INVALID_SIZE;
        }
        
        memset(buffer, 0U, buffer_size);
            
        uint8_t *a_curr_base_addr = buffer + 5 * sizeof(uint32_t);
        
        uint8_t *a_volt_base_addr = a_curr_base_addr + a_current_in.size() * sizeof(uint16_t);
        
        uint8_t *pulse_base_addr = a_volt_base_addr + a_volt_in.size() * sizeof(uint16_t);

        uint8_t *temperature_base_addr = pulse_base_addr + pulse_in.size() * sizeof(uint16_t);


        static_assert(EDGE_SREV < 16);
        static_assert(EDGE_SMINOR < 16);
        static_assert(CONFIG_IOT_BOARD_HREV < 256);
        static_assert(io_n_current < 16);
        static_assert(io_n_voltage < 16);
        static_assert(io_n_pulse < 16);
        static_assert(io_n_temperature < 16);
        static_assert(io_n_latch < 256);
        static_assert(io_n_relay < 256);
        static_assert(io_n_digital < 256);

        // 1st DWORD
        buffer[0] = CONFIG_IOT_BOARD_HREV;
        buffer[1] = EDGE_SREV << 4 | EDGE_SMINOR;
        buffer[2] = io_n_current << 4 | io_n_voltage;
        buffer[3] = io_n_pulse << 4 | io_n_temperature;

        // 2st DWORD
        buffer[4] = io_n_latch;
        buffer[5] = io_n_relay;
        buffer[6] = io_n_digital;
        buffer[7] = 0U;


        xSemaphoreTake(io_mutex, portMAX_DELAY);

        // Latch 
        uint32_t latch = 0;
        for (int k = 0; k < latch_out.size(); k++) {
            if (latch_status[k])
                latch |= (1 << k);
        }
        if (!clab::iot_services::is_little_endian())
            latch = clab::iot_services::swap_uint32(latch);
        memcpy(&(buffer[8]), &latch, sizeof(uint32_t));

        // Relay
        uint32_t relay = 0;
        for (int k = 0; k < relay_out.size(); k++) {
            if (relay_status[k])
                relay |= (1 << k);
        }
        if (!clab::iot_services::is_little_endian())
            relay = clab::iot_services::swap_uint32(relay);
        memcpy(&(buffer[12]), &relay, sizeof(uint32_t));

        xSemaphoreGive(io_mutex);

        // Digital inputs
        uint32_t digital = 0;
        for (int k = 0; k < digital_in.size(); k++) {
            if (esp32_io_digital_get(k))
                digital |= (1 << k);
        }
        if (!clab::iot_services::is_little_endian())
            digital = clab::iot_services::swap_uint32(digital);
        memcpy(&(buffer[16]), &digital, sizeof(uint32_t));

        // Analog current inputs
        for (int k = 0; k < a_current_in.size(); k++) {
            auto value = (uint16_t)(
                    std::floor(std::max(0.0f, esp32_io_analog_current_get(k, ADC_C_SENSE_RESISTOR)) * 1000.0f)); //uA
            
            ESP_LOGD(TAG, "C[%d]: %d uA", k, value);
            
            if (!clab::iot_services::is_little_endian())
                value = clab::iot_services::swap_uint16(value);

            memcpy(a_curr_base_addr + k * sizeof(uint16_t), &value, sizeof(uint16_t));
        }

        // Analog voltage inputs
        for (int k = 0; k < a_volt_in.size(); k++) {
            auto value = (uint16_t)(
                    std::floor(std::max(0.0f, esp32_io_analog_voltage_get(k, a_volt_scales[k])))); //mV
            
            ESP_LOGD(TAG, "V[%d]: %d mV", k, value);

            
            if (!clab::iot_services::is_little_endian())
                value = clab::iot_services::swap_uint16(value);

            memcpy(a_volt_base_addr + k * sizeof(uint16_t), &value, sizeof(uint16_t));
        }

        // Pulse inputs
        xSemaphoreTake(io_mutex, portMAX_DELAY);

        for (int k = 0; k < pulse_in.size(); k++) {
            auto value = esp32_io_pulse_get(k);

            ESP_LOGD(TAG, "P[%d]: %d", k, value);
            
            if (save_pulses)
                esp32_io_pulse_save(k, value);

            if (!clab::iot_services::is_little_endian())
                value = clab::iot_services::swap_uint16(value);

            memcpy(pulse_base_addr + k * sizeof(uint16_t), &value, sizeof(uint16_t));
        }

        xSemaphoreGive(io_mutex);

        for (int k = 0; k < io_n_temperature; k++) {

            auto value = (uint16_t)(
                    std::floor(std::max(0.0f, esp32_io_temperature_get(k) * 100.0f))); //100 * K
        
            ESP_LOGD(TAG, "T[%d]: %f K", k, value / 100.0f);

            
            if (!clab::iot_services::is_little_endian())
                value = clab::iot_services::swap_uint16(value);

            memcpy(temperature_base_addr + k * sizeof(uint16_t), &value, sizeof(uint16_t));
        }
        
        return ESP_OK;
    }

    void io_telem_register_callback(void (*method)(const uint8_t *payload, int payload_size)) {
        ESP_LOGI(TAG, "Registered telem callback...");
        telem_callback = method;
    }

    esp_err_t io_pulse_filter_set(uint16_t *delays, size_t delays_size) {
        if (delays_size != io_n_pulse) {
            ESP_LOGE(TAG, "delays dimension mismatch!");
            return ESP_ERR_INVALID_SIZE;
        }
        
        #if IOT_BOARD_N_PULSE
            xSemaphoreTake(io_mutex, portMAX_DELAY);
            for (int k = 0; k < IOT_BOARD_N_PULSE; k++) {
                pulse_disable[k] = delays[k];
            }
            xSemaphoreGive(io_mutex);
        #endif

        return ESP_OK;
    }

    esp_err_t io_pulse_filter_set(size_t idx, uint16_t delay) {
        if (idx >= io_n_pulse) {
            ESP_LOGE(TAG, "idx out of bounds!");
            return ESP_ERR_INVALID_SIZE;
        }
        
        #if IOT_BOARD_N_PULSE
            xSemaphoreTake(io_mutex, portMAX_DELAY);
            pulse_disable[pulse_in[idx]] = delay;
            xSemaphoreGive(io_mutex);
        #endif

        return ESP_OK;
    }

    bool io_check_output_active() {

        xSemaphoreTake(io_mutex, portMAX_DELAY);

        #ifdef IOT_BOARD_RELAY_MACRO
            for (int k = 0; k < relay_out.size(); k++) {
                if (relay_status[k])
                    return true;
            }
        #endif
        
        for (int k = 0; k < latch_out.size(); k++) {
            if (latch_status[k])
                return true;
        }

        xSemaphoreGive(io_mutex);

        return false;
    }


    ///-------------------------- private implementation:
    uint16_t esp32_io_pulse_get(uint8_t pulse) {
        if (pulse > pulse_in.size()) {
            ESP_LOGE(TAG, "P[%d] outside of bounds, ignoring...", pulse);
            return 0;
        }
        return irq_counts[pulse_in[pulse]];
    }

    esp_err_t esp32_io_pulse_save(uint8_t pulse, uint16_t value) {
        char pulse_key_buf[8];
        snprintf(pulse_key_buf, 8, "p%u", pulse);
        return storage_db_set(CONFIG_IOT_IO_STORAGE_NAMESPACE, pulse_key_buf, ((const char *)&value), sizeof(uint16_t));
    }

    uint16_t esp32_io_pulse_restore(uint8_t pulse) {
        char pulse_key_buf[8];
        snprintf(pulse_key_buf, 8, "p%u", pulse);

        uint16_t pulse_value = 0;
        size_t read_bytes = 0;
        esp_err_t result = storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, pulse_key_buf, ((char *)&pulse_value), sizeof(uint16_t), &read_bytes);
        if (result != ESP_OK || read_bytes != sizeof(uint16_t)) {
            ESP_LOGW(TAG, "P[%d] not in memory, returning 0", pulse);
            return 0;
        }
        ESP_LOGI(TAG, "P[%d] restored: %u", pulse, pulse_value);
        return pulse_value;
    }

    esp_err_t esp32_io_pulse_filter_restore() {
        #if IOT_BOARD_N_PULSE
            char        buffer[clab::iot_services::alligned_big_enough(sizeof(uint16_t) * io_n_pulse * 2)];
            size_t      read_bytes = 0;
            size_t      written_bytes = 0;
            
            uint16_t    value;

            uint16_t    disables_defaults[IOT_BOARD_N_PULSE] = IOT_BOARD_PULSE_DISABLE_DEFAULTS_MILLIS;
            int cnt = 0;
            for (auto const &key : IOT_BOARD_PULSE_MACRO) {
                pulse_disable.emplace(key, disables_defaults[cnt]);
                cnt++;
            }

            cnt = 0;
            for (auto const &key : IOT_BOARD_PULSE_MACRO) {
                char key_buf[8];
                snprintf(key_buf, 8, "pf%d", cnt);
            
                if (clab::iot_services::storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, key_buf, (char *)buffer, sizeof(buffer), &read_bytes) == ESP_OK) {
                    ESP_LOGI(TAG, "Founded setting \"%s\" - size: %u", key_buf, read_bytes);

                    if (mbedtls_base64_decode((unsigned char *)&value, sizeof(uint16_t), &written_bytes, 
                        (unsigned char *)buffer, read_bytes) == 0) {
                        
                        if (!clab::iot_services::is_little_endian())
                            value = clab::iot_services::swap_uint16(value);

                        pulse_disable[key] = value;

                        ESP_LOGI(TAG, "P[%d] delay is: %u ms", cnt, pulse_disable[key]);
                        
                    }
                }
                cnt++;
            }

        #endif

        return ESP_OK;
    }
          

    esp_err_t esp32_io_pulse_init() {
        #ifdef IOT_BOARD_PULSE_MACRO
            ESP_LOGI(TAG, "Starting up pulse interface...");

            // Init IRQ pins and attach callbacks
            // NOTE: .first holds the channel pin and .second holds the counter
            int pulse_cnt = 0;
            esp_err_t result;

            for(auto const& key : IOT_BOARD_PULSE_MACRO) {
                irq_counts.emplace(key, 0U);
                last_irq.emplace(key, esp_timer_get_time());
            }

            //install gpio isr service
            result = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during isr configuration!");
                return result;
            }

            result = esp32_io_pulse_filter_restore();
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during pulse filter configuration!");
                return result;
            }

            for (const auto& irq : irq_counts) {
                // Init pins
                #ifdef IOT_BOARD_PULSE_RESTORE
                    irq_counts[irq.first] = esp32_io_pulse_restore(pulse_cnt);
                #else 
                    irq_counts[irq.first] = 0U;
                #endif
                
                

                gpio_config_t config = {
                    .pin_bit_mask = (1ULL << irq.first),
                    .mode = GPIO_MODE_INPUT,
                    .pull_up_en = gpio_pullup_t::GPIO_PULLUP_DISABLE,
                    .pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE,
                    .intr_type = gpio_int_type_t::GPIO_INTR_NEGEDGE,
                };

                result = gpio_config(&config);
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Error during pulse input configuration!");
                    return result;
                }

                // Create a type alias helper
                using IrqCount = std::pair<const uint8_t, std::atomic<uint16_t>>;

                // Define the IRQ callback as lambda function
                // Will receive an entry from the irqCounts map:
                auto isr = [](void* arg) { 
                    IrqCount * ic = (IrqCount *)arg; 
                    if (esp_timer_get_time() - last_irq[(*ic).first] >= 1000 * pulse_disable[(*ic).first]) {
                        (*ic).second++; 
                        last_irq[(*ic).first] = esp_timer_get_time();
                    }
                };

                pulse_cnt++;

                // attach the callback passing the current map entry as parameter
                result = gpio_isr_handler_add(static_cast<gpio_num_t>(irq.first), isr, (void*)&irq);
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Error during pulse input handler configuration!");
                    return result;
                }
            }

            ESP_LOGI(TAG, "Pulse inputs up...");
        #endif
        return ESP_OK;
    }

    esp_err_t esp32_io_pulse_deinit() {
        #ifdef IOT_BOARD_PULSE_MACRO
            esp_err_t result;
            for (const auto& irq : irq_counts) {
                result = gpio_isr_handler_remove(static_cast<gpio_num_t>(irq.first));
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Error during pulse input handler configuration!");
                    return result;
                }

                gpio_config_t config = {
                    .pin_bit_mask = (1UL << irq.first),
                    .mode = GPIO_MODE_INPUT,
                    .pull_up_en = gpio_pullup_t::GPIO_PULLUP_ENABLE,
                    .pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE,
                    .intr_type = gpio_int_type_t::GPIO_INTR_DISABLE,
                };

                result = gpio_config(&config);
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Error during pulse input configuration!");
                    return result;
                }
            }

            //install gpio isr service
            gpio_uninstall_isr_service();

            ESP_LOGI(TAG, "Pulse inputs down...");
        #endif

        return ESP_OK;
    }

    bool esp32_check_relay_exists(uint8_t relay) {
        if (relay < relay_out.size())
            return true;

        return false;
    }

    esp_err_t esp32_io_relay_cmd(uint8_t relay, bool status) {
        if (!esp32_check_relay_exists(relay)) {
            ESP_LOGE(TAG, "Relay[%d] outside of bounds, ignoring...", relay);
            return ESP_ERR_INVALID_ARG;
        }
        
        ESP_LOGI(TAG, "Relay[%d] new status: %s", relay, status ? "on" : "off");

        if (relay < relay_out.size()) {
            
            xSemaphoreTake(io_mutex, portMAX_DELAY);

            relay_status[relay] = status;
            gpio_set_level(static_cast<gpio_num_t>(relay_out[relay]), status ? 1 : 0);

            xSemaphoreGive(io_mutex);

            ESP_LOGI(TAG, "Relay[%d] done", relay);
        }

        return ESP_OK;
    }

    bool esp32_check_latch_exists(uint8_t latch) {
        if (latch < latch_out.size())
            return true;

        return false;
    }

    esp_err_t esp32_io_latch_cmd(uint8_t latch, bool status) {
        if (!esp32_check_latch_exists(latch)) {
            ESP_LOGE(TAG, "Latch[%d] outside of bounds, ignoring...", latch);
            return ESP_ERR_INVALID_ARG;
        }

        ESP_LOGI(TAG, "Latch[%d] new status: %s", latch, status ? "on" : "off");

        if (latch < latch_out.size()) {
            xSemaphoreTake(io_mutex, portMAX_DELAY);

            latch_status[latch] = status;
            if (status) {
                gpio_set_level(static_cast<gpio_num_t>(std::get<1>(latch_out[latch])), 0);
                gpio_set_level(static_cast<gpio_num_t>(std::get<0>(latch_out[latch])), 1);
            }
            else {
                gpio_set_level(static_cast<gpio_num_t>(std::get<0>(latch_out[latch])), 0);
                gpio_set_level(static_cast<gpio_num_t>(std::get<1>(latch_out[latch])), 1);
            }
            // generate a strobe
            vTaskDelay(pdMS_TO_TICKS(IOT_BOARD_STROBE_DURATION));

            //turn off
            gpio_set_level(static_cast<gpio_num_t>(std::get<0>(latch_out[latch])), 0);
            gpio_set_level(static_cast<gpio_num_t>(std::get<1>(latch_out[latch])), 0);

            xSemaphoreGive(io_mutex);

        }

        return ESP_OK;
    }

    void esp32_io_telem_emit(const uint8_t *payload, int payload_size) {
        if (telem_callback != nullptr) {
            ESP_LOGI(TAG, "Emitting telemetry: ");
            ESP_LOG_BUFFER_HEX_LEVEL(TAG, payload, payload_size, ESP_LOG_INFO);

            telem_callback(payload, payload_size);
        }
    }

    bool esp32_io_digital_get(uint8_t input) {
        if (input > digital_in.size()) {
            ESP_LOGE(TAG, "DIN[%d] not defined!", input);
            return false;
        }

        auto value = gpio_get_level(static_cast<gpio_num_t>(digital_in[input]));
        if (digital_invert[input])
            value = value * -1 + 1;

        return value;
    }

    float esp32_io_analog_current_get(uint8_t input, float resistor) {
        if (input > a_current_in.size()) {
            ESP_LOGE(TAG, "C[%d] outside of bounds, ignoring...", input);
            return NAN;
        }
        
        #if defined(IOT_BOARD_USES_I2C_ADC) && defined(IOT_BOARD_CURRENT_I2C_ADDR_MACRO)
            float v_read = 0;
            esp_err_t result = adc_i2c_service_measure_voltage(a_current_in[input], ADC_N_SAMPLE_DEFAULT, v_read);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "C[%d]: error occurred during measurement!", input);
                return NAN;
            }

            v_read = v_read / resistor;
            ESP_LOGD(TAG, "C[%d] reading mA: %f", input, v_read);
            return v_read;
        #elif defined(IOT_BOARD_CURRENT_MACRO)
            float v_read = adc_service_measure_voltage(static_cast<gpio_num_t>(a_current_in[input]), a_current_calib[input], ADC_N_SAMPLE_DEFAULT);
            
            v_read = v_read / resistor;
            ESP_LOGD(TAG, "C[%d] reading mA: %f", input, v_read);
            return v_read;
        #else
            ESP_LOGE(TAG, "This code should not be reachable!");
            return NAN;
        #endif
    }

    float esp32_io_analog_voltage_get(uint8_t input, float scale) {
        if (input > a_volt_in.size()) {
            ESP_LOGE(TAG, "V[%d] outside of bounds, ignoring...", input);
            return NAN;
        }

        #if defined(IOT_BOARD_USES_I2C_ADC) && defined(IOT_BOARD_VOLTAGE_I2C_ADDR_MACRO)
            float v_read = 0;
            esp_err_t result = adc_i2c_service_measure_voltage(a_volt_in[input], ADC_N_SAMPLE_DEFAULT, v_read);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "V[%d]: error occurred during measurement!", input);
                return NAN;
            }

            v_read *= scale;
            ESP_LOGD(TAG, "V[%d] reading mV: %f", input, v_read);
            return v_read;
        #elif defined(IOT_BOARD_VOLTAGE_MACRO)
            float v_read = adc_service_measure_voltage(static_cast<gpio_num_t>(a_volt_in[input]), a_volt_calib[input], ADC_N_SAMPLE_DEFAULT);
            
            v_read *= scale;
            ESP_LOGD(TAG, "V[%d] reading mV: %f", input, v_read);
            return v_read;
        #else
            ESP_LOGE(TAG, "This code should not be reachable!");
            return NAN;
        #endif

        // v_read = max(0, min(v_read, 5.0)); //max voltage = 20mA * 220Omh
    }

    #ifdef IOT_BOARD_TEMPERATURE_USES_LM75D
        int16_t lm75d_i2c_read_value(uint8_t msb, uint8_t lsb) {
            int16_t to_ret = (msb << 8 ) | 
                    (lsb & __LM75D_VALUES_LSB_CONVERSION_MASK);
            return to_ret >> 5;
        }
    #endif

    float esp32_io_temperature_get(uint8_t input) {
        if (input > temp_in.size()) {
            ESP_LOGE(TAG, "T[%d] outside of bounds, ignoring...", input);
            return NAN;
        }
        
        #if defined(IOT_BOARD_USES_I2C_TEMPERATURE) && defined(IOT_BOARD_TEMPERATURE_I2C_ADDR_MACRO)
            #ifdef IOT_BOARD_TEMPERATURE_USES_LM75D
                esp_err_t result;
                result = i2c_bus_register_write_byte(temp_in[input], __LM75D_CONF_REGISTER, 
                        __LM75D_NORMAL_BEHAVIOUR, __LM75D_DEFAULT_TIMEOUT);
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Error during temperature sensor configuration!");
                    return result;
                }

                vTaskDelay(pdMS_TO_TICKS(__LM75D_WAIT_MEASURE_MILLIS));

                uint8_t buffer[2];
                result = i2c_bus_register_read(temp_in[input], 
                        __LM75D_TEMP_REGISTER, buffer, sizeof(buffer), __LM75D_DEFAULT_TIMEOUT);
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Error during temperature reading!");
                    return result;
                }

                result = i2c_bus_register_write_byte(temp_in[input], __LM75D_CONF_REGISTER, 
                    __LM75D_NORMAL_BEHAVIOUR | __LM75D_SHUTDOWN, __LM75D_DEFAULT_TIMEOUT);
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Error during temperature sensor configuration!");
                    return result;
                }

                float to_ret = lm75d_i2c_read_value(buffer[0], buffer[1]) * __LM75D_VALUES_SCALE_FACTOR;
                return to_ret + 273.15f;
            #endif
        #else
            return NAN;
        #endif

    }

}

#include "plugin/startup.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "iot_services/i2c_bus_service.h"

#if defined(EDGE_BOARD_USES_I2C_ADC)
    #include "iot_services/adc_i2c_service.h"
#else
    #include "iot_services/adc_service.h"
#endif

#include "iot_services/rtc_service.h"
#include "iot_services/storage_service.h"
#include "iot_services/io_service.h"

static const char *TAG = "plugins::startup";


namespace clab::plugins {

    TaskHandle_t power_opt_task_handle = NULL;
    
    esp_err_t startup_plugin() {
        ESP_ERROR_CHECK(clab::iot_services::i2c_bus_init());

        #if defined(EDGE_BOARD_USES_I2C_ADC)
            ESP_ERROR_CHECK(irreo::iot_services::adc_i2c_service_init());
        #else
            adc_service_init();
        #endif
        ESP_LOGI(TAG, "ADC service intialized...");
        
        ESP_ERROR_CHECK(clab::iot_services::storage_init());
        ESP_LOGI(TAG, "Storage service intialized...");
        ESP_ERROR_CHECK(clab::iot_services::rtc_init());
        ESP_LOGI(TAG, "RTC service intialized...");
        ESP_ERROR_CHECK(clab::iot_services::io_init());
        ESP_LOGI(TAG, "I/O service intialized...");

        return ESP_OK;
    }
}
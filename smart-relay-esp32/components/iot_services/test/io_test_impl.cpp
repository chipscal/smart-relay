#include <stdio.h>
#include <inttypes.h>
#include "unity.h"
#include "string.h"
#include "esp_log.h"
#include "esp_system.h"
#include "iot_services/adc_service.h"
#include "iot_services/storage_service.h"
#include "iot_services/i2c_bus_service.h"
#include "iot_services/io_service.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "iot_services::io_tests";

#ifdef __cplusplus
extern "C" {
#endif

void io_test_impl() {
    adc_service_init();
    ESP_LOGI(TAG, "ADC service intialized...");

    esp_err_t result;

    result = clab::iot_services::storage_init();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Storage service intialized...");

    result = clab::iot_services::i2c_bus_init();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "I2C bus service intialized...");
    

    result = clab::iot_services::io_init();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "I/O service intialized...");

    clab::iot_services::io_led_cmd(0, true);
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Led[0] switched on...");

    auto led_status = clab::iot_services::io_led_status(0);
    TEST_ASSERT(led_status == true);
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (clab::iot_services::io_n_latch > 0) {
        for (int k = 0; k < clab::iot_services::io_n_latch; k++) {
            result = clab::iot_services::io_latch_cmd(k, true);
            TEST_ASSERT(result == ESP_OK);
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        for (int k = 0; k < clab::iot_services::io_n_latch; k++) {
            result = clab::iot_services::io_latch_cmd(k, false);
            TEST_ASSERT(result == ESP_OK);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        ESP_LOGI(TAG, "Latch test passed...");
    }


    for (int k = 0; k < clab::iot_services::io_n_relay; k++) {
        result = clab::iot_services::io_relay_cmd(k, true);
        TEST_ASSERT(result == ESP_OK);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    for (int k = 0; k < clab::iot_services::io_n_relay; k++) {
        result = clab::iot_services::io_relay_cmd(k, false);
        TEST_ASSERT(result == ESP_OK);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    ESP_LOGI(TAG, "Relay test passed...");


    auto telem_callback = [](const uint8_t *payload, int payload_size) {
        TEST_ASSERT(payload_size == clab::iot_services::io_buffer_report_size);
        ESP_LOG_BUFFER_HEX(TAG, payload, payload_size);
    };

    clab::iot_services::io_telem_register_callback(telem_callback);

    result = clab::iot_services::io_telem_report(true, true);
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Telemetry test passed...");

    clab::iot_services::io_led_cmd(0, false);
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Led[0] switched off...");

    result = clab::iot_services::io_deinit();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "I/O service deintialized...");

    result = clab::iot_services::i2c_bus_deinit();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "I2C bus service intialized...");

    result = clab::iot_services::storage_deinit();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Storage service deintialized...");

    adc_service_deinit();
    ESP_LOGI(TAG, "ADC service deintialized...");
}

#ifdef __cplusplus
}
#endif
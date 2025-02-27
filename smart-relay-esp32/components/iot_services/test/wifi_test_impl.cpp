#include <stdio.h>
#include <inttypes.h>
#include "unity.h"
#include "string.h"
#include "esp_log.h"
#include "esp_system.h"
#include "iot_services/storage_service.h"
#include "iot_services/wifi_service.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "iot_services::wifi_tests";

#ifdef __cplusplus
extern "C" {
#endif

void wifi_test_impl() {
    
    esp_err_t result;
    result = clab::iot_services::storage_init();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Storage intialized...");

    result = clab::iot_services::wifi_init();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Wifi intialized...");

    vTaskDelay(pdMS_TO_TICKS(10000));
    
    result = clab::iot_services::wifi_deinit();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Wifi deintialized...");

    result = clab::iot_services::storage_deinit();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Storage deintialized...");
}

#ifdef __cplusplus
}
#endif
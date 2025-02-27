#include "unity.h"
#include "esp_log.h"
#include "iot_services/storage_service.h"
#include "iot_services/rtc_service.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



static const char *TAG = "iot_services::rtc_tests";

#ifdef __cplusplus
extern "C" {
#endif

void rtc_service_test_impl() {

    esp_err_t result;
    result = clab::iot_services::storage_init();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Storage service intialized...");

    result = clab::iot_services::rtc_init();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "RTC service intialized...");

    uint32_t to_set = 1698331467;
    result = clab::iot_services::rtc_set_utc(to_set);
    TEST_ASSERT(result == ESP_OK);

    auto value = clab::iot_services::rtc_get_utc();
    TEST_ASSERT(value - to_set < 2);
}

#ifdef __cplusplus
}
#endif
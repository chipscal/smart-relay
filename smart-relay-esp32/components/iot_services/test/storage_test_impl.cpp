#include <stdio.h>
#include <inttypes.h>
#include "unity.h"
#include "string.h"
#include "esp_log.h"
#include "esp_system.h"
#include "iot_services/storage_service.h"



static const char *TAG = "iot_services::storage_tests";

#ifdef __cplusplus
extern "C" {
#endif

void storage_test_impl() {
    
    const char *namespace_name = "dgdhtjklgllg";
    const char *key = "one";
    const char *payload = "helloo0000ooo";
    int payload_size = strlen(payload) + 1;
     
    esp_err_t result;
    result = clab::iot_services::storage_init();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Storage intialized...");

    result = clab::iot_services::storage_db_set(namespace_name, key, payload, payload_size);
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Storage intialized...");

    size_t data_received_size = 0;
    result = clab::iot_services::storage_db_get(namespace_name, key, NULL, 0, &data_received_size);
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Data size: %d", data_received_size);

    char *buffer = (char *)malloc(data_received_size * sizeof(char));
    result = clab::iot_services::storage_db_get(namespace_name, key, buffer, data_received_size, &data_received_size);
    TEST_ASSERT(result == ESP_OK);
    TEST_ASSERT(data_received_size > 0);
    TEST_ASSERT(strcmp(payload, buffer) == 0);
    ESP_LOGI(TAG, "Retrived data match the input");

    result = clab::iot_services::storage_deinit();
    TEST_ASSERT(result == ESP_OK);
    ESP_LOGI(TAG, "Storage deintialized...");

    free(buffer);
}

#ifdef __cplusplus
}
#endif
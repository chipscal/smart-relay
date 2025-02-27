#include "nvs.h"
#include <stdio.h>
#include "esp_log.h"
#include <inttypes.h>
#include "nvs_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iot_services/storage_service.h"

static const char *TAG = "iot_services::storage_service";

namespace clab::iot_services {

    esp_err_t storage_init() {

        esp_err_t result = nvs_flash_init();
        
        if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            result = nvs_flash_init();
        }    
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "NVM storage initialized.");
        }
        return result;
    }

    esp_err_t storage_deinit() {

        esp_err_t result = nvs_flash_deinit();    
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during deinit (%s)", esp_err_to_name(result));
            return result;
        }
        ESP_LOGI(TAG, "NVM storage deinitialized.");
        return result;
    }

    esp_err_t storage_db_set(const char *namespace_name, const char *key, const char *payload, size_t payload_size) {

        nvs_handle_t handle;

        esp_err_t result = nvs_open(namespace_name, NVS_READWRITE, &handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error opening NVS namespace '%s'", namespace_name);
            return result;
        }

        result = nvs_set_blob(handle, key, payload, payload_size);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error writing key '%s' to NVS (%s)", key, esp_err_to_name(result));
            nvs_close(handle);
            return result;
        }

        result = nvs_commit(handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during commit namespace '%s'", namespace_name);
            nvs_close(handle);
            return result;
        }

        nvs_close(handle);
        return ESP_OK;
    }

    esp_err_t storage_db_get(const char *namespace_name, const char *key, char *buffer, size_t buffer_size, size_t *out_size) {
        
        nvs_handle_t handle;

        esp_err_t result = nvs_open(namespace_name, NVS_READONLY, &handle);
        if (result != ESP_OK) {    
            ESP_LOGE(TAG, "Error opening NVS namespace '%s'", namespace_name);
            return result;
        }

        *out_size = buffer_size;
        result = nvs_get_blob(handle, key, buffer != NULL ? buffer : NULL, out_size);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error getting key '%s' (%s)", key, esp_err_to_name(result));
            nvs_close(handle);
            return result;
        }

        nvs_close(handle);
        return ESP_OK; 
    }

    esp_err_t storage_db_remove(const char *namespace_name, const char *key)
    { 
        nvs_handle_t handle;
        esp_err_t result = nvs_open(namespace_name, NVS_READWRITE, &handle);
        if (result != ESP_OK) {    
            ESP_LOGE(TAG, "Error opening NVS namespace '%s'", namespace_name);
            return result;
        }

        result = nvs_erase_key(handle, key);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during erasing key: %s", key);
            nvs_close(handle);
            return result;
        }
        
        result = nvs_commit(handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during commit namespace '%s'", namespace_name);
            nvs_close(handle);
            return result;
        }
        
        nvs_close(handle);
        return ESP_OK;
    }

    esp_err_t storage_db_erase(const char *namespace_name){
       
        nvs_handle_t handle;

        esp_err_t result = nvs_open(namespace_name, NVS_READWRITE, &handle);
        if (result != ESP_OK) {    
            ESP_LOGE(TAG, "Error opening NVS namespace '%s'", namespace_name);
            return result;
        }

        result = nvs_erase_all(handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during erasing db");
            nvs_close(handle);
            return result;
        }
        
        result = nvs_commit(handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during commit namespace '%s'", namespace_name);
            nvs_close(handle);
            return result;
        }
        
        nvs_close(handle);
        return ESP_OK;
    }
}


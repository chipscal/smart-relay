#pragma once

#include "esp_err.h"
#include <stdint.h>


namespace clab::iot_services
{ 
    /// @brief Initialize the NVM storage. 
    /// @return ESP_OK on success.
    /// @note this function is NOT thread safe and must me called only one time during initialization!
    esp_err_t storage_init();

    /// @brief Deinitialize the NVM storage. 
    /// @return ESP_OK on success.
    /// @note this function is NOT thread safe and must me called only one time during deinitialization!
    esp_err_t storage_deinit();

    /// @brief Get key/data value.
    /// @param namespace_name db
    /// @param key name
    /// @param buffer where to store retrieved data, if NULL no copy is made and only data size is returned
    /// @param buffer_size size of the buffer
    /// @param[out] out_size size of the stored data
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t storage_db_get(const char *namespace_name, const char *key, char *buffer, size_t buffer_size, size_t *out_size);

    /// @brief Set key/data value.
    /// @param namespace_name db
    /// @param key name
    /// @param payload data to be stored 
    /// @param payload_size size of the data.
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t storage_db_set(const char *namespace_name, const char *key, const char *payload, size_t payload_size);

    /// @brief Erase key/data value.
    /// @param key name
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t storage_db_remove(const char *namespace_name, const char *key);

    /// @brief Erases all keys in the namespace. 
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t storage_db_erase(const char *namespace_name);
}
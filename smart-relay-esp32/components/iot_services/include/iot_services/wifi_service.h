#pragma once

#include <stdint.h>
#include "esp_err.h"

#include "pinout.h"



namespace clab::iot_services { 

    /// @brief Init wifi service.
    /// @return ESP_OK on success.
    /// @note this function is NOT thread safe and must me called only one time during initialization!
    esp_err_t   wifi_init();

     /// @brief Deinitialize wifi service.
    /// @return ESP_OK on success.
    /// @note this function is NOT thread safe and must me called only one time during initialization!
    esp_err_t   wifi_deinit();

    /// @brief Get wifi ip.
    /// @param[out] address where to store address.
    /// @return ESP_OK on success.
    esp_err_t   wifi_get_ip(uint32_t *address);
}
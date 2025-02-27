#pragma once
#include "esp_err.h"

namespace clab::plugins {

    /// @brief Starts up base iot board services.
    /// @return ESP_OK if success.
    esp_err_t startup_plugin();

}
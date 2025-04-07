#pragma once
#include "esp_err.h"

namespace clab::plugins {

    /// @brief Starts up control plugin.
    /// @return ESP_OK if success.
    esp_err_t control_plugin();

}

#pragma once
#include "esp_err.h"


namespace clab::iot_services
{
    esp_err_t   rtc_init();

    esp_err_t   rtc_set_utc(uint32_t timestamp);

    esp_err_t   rtc_set_utc(const char *utc_timestring);

    uint32_t    rtc_get_utc();

    size_t      rtc_get_utc_timestring(char *buffer, size_t buffer_size);
}
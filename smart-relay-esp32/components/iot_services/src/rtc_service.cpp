#include "iot_services/rtc_service.h"
#include "iot_services/storage_service.h"

#include "esp_log.h"
#include "esp_err.h"

#include <time.h>
#include <sys/time.h>
#include <cstring>

static const char *TAG = "iot_services::rtc_service";

namespace clab::iot_services
{
    time_t  datetime_to_utc(const char *date, const char *time, bool local_time = true, int tz = 0);
    time_t  datetime_to_utc(const char *utc_timestring, bool local_time = true, int tz = 0);
    int     set_system_clock_from_build_time_rtc(const char *build_date, const char *build_time);
    

    esp_err_t rtc_init() {
        int result = set_system_clock_from_build_time_rtc(__DATE__, __TIME__);
        if (result < 0) {
            ESP_LOGE(TAG, "Error during rtc service initialization!");
            return ESP_FAIL;
        }

        uint32_t    last_rtc_value = 0;
        size_t      read_bytes = 0;

        esp_err_t storage_result = storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_rtc", ((char *)&last_rtc_value), sizeof(uint32_t), &read_bytes);
        if (storage_result == ESP_OK && read_bytes == sizeof(uint32_t)) {
            ESP_LOGI(TAG, "Found <<<last_rtc>>>: %lu", last_rtc_value);
            rtc_set_utc(last_rtc_value);
        }
        // cleaning up
        storage_result = storage_db_remove(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_rtc");
        if (storage_result == ESP_OK) {
            ESP_LOGW(TAG, "Unable to remove <<<last_rtc>>>!");
        }

        char buffer[32];
        rtc_get_utc_timestring(buffer, sizeof(buffer));
        ESP_LOGI(TAG, "RTC ts: %lu, RTC Date and Time: %s", rtc_get_utc(), buffer);

        return ESP_OK;
    }

    esp_err_t rtc_set_utc(uint32_t timestamp) {
        
        struct timeval now = { .tv_sec = timestamp };
        int result = settimeofday(&now, NULL);
        if (result < 0) {
            ESP_LOGE(TAG, "Error during rtc set!");
            return ESP_FAIL;
        }

        char buffer[32];
        rtc_get_utc_timestring(buffer, sizeof(buffer));
        ESP_LOGI(TAG, "RTC ts: %lu, RTC Date and Time: %s", rtc_get_utc(), buffer);

        return ESP_OK;
    }

    esp_err_t rtc_set_utc(const char *utc_timestring) {
        struct timeval now = { .tv_sec = datetime_to_utc(utc_timestring) };
        int result = settimeofday(&now, NULL);
        if (result < 0) {
            ESP_LOGE(TAG, "Error during rtc set!");
            return ESP_FAIL;
        }

        char buffer[32];
        rtc_get_utc_timestring(buffer, sizeof(buffer));
        ESP_LOGI(TAG, "RTC ts: %lu, RTC Date and Time: %s", rtc_get_utc(), buffer);

        return ESP_OK;
    }

    uint32_t rtc_get_utc() {
        return (uint32_t) time(NULL);
    }

    size_t rtc_get_utc_timestring(char *buffer, size_t buffer_size) {
        time_t now = time(0);
        tm *t = localtime(&now);
        
        size_t size = strftime(buffer, buffer_size, "%Y-%m-%dT%k:%M:%SZ", t);
        if (size == 0) {
            ESP_LOGE(TAG, "Error during timestamp format - buffer too small!");
        }

        return size;
    }

    ///-------------------------- private implementation:
    
    // Convert compile time to system time
    time_t datetime_to_utc(const char *date, const char *time, bool local_time, int tz) {
        char s_month[5];
        int year;

        tm t;
        time_t seconds;

        static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
        sscanf(date, "%s %d %d", s_month, &t.tm_mday, &year);
        sscanf(time, "%2d %*c %2d %*c %2d", &t.tm_hour, &t.tm_min, &t.tm_sec);

        // Find where is s_month in month_names. Deduce month value.
        t.tm_mon = (strstr(month_names, s_month) - month_names) / 3;
        t.tm_year = year - 1900;

        seconds = mktime(&t);

        if (!local_time) {
            if (tz > 200) {
                tz = 0x100 - tz; // Handle negative values
                seconds += (3600UL) * tz;
            } else {
                seconds -= (3600UL) * tz;
            }
        }

        return seconds;
    }

    // Convert utc timestring to system time
    time_t datetime_to_utc(const char *utc_timestring, bool local_time, int tz) {
        int year;

        tm t;
        time_t seconds;

        sscanf(utc_timestring, "%d-%d-%dT%d:%d:%dZ", &year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);

        t.tm_year = year - 1900;
        t.tm_mon = t.tm_mon - 1;

        seconds = mktime(&t);

        if (!local_time) {
            if (tz > 200) {
                tz = 0x100 - tz; // Handle negative values
                seconds += (3600UL) * tz;
            } else {
                seconds -= (3600UL) * tz;
            }
        }

        return seconds;
    }

    int set_system_clock_from_build_time_rtc(const char *build_date, const char *build_time) {
        // Retrieve clock time from compile date...
        auto build_date_time = datetime_to_utc(build_date, build_time, false, 2);
        // ... ore use the one from integrated RTC.
        auto rtcTime = time(NULL);

        // Remember to connect at least the CR2032 battery
        // to keep the RTC running.
        auto actual_time = rtcTime > build_date_time ? rtcTime : build_date_time;

        // Set both system time
        struct timeval now = { .tv_sec = actual_time };
        return settimeofday(&now, NULL);
    }
}
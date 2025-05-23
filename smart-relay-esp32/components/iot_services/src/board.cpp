#include "iot_services/board.h"
#include "iot_services/iot_services.h"
#include "iot_services/storage_service.h"
#include "iot_services/io_service.h"
#include "iot_services/rtc_service.h"
#include "iot_services/ctrl_service.h"

#include "esp_log.h"

static const char *TAG = "iot_services::board";


namespace clab::iot_services {
    
    void board_clean_restart() {
        ESP_LOGI(TAG, "Restart procedure initialized... Collecting status...");

        uint8_t buffer[io_buffer_report_size];
        esp_err_t result = io_buffer_report(buffer, io_buffer_report_size, true);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Report generation error: %d", result);
        }
        else {
            result = storage_db_set(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_report", (char *)buffer, io_buffer_report_size);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Unable to save report!");
            }
        }

        uint32_t last_rtc = rtc_get_utc();
        result = storage_db_set(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_rtc", (char *)(&last_rtc), sizeof(uint32_t));
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to save rtc!");
        }

        result = ctrl_save_status();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to save control status!");
        }

        ESP_LOGI(TAG, "Collecting status completed! Abort()...");
        abort();
    }

}
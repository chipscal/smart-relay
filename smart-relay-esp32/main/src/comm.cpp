#include "plugin/comm.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mbedtls/base64.h"

#include "string.h"
#include <string>
#include <array>

#include "iot_services/iot_services.h"
#include "iot_services/rtc_service.h"
#include "iot_services/storage_service.h"
#include "iot_services/wifi_service.h"
#include "iot_services/board.h"



#ifdef CONFIG_EDGE_LORA_SUPPORTED
    #include "iot_services/lora_service.h"
#elif CONFIG_EDGE_GSM_SUPPORTED
    #include "iot_services/gsm_service.h"
#endif

static const char *TAG = "plugins::comm";

namespace clab::plugins {


    esp_err_t comm_plugin() {

        esp_err_t result = clab::iot_services::wifi_init();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to init wifi_service!");
            return result;
        }

        return ESP_OK;
    }

    const char *comm_get_device_uid(){
        return "TEMPORARY1234567";
    }

    

    esp_err_t comm_refresh_rtc() {

        return ESP_OK;
    }

    esp_err_t comm_pub_message(const char *topic, const uint8_t *payload, size_t payload_size) {

        return ESP_OK;
    }

}
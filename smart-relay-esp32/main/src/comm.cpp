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

#include "mosq_broker.h"

static const char *TAG = "plugins::comm";

namespace clab::plugins {

    TaskHandle_t comm_mqtt_broker_task_handle = NULL;

    void comm_mqtt_broker_task(void *params) {
        

        struct mosq_broker_config config = {
            .host = CONFIG_MAIN_MQTT_BROKER_LISTEN_ON,
            .port = CONFIG_MAIN_MQTT_BROKER_LISTEN_PORT,
            .tls_cfg = NULL,
        };

        mosq_broker_run(&config);
    }

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

    esp_err_t comm_mqtt_broker_start() {
        
        auto task_result = xTaskCreate(comm_mqtt_broker_task, "mqtt_broker", 4096, NULL, tskIDLE_PRIORITY, &comm_mqtt_broker_task_handle); 
        if (task_result != pdPASS ) {
            ESP_LOGE(TAG, "Error occurred during send task creation...");
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "Power opt task launched...");

        return ESP_OK;
    }


    

    esp_err_t comm_refresh_rtc() {

        return ESP_OK;
    }

    esp_err_t comm_pub_message(const char *topic, const uint8_t *payload, size_t payload_size) {

        return ESP_OK;
    }

}
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
#include "mqtt_client.h"

static const char *TAG = "plugins::comm";

namespace clab::plugins {

    TaskHandle_t comm_mqtt_broker_task_handle = NULL;

    esp_mqtt_client_handle_t comm_mqtt_client_handle = NULL;

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
        ESP_LOGI(TAG, "MQTT broker task launched...");

        return ESP_OK;
    }

    /*
    * @brief Event handler registered to receive MQTT events
    *
    *  This function is called by the MQTT client event loop.
    *
    * @param handler_args user data registered to the event.
    * @param base Event base for the handler(always MQTT Base in this example).
    * @param event_id The id for the received event.
    * @param event_data The data for the event, esp_mqtt_event_handle_t.
    */
    static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
    {
        ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
        esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
        esp_mqtt_client_handle_t client = event->client;
        int msg_id;
        switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
                ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                        strerror(event->error_handle->esp_transport_sock_errno));
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
        }
    }

    esp_err_t comm_mqtt_client_start(bool is_local_broker) {
        esp_mqtt_client_config_t mqtt_cfg = {
            .broker = {
                .address = {
                    .hostname = is_local_broker ? "127.0.0.1" : CONFIG_MAIN_MQTT_BROKER_DEFAULT_URI,
                    .transport = MQTT_TRANSPORT_OVER_TCP,
                    .port = CONFIG_MAIN_MQTT_BROKER_LISTEN_PORT
                }
            }
        };

        if (strcmp(mqtt_cfg.broker.address.uri, "") == 0) {
            //TODO: auto broker discovery
        }

        comm_mqtt_client_handle = esp_mqtt_client_init(&mqtt_cfg);
        if (comm_mqtt_client_handle == NULL) {
            ESP_LOGE(TAG, "Cannot init MQTT client!");
            return ESP_FAIL;
        }
        /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
        
        esp_err_t result = esp_mqtt_client_register_event(comm_mqtt_client_handle, esp_mqtt_event_id_t::MQTT_EVENT_ANY, 
                mqtt_event_handler, NULL);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to register mqtt event handler!");
            return result;
        }

        result = esp_mqtt_client_start(comm_mqtt_client_handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to start MQTT client!");
            return result;
        }

        return ESP_OK;
    }


    

    esp_err_t comm_refresh_rtc() {

        return ESP_OK;
    }

    esp_err_t comm_pub_message(const char *topic, const uint8_t *payload, size_t payload_size) {

        return ESP_OK;
    }

}
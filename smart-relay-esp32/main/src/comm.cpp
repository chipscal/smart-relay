#include "plugin/comm.h"

#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mbedtls/base64.h"
#include "esp_wifi.h"

#include "string.h"
#include <string>
#include <string_view>
#include <algorithm>
#include <array>
#include <unordered_map>

#include "iot_services/iot_services.h"
#include "iot_services/rtc_service.h"
#include "iot_services/storage_service.h"
#include "iot_services/wifi_service.h"
#include "iot_services/board.h"

#include "mosq_broker_c_api.h"
#include "mqtt_client.h"

static const char *TAG = "plugins::comm";

namespace clab::plugins {

    TaskHandle_t                        comm_mqtt_broker_task_handle = NULL;

    esp_mqtt_client_handle_t            comm_mqtt_client_handle = NULL;

    std::unordered_map<std::string, comm_message_callback_t>  comm_callbacks;

    char                                comm_device_address[CONFIG_IOT_DEVICEUID_MAX_SIZE + 1] = "R1XXXXXXXXXXXX";

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

        uint8_t mac[6];
        result = esp_wifi_get_mac(wifi_interface_t::WIFI_IF_STA, mac);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to get wifi mac!");
            return result;
        }

        clab::iot_services::sprint_array_hex(comm_device_address + 2, mac, 6);

        return ESP_OK;
    }

    const char *comm_get_device_uid(){
        
        return comm_device_address;
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

    static bool compare_with_wildcards(std::string subject, std::string test) {

        auto subject_iter = subject.begin();
        auto test_iter = test.begin();

        while (subject_iter < subject.end() && test_iter < test.end()) {
            
            auto subject_delim = std::find(subject_iter, subject.end(), '/');
            auto subject_sub = std::string_view(subject_iter, subject_delim);
            subject_iter = subject_delim + 1;

            auto test_delim = std::find(test_iter, test.end(), '/');
            auto test_sub = std::string_view(test_iter, test_delim);
            test_iter = test_delim + 1;

            if (subject_delim == subject.end() && subject_sub.compare("#") == 0)
                break;
                
            if (subject_sub.compare("+") != 0 && subject_sub.compare(test_sub) != 0)
                return false;
        }

        return true;
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
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
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

            for (auto callback_kv : comm_callbacks) {
                ESP_LOGI(TAG, "Comparing with: %s", callback_kv.first.c_str());
                if (compare_with_wildcards(callback_kv.first, std::string(event->topic, event->topic_len))) {
                    ESP_LOGI(TAG, "Found!");
                    comm_callbacks[callback_kv.first](event->topic, event->data, event->data_len);
                    break;
                }
            }
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

        int msg_id = esp_mqtt_client_publish(comm_mqtt_client_handle, topic, (const char *)(payload), payload_size, 0, 0);
        if (msg_id < 0) {
            ESP_LOGE(TAG, "Error occurred during publish: %s - %d", topic, msg_id);
            return ESP_FAIL;
        }

        return ESP_OK;
    }

    esp_err_t comm_sub_message(const char *topic, comm_message_callback_t callback) {
        if (!comm_callbacks.contains(topic)) {

            int msg_id = esp_mqtt_client_subscribe(comm_mqtt_client_handle, topic, 0);
            if (msg_id < 0) {
                ESP_LOGE(TAG, "Error occurred during subscribe: %s", topic);
                return ESP_FAIL;
            }

            comm_callbacks[topic] = callback;
            return ESP_OK;
        }

        return ESP_ERR_NOT_ALLOWED;
    }

}
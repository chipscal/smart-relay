#include "plugin/control.h"

#include "esp_log.h"
#include "esp_system.h"
#include "mbedtls/base64.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "string.h"
#include <string>
#include <string_view>
#include <array>
#include <algorithm>

#include "iot_services/iot_services.h"
#include "iot_services/rtc_service.h"
#include "iot_services/io_service.h"
#include "iot_services/storage_service.h"
#include "iot_services/comm_api.h"
#include "iot_services/ctrl_service.h"
#include "iot_services/board.h"
#include "plugin/comm.h"

static const char *TAG = "plugins::control";

// Defined in ctrl_service.cpp
namespace clab::iot_services {
    extern void ctrl_copy_des_status(des_status_t &from, des_status_t &to);
}

namespace clab::plugins {

    TaskHandle_t    control_task_handle             = NULL;
    uint32_t        control_task_loop_cnt           = 0;
    uint32_t        control_latest_telem_loop_cnt   = 0;


    // clab::iot_services::dev_status_t                                       status;

    clab::iot_services::des_status_t                                       overrides;

    clab::iot_services::des_status_t                                       latest_o;

    SemaphoreHandle_t                                                      control_mutex = NULL;


    static void on_property_update(const char *topic, const char *payload, size_t payload_size);
    static void on_cmd_received(const char *topic, const char *payload, size_t payload_size);

    

    void control_task(void *params) {

        uint8_t buffer[clab::iot_services::alligned_big_enough(clab::iot_services::io_buffer_report_size + 2)];
        memset(&latest_o, 0, sizeof(clab::iot_services::des_status_t));
        clab::iot_services::dev_status_t actual_status(buffer, sizeof(buffer));

        char topic_buffer[64];
        sprintf(topic_buffer, "/dev/%s/telem", clab::plugins::comm_get_device_uid());

        size_t  encoded_size;
        uint8_t encoded_buffer[clab::iot_services::alligned_big_enough(clab::iot_services::io_buffer_report_size * 2)];


        while (true) {
            vTaskDelay(pdMS_TO_TICKS(CONFIG_CONTROL_LOOP_INTERVAL_MILLIS));

            esp_err_t result;
            uint32_t actual_ts = clab::iot_services::rtc_get_utc();

            // status update
            control_task_loop_cnt++;
            result = clab::iot_services::io_buffer_report(buffer, sizeof(buffer), true);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during full status report!");
                //TODO: che fare?
            }


            clab::iot_services::des_status_t tmp_latest_o;
            clab::iot_services::ctrl_copy_des_status(latest_o, tmp_latest_o);

            result = clab::iot_services::ctrl_loop(actual_ts, actual_status, overrides, &tmp_latest_o);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during ctrl loop!");
                //TODO: che fare?
            }

            if (control_task_loop_cnt - control_latest_telem_loop_cnt > CONFIG_CONTROL_SEND_STATUS_MAX_EVERY_LOOPS &&
                    (tmp_latest_o.latch_mask != latest_o.latch_mask || tmp_latest_o.relay_mask != latest_o.relay_mask)) {
                
                result = clab::iot_services::io_buffer_report(buffer, sizeof(buffer), true);
                if (result == ESP_OK) {
                    ESP_LOGI(TAG, "Sending telemetry due output ports change!");
                    if (mbedtls_base64_encode((unsigned char *)encoded_buffer, sizeof(encoded_buffer), 
                            &encoded_size, buffer, clab::iot_services::io_buffer_report_size) < 0) {
                        ESP_LOGE(TAG, "Unable to byte64 encode telemetry, too big!");
                        return;
                    }
                    if (encoded_size == 256) {
                        ESP_LOGE(TAG, "Unable to null terminate encoded telemetry, too big!");
                        return;
                    }
                    encoded_buffer[encoded_size] = '\0';

                    clab::plugins::comm_pub_message(topic_buffer, encoded_buffer, encoded_size);
                }
                control_latest_telem_loop_cnt = control_task_loop_cnt;
            }

            clab::iot_services::ctrl_copy_des_status(tmp_latest_o, latest_o);
            
            ESP_LOGD(TAG, "Control loop done...");
        }
    }
    
    esp_err_t control_plugin() {
        constexpr unsigned int buffer_needs[] = {
                clab::iot_services::io_buffer_report_size, 
                32 + 1
        };

        if (control_mutex == NULL) {
            control_mutex = xSemaphoreCreateMutex(); //In tests may have been already allocated
            ESP_LOGI(TAG, "Control plugin mutex created...");
        }

        uint8_t buffer[clab::iot_services::alligned_big_enough(clab::iot_services::array_max(buffer_needs))];
        size_t  out_size = 0;
        uint8_t prop_value_buffer[clab::iot_services::alligned_big_enough(2 * sizeof(uint32_t))];
        size_t  prop_size = 0;

        memset(&overrides, 0, sizeof(clab::iot_services::des_status_t));

        ESP_ERROR_CHECK(clab::iot_services::ctrl_init(true));
        ESP_LOGI(TAG, "Control service intialized...");

        char topic_buffer[64];
        sprintf(topic_buffer, "/dev/%s/prop/+/desired", clab::plugins::comm_get_device_uid());
        ESP_ERROR_CHECK(comm_sub_message(topic_buffer, on_property_update));

        sprintf(topic_buffer, "/dev/%s/cmd/+/exec", clab::plugins::comm_get_device_uid());
        ESP_ERROR_CHECK(comm_sub_message(topic_buffer, on_cmd_received));

        if ( clab::iot_services::storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "over", (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
            ESP_LOGI(TAG, "Founded setting \"over\":%s - size: %u", buffer, out_size);
            
            if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size - 1) == 0) {
                
                clab::iot_services::sprint_array_hex((char *)buffer, prop_value_buffer, prop_size);
                ESP_LOGI(TAG, "Property: %s, Decoded size: %u", buffer, prop_size);

                uint32_t mask = 0;
                memcpy((uint8_t *)&(mask), prop_value_buffer, sizeof(uint32_t));
                if (!clab::iot_services::is_little_endian()) {
                    mask = clab::iot_services::swap_uint32(mask);
                }
                overrides.latch_mask = mask;

                memcpy((uint8_t *)&(overrides.relay_mask), prop_value_buffer + sizeof(uint32_t), sizeof(uint32_t));
                if (!clab::iot_services::is_little_endian()) {
                    overrides.relay_mask = clab::iot_services::swap_uint32(overrides.relay_mask);
                }
            } 
            
            clab::iot_services::sprint_uint32_binary((char *)buffer, overrides.latch_mask);
            ESP_LOGI(TAG, "Latch override: %s", buffer);

            clab::iot_services::sprint_uint32_binary((char *)buffer, overrides.relay_mask);
            ESP_LOGI(TAG, "Relay override: %s", buffer);
        }

        control_task_loop_cnt = 0;
        auto task_result = xTaskCreate(control_task, "control", 4096, NULL, tskIDLE_PRIORITY, &control_task_handle); 
        if (task_result != pdPASS ) {
            ESP_LOGE(TAG, "Error occurred during send task creation...");
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "Control task launched...");

        return ESP_OK;
    }

    // ----------------------------private implementations:

    static std::string extract_from_topic(std::string topic, int position) {

        auto topic_iter = topic.begin();

        while (topic_iter < topic.end()) {
            
            auto topic_delim = std::find(topic_iter, topic.end(), '/');
            auto topic_sub = std::string_view(topic_iter, topic_delim);
            topic_iter = topic_delim + 1;

            if (position == 0)
                return std::string(topic_sub);
            
            position--;
        }

        return "";
    }

    static bool on_property_update(const char *topic, const char *payload, size_t payload_size) {
        esp_err_t result;

        auto alias_string = extract_from_topic(topic, 4);
        if (alias_string.empty())
            return false;

        ESP_LOGI(TAG, "Received property: %s", alias_string.c_str());

        if (alias_string.compare("ports") == 0) {
            clab::iot_services::ports_conf_t<clab::iot_services::io_n_latch, clab::iot_services::io_n_relay> ports;
            if (ports.from_buffer(payload, payload_size) != ESP_OK) {
                ESP_LOGE(TAG, "Unable to deserialize ports!");
                return false;
            }
            result = clab::iot_services::ctrl_set_ports(ports);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Ports Unable to set!");
                return false;
            }
        }

        if (alias_string.compare("over") == 0) {
            memcpy(&(overrides.latch_mask), payload, sizeof(uint32_t));
            if (!clab::iot_services::is_little_endian()) {
                overrides.latch_mask = clab::iot_services::swap_uint32(overrides.latch_mask);
            }

            memcpy(&(overrides.relay_mask), payload + sizeof(uint32_t), sizeof(uint32_t));
            if (!clab::iot_services::is_little_endian()) {
                overrides.relay_mask = clab::iot_services::swap_uint32(overrides.relay_mask);
            }
        }

        if (alias_string.compare("pdelay") == 0) {
            uint16_t delays[clab::iot_services::io_n_pulse];
            if (payload_size != sizeof(delays)) {
                ESP_LOGE(TAG, "pdelay size is invalid!");
                return false;
            }

            for (int k = 0; k < clab::iot_services::io_n_pulse; k++) {
                uint16_t value = 0;
                memcpy((uint8_t *)&value, payload + k * sizeof(uint16_t), sizeof(uint16_t));

                if (!clab::iot_services::is_little_endian()) {
                    value = clab::iot_services::swap_uint16(value);
                }

                delays[k] = value;
            }

            esp_err_t result = clab::iot_services::io_pulse_filter_set(delays, clab::iot_services::io_n_pulse);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Unable to set pulse filter delays!");
                return false;
            }
        }

        return true;
    }

    static bool on_cmd_received(const char *topic, const char *payload, size_t payload_size) {
        esp_err_t result;

        auto cmd_string = extract_from_topic(topic, 4);
        if (cmd_string.empty())
            return false;

        ESP_LOGI(TAG, "Received cmd: %s", cmd_string.c_str());
        
        if (cmd_string.compare("restart") == 0) {
            ESP_LOGI(TAG, "Board is going to be restarted...");
            clab::iot_services::board_clean_restart();
        }
        else if (cmd_string.compare("restart-hard") == 0) {
            ESP_LOGI(TAG, "Board is going to be restarted...");
            esp_restart();
        }
        else if (cmd_string.compare("refresh") == 0) {
            ESP_LOGI(TAG, "Starting latch refresh sequence...");
            result = clab::iot_services::io_latch_refresh();
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during latch refresh: %s", esp_err_to_name(result));
                return false;
            }
        }

        return true;
    }
}
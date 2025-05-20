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
#include <cmath>

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
    static void on_telem_received(const char *topic, const char *payload, size_t payload_size);
    
    


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
                //TODO: what to do?
            }


            clab::iot_services::des_status_t tmp_latest_o;
            clab::iot_services::ctrl_copy_des_status(latest_o, tmp_latest_o);


            result = clab::iot_services::ctrl_loop(actual_ts, actual_status, overrides, &tmp_latest_o);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error during ctrl loop!");
                //TODO: what to do?
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
                2 * sizeof(uint32_t), 
                32 + 1,
                255
        };

        if (control_mutex == NULL) {
            control_mutex = xSemaphoreCreateMutex(); //In tests may have been already allocated
            ESP_LOGI(TAG, "Control plugin mutex created...");
        }

        uint8_t buffer[clab::iot_services::alligned_big_enough(clab::iot_services::array_max(buffer_needs))];
        size_t  out_size = 0;
        uint8_t prop_value_buffer[clab::iot_services::alligned_big_enough(clab::iot_services::array_max(buffer_needs))];
        size_t  prop_size = 0;

        memset(&overrides, 0, sizeof(clab::iot_services::des_status_t));

        ESP_ERROR_CHECK(clab::iot_services::ctrl_init(true));
        ESP_LOGI(TAG, "Control service intialized...");

        char topic_buffer[64];
        sprintf(topic_buffer, "/dev/%s/prop/+/desired", clab::plugins::comm_get_device_uid());
        ESP_ERROR_CHECK(comm_sub_message(topic_buffer, on_property_update));

        sprintf(topic_buffer, "/dev/%s/cmd/+/exec", clab::plugins::comm_get_device_uid());
        ESP_ERROR_CHECK(comm_sub_message(topic_buffer, on_cmd_received));

        ESP_ERROR_CHECK(comm_sub_message("/dev/+/telem", on_telem_received));

        if (clab::iot_services::storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "over", (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
            ESP_LOGI(TAG, "Founded setting \"over\":%.*s - size: %u", out_size, buffer, out_size);
            
            if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                
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

    static void on_property_update(const char *topic, const char *payload, size_t payload_size) {
        esp_err_t result;
        uint8_t decoded_buffer[256];
        size_t  decoded_size;
        char topic_buffer[64];

        auto alias_string = extract_from_topic(topic, 4);
        if (alias_string.empty()) {
            ESP_LOGE(TAG, "Invalid empty property! Ignoring...");
            return;
        }
        
        ESP_LOGI(TAG, "Received property: %s", alias_string.c_str());
        bool prop_is_ok = false;

        if (alias_string.compare("ports") == 0) {
            if (mbedtls_base64_decode(decoded_buffer, sizeof(decoded_buffer), &decoded_size, 
                    (const unsigned char *)payload, payload_size) != 0) {
                ESP_LOGE(TAG, "Malformed or too big message!");
                return;
            }

            clab::iot_services::ports_conf_t<clab::iot_services::io_n_latch, clab::iot_services::io_n_relay> ports;
            if (ports.from_buffer(decoded_buffer, decoded_size) != ESP_OK) {
                ESP_LOGE(TAG, "Unable to deserialize ports!");
                return;
            }
            result = clab::iot_services::ctrl_set_ports(ports);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Ports Unable to set!");
                return;
            }

            prop_is_ok = true;
        }

        if (alias_string.compare("over") == 0) {
            if (mbedtls_base64_decode(decoded_buffer, sizeof(decoded_buffer), &decoded_size, 
                    (const unsigned char *)payload, payload_size) != 0) {
                ESP_LOGE(TAG, "Malformed or too big message!");
                return;
            }

            memcpy(&(overrides.latch_mask), decoded_buffer, sizeof(uint32_t));
            if (!clab::iot_services::is_little_endian()) {
                overrides.latch_mask = clab::iot_services::swap_uint32(overrides.latch_mask);
            }

            memcpy(&(overrides.relay_mask), decoded_buffer + sizeof(uint32_t), sizeof(uint32_t));
            if (!clab::iot_services::is_little_endian()) {
                overrides.relay_mask = clab::iot_services::swap_uint32(overrides.relay_mask);
            }

            prop_is_ok = true;
        }

        if (alias_string.compare("pdelay") == 0) {
            if (mbedtls_base64_decode(decoded_buffer, sizeof(decoded_buffer), &decoded_size, 
                    (const unsigned char *)payload, payload_size) != 0) {
                ESP_LOGE(TAG, "Malformed or too big message!");
                return;
            }

            uint16_t delays[clab::iot_services::io_n_pulse];
            if (decoded_size != sizeof(delays)) {
                ESP_LOGE(TAG, "pdelay size is invalid!");
                return;
            }

            for (int k = 0; k < clab::iot_services::io_n_pulse; k++) {
                uint16_t value = 0;
                memcpy((uint8_t *)&value, decoded_buffer + k * sizeof(uint16_t), sizeof(uint16_t));

                if (!clab::iot_services::is_little_endian()) {
                    value = clab::iot_services::swap_uint16(value);
                }

                delays[k] = value;
            }

            esp_err_t result = clab::iot_services::io_pulse_filter_set(delays, clab::iot_services::io_n_pulse);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Unable to set pulse filter delays!");
                return;
            }

            prop_is_ok = true;
        }

        if (alias_string.starts_with("rule")) {
            if (mbedtls_base64_decode(decoded_buffer, sizeof(decoded_buffer), &decoded_size, 
                    (const unsigned char *)payload, payload_size) != 0) {
                ESP_LOGE(TAG, "Malformed or too big message!");
                return;
            }
            decoded_buffer[decoded_size] = '\0';

            auto suffix = alias_string.substr(4);
            auto idx = static_cast<uint8_t>(std::stod(suffix));

            clab::iot_services::combined_rule_t<4, 14> received_rule;
            
            if (received_rule.parse_from((char *)decoded_buffer) == ESP_OK) {

                esp_err_t result = clab::iot_services::ctrl_rule_set(idx, received_rule);
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Unable to set rule!");
                    return;
                }
                prop_is_ok = true;
            }

        }

        if (prop_is_ok) {
            // Save on storage...
            ESP_LOGI(TAG, "Saving data: %.*s, (%d bytes)", payload_size, payload, payload_size);
            result = clab::iot_services::storage_db_set(CONFIG_IOT_IO_STORAGE_NAMESPACE, 
                    alias_string.c_str(), payload, payload_size); 
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Unable to save property!");
                return;
            }
    
            // Send notify
            sprintf(topic_buffer, "/dev/%s/prop/%s/value", clab::plugins::comm_get_device_uid(), alias_string.c_str());
            result = comm_pub_message(topic_buffer, (const uint8_t *)payload, payload_size);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Cannot pub telemetry!");
                return;
            }
        }
    }

    static void on_cmd_received(const char *topic, const char *payload, size_t payload_size) {
        esp_err_t result;
        uint8_t decode_encode_buffer[256];
        size_t  decoded_encoded_size;
        char topic_buffer[64];
        char key_buffer[32];

        auto cmd_string = extract_from_topic(topic, 4);
        if (cmd_string.empty())
            return;

        ESP_LOGI(TAG, "Received cmd: %s", cmd_string.c_str());
        

        if (mbedtls_base64_decode(decode_encode_buffer, sizeof(decode_encode_buffer), &decoded_encoded_size, 
                (const unsigned char *)payload, payload_size) != 0) {
            ESP_LOGE(TAG, "Malformed or too big message!");
            return;
        }

        bool do_ack = true;
        bool ack    = true;

        // Message format:
        // TIME0|TIME1|TIME2|TIME3|PAYLOAD...
        if (decoded_encoded_size < 4) {
            ESP_LOGE(TAG, "Malformed message!");
            return;
        }

        uint32_t ts = 0;
        memcpy(&ts, decode_encode_buffer, sizeof(uint32_t));
        if (!clab::iot_services::is_little_endian())
            ts = clab::iot_services::swap_uint32(ts);

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
                return;
            }
        }
        else if (cmd_string.compare("query") == 0) {
            do_ack = false;
            size_t out_size;

            bool property_valid = true;
            if (decoded_encoded_size - sizeof(uint32_t) + 1 > sizeof(key_buffer)) {
                ESP_LOGE(TAG, "Property alias too big!");
                property_valid = false;
            }
            else {

                sprintf(key_buffer, "%.*s", decoded_encoded_size - sizeof(uint32_t), decode_encode_buffer + sizeof(uint32_t));
                sprintf(topic_buffer, "/dev/%s/prop/%.*s/value", clab::plugins::comm_get_device_uid(), decoded_encoded_size - sizeof(uint32_t), decode_encode_buffer + sizeof(uint32_t));
                ESP_LOGI(TAG, "Requested property: %s", key_buffer);
            } 

            if (property_valid && clab::iot_services::storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, key_buffer, (char *)decode_encode_buffer, sizeof(decode_encode_buffer), &out_size) == ESP_OK) {
                result = comm_pub_message(topic_buffer, decode_encode_buffer, out_size);
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Cannot pub telemetry!");
                    return;
                }
            }
            else {
                do_ack = true;
                ack = false;
            }
        }

        if (do_ack) {
            if (mbedtls_base64_encode((unsigned char *)decode_encode_buffer, sizeof(decode_encode_buffer), 
                    &decoded_encoded_size, (unsigned char *)&ts, sizeof(uint32_t)) < 0) {
                ESP_LOGE(TAG, "Unable to byte64 encode command ack, too big!");
                return;
            }

            sprintf(topic_buffer, "/dev/%s/cmd/%s/%s", clab::plugins::comm_get_device_uid(), cmd_string.c_str(), ack ? "ack" : "nack");
            result = comm_pub_message(topic_buffer, decode_encode_buffer, decoded_encoded_size);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Cannot pub telemetry!");
                return;
            }
        }
    }

    static void on_telem_received(const char *topic, const char *payload, size_t payload_size) {
        uint8_t decoded_buffer[256];
        size_t  decoded_size;
        
        ESP_LOGI(TAG, "Message: %.*s", payload_size, payload);

        auto sender = extract_from_topic(topic, 2);
        ESP_LOGI(TAG, "Received telemetry from: %s", sender.c_str());
        
        if (mbedtls_base64_decode(decoded_buffer, sizeof(decoded_buffer), &decoded_size, 
                (const unsigned char *)payload, payload_size) != 0) {
            ESP_LOGE(TAG, "Malformed or too big message!");
            return;
        }

        esp_err_t result = clab::iot_services::ctrl_rules_eval(sender.c_str(), decoded_buffer, decoded_size);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to evaluate rules using provided payload!");
            return;
        }

    }
}
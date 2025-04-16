#include "iot_services/ctrl_service.h"

#include "iot_services/iot_services.h"
#include "iot_services/io_service.h"
#include "iot_services/rtc_service.h"
#include "iot_services/storage_service.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "mbedtls/base64.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include <array>
#include <map>
#include <cmath>
#include "string.h"

static const char *TAG = "iot_services::ctrl_service";


namespace clab::iot_services {

    ports_conf_t<io_n_latch, io_n_relay>                ports;
    std::array<uint32_t, 32>                            latch_last_switch;
    std::array<bool, 32>                                latch_logic_status;
    std::array<uint32_t, 32>                            relay_last_switch;
    std::array<bool, 32>                                relay_logic_status;


    SemaphoreHandle_t               ctrl_mutex = NULL;

    esp_err_t ctrl_init(bool init_from_storage) {
        constexpr unsigned int prop_needs[] = {
                sizeof(port_conf_t) * (io_n_latch + io_n_relay), 
                clab::iot_services::io_buffer_report_size
        };

        
        if (ctrl_mutex == NULL) {
            ctrl_mutex = xSemaphoreCreateMutex(); //In tests may have been already allocated
            ESP_LOGI(TAG, "Control service mutex created...");
        }

        uint8_t buffer[256];
        size_t  out_size = 0;
        uint8_t prop_value_buffer[clab::iot_services::alligned_big_enough(clab::iot_services::array_max(prop_needs))];
        size_t  prop_size = 0;

        //try to restore last settings...
        //ports
        if (init_from_storage && storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "ports", (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
            ESP_LOGI(TAG, "Founded setting \"ports\":%s", buffer);

            if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size - 1) == 0) {
                
                clab::iot_services::sprint_array_hex((char *)buffer, prop_value_buffer, prop_size);
                ESP_LOGI(TAG, "Property: %s, Decoded size: %u", buffer, prop_size);

                if (ports.from_buffer(prop_value_buffer, prop_size)) {

                    ESP_LOGE(TAG, "Deserialization error! Ignoring...");

                    memset(&ports, 0, ports.size());
                }
            } 

        }
        else {
            memset(&ports, 0, ports.size());
        }

        for (int k = 0; k < latch_logic_status.size(); k++)
            latch_logic_status[k] = false;

        for (int k = 0; k < relay_logic_status.size(); k++)
            relay_logic_status[k] = false;

        ESP_LOGI(TAG, "Port configuration... Ok...");

        return ESP_OK;
    }

    esp_err_t ctrl_deinit() {
        //ports
        memset(&ports, 0, ports.size());
        
        ESP_LOGI(TAG, "Control service down...");
        return ESP_OK;
    }

    void ctrl_copy_ports(ports_conf_t<io_n_latch, io_n_relay> &from, ports_conf_t<io_n_latch, io_n_relay> &to) {
        for (int k = 0; k < io_n_latch; k++) {
            to.latch_conf[k].init_d = from.latch_conf[k].init_d;
            to.latch_conf[k].stop_d = from.latch_conf[k].stop_d;
        }

        for (int k = 0; k < io_n_relay; k++) {
            to.relay_conf[k].init_d = from.relay_conf[k].init_d;
            to.relay_conf[k].stop_d = from.relay_conf[k].stop_d;
        }
    }

    esp_err_t ctrl_set_ports(ports_conf_t<io_n_latch, io_n_relay> &port_conf) {
        xSemaphoreTake(ctrl_mutex, portMAX_DELAY);
        ctrl_copy_ports(port_conf, ports);
        xSemaphoreGive(ctrl_mutex);

        return ESP_OK;
    }


    esp_err_t ctrl_port_loop(uint32_t actual_ts, dev_status_t &status, des_status_t &des_status, 
            des_status_t *out_logic, bool output_enabled) {
        #if CONFIG_LOG_DEFAULT_LEVEL > 3 //if >= ESP_LOG_DEBUG
            char log_buffer[256];
            size_t cnt = sprintf(log_buffer, "[%4lu]", actual_ts);
            cnt += sprintf(log_buffer + cnt, ", l:");
            cnt += sprint_uint32_binary(log_buffer + cnt, des_status.latch_mask);
            cnt += sprintf(log_buffer + cnt, ", r:");
            cnt += sprint_uint32_binary(log_buffer + cnt, des_status.relay_mask);

            ESP_LOGD(TAG, "%s", log_buffer);
        #endif
        
        for (int k = 0; k < io_n_latch; k++) {
            ESP_LOGD(TAG, "Latch[%d] des status: %s", k, des_status.latch_status(k) ? "on" : "off");

            if (des_status.latch_status(k)) {
                if (!latch_logic_status[k]) {
                    latch_logic_status[k] = true;
                    latch_last_switch[k] = actual_ts;
                }

                if (!status.latch_status(k) &&
                        actual_ts - latch_last_switch[k] >= ports.latch_conf[k].init_d) {
                    ESP_LOGD(TAG, "Latch[%d]  off -> on", k);

                    if (out_logic != NULL)
                        out_logic->latch_status(k, true);

                    if (output_enabled) {
                        esp_err_t result = io_latch_cmd(k, true);
                        if (result != ESP_OK) {
                            ESP_LOGE(TAG, "Latch[%d]: unable to set!", k);
                            return result;
                        }
                    }
                }
            }
            else {
                if (latch_logic_status[k]) {
                    latch_logic_status[k] = false;
                    latch_last_switch[k] = actual_ts;
                }

                if (status.latch_status(k) &&
                        actual_ts - latch_last_switch[k] >= ports.latch_conf[k].stop_d) {
                    ESP_LOGD(TAG, "Latch[%d]  on -> off", k);

                    if (out_logic != NULL)
                        out_logic->latch_status(k, false);

                    if (output_enabled) {
                        esp_err_t result = io_latch_cmd(k, false);
                        if (result != ESP_OK) {
                            ESP_LOGE(TAG, "Latch[%d]: unable to set!", k);
                            return result;
                        }
                    }
                }
            }
        }


        
        for (int k = 0; k < io_n_relay; k++) {
            ESP_LOGD(TAG, "Relay[%d] des status: %s", k, des_status.relay_status(k) ? "on" : "off");
            if (des_status.relay_status(k)) {
                if (!relay_logic_status[k]) {
                    relay_logic_status[k] = true;
                    relay_last_switch[k] = actual_ts;
                }

                if (!status.relay_status(k) &&
                        actual_ts - relay_last_switch[k] >= ports.relay_conf[k].init_d) {
                    ESP_LOGD(TAG, "Relay[%d]  off -> on", k);

                    if (out_logic != NULL)
                        out_logic->relay_status(k, true);

                    if (output_enabled) {
                        esp_err_t result = io_relay_cmd(k, true);
                        if (result != ESP_OK) {
                            ESP_LOGE(TAG, "Relay[%d]: unable to set!", k);
                            return result;
                        }
                    }
                }
            }
            else {
                if (relay_logic_status[k]) {
                    relay_logic_status[k] = false;
                    relay_last_switch[k] = actual_ts;
                }
                // char deb_buffer[128];
                // sprint_uint32_binary(deb_buffer, status.relay_mask);
                // ESP_LOGW(TAG, "%s", deb_buffer);

                if (status.relay_status(k) &&
                        actual_ts - relay_last_switch[k] >= ports.relay_conf[k].stop_d) {
                    ESP_LOGD(TAG, "Relay[%d]  on -> off", k);

                    if (out_logic != NULL)
                        out_logic->relay_status(k, false);

                    if (output_enabled) {
                        esp_err_t result = io_relay_cmd(k, false);
                        if (result != ESP_OK) {
                            ESP_LOGE(TAG, "Relay[%d]: unable to set!", k);
                            return result;
                        }
                    }
                }
            }
        }

        return ESP_OK;
    }

    void ctrl_copy_des_status(des_status_t &from, des_status_t &to) {
        to.latch_mask = from.latch_mask;
        to.relay_mask = from.relay_mask;
    }

    esp_err_t ctrl_loop(uint32_t actual_ts, dev_status_t &status, des_status_t &overrides, 
            des_status_t *out_logic, bool output_enabled) {

        des_status_t desired;
        ctrl_copy_des_status(overrides, desired);

        xSemaphoreTake(ctrl_mutex, portMAX_DELAY);

        //TODO: can add device program here!
        
        esp_err_t result = ctrl_port_loop(actual_ts, status, desired, out_logic, output_enabled);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Port controller logic returned error!");
        }
        else {
            ESP_LOGD(TAG, "Port controller logic... done!");
        }

        xSemaphoreGive(ctrl_mutex);

        return result;
    }

}
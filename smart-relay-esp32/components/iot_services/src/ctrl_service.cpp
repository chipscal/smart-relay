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

    std::array<dev_program_t, CONFIG_IOT_PROGRAM_MAX>                   programs;


    clab::iot_services::des_status_t                                    rules_action;
    std::array<clab::iot_services::combined_rule_t
            <CONFIG_IOT_CRULE_SIZE, CONFIG_IOT_DEVICEUID_MAX_SIZE>, CONFIG_IOT_CRULE_MAX>              
                                                                        control_rules;
    std::array<bool, CONFIG_IOT_CRULE_SIZE * CONFIG_IOT_CRULE_MAX>      control_inner_eval;
    std::array<float, CONFIG_IOT_CRULE_SIZE * CONFIG_IOT_CRULE_MAX>     control_inner_last_measure;


    SemaphoreHandle_t               ctrl_mutex = NULL;

    esp_err_t ctrl_init(bool init_from_storage) {
        constexpr unsigned int prop_needs[] = {
                sizeof(port_conf_t) * (io_n_latch + io_n_relay), 
                clab::iot_services::io_buffer_report_size,
                256
        };

        
        if (ctrl_mutex == NULL) {
            ctrl_mutex = xSemaphoreCreateMutex(); //In tests may have been already allocated
            ESP_LOGI(TAG, "Control service mutex created...");
        }

        esp_err_t result;

        uint8_t buffer[256];
        size_t  out_size = 0;
        uint8_t prop_value_buffer[clab::iot_services::alligned_big_enough(clab::iot_services::array_max(prop_needs))];
        size_t  prop_size = 0;

        //try to restore last settings...
        //------------------------ ports:
        ports = clab::iot_services::ports_conf_t<io_n_latch, io_n_relay>();

        for (int k = 0; k < io_n_relay; k++) {
            char key_buf[8];
            snprintf(key_buf, 8, "rd%d", k);
           
            if (init_from_storage && clab::iot_services::storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, key_buf, (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
                ESP_LOGI(TAG, "Founded setting \"%s\":%.*s - size: %u", key_buf, out_size, buffer, out_size);

                if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                    
                    auto port_conf = clab::iot_services::port_conf_t();
                    if (port_conf.from_buffer(prop_value_buffer, prop_size) != ESP_OK) {
                        ESP_LOGE(TAG, "Deserialization error! Ignoring...");
                        continue;
                    }

                    ports.set_relay_port(k, port_conf);
                     
                }
            }
        }

        for (int k = 0; k < io_n_latch; k++) {
            char key_buf[8];
            snprintf(key_buf, 8, "ld%d", k);
           
            if (init_from_storage && clab::iot_services::storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, key_buf, (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
                ESP_LOGI(TAG, "Founded setting \"%s\":%.*s - size: %u", key_buf, out_size, buffer, out_size);

                if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                    
                    auto port_conf = clab::iot_services::port_conf_t();
                    if (port_conf.from_buffer(prop_value_buffer, prop_size) != ESP_OK) {
                        ESP_LOGE(TAG, "Deserialization error! Ignoring...");
                        continue;
                    }

                    ports.set_latch_port(k, port_conf);
                     
                }
            }
        }

        // --------------------- control_rules:
        for (int k = 0; k < programs.size(); k++) {
            char key_buf[8];
            snprintf(key_buf, 8, "prog%d", k);

            programs[k] = clab::iot_services::dev_program_t();
           
            if (init_from_storage && clab::iot_services::storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, key_buf, (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
                ESP_LOGI(TAG, "Founded setting \"%s\":%.*s - size: %u", key_buf, out_size, buffer, out_size);

                if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                    prop_value_buffer[out_size] = '\0';
                    if (programs[k].parse_from((char *)prop_value_buffer) != ESP_OK) {
                        ESP_LOGE(TAG, "Deserialization error! Ignoring...");
                        memset(&(programs[k]), 0, sizeof(clab::iot_services::dev_program_t));
                    }
                }
            }
        }

        // --------------------- rule action:
        memset(&rules_action, 0, sizeof(clab::iot_services::des_status_t));
        if (init_from_storage && storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_r_action", (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
            ESP_LOGI(TAG, "Founded setting \"last_r_action\":%.*s (%d bytes)", out_size, buffer, out_size);

            if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                
                clab::iot_services::sprint_array_hex((char *)buffer, prop_value_buffer, prop_size);
                ESP_LOGI(TAG, "Property: %s, Decoded size: %u", buffer, prop_size);

                if (prop_size != sizeof(clab::iot_services::des_status_t)) {
                    ESP_LOGE(TAG, "Deserialization error! Ignoring...");
                }
                else {
                    memcpy(&rules_action, prop_value_buffer, prop_size);
                }
            } 

        }
        // cleaning up
        result = storage_db_remove(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_r_action");
        if (result != ESP_OK) {
            ESP_LOGW(TAG, "Unable to remove <<<last_r_action>>>!");
        }

        // --------------------- control_rules:
        for (int k = 0; k < control_rules.size(); k++) {
            char key_buf[8];
            snprintf(key_buf, 8, "rule%d", k);

            control_rules[k] = clab::iot_services::combined_rule_t<CONFIG_IOT_CRULE_SIZE, CONFIG_IOT_DEVICEUID_MAX_SIZE>();
           
            if (init_from_storage && clab::iot_services::storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, key_buf, (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
                ESP_LOGI(TAG, "Founded setting \"%s\":%.*s - size: %u", key_buf, out_size, buffer, out_size);

                if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                    prop_value_buffer[out_size] = '\0';
                    if (control_rules[k].parse_from((char *)prop_value_buffer) != ESP_OK) {
                        ESP_LOGE(TAG, "Deserialization error! Ignoring...");
                        memset(&(control_rules[k]), 0, sizeof(clab::iot_services::combined_rule_t<CONFIG_IOT_CRULE_SIZE, CONFIG_IOT_DEVICEUID_MAX_SIZE>));
                    }
                }
            }
        }

        // --------------------- control_inner_eval:
        memset(&control_inner_eval, 0, control_inner_eval.size());
        if (init_from_storage && storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_r_eval", (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
            ESP_LOGI(TAG, "Founded setting \"last_r_eval\":%.*s (%d bytes)", out_size, buffer, out_size);

            if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                
                clab::iot_services::sprint_array_hex((char *)buffer, prop_value_buffer, prop_size);
                ESP_LOGI(TAG, "Property: %s, Decoded size: %u", buffer, prop_size);

                if (prop_size != control_inner_eval.size()) {
                    ESP_LOGE(TAG, "Deserialization error! Ignoring...");
                }
                else {
                    memcpy(&control_inner_eval, prop_value_buffer, prop_size);
                }
            } 

        }
        // cleaning up
        result = storage_db_remove(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_r_eval");
        if (result != ESP_OK) {
            ESP_LOGW(TAG, "Unable to remove <<<last_r_eval>>>!");
        }

        // --------------------- control_inner_last_measure:
        memset(&control_inner_last_measure, 0, control_inner_last_measure.size());
        if (init_from_storage && storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_r_measure", (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
            ESP_LOGI(TAG, "Founded setting \"last_r_measure\":%.*s (%d bytes)", out_size, buffer, out_size);

            if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                
                clab::iot_services::sprint_array_hex((char *)buffer, prop_value_buffer, prop_size);
                ESP_LOGI(TAG, "Property: %s, Decoded size: %u", buffer, prop_size);

                if (prop_size != control_inner_last_measure.size()) {
                    ESP_LOGE(TAG, "Deserialization error! Ignoring...");
                }
                else {
                    memcpy(&control_inner_last_measure, prop_value_buffer, prop_size);
                }
            } 

        }
        // cleaning up
        result = storage_db_remove(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_r_measure");
        if (result != ESP_OK) {
            ESP_LOGW(TAG, "Unable to remove <<<last_r_measure>>>!");
        }

        // --------------------- latch_logic_status:
        for (int k = 0; k < latch_logic_status.size(); k++)
            latch_logic_status[k] = false;

        if (init_from_storage && storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_l_status", (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
            ESP_LOGI(TAG, "Founded setting \"last_l_status\":%.*s (%d bytes)", out_size, buffer, out_size);

            if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                
                clab::iot_services::sprint_array_hex((char *)buffer, prop_value_buffer, prop_size);
                ESP_LOGI(TAG, "Property: %s, Decoded size: %u", buffer, prop_size);

                if (prop_size != latch_logic_status.size()) {
                    ESP_LOGE(TAG, "Deserialization error! Ignoring...");
                }
                else {
                    memcpy(&latch_logic_status, prop_value_buffer, prop_size);
                }
            } 
        }
        // cleaning up
        result = storage_db_remove(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_l_status");
        if (result != ESP_OK) {
            ESP_LOGW(TAG, "Unable to remove <<<last_l_status>>>!");
        }
        
        // --------------------- relay_logic_status:
        for (int k = 0; k < relay_logic_status.size(); k++)
            relay_logic_status[k] = false;

        if (init_from_storage && storage_db_get(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_r_status", (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
            ESP_LOGI(TAG, "Founded setting \"last_r_status\":%.*s (%d bytes)", out_size, buffer, out_size);

            if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size) == 0) {
                
                clab::iot_services::sprint_array_hex((char *)buffer, prop_value_buffer, prop_size);
                ESP_LOGI(TAG, "Property: %s, Decoded size: %u", buffer, prop_size);

                if (prop_size != relay_logic_status.size()) {
                    ESP_LOGE(TAG, "Deserialization error! Ignoring...");
                }
                else {
                    memcpy(&relay_logic_status, prop_value_buffer, prop_size);
                }
            } 
        }
        // cleaning up
        result = storage_db_remove(CONFIG_IOT_IO_STORAGE_NAMESPACE, "last_r_status");
        if (result != ESP_OK) {
            ESP_LOGW(TAG, "Unable to remove <<<last_r_status>>>!");
        }

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

    esp_err_t ctrl_latch_set_port(size_t idx, port_conf_t &port_conf) {
        
        esp_err_t result;
        
        xSemaphoreTake(ctrl_mutex, portMAX_DELAY);
        result = ports.set_latch_port(idx, port_conf);
        xSemaphoreGive(ctrl_mutex);

        return result;
    }

    
    esp_err_t ctrl_relay_set_port(size_t idx, port_conf_t &port_conf) {
        
        esp_err_t result;
        
        xSemaphoreTake(ctrl_mutex, portMAX_DELAY);
        result = ports.set_relay_port(idx, port_conf);
        xSemaphoreGive(ctrl_mutex);

        return result;
    }

    esp_err_t ctrl_rule_set(size_t idx, clab::iot_services::combined_rule_t<CONFIG_IOT_CRULE_SIZE, CONFIG_IOT_DEVICEUID_MAX_SIZE> &rule){
        if (idx > control_rules.size()) {
            ESP_LOGE(TAG, "Rule[%d] outside of bounds, ignoring...", idx);
            return ESP_FAIL;
        }
        
        xSemaphoreTake(ctrl_mutex, portMAX_DELAY);

        control_rules[idx].action.index = rule.action.index;
        control_rules[idx].action.type = rule.action.type;
        
        for (int k = 0; k <  control_rules[idx].n_rules(); k++) {
            control_rules[idx].rules[k].op = rule.rules[k].op;
            memcpy(control_rules[idx].rules[k].target, rule.rules[k].target, control_rules[idx].rules[k].target_size());
            control_rules[idx].rules[k].port.index = rule.rules[k].port.index;
            control_rules[idx].rules[k].port.type = rule.rules[k].port.type;
            control_rules[idx].rules[k].value = rule.rules[k].value;
        }

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
        // to.led_mask = from.led_mask;
    }

    esp_err_t ctrl_rules_eval(const char *sender, const uint8_t *payload, size_t payload_size) {

        clab::iot_services::dev_status_t received_status(payload, payload_size);
        if (!received_status.is_valid()) {
            ESP_LOGE(TAG, "Received device status is not valid!");
            return ESP_ERR_INVALID_ARG;
        }

        esp_err_t to_ret = ESP_OK;
        
        xSemaphoreTake(ctrl_mutex, portMAX_DELAY);
        for (size_t rule_idx = 0; rule_idx < control_rules.size(); rule_idx++) {
            ESP_LOGI(TAG, "Rule[%d]  starting comparison...", rule_idx);
            
            for (size_t inner_idx = 0; inner_idx < control_rules[rule_idx].n_rules(); inner_idx++) {
                
                auto& inner = control_rules[rule_idx].rules[inner_idx];

                if (inner.op == clab::iot_services::unary_op_t::NOPE) {
                    ESP_LOGI(TAG, "Rule[%d, %d] is empty! ...so is good!", rule_idx, inner_idx);
                    control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx] = true;
                    control_inner_last_measure[rule_idx * control_rules[rule_idx].n_rules() + inner_idx] = NAN;
                    continue;
                }

                ESP_LOGI(TAG, "Rule[%d, %d] target: %s - sender: %s", rule_idx, inner_idx, inner.target, sender);
                
                if (strcmp(inner.target, sender) == 0) {
                    float measure = NAN;
                    ESP_LOGI(TAG, "Rule[%d, %d] target match: %s", rule_idx, inner_idx, inner.target);

                    
                    if (inner.port.type == clab::iot_services::port_type_t::RELAY) {
                        measure = received_status.relay_status(inner.port.index) ? 1 : 0;
                    }
                    else if (inner.port.type == clab::iot_services::port_type_t::LATCH) {
                        measure = received_status.latch_status(inner.port.index) ? 1 : 0;
                    }
                    else if (inner.port.type == clab::iot_services::port_type_t::DIGITAL) {
                        measure = received_status.digital_status(inner.port.index) ? 1 : 0;
                    }
                    else if (inner.port.type == clab::iot_services::port_type_t::PULSE) {
                        if (inner.port.index > received_status.n_pulse()) {
                            ESP_LOGE(TAG, "Rule<%d> is malformed! Requested P[%d, %s] (max: P[%d])", rule_idx, inner.port.index, inner.target, received_status.n_pulse());
                            to_ret = ESP_ERR_INVALID_STATE;
                        }
                        else {
                            measure = received_status.pulse_value(inner.port.index);
                        }
                    }
                    else if (inner.port.type == clab::iot_services::port_type_t::CURRENT) {
                        if (inner.port.index > received_status.n_curr()) {
                            ESP_LOGE(TAG, "Rule<%d> is malformed! Requested C[%d, %s] (max: C[%d])", rule_idx, inner.port.index, inner.target, received_status.n_curr());
                            to_ret = ESP_ERR_INVALID_STATE;
                        }
                        else {
                            measure = received_status.current_value(inner.port.index);
                        }
                    }
                    else if (inner.port.type == clab::iot_services::port_type_t::VOLTAGE) {
                        if (inner.port.index > received_status.n_volt()) {
                            ESP_LOGE(TAG, "Rule<%d> is malformed! Requested V[%d, %s] (max: V[%d])", rule_idx, inner.port.index, inner.target, received_status.n_volt());
                            to_ret = ESP_ERR_INVALID_STATE;
                        }
                        else {
                            measure = received_status.voltage_value(inner.port.index);
                        }
                    }
                    else if (inner.port.type == clab::iot_services::port_type_t::TEMPERATURE) {
                        if (inner.port.index > received_status.n_temperature()) {
                            ESP_LOGE(TAG, "Rule<%d> is malformed! Requested T[%d, %s] (max: T[%d])", rule_idx, inner.port.index, inner.target, received_status.n_temperature());
                            to_ret = ESP_ERR_INVALID_STATE;
                        }
                        else {
                            measure = received_status.temperature_value(inner.port.index);
                        }
                    }

                    ESP_LOGI(TAG, "Rule[%d, %d] measure: %f", rule_idx, inner_idx, measure);

                    if (!std::isnan(measure)) {

                        if (inner.op == clab::iot_services::unary_op_t::EQ) {
                            control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx]  = (measure == inner.value);
                        }
                        else if (inner.op == clab::iot_services::unary_op_t::NEQ) {
                            control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx]  = (measure != inner.value);
                        }
                        else if (inner.op == clab::iot_services::unary_op_t::GT) {
                            control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx]  = (measure > inner.value);
                        }
                        else if (inner.op == clab::iot_services::unary_op_t::GTE) {
                            control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx]  = (measure >= inner.value);
                        }
                        else if (inner.op == clab::iot_services::unary_op_t::LT) {
                            control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx]  = (measure < inner.value);
                        }
                        else if (inner.op == clab::iot_services::unary_op_t::LTE) {
                            control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx]  = (measure <= inner.value);
                        }
                        else if (inner.op == clab::iot_services::unary_op_t::CHANGE) {
                            auto last_measure = control_inner_last_measure[rule_idx * control_rules[rule_idx].n_rules() + inner_idx];
                            if (!std::isnan(last_measure)) {
                                control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx]  = (std::abs(measure - last_measure) >= inner.value);
                            }
                        }
                    }

                    ESP_LOGI(TAG, "Rule[%d, %d] evaluated: %d", rule_idx, inner_idx, control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx] ? 1 : 0);

                    control_inner_last_measure[rule_idx * control_rules[rule_idx].n_rules() + inner_idx] = measure;

                }
            }

            // Now all inner rules have been evaluated (i.e. whenever each target device has sent at least one telemetry)
            bool active = true;
            for (size_t inner_idx = 0; inner_idx < control_rules[rule_idx].n_rules(); inner_idx++) {
                active &= control_inner_eval[rule_idx * control_rules[rule_idx].n_rules() + inner_idx];
            }
            ESP_LOGI(TAG, "Rule[%d] evaluated: %d", rule_idx, active ? 1 : 0);

            if (control_rules[rule_idx].action.type == clab::iot_services::port_type_t::RELAY) {
                rules_action.relay_status(control_rules[rule_idx].action.index, active);
                if (active) {
                    ESP_LOGI(TAG, "Rule<%d> active! R[%d] desired on!", rule_idx, control_rules[rule_idx].action.index);
                }
            }
            else if (control_rules[rule_idx].action.type == clab::iot_services::port_type_t::LATCH) {
                rules_action.latch_status(control_rules[rule_idx].action.index, active);
                if (active) {
                    ESP_LOGI(TAG, "Rule<%d> active! L[%d] desired on!", rule_idx, control_rules[rule_idx].action.index);
                }
            }
            // else if (control_rules[rule_idx].action.type == clab::iot_services::port_type_t::LED) {
            //     rules_action.led_status(control_rules[rule_idx].action.index, active);
            //     if (active) {
            //         ESP_LOGI(TAG, "Rule<%d> active! Led[%d] desired on!", rule_idx, control_rules[rule_idx].action.index);
            //     }
            // }
        }
        xSemaphoreGive(ctrl_mutex);

        return to_ret;
    }

    void ctrl_program_loop(uint32_t actual_ts, dev_status_t &status, des_status_t &des_status, int k) {
        
        if (programs[k].end_ts > actual_ts && actual_ts >= programs[k].start_ts) {

            auto relative_ts = actual_ts % programs[k].period();

            if (relative_ts <= programs[k].duration) {
                ESP_LOGI(TAG, "Prog[%d] active!", k);
  
                for (int idx = 0; idx < 32; idx++) {
                    if (programs[k].latch_status(idx)) {
                        des_status.latch_status(idx, true);
                    }
    
                    if (programs[k].relay_status(idx)) {
                        des_status.relay_status(idx, true);
                    }
                }
            }

        }
    }

    esp_err_t ctrl_loop(uint32_t actual_ts, dev_status_t &status, des_status_t &overrides, 
            des_status_t *out_logic, bool output_enabled) {

        des_status_t desired;
        ctrl_copy_des_status(overrides, desired);

        xSemaphoreTake(ctrl_mutex, portMAX_DELAY);

        // merge rule override
        desired.latch_mask  |= rules_action.latch_mask;
        desired.relay_mask  |= rules_action.relay_mask;
        // desired.led_mask    |= rules_action.led_mask;

        for (int k = 0; k < programs.size(); k++) 
            ctrl_program_loop(actual_ts, status, desired, k);
        
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

    esp_err_t ctrl_save_status() {

        char        encoded_buffer[256];
        size_t      encoded_size;
        esp_err_t   result;

        // --------------------- rule action:
        if (mbedtls_base64_encode((unsigned char *)encoded_buffer, sizeof(encoded_buffer), 
                &encoded_size, (const unsigned char *)&rules_action, sizeof(clab::iot_services::des_status_t)) < 0) {
            ESP_LOGE(TAG, "Unable to byte64 encode rules_action, too big!");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Saving <last_r_action>: %.*s, (%d bytes)", encoded_size, encoded_buffer, encoded_size);
        result = clab::iot_services::storage_db_set(CONFIG_IOT_IO_STORAGE_NAMESPACE, 
                "last_r_action", encoded_buffer, encoded_size); 
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to save property!");
            return ESP_FAIL;
        }

        // --------------------- control_inner_eval:
        if (mbedtls_base64_encode((unsigned char *)encoded_buffer, sizeof(encoded_buffer), 
                &encoded_size, (const unsigned char *)&control_inner_eval, control_inner_eval.size()) < 0) {
            ESP_LOGE(TAG, "Unable to byte64 encode last_r_eval, too big!");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Saving <last_r_eval>: %.*s, (%d bytes)", encoded_size, encoded_buffer, encoded_size);
        result = clab::iot_services::storage_db_set(CONFIG_IOT_IO_STORAGE_NAMESPACE, 
                "last_r_eval", encoded_buffer, encoded_size); 
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to save property!");
            return ESP_FAIL;
        }

        // --------------------- control_inner_last_measure:
        if (mbedtls_base64_encode((unsigned char *)encoded_buffer, sizeof(encoded_buffer), 
                &encoded_size, (const unsigned char *)&control_inner_last_measure, control_inner_last_measure.size()) < 0) {
            ESP_LOGE(TAG, "Unable to byte64 encode last_r_measure, too big!");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Saving <last_r_measure>: %.*s, (%d bytes)", encoded_size, encoded_buffer, encoded_size);
        result = clab::iot_services::storage_db_set(CONFIG_IOT_IO_STORAGE_NAMESPACE, 
                "last_r_measure", encoded_buffer, encoded_size); 
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to save property!");
            return ESP_FAIL;
        }

        // --------------------- latch_logic_status:
        if (mbedtls_base64_encode((unsigned char *)encoded_buffer, sizeof(encoded_buffer), 
                &encoded_size, (const unsigned char *)&latch_logic_status, latch_logic_status.size()) < 0) {
            ESP_LOGE(TAG, "Unable to byte64 encode last_l_status, too big!");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Saving <last_l_status>: %.*s, (%d bytes)", encoded_size, encoded_buffer, encoded_size);
        result = clab::iot_services::storage_db_set(CONFIG_IOT_IO_STORAGE_NAMESPACE, 
                "last_l_status", encoded_buffer, encoded_size); 
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to save property!");
            return ESP_FAIL;
        }

        // --------------------- relay_logic_status:
        if (mbedtls_base64_encode((unsigned char *)encoded_buffer, sizeof(encoded_buffer), 
                &encoded_size, (const unsigned char *)&relay_logic_status, relay_logic_status.size()) < 0) {
            ESP_LOGE(TAG, "Unable to byte64 encode last_r_status, too big!");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Saving <last_r_status>: %.*s, (%d bytes)", encoded_size, encoded_buffer, encoded_size);
        result = clab::iot_services::storage_db_set(CONFIG_IOT_IO_STORAGE_NAMESPACE, 
                "last_r_status", encoded_buffer, encoded_size); 
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to save property!");
            return ESP_FAIL;
        }

        return ESP_OK;
    }

}
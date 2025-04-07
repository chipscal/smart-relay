// #include "plugin/control.h"

// #include "esp_log.h"
// #include "esp_system.h"
// #include "mbedtls/base64.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/semphr.h"
// #include "freertos/task.h"

// #include "string.h"
// #include <string>
// #include <array>

// #include "iot_services/iot_services.h"
// #include "iot_services/rtc_service.h"
// #include "iot_services/io_service.h"
// #include "iot_services/storage_service.h"
// #include "iot_services/comm_api.h"
// #include "iot_services/board.h"
// #include "plugin/comm.h"

// static const char *TAG = "plugins::control";

// namespace clab::plugins {

//     TaskHandle_t    control_task_handle             = NULL;
//     uint32_t        control_task_loop_cnt           = 0;
//     uint32_t        control_latest_telem_loop_cnt   = 0;


//     clab::iot_services::dev_status_t                                       status;

//     clab::iot_services::des_status_t                                       overrides;

//     clab::iot_services::des_status_t                                       latest_o;

//     SemaphoreHandle_t                                                      control_mutex = NULL;


//     static bool on_property_update(const char *alias, uint8_t *payload, size_t payload_size);
//     static bool on_cmd_received(const char *command);

//     void control_task(void *params) {

//         uint8_t buffer[clab::iot_services::alligned_big_enough(clab::iot_services::io_buffer_report_size + 2)];
//         memset(&latest_o, 0, sizeof(clab::iot_services::des_status_t));

//         while (true) {
//             vTaskDelay(pdMS_TO_TICKS(CONFIG_CONTROL_LOOP_INTERVAL_MILLIS));

//             esp_err_t result;
//             uint32_t actual_ts = clab::iot_services::rtc_get_utc();

//             // status update
//             control_task_loop_cnt++;
//             if (control_task_loop_cnt % CONFIG_CONTROL_FULL_STATUS_EVERY_LOOPS == 0) {
//                 result = clab::iot_services::io_buffer_report(buffer, sizeof(buffer), true, false);
//                 if (result != ESP_OK) {
//                     ESP_LOGE(TAG, "Error during full status report!");
//                     //TODO: che fare?
//                 }
//                 else {
//                     if (!clab::iot_services::edge_comm_status_from(buffer, sizeof(buffer), status)) {
//                         ESP_LOGE(TAG, "Error during status deserialization!");
//                         //TODO: che fare?
//                     }
//                 }

//             }
//             else {
//                 result = clab::iot_services::io_buffer_report(buffer, sizeof(buffer), false, false);
//                 if (result != ESP_OK) {
//                     ESP_LOGE(TAG, "Error during partial status report!");
//                     //TODO: che fare?
//                 }
//                 else {
//                     clab::iot_services::dev_status_t tmp_status;
//                     if (!clab::iot_services::edge_comm_status_from(buffer, sizeof(buffer), tmp_status)) {
//                         ESP_LOGE(TAG, "Error during status deserialization!");
//                         //TODO: che fare?
//                     }
//                     else {
//                         // Note: we ignore analog values cos requires power 
//                         // (we will meausure them every CONFIG_CONTROL_FULL_STATUS_EVERY_LOOPS);
//                         status.hrev = tmp_status.hrev;
//                         status.srev = tmp_status.srev;
//                         status.n_ext = tmp_status.n_ext;
//                         status.battery = tmp_status.battery;
//                         status.latch_mask = tmp_status.latch_mask;
//                         status.relay_mask = tmp_status.relay_mask;
//                         for (int k = 0; k < clab::iot_services::io_n_pulse; k++)
//                             status.pulse[k] = tmp_status.pulse[k];
//                     }
//                 }
//             }

//             clab::iot_services::des_status_t tmp_latest_o;
//             clab::iot_services::ctrl_copy_des_status(latest_o, tmp_latest_o);

//             result = clab::iot_services::ctrl_loop(actual_ts, status, overrides, &tmp_latest_o);
//             if (result != ESP_OK) {
//                 ESP_LOGE(TAG, "Error during ctrl loop!");
//                 //TODO: che fare?
//             }

//             if (control_task_loop_cnt - control_latest_telem_loop_cnt > CONFIG_CONTROL_SEND_STATUS_MAX_EVERY_LOOPS &&
//                     (tmp_latest_o.latch_mask != latest_o.latch_mask || tmp_latest_o.relay_mask != latest_o.relay_mask)) {
                
//                 result = clab::iot_services::io_buffer_report(buffer, sizeof(buffer), true);
//                 if (result == ESP_OK) {
//                     ESP_LOGI(TAG, "Sending telemetry due output ports change!");
//                     result = comm_emit_telem(buffer, clab::iot_services::io_buffer_report_size);
//                     if (result != ESP_OK) {
//                         ESP_LOGE(TAG, "Error during telem queue!");
//                         //TODO: che fare?
//                     }
//                 }
//                 control_latest_telem_loop_cnt = control_task_loop_cnt;
//             }

//             clab::iot_services::ctrl_copy_des_status(tmp_latest_o, latest_o);
            
//             ESP_LOGD(TAG, "Control loop done...");
//         }
//     }
    
//     esp_err_t control_plugin() {
//         constexpr unsigned int buffer_needs[] = {
//                 clab::iot_services::io_buffer_report_size, 
//                 32 + 1
//         };

//         if (control_mutex == NULL) {
//             control_mutex = xSemaphoreCreateMutex(); //In tests may have been already allocated
//             ESP_LOGI(TAG, "Control plugin mutex created...");
//         }

//         uint8_t buffer[clab::iot_services::alligned_big_enough(clab::iot_services::array_max(buffer_needs))];
//         size_t  out_size = 0;
//         uint8_t prop_value_buffer[clab::iot_services::alligned_big_enough(2 * sizeof(uint32_t))];
//         size_t  prop_size = 0;

//         memset(&overrides, 0, sizeof(clab::iot_services::des_status_t));

//         ESP_ERROR_CHECK(clab::iot_services::io_buffer_report(buffer, sizeof(buffer), true));
//         if (!clab::iot_services::edge_comm_status_from(buffer, sizeof(buffer), status)) {
//             ESP_LOGE(TAG, "Error during status deserialization!");
//             abort();
//         }

//         ESP_ERROR_CHECK(clab::iot_services::ctrl_init(status, true));
//         ESP_LOGI(TAG, "Control service intialized...");

//         ESP_ERROR_CHECK(comm_register_prop_listener(0, on_property_update));
//         ESP_ERROR_CHECK(comm_register_cmd_listener(0, on_cmd_received));

//         if ( clab::iot_services::storage_db_get(CONFIG_EDGE_IO_STORAGE_NAMESPACE, "over", (char *)buffer, sizeof(buffer), &out_size) == ESP_OK) {
//             ESP_LOGI(TAG, "Founded setting \"over\":%s - size: %u", buffer, out_size);
            
//             if (mbedtls_base64_decode(prop_value_buffer, sizeof(prop_value_buffer), &prop_size, buffer, out_size - 1) == 0) {
                
//                 clab::iot_services::sprint_array_hex((char *)buffer, prop_value_buffer, prop_size);
//                 ESP_LOGI(TAG, "Property: %s, Decoded size: %u", buffer, prop_size);

//                 uint32_t mask = 0;
//                 memcpy((uint8_t *)&(mask), prop_value_buffer, sizeof(uint32_t));
//                 if (!clab::iot_services::is_little_endian()) {
//                     mask = clab::iot_services::swap_uint32(mask);
//                 }
//                 overrides.latch_mask = mask;

//                 memcpy((uint8_t *)&(overrides.relay_mask), prop_value_buffer + sizeof(uint32_t), sizeof(uint32_t));
//                 if (!clab::iot_services::is_little_endian()) {
//                     overrides.relay_mask = clab::iot_services::swap_uint32(overrides.relay_mask);
//                 }
//             } 
            
//             clab::iot_services::sprint_uint32_binary((char *)buffer, overrides.latch_mask);
//             ESP_LOGI(TAG, "Latch override: %s", buffer);

//             clab::iot_services::sprint_uint32_binary((char *)buffer, overrides.relay_mask);
//             ESP_LOGI(TAG, "Relay override: %s", buffer);
//         }

//         control_task_loop_cnt = 0;
//         auto task_result = xTaskCreate(control_task, "control", 4096, NULL, tskIDLE_PRIORITY, &control_task_handle); 
//         if (task_result != pdPASS ) {
//             ESP_LOGE(TAG, "Error occurred during send task creation...");
//             return ESP_FAIL;
//         }
//         ESP_LOGI(TAG, "Control task launched...");

//         return ESP_OK;
//     }

//     // ----------------------------private implementations:

//     static bool on_property_update(const char *alias, uint8_t *payload, size_t payload_size) {
//         esp_err_t result;
//         std::string alias_string = std::string(alias, strlen(alias));

//         ESP_LOGI(TAG, "Received property: %s", alias);

//         if (alias_string.compare("ports") == 0) {
//             clab::iot_services::ports_conf_t ports;
//             if (!clab::iot_services::edge_comm_ports_conf_from(payload, payload_size, ports)) {
//                 ESP_LOGE(TAG, "Unable to deserialize ports!");
//                 return false;
//             }
//             result = clab::iot_services::ctrl_set_ports(ports);
//             if (result != ESP_OK) {
//                 ESP_LOGE(TAG, "Ports Unable to set!");
//                 return false;
//             }
//         }

//         if (alias_string.compare("over") == 0) {
//             memcpy(&(overrides.latch_mask), payload, sizeof(uint32_t));
//             if (!clab::iot_services::is_little_endian()) {
//                 overrides.latch_mask = clab::iot_services::swap_uint32(overrides.latch_mask);
//             }

//             memcpy(&(overrides.relay_mask), payload + sizeof(uint32_t), sizeof(uint32_t));
//             if (!clab::iot_services::is_little_endian()) {
//                 overrides.relay_mask = clab::iot_services::swap_uint32(overrides.relay_mask);
//             }
//         }

//         if (alias_string.compare("pdelay") == 0) {
//             uint16_t delays[clab::iot_services::io_n_pulse];
//             if (payload_size != sizeof(delays)) {
//                 ESP_LOGE(TAG, "pdelay size is invalid!");
//                 return false;
//             }

//             for (int k = 0; k < clab::iot_services::io_n_pulse; k++) {
//                 uint16_t value = 0;
//                 memcpy((uint8_t *)&value, payload + k * sizeof(uint16_t), sizeof(uint16_t));

//                 if (!clab::iot_services::is_little_endian()) {
//                     value = clab::iot_services::swap_uint16(value);
//                 }

//                 delays[k] = value;
//             }

//             esp_err_t result = clab::iot_services::io_pulse_filter_set(delays, clab::iot_services::io_n_pulse);
//             if (result != ESP_OK) {
//                 ESP_LOGE(TAG, "Unable to set pulse filter delays!");
//                 return false;
//             }
//         }

//         return true;
//     }

//     static bool on_cmd_received(const char *command) {
//         esp_err_t result;
//         std::string cmd_string = std::string(command, strlen(command));

//         ESP_LOGI(TAG, "Received cmd: %s", command);
        
//         if (cmd_string.compare("restart") == 0) {
//             ESP_LOGI(TAG, "Board is going to be restarted...");
//             clab::iot_services::board_clean_restart();
//         }
//         else if (cmd_string.compare("restart-hard") == 0) {
//             ESP_LOGI(TAG, "Board is going to be restarted...");
//             esp_restart();
//         }
//         else if (cmd_string.compare("refresh") == 0) {
//             ESP_LOGI(TAG, "Starting latch refresh sequence...");
//             result = clab::iot_services::io_latch_refresh();
//             if (result != ESP_OK) {
//                 ESP_LOGE(TAG, "Error during latch refresh: %s", esp_err_to_name(result));
//                 return false;
//             }
//         }

//         return true;
//     }
// }
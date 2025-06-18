#include <stdio.h>

#include "esp_check.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "iot_services/iot_services.h"
#include "iot_services/io_service.h"

#include "plugin/startup.h"
#include "plugin/comm.h"
#include "plugin/control.h"


#include "esp_log.h"
#include "ble/gap.h"
#include "nvs_flash.h"
#include "ble/gatt_svr.h"
#include "esp_ota_ops.h"
#include "mbedtls/base64.h"


const char *MAIN_APP_TAG    = "MAIN";
const char *OTA_APP_TAG     = "MAIN_OTA";

uint32_t     main_app_time_refresh = 0;
int64_t      main_ble_disconnected_from_micros = 0;
bool         main_ble_is_active = true;


bool run_diagnostics() {
  // do some diagnostics
  return true;
}
extern "C" void app_main(void)
{
    
    const esp_partition_t *partition = esp_ota_get_running_partition();

    switch (partition->address) {
        
        case 0x00010000:
        ESP_LOGI(OTA_APP_TAG, "Running partition: factory");
        break;
        case 0x00110000:
        ESP_LOGI(OTA_APP_TAG, "Running partition: ota_0");
        break;
        case 0x00210000:
        ESP_LOGI(OTA_APP_TAG, "Running partition: ota_1");
        break;

        default:
        ESP_LOGE(OTA_APP_TAG, "Running partition: unknown");
        break;
    }

    // check if an OTA has been done, if so run diagnostics
    esp_ota_img_states_t ota_state;


    if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK) {

        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
        ESP_LOGI(OTA_APP_TAG, "An OTA update has been detected.");
        if (run_diagnostics()) {
            ESP_LOGI(OTA_APP_TAG,
                    "Diagnostics completed successfully! Continuing execution.");
            esp_ota_mark_app_valid_cancel_rollback();
        } else {
            ESP_LOGE(OTA_APP_TAG,
                    "Diagnostics failed! Start rollback to the previous version.");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }
        }
    }

    // Initialize NVS
    auto value = clab::iot_services::is_little_endian();
    ESP_LOGI(MAIN_APP_TAG, "The system is little endian: %s", value ? "true" : "false");

    ESP_ERROR_CHECK(clab::plugins::startup_plugin());

    ESP_ERROR_CHECK(clab::iot_services::io_led_cmd(0, true));
    
    //-------------------------------------- TODO: others plugin goes here:
    ESP_ERROR_CHECK(clab::plugins::comm_plugin());

    
    bool is_local_broker = false;
    #if CONFIG_MAIN_MQTT_BROKER_DIN_CONF_CHANNEL != UINT8_MAX
    if (clab::iot_services::io_digital_input_status(CONFIG_MAIN_MQTT_BROKER_DIN_CONF_CHANNEL)) {
            
            ESP_ERROR_CHECK(clab::plugins::comm_discovery_server_start());
            
            ESP_ERROR_CHECK(clab::plugins::comm_mqtt_broker_start());
            vTaskDelay(pdMS_TO_TICKS(10000));
            is_local_broker = true;
        }
    #endif


    //TODO: do only if not local broker
    clab::plugins::comm_discovery_request_server_info(NULL, 16, NULL);

    clab::plugins::comm_mqtt_client_start(is_local_broker);
    vTaskDelay(pdMS_TO_TICKS(5000));

    ESP_ERROR_CHECK(clab::plugins::control_plugin());

    // ...
    //--------------------------------------------------------------------

    
    // ----------------------------------------------- BLE Setup:

    // initialize BLE controller and nimble stack
    esp_nimble_hci_init(); 
    ESP_ERROR_CHECK(nimble_port_init());

    // register sync and reset callbacks
    ble_hs_cfg.sync_cb = gap_sync_cb;
    ble_hs_cfg.reset_cb = gap_reset_cb;

    // initialize service table
    gatt_svr_init();

    // set device name and start host task
    //clab-xxx
    strcpy(gap_device_name, clab::plugins::comm_get_device_uid());
    ble_svc_gap_device_name_set(gap_device_name);

    nimble_port_freertos_init(gap_host_task);
    main_ble_is_active = true;
    main_ble_disconnected_from_micros = esp_timer_get_time();
    
    vTaskDelay(pdMS_TO_TICKS(CONFIG_STARTUP_LED_TIME_ON_MILLIS));
    ESP_ERROR_CHECK(clab::iot_services::io_led_cmd(0, false));

    char topic_buffer[64];
    sprintf(topic_buffer, "/dev/%s/telem", clab::plugins::comm_get_device_uid());
    
    uint8_t telem_buffer[clab::iot_services::alligned_big_enough(clab::iot_services::io_buffer_report_size)];
    
    size_t  encoded_size;
    uint8_t encoded_buffer[clab::iot_services::alligned_big_enough(clab::iot_services::io_buffer_report_size * 2)];

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(CONFIG_MAIN_LOOP_INTERVAL_MILLIS));
        
        // ------------------------- Start Telemetry publish
        esp_err_t result = clab::iot_services::io_buffer_report(telem_buffer, sizeof(telem_buffer), true);
        if (result == ESP_OK) {
            if (mbedtls_base64_encode((unsigned char *)encoded_buffer, sizeof(encoded_buffer), 
                    &encoded_size, telem_buffer, clab::iot_services::io_buffer_report_size) < 0) {
                ESP_LOGE(MAIN_APP_TAG, "Unable to byte64 encode telemetry, too big!");
                return;
            }
            if (encoded_size == 256) {
                ESP_LOGE(MAIN_APP_TAG, "Unable to null terminate encoded telemetry, too big!");
                return;
            }
            encoded_buffer[encoded_size] = '\0';

            result = clab::plugins::comm_pub_message(topic_buffer, encoded_buffer, encoded_size);
            if (result != ESP_OK) {
                ESP_LOGE(MAIN_APP_TAG, "Cannot pub telemetry!");
            }
        }
        else {
            ESP_LOGE(MAIN_APP_TAG, "Report generation failed!");
        }

        

        // ------------------------- End Telemetry publish
        if (main_app_time_refresh % CONFIG_MAIN_REFRESH_TIME_EVERY_LOOPS == 0) {
            clab::plugins::comm_refresh_rtc();
        }

        if (main_ble_is_active) {
            if (gap_is_connected)
                main_ble_disconnected_from_micros = esp_timer_get_time();

            #if CONFIG_MAIN_DISABLE_BLE_AFTER_MILLIS > 0
                if (esp_timer_get_time() - main_ble_disconnected_from_micros > CONFIG_MAIN_DISABLE_BLE_AFTER_MILLIS * 1000LL) {
                    nimble_port_deinit();
                    esp_nimble_hci_deinit();
                    main_ble_is_active = false;
                    ESP_LOGW(MAIN_APP_TAG, "BLE has been turned off due to inactiviy!");
                }
            #endif
        }

        main_app_time_refresh++;
        ESP_LOGI(MAIN_APP_TAG, "Everything is going smooth as hell!");
    }
    
}
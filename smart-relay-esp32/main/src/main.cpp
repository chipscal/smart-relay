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


#include "esp_log.h"
#include "ble/gap.h"
#include "nvs_flash.h"
#include "ble/gatt_svr.h"
#include "esp_ota_ops.h"


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
    //irreo-xxx
    strcpy(gap_device_name + 6, clab::plugins::comm_get_device_uid());
    ble_svc_gap_device_name_set(gap_device_name);

    nimble_port_freertos_init(gap_host_task);
    main_ble_is_active = true;
    main_ble_disconnected_from_micros = esp_timer_get_time();
    
    vTaskDelay(pdMS_TO_TICKS(CONFIG_STARTUP_LED_TIME_ON_MILLIS));
    ESP_ERROR_CHECK(clab::iot_services::io_led_cmd(0, false));

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(CONFIG_MAIN_LOOP_INTERVAL_MILLIS));
        
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
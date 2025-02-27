#include "ble/gatt_svr.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

uint8_t gatt_svr_chr_ota_control_val;
uint8_t gatt_svr_chr_ota_data_val[512];

uint16_t ota_control_val_handle;
uint16_t ota_data_val_handle;

uint16_t gatt_svr_notification_handle = 0;
uint16_t num_pkgs_received = 0;
uint16_t ble_packet_size = 0;

bool ota_updating = false;
const char *manuf_name = CONFIG_MANUFACTURE_NAME;
const char *model_num = CONFIG_MODEL_NAME;

TaskHandle_t gap_restart_task_handle = NULL;

const esp_partition_t *update_partition;
esp_ota_handle_t ota_update_handle;

int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len,
                       uint16_t max_len, void *dst, uint16_t *len);

int gatt_svr_chr_ota_control_cb(uint16_t conn_handle,
                                uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt,
                                void *arg);

int gatt_svr_chr_ota_data_cb(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg);

int gatt_svr_chr_access_device_info(uint16_t conn_handle,
                                    uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt,
                                    void *arg);

int gatt_device_write(uint16_t conn_handle,
                      uint16_t attr_handle,
                      struct ble_gatt_access_ctxt *ctxt,
                      void *arg);

int gatt_device_read(uint16_t conn_handle,
                     uint16_t attr_handle,
                     struct ble_gatt_access_ctxt *ctxt,
                     void *arg);

const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {// Service: Device Information
     .type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
     .characteristics =
         (struct ble_gatt_chr_def[]){
             {
                 // Characteristic: Manufacturer Name
                 .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
                 .access_cb = gatt_svr_chr_access_device_info,
                 .flags = BLE_GATT_CHR_F_READ,
             },
             {
                 // Characteristic: Model Number
                 .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
                 .access_cb = gatt_svr_chr_access_device_info,
                 .flags = BLE_GATT_CHR_F_READ,
             },
             {
                 0,
             },
         }},

    {
        // Service: OTA Service
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_ota_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    // characteristic: OTA control
                    .uuid = &gatt_svr_chr_ota_control_uuid.u,
                    .access_cb = gatt_svr_chr_ota_control_cb,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                             BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &ota_control_val_handle,
                },
                {
                    // characteristic: OTA data
                    .uuid = &gatt_svr_chr_ota_data_uuid.u,
                    .access_cb = gatt_svr_chr_ota_data_cb,
                    .flags = BLE_GATT_CHR_F_WRITE,
                    .val_handle = &ota_data_val_handle,
                },
                {
                    0,
                }},
    },
    {
        // Service: Client Read/Write
        .type = BLE_GATT_SVC_TYPE_PRIMARY, // primary service
        .uuid = BLE_UUID16_DECLARE(0x007),
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    // Characteristic: Client write into esp
                    .uuid = BLE_UUID16_DECLARE(0xABCD),
                    .flags = BLE_GATT_CHR_F_WRITE,
                    .access_cb = gatt_device_write,
                },
                {
                    // Characteristic: Client read from esp
                    .uuid = BLE_UUID16_DECLARE(0xEACD),
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &gatt_svr_notification_handle,
                    .access_cb = gatt_device_read,
                },
                {0}, // End of characteristics} ,
            },
    },
    {
        0,
    }, // End of services
};

// Write data to ESP32 defined as server
int gatt_device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    printf("Data from the client: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);
    return 0;
}

// Read data from ESP32 defined as server
int gatt_device_read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    os_mbuf_append(ctxt->om, "test send/read from server", strlen("send/read test server"));
    return 0;
}

int gatt_svr_chr_access_device_info(uint16_t conn_handle,
                                    uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt,
                                    void *arg) {
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_MODEL_NUMBER_UUID)
    {
        rc = os_mbuf_append(ctxt->om, model_num, strlen(model_num));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_MANUFACTURER_NAME_UUID)
    {
        rc = os_mbuf_append(ctxt->om, manuf_name, strlen(manuf_name));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len,
                       uint16_t max_len, void *dst, uint16_t *len) {
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len)
    {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

void gap_restart_task(void *params) {
    vTaskDelay(pdMS_TO_TICKS(REBOOT_DEEP_SLEEP_TIMEOUT));
    esp_restart();
}

void update_ota_control(uint16_t conn_handle) {
    struct os_mbuf *om;
    esp_err_t err;

    // check which value has been received
    switch (gatt_svr_chr_ota_control_val)
    {
    case SVR_CHR_OTA_CONTROL_REQUEST:
        // OTA request
        ESP_LOGI(LOG_TAG_GATT_SVR, "OTA has been requested via BLE.");
        // get the next free OTA partition
        const esp_partition_t *partition = esp_ota_get_running_partition();
        if (partition == NULL)
        {
            ESP_LOGE("", "esp_ota_get_running_partition returned NULL");
        }
        else
        {
            ESP_LOGI("", "Ok, esp_ota_get_running_partition succeeded");
        }
        update_partition = esp_ota_get_next_update_partition(partition);
        // start the ota update
        err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES,
                            &ota_update_handle);
        if (err != ESP_OK)
        {
            ESP_LOGE(LOG_TAG_GATT_SVR, "esp_ota_begin failed (%s)",
                     esp_err_to_name(err));
            esp_ota_abort(ota_update_handle);
            gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_REQUEST_NAK;
        }
        else
        {
            gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_REQUEST_ACK;
            ota_updating = true;

            // retrieve the packet size from OTA data
            ble_packet_size =
                (gatt_svr_chr_ota_data_val[1] << 8) + gatt_svr_chr_ota_data_val[0];
            ESP_LOGI(LOG_TAG_GATT_SVR, "Packet size is: %d", ble_packet_size);

            num_pkgs_received = 0;
        }

        // notify the client via BLE that the OTA has been acknowledged (or not)
        om = ble_hs_mbuf_from_flat(&gatt_svr_chr_ota_control_val,
                                   sizeof(gatt_svr_chr_ota_control_val));
        ble_gattc_notify_custom(conn_handle, ota_control_val_handle, om);
        ESP_LOGI(LOG_TAG_GATT_SVR, "OTA request acknowledgement has been sent.");

        break;

    case SVR_CHR_OTA_CONTROL_DONE:

        ota_updating = false;

        // end the OTA and start validation
        err = esp_ota_end(ota_update_handle);
        if (err != ESP_OK)
        {
            if (err == ESP_ERR_OTA_VALIDATE_FAILED)
            {
                ESP_LOGE(LOG_TAG_GATT_SVR,
                         "Image validation failed, image is corrupted!");
            }
            else
            {
                ESP_LOGE(LOG_TAG_GATT_SVR, "esp_ota_end failed (%s)!",
                         esp_err_to_name(err));
            }
        }
        else
        {
            // select the new partition for the next boot
            err = esp_ota_set_boot_partition(update_partition);
            if (err != ESP_OK)
            {
                ESP_LOGE(LOG_TAG_GATT_SVR, "esp_ota_set_boot_partition failed (%s)!",
                         esp_err_to_name(err));
            }
        }

        // set the control value
        if (err != ESP_OK)
        {
            gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_DONE_NAK;
        }
        else
        {
            gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_DONE_ACK;
        }

        // notify the client via BLE that DONE has been acknowledged
        om = ble_hs_mbuf_from_flat(&gatt_svr_chr_ota_control_val,
                                   sizeof(gatt_svr_chr_ota_control_val));
        ble_gattc_notify_custom(conn_handle, ota_control_val_handle, om);
        ESP_LOGI(LOG_TAG_GATT_SVR, "OTA DONE acknowledgement has been sent.");

        // restart the ESP to finish the OTA
        if (err == ESP_OK)
        {
            ESP_LOGI(LOG_TAG_GATT_SVR, "Preparing to restart!");
            BaseType_t task_result = xTaskCreate(gap_restart_task, "gap_restart_task", 1024, NULL, tskIDLE_PRIORITY, &gap_restart_task_handle);
            if (task_result != pdPASS)
            {
                ESP_LOGE(LOG_TAG_GATT_SVR, "Error creating restart task... Restarting now...");
                esp_restart();
            }
        }

        break;

    default:
        break;
    }
}

int gatt_svr_chr_ota_control_cb(uint16_t conn_handle,
                                uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt,
                                void *arg) {
    int rc;
    uint8_t length = sizeof(gatt_svr_chr_ota_control_val);

    switch (ctxt->op)
    {

    case BLE_GATT_ACCESS_OP_READ_CHR:
        // a client is reading the current value of ota control
        rc = os_mbuf_append(ctxt->om, &gatt_svr_chr_ota_control_val, length);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        // a client is writing a value to ota control
        rc = gatt_svr_chr_write(ctxt->om, 1, length,
                                &gatt_svr_chr_ota_control_val, NULL);
        // update the OTA state with the new value
        update_ota_control(conn_handle);
        return rc;
        break;

    default:
        break;
    }

    // this shouldn't happen
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

int gatt_svr_chr_ota_data_cb(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg) {
    int rc;
    esp_err_t err;

    // store the received data into gatt_svr_chr_ota_data_val
    rc = gatt_svr_chr_write(ctxt->om, 1, sizeof(gatt_svr_chr_ota_data_val),
                            gatt_svr_chr_ota_data_val, NULL);

    // write the received packet to the partition
    if (ota_updating)
    {
        err = esp_ota_write(ota_update_handle, (const void *)gatt_svr_chr_ota_data_val,
                            ble_packet_size);
        if (err != ESP_OK)
        {
            ESP_LOGE(LOG_TAG_GATT_SVR, "esp_ota_write failed (%s)!",
                     esp_err_to_name(err));
        }

        num_pkgs_received++;
        ESP_LOGI(LOG_TAG_GATT_SVR, "Received packet %d", num_pkgs_received);
    }
    return rc;
}

void gatt_svr_init() {

    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);
}

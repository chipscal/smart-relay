#include "ble/gap.h"

uint8_t gap_addr_type;

char gap_device_name[GAP_DEVICE_NAME_MAX_SIZE] = "irreo-XXXXXXXXXXXXXXXX";
bool gap_is_connected = false;

int gap_event_handler(struct ble_gap_event *event, void *arg);

void gap_advertise() {

    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    memset(&fields, 0, sizeof(fields));

    // flags: discoverability + BLE only
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    // include power levels
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    // include device name
    fields.name = (uint8_t *)gap_device_name;
    fields.name_len = strlen(gap_device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0)
    {
        ESP_LOGI(LOG_TAG_GAP, "Error setting advertisement data: rc=%d", rc);
        return;
    }

    // start advertising
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(gap_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
                           gap_event_handler, NULL);
    if (rc != 0)
    {
        ESP_LOGI(LOG_TAG_GAP, "Error enabling advertisement data: rc=%d", rc);
        return;
    }
}

void gap_reset_cb(int reason) {
    ESP_LOGE(LOG_TAG_GAP, "BLE reset: reason = %d", reason);
}

void gap_sync_cb(void) {
    // determine best adress type
    ble_hs_id_infer_auto(0, &gap_addr_type);

    // start avertising
    gap_advertise();
}

int gap_event_handler(struct ble_gap_event *event, void *arg) {
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        // A new connection was established or a connection attempt failed
        ESP_LOGI(LOG_TAG_GAP, "GAP: Connection %s: status=%d",
                 event->connect.status == 0 ? "established" : "failed",
                 event->connect.status);
        gap_is_connected = true;
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(LOG_TAG_GAP, "GAP: Disconnect: reason=%d\n",
                 event->disconnect.reason);

        // Connection terminated; resume advertising
        gap_advertise();
        gap_is_connected = false;
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(LOG_TAG_GAP, "GAP: adv complete");
        gap_advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(LOG_TAG_GAP, "GAP: Subscribe: conn_handle=%d",
                 event->connect.conn_handle);
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(LOG_TAG_GAP, "GAP: MTU update: conn_handle=%d, mtu=%d",
                 event->mtu.conn_handle, event->mtu.value);
        break;
    }

    return 0;
}

void gap_host_task(void *param) {
    // returns only when nimble_port_stop() is executed
    nimble_port_run();
    nimble_port_freertos_deinit();
}

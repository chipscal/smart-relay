#pragma once

#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#define LOG_TAG_GAP "gap"

#define GAP_DEVICE_NAME_MAX_SIZE 23

#if __cplusplus
extern "C" {
#endif

    /// @brief BLE displayed name.
    extern char    gap_device_name[GAP_DEVICE_NAME_MAX_SIZE];

    /// @brief True if some client is connected.
    extern bool    gap_is_connected;

    /// @brief Starts to advertise with the given advertisment parameters. Throws an error in advertisment data if in case of failure.
    void    gap_advertise();
 
    /// @brief Used when host resets.
    /// @param reason reason for the reset/disconnetion. 
    void    gap_reset_cb(int reason);
    
    void    gap_sync_cb(void);
 
    /// @brief Using nimble api to stop the nimble process. 
    /// @param param 
    void    gap_host_task(void *param);


#if __cplusplus
}
#endif
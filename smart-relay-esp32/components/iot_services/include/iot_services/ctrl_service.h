#pragma once

#include "iot_services/comm_api.h"
#include "iot_services/io_service.h"

#include "esp_err.h"

namespace clab::iot_services {
    
    /// @brief Initialize control service and loads last saved programs and controllers from flash.
    /// @return ESP_OK on success.
    /// @note this function is NOT thread safe and must me called only one time during deinitialization!
    esp_err_t        ctrl_init(bool init_from_storage = true);
    
    /// @brief Cleans up resources.
    /// @return ESP_OK on success.
    /// @note this function is NOT thread safe and must me called only one time during deinitialization!
    esp_err_t        ctrl_deinit();
    
    /// @brief Set port configuration.
    /// @param ports ports to copy
    /// @return ESP_OK on success
    /// @note this function is thread safe.
    esp_err_t        ctrl_set_ports(ports_conf_t<io_n_latch, io_n_relay> &port_conf);


    /// @brief Copies rule configuration to given idx slot
    /// @param idx of the rule
    /// @param rule from where to copy
    /// @return ESP_OK on success
    /// @note this function is thread safe.
    esp_err_t        ctrl_rule_set(size_t idx, clab::iot_services::combined_rule_t<4, 14> &rule);

    /// @brief Evaluates rules using provided payload
    /// @param sender of the device status
    /// @param payload device status payload
    /// @param payload_size size of the payload
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    /// @note If a rule uses multiple targets last received status from such targets is used.
    /// @note This function does not store device statuses internally - no need!
    esp_err_t ctrl_rules_eval(const char *sender, const uint8_t *payload, size_t payload_size);

    /// @brief Executes control loop logic.
    /// @param actual_ts actual timestamp 
    /// @param status device status report
    /// @param overrides used to force outputs
    /// @param out_logic if not NULL the output actual status is reported here (usefull for test)
    /// @param output_enabled if true enebles hardware output control
    /// @return ESP_OK on success.
    /// @note this function is MILDLY thread safe, i.e. programs or control parameters can be modified or used from control logic
    /// one task at time, but ONLY one task should call periodically this function as the usage of an internal status is involved.
    esp_err_t        ctrl_loop(uint32_t actual_ts, dev_status_t &status, des_status_t &overrides, 
            des_status_t *out_logic = NULL, bool output_enabled = true);

}
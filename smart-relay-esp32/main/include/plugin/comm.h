#pragma once

#include "esp_err.h"


namespace clab::plugins {


    typedef void (*comm_message_callback_t)(const char *topic, const char *payload, size_t payload_size);
    
    constexpr int comm_safe_max_packet_size { 1024 };

    /// @brief Starts up comm services.
    /// @return ESP_OK if success.
    esp_err_t comm_plugin();

    /// @brief Updateds the RTC actual timestamp via the network provider.
    /// @return ESP_OK on success.
    esp_err_t comm_refresh_rtc();

    /// @brief Starts the MQTT broker locally.
    /// @return ESP_OK on success.
    esp_err_t comm_mqtt_broker_start();

    /// @brief Starts the MQTT client.
    /// @param is_local_broker if true connects to local broker
    /// @return ESP_OK on success.
    esp_err_t comm_mqtt_client_start(bool is_local_broker);
    
    /// @brief Emit property message.
    /// @param topic of the message
    /// @param payload to send
    /// @param payload_size size of the payload
    /// @return ESP_OK on success.
    esp_err_t comm_pub_message(const char *topic, const uint8_t *payload, size_t payload_size);

    /// @brief Subscribe to topic messages
    /// @param topic to subscribe
    /// @param callback to execute whenever topic message is received
    /// @return ESP_OK on success.
    esp_err_t comm_sub_message(const char *topic, comm_message_callback_t callback);

    /// @brief Get the id of the DEVEUID of Modem.
    /// @return String i.e the DEVEUI.
    const char *comm_get_device_uid();

}

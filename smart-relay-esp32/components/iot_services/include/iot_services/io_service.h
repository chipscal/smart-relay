#pragma once

#include <stdint.h>
#include "esp_err.h"

#include "pinout.h"



namespace clab::iot_services
{ 
    /// @brief #of latch outputs.
    constexpr unsigned int      io_n_latch          { IOT_BOARD_N_LATCH };
    /// @brief #of relay outputs.
    constexpr unsigned int      io_n_relay          { IOT_BOARD_N_RELAY };
    /// @brief #of current sense inputs.
    constexpr unsigned int      io_n_current        { IOT_BOARD_N_CURRENT };
    /// @brief #of voltage sense inputs.
    constexpr unsigned int      io_n_voltage        { IOT_BOARD_N_VOLTAGE };
    /// @brief #of pulse counter inputs.
    constexpr unsigned int      io_n_pulse          { IOT_BOARD_N_PULSE };
    /// @brief #of digitl inputs.
    constexpr unsigned int      io_n_digital        { IOT_BOARD_N_DIGITAL };
    /// @brief #of temperature inputs.
    constexpr unsigned int      io_n_temperature    { IOT_BOARD_N_TEMPERATURE };
    

    /// @brief Report buffer required minimum size.
    constexpr unsigned int      io_buffer_report_size { 
        sizeof(uint32_t) + // HREV|SREV|n_curr[0:3],n_volt[4:7]|n_pulse[0:3],n_temperature[4:7]|
        2 * sizeof(uint32_t) + // Latch0|Latch1|Latch2|Latch3|Relay0|Relay1|Relay2|Relay3 (bit mask)
        sizeof(uint32_t) + // Digital0|Digital1|Digital2|Digital3 (bit mask)
        io_n_current * sizeof(uint16_t) + 
        io_n_voltage * sizeof(uint16_t) + 
        io_n_pulse * sizeof(uint16_t) +
        io_n_temperature * sizeof(uint16_t) 
    };

    /// @brief Initialize io service.
    /// @return ESP_OK on success.
    /// @note this function is NOT thread safe and must me called only one time during deinitialization!
    esp_err_t   io_init();

    /// @brief Cleans up resources.
    /// @retuulsern ESP_OK on success.
    /// @note this function is NOT thread safe and must me called only one time during deinitialization!
    esp_err_t   io_deinit();

    /// @brief Commands onboard LEDs. 
    /// @param led channel,
    /// @param status desired,
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t   io_led_cmd(uint8_t led, bool status);

    /// @brief Check actual LED status.
    /// @param led channel,
    /// @return actual status.
    /// @note this function is thread safe.
    bool        io_led_status(uint8_t led);

    /// @brief Commands relays status (either onboard and extensions).
    /// @param relay logic channel,
    /// @param status desired,
    /// @return ESP_OK on success
    /// @note this function is thread safe.
    esp_err_t   io_relay_cmd(uint8_t relay, bool status);

    /// @brief Commands latches status (either onboard and extensions).
    /// @param latch logic channel,
    /// @param status desired,
    /// @return ESP_OK on success
    /// @note this function is thread safe.
    esp_err_t   io_latch_cmd(uint8_t latch, bool status);

    /// @brief Forces latches status refresh.
    /// @return ESP_OK on success
    /// @note this function is thread safe.
    esp_err_t   io_latch_refresh();

    /// @brief Generates and stores the report inside an internal buffer and invokes the registered (if so) telem callback passing it.
    /// @param enable_power true if 12V should be switched on (i.e. if not analog measures may be compromised),
    /// @param save_pulses true if actual pulse counters should be saved on flash,
    /// @return ESP_Ok on success.
    /// @note Do not to save too much times pulse counters over flash to avoid to rapidly break it (check max write cycles per cell).
    /// @note this function is thread safe.
    esp_err_t   io_telem_report(bool enable_power, bool save_pulses);

    /// @brief Generates and stores to a buffer the actual report.
    /// @param buffer where to store the report,
    /// @param buffer_size size of the buffer,
    /// @param enable_power true if 12V should be switched on (i.e. if not analog measures may be compromised),
    /// @param save_pulses true if actual pulse counters should be saved on flash,
    /// @return ESP_Ok on success.
    /// @note Do not to save too much times pulse counters over flash to avoid to rapidly break it (check max write cycles per cell).
    /// @note this function is thread safe.
    esp_err_t   io_buffer_report(uint8_t *buffer, int buffer_size, bool enable_power, bool save_pulses);

    /// @brief Registers a callback executed everytime io_telem_report is invoked.
    /// @param method a method that elaborates the serialized report.
    /// @note this function is NOT thread safe and must me called only one time during deinitialization!
    void        io_telem_register_callback(void (*method)(const uint8_t *payload, int payload_size));

    /// @brief Set pulse counter filter delays.
    /// @param delays array of delays
    /// @param delays_size array size 
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t   io_pulse_filter_set(uint16_t *delays, size_t delays_size);

    /// @brief Check if any  output is active.
    /// @return true if any relay or latch is active.
    /// @note this function is thread safe.
    bool        io_check_output_active();

}
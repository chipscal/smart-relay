#pragma once

#include "esp_check.h"
#include "iot_services/pinout.h"

#define I2C_DEFAULT_TIMEOUT_MS       1000

namespace clab::iot_services
{

    /// @brief Initialize I2C bus service 
    /// @note this function is NOT thread safe and must me called only one time during initialization!
    esp_err_t i2c_bus_init();

    /// @brief Deinitilize I2C bus service and clean up resources. 
    /// @note this function is NOT thread safe and must me called only one time during deinitialization!
    esp_err_t i2c_bus_deinit();

    /// @brief Reads a specified register.
    /// @param dev_addr device address,
    /// @param reg_addr registry address,
    /// @param buffer buffer where to store bytes read, 
    /// @param n_reads number of bytes to read,
    /// @param wait_millis maximum millis to wait for response
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t i2c_bus_register_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buffer, size_t n_reads, uint32_t wait_millis);

    /// @brief Reads the default register.
    /// @param dev_addr device address,
    /// @param buffer buffer where to store bytes read, 
    /// @param n_reads number of bytes to read,
    /// @param wait_millis maximum millis to wait for response
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t i2c_bus_register_read(uint8_t dev_addr, uint8_t *buffer, size_t n_reads, uint32_t wait_millis);

    /// @brief Writes a single byte register
    /// @param dev_addr device address,
    /// @param reg_addr registry address,
    /// @param data byte to write
    /// @param wait_millis maximum millis to wait for response
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t i2c_bus_register_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data, uint32_t wait_millis);

    /// @brief Writes a two bytes register
    /// @param dev_addr device address,
    /// @param reg_addr registry address,
    /// @param msb MSB part og the data to write
    /// @param lsb LSB part og the data to write
    /// @param wait_millis maximum millis to wait for response
    /// @return ESP_OK on success.
    /// @note this function is thread safe.
    esp_err_t i2c_bus_register_write_2bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t msb, uint8_t lsb, uint32_t wait_millis);
}


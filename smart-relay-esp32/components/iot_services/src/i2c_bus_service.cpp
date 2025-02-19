#include "iot_services/i2c_bus_service.h"
#include "iot_services/iot_services.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#include "esp_log.h"

static const char *TAG = "iot_services::i2c_bus_service";

namespace clab::iot_services {

    SemaphoreHandle_t   i2c_bus_mutex = NULL;

    
    esp_err_t i2c_bus_init(void) {

        if (i2c_bus_mutex == NULL)
            i2c_bus_mutex = xSemaphoreCreateMutex(); //In tests may have been already allocated
        ESP_LOGI(TAG, "I2C bus service mutex created...");

        i2c_config_t i2c_conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = I2C0_SDA,
            .scl_io_num = I2C0_SCL,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master = { 
                .clk_speed = I2C0_MASTER_FREQ_HZ,
            },
            .clk_flags = 0
        };

        i2c_port_t port = static_cast<i2c_port_t>(I2C0_MASTER_NUM);

        i2c_param_config(port, &i2c_conf);

        esp_err_t result = i2c_driver_install(port, i2c_conf.mode, 0, 0, 0);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during solar charge status pin configuration!");
        }
        return result;
    }

    esp_err_t i2c_bus_deinit(void) {
        i2c_port_t port = static_cast<i2c_port_t>(I2C0_MASTER_NUM);

        esp_err_t result = i2c_driver_delete(port);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during i2c driver dismiss!");
        }
        return result;
    }

    esp_err_t i2c_bus_register_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buffer, size_t n_reads, uint32_t wait_millis) {
        uint8_t write_buf[1] = {reg_addr};
        i2c_port_t port = static_cast<i2c_port_t>(I2C0_MASTER_NUM);
        
        xSemaphoreTake(i2c_bus_mutex,  portMAX_DELAY);
        esp_err_t result = i2c_master_write_read_device(port, dev_addr, write_buf, sizeof(write_buf), buffer, n_reads, 
                pdMS_TO_TICKS(wait_millis));
        xSemaphoreGive(i2c_bus_mutex);
        return result;
    }

    esp_err_t i2c_bus_register_read(uint8_t dev_addr, uint8_t *buffer, size_t n_reads, uint32_t wait_millis) {
        i2c_port_t port = static_cast<i2c_port_t>(I2C0_MASTER_NUM);
        
        xSemaphoreTake(i2c_bus_mutex,  portMAX_DELAY);
        esp_err_t result = i2c_master_read_from_device(port, dev_addr, buffer, n_reads, 
                pdMS_TO_TICKS(wait_millis));
        xSemaphoreGive(i2c_bus_mutex);
        return result;
    }

    esp_err_t i2c_bus_register_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data, uint32_t wait_millis) {
        uint8_t write_buf[2] = {reg_addr, data};
        i2c_port_t port = static_cast<i2c_port_t>(I2C0_MASTER_NUM);

        xSemaphoreTake(i2c_bus_mutex,  portMAX_DELAY);
        esp_err_t result = i2c_master_write_to_device(port, dev_addr, write_buf, sizeof(write_buf), 
                pdMS_TO_TICKS(wait_millis));
        xSemaphoreGive(i2c_bus_mutex);
        return result;
    }

    esp_err_t i2c_bus_register_write_2bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t msb, uint8_t lsb, uint32_t wait_millis) {
        uint8_t write_buf[3] = {reg_addr, msb, lsb};
        i2c_port_t port = static_cast<i2c_port_t>(I2C0_MASTER_NUM);

        xSemaphoreTake(i2c_bus_mutex,  portMAX_DELAY);
        esp_err_t result = i2c_master_write_to_device(port, dev_addr, write_buf, sizeof(write_buf), 
                pdMS_TO_TICKS(wait_millis));
        xSemaphoreGive(i2c_bus_mutex);
        return result;
    }
}
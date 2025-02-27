idf_component_register(SRC_DIRS "src" "src/ble"
                    INCLUDE_DIRS "include"
                    REQUIRES "nvs_flash" "driver" "freertos" "esp_rom" "esp_timer" "iot_services" "mbedtls" "bt" "app_update")

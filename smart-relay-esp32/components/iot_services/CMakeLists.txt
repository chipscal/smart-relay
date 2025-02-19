idf_component_register(
        SRC_DIRS "src"
        INCLUDE_DIRS "include"
        REQUIRES "driver" "freertos" "esp_rom" "esp_timer" "esp_adc" "nvs_flash"  "mbedtls")


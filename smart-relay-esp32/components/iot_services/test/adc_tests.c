#include "unity.h"
#include "esp_log.h"

#include "iot_services/pinout.h"

#ifdef IOT_BOARD_USES_INTERNAL_ADC

#include "iot_services/adc_service.h"

#if defined(CONFIG_SMART_RELAY_WIFI_R1)
    #define TEST_ANALOG_GPIO    AIN1
    #define TEST_ANALOG_SCALE   ADC_V_SCALE_VOLTAGE
#else
    // put othher definitions here
#endif

static const char *TAG = "iot_services::adc_tests";

TEST_CASE("adc test", "[adc]") {
    adc_service_init();
    ESP_LOGI(TAG, "ADC service intialized...");


    adc_cali_handle_t calibration;
    adc_service_calibration_init(TEST_ANALOG_GPIO, ADC_ATTEN_DB_12, &calibration);
    ESP_LOGI(TAG, "ADC calibrated...");

    int raw = adc_service_measure_raw(TEST_ANALOG_GPIO, 100);
    ESP_LOGI(TAG, "Measured raw value: %d", raw);


    int volt = adc_service_measure_voltage(TEST_ANALOG_GPIO, calibration, 100);
    ESP_LOGI(TAG, "Measured voltage value: %d mV", volt);

    adc_service_calibration_deinit(calibration);
    ESP_LOGI(TAG, "ADC calibration destroyed...");

    adc_service_deinit();
    ESP_LOGI(TAG, "ADC service deintialized...");

}

#endif
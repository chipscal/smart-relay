#include "iot_services/adc_service.h"

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


static const char *TAG = "iot_services::adc_service";

#ifdef IOT_BOARD_USES_INTERNAL_ADC

SemaphoreHandle_t               adc_service_mutex = NULL;

static adc_oneshot_unit_handle_t adc1_handle;
#ifdef ADC_USE_ADC2
static adc_oneshot_unit_handle_t adc2_handle;
#endif


static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);

esp_err_t adc_service_init(void) {
    if (adc_service_mutex == NULL) {
        adc_service_mutex = xSemaphoreCreateMutex(); //In tests may have been already allocated
        ESP_LOGI(TAG, "ADC service mutex created...");
    }

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = adc_oneshot_clk_src_t::ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    #ifdef ADC_USE_ADC2
        adc_oneshot_unit_init_cfg_t init_config2 = {
            .unit_id = ADC_UNIT_2,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };
    #endif

    esp_err_t result;

    result = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Error during ADC1 configuration!");
        return result;
    }
    ESP_LOGI(TAG, "ADC1 up...");
    
    #ifdef ADC_USE_ADC2
        result = adc_oneshot_new_unit(&init_config2, &adc2_handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during ADC2 configuration!");
            return result;
        }
        ESP_LOGI(TAG, "ADC2 up...");
    #endif

    return ESP_OK;
}

esp_err_t adc_service_deinit(void) {
    esp_err_t result;

    result = adc_oneshot_del_unit(adc1_handle);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Error during ADC1 deinit!");
        return result;
    }
    ESP_LOGI(TAG, "ADC1 down...");

    #ifdef ADC_USE_ADC2
        result = adc_oneshot_del_unit(adc2_handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during ADC2 deinit!");
            return result;
        }
        ESP_LOGI(TAG, "ADC2 down...");
    #endif

    return ESP_OK;
}

esp_err_t adc_service_calibration_init(gpio_num_t gpio, adc_atten_t atten, adc_cali_handle_t *out_handle) {
    adc_unit_t unit;
    adc_channel_t channel;
    esp_err_t result;

    result = adc_oneshot_io_to_channel(gpio, &unit, &channel);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Gpio<%d> Error during channel discovery!", gpio);
        return result;
    }

    ESP_LOGI(TAG, "Found - Unit: %d, Channel: %d", unit, channel);

    
    if (unit == adc_unit_t::ADC_UNIT_1) {
        adc_oneshot_chan_cfg_t config = {
            .atten = atten,
            .bitwidth = static_cast<adc_bitwidth_t>(ADC_BIT_RESOLUTION),
        };
        result = adc_oneshot_config_channel(adc1_handle, channel, &config);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Gpio<%d> Unable to configure!", gpio);
            return result;
        }
    }
    else {
        #ifdef ADC_USE_ADC2
            result = adc_oneshot_config_channel(adc2_handle, channel, &config);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Gpio<%d> Unable to configure!", gpio);
                return result;
            }
        #else
            ESP_LOGE(TAG, "Gpio<%d> uses ADC2, but is not enabled!", gpio);
            return ESP_ERR_NOT_SUPPORTED;
        #endif
    }
    
    if (!adc_calibration_init(unit, channel, atten, out_handle)) {
        ESP_LOGE(TAG, "Gpio<%d> Calibration failed!", gpio);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t adc_service_calibration_deinit(adc_cali_handle_t handle) {
    esp_err_t result;
    #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
        result = adc_cali_delete_scheme_curve_fitting(handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during calibration deinit!");
            return result;
        }
    #elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
        result = adc_cali_delete_scheme_line_fitting(handle);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error during calibration deinit!");
            return result;
        }
    #endif

    return ESP_OK;
}

int adc_service_measure_raw(gpio_num_t gpio, int n_samples) {
    adc_unit_t unit;
    adc_channel_t channel;
    esp_err_t result;

    result = adc_oneshot_io_to_channel(gpio, &unit, &channel);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Gpio<%d> Unable to map!", gpio);
        return -1;
    }
    
    xSemaphoreTake(adc_service_mutex, portMAX_DELAY);

    int to_ret = 0;
    if (unit == adc_unit_t::ADC_UNIT_1) {
        
        for (int k = 0; k < n_samples; k++) {
            int measure = 0;
            result = adc_oneshot_read(adc1_handle, channel, &measure);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Gpio<%d> Invalid measurment!", gpio);
                xSemaphoreGive(adc_service_mutex);
                return -1;
            }
            to_ret += measure;  
        }
    }
    else {
        #ifdef ADC_USE_ADC2
            for (int k = 0; k < n_samples; k++) {
                int measure = 0;
                result = adc_oneshot_read(adc2_handle, channel, &measure);  
                if (result != ESP_OK) {
                    ESP_LOGE(TAG, "Gpio<%d> Invalid measurment!", gpio);
                    xSemaphoreGive(adc_service_mutex);
                    return -1;
                }
                to_ret += measure;  
            }
        #else
            ESP_LOGE(TAG, "Gpio<%d> uses ADC2, but is not enabled!", gpio);
        #endif
    }

    xSemaphoreGive(adc_service_mutex);


    return to_ret / n_samples;
}

int adc_service_measure_voltage(gpio_num_t gpio, adc_cali_handle_t handle, int n_samples) {
    int to_ret;
    esp_err_t result;
    auto measure = adc_service_measure_raw(gpio, n_samples);

    if (measure < 0) {
        ESP_LOGE(TAG, "Gpio<%d> Invalid raw measure!", gpio);
        return -1;
    }

    if (measure <= ADC_MIN_MEASURE_IGNORE)
        return 0;

    result = adc_cali_raw_to_voltage(handle, measure, &to_ret);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Gpio<%d> Unable to calibrate measure!", gpio);
        return -1;
    }

    return to_ret;
}

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle) {
adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        if (!calibrated) {
            ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
            adc_cali_curve_fitting_config_t cali_config = {
                .unit_id = unit,
                .chan = channel,
                .atten = atten,
                .bitwidth = ADC_BITWIDTH_DEFAULT,
            };
            ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
            if (ret == ESP_OK) {
                calibrated = true;
            }
        }
    #endif

    #if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        if (!calibrated) {
            ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
            adc_cali_line_fitting_config_t cali_config = {
                .unit_id = unit,
                .atten = atten,
                .bitwidth = static_cast<adc_bitwidth_t>(ADC_BIT_RESOLUTION),
            };
            ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
            if (ret == ESP_OK) {
                calibrated = true;
            }
        }
    #endif

*out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

#endif
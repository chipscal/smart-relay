#include "iot_services/wifi_service.h"

#include "esp_err.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


static const char *TAG = "iot_services::wifi_service";

#ifndef CONFIG_IOT_WIFI_WIFI_PW_ID
    #define CONFIG_IOT_WIFI_WIFI_PW_ID ""
#endif

#if CONFIG_IOT_WIFI_WPA3_SAE_PWE_HUNT_AND_PECK
#define IOT_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define IOT_WIFI_H2E_IDENTIFIER ""
#elif CONFIG_IOT_WIFI_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define IOT_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define IOT_WIFI_H2E_IDENTIFIER CONFIG_IOT_WIFI_WIFI_PW_ID
#elif CONFIG_IOT_WIFI_WPA3_SAE_PWE_BOTH
#define IOT_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define IOT_WIFI_H2E_IDENTIFIER CONFIG_IOT_WIFI_WIFI_PW_ID
#endif

#if CONFIG_IOT_WIFI_AUTH_OPEN
#define IOT_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_IOT_WIFI_AUTH_WEP
#define IOT_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_IOT_WIFI_AUTH_WPA_PSK
#define IOT_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_IOT_WIFI_AUTH_WPA2_PSK
#define IOT_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_IOT_WIFI_AUTH_WPA_WPA2_PSK
#define IOT_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_IOT_WIFI_AUTH_WPA3_PSK
#define IOT_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_IOT_WIFI_AUTH_WPA2_WPA3_PSK
#define IOT_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_IOT_WIFI_AUTH_WAPI_PSK
#define IOT_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif



/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define IOT_WIFI_CONNECTED_BIT BIT0
#define IOT_WIFI_FAIL_BIT      BIT1

namespace clab::iot_services {

    static bool                 netif_init = false;
    /* FreeRTOS event group to signal when we are connected*/
    static EventGroupHandle_t   wifi_event_group;

    static int                  wifi_retry_num = 0;

    static esp_netif_t          *wifi_netif_handler = NULL;

    static esp_event_handler_instance_t     wifi_instance_any_id;
    static esp_event_handler_instance_t     wifi_instance_got_ip;

    static void event_handler(void* arg, esp_event_base_t event_base,
            int32_t event_id, void* event_data) {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } 
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            if (wifi_retry_num < CONFIG_IOT_WIFI_MAXIMUM_RETRY) {
                esp_wifi_connect();
                wifi_retry_num++;
                ESP_LOGI(TAG, "retry to connect to the AP");
            } 
            else {
                xEventGroupSetBits(wifi_event_group, IOT_WIFI_FAIL_BIT);
            }
            ESP_LOGI(TAG,"connect to the AP fail");
        } 
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            wifi_retry_num = 0;
            xEventGroupSetBits(wifi_event_group, IOT_WIFI_CONNECTED_BIT);
        }
    }

    esp_err_t wifi_init() {
        if (wifi_event_group == NULL)
            wifi_event_group = xEventGroupCreate();

        esp_err_t result;

        if (!netif_init) {
            result = esp_netif_init();
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Unable to init <netif> stack!");
                return result;
            }
            netif_init = true;
        }

        result = esp_event_loop_create_default();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to init <defult event loop> stack!");
            return result;
        }

        //------------------------ esp_netif_create_default_wifi_sta();
        esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_WIFI_STA();
        wifi_netif_handler = esp_netif_new(&netif_cfg);
        if (wifi_netif_handler == NULL) {
            ESP_LOGE(TAG, "Unable to allocate <netif>!");
            return ESP_ERR_NO_MEM;
        }

        result = esp_netif_attach_wifi_station(wifi_netif_handler);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to attach wifi station!");
            return result;
        }

        result = esp_wifi_set_default_wifi_sta_handlers();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to set default wifi handlers!");
            return result;
        }
        //------------------------------------------------------------;

        wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
        result = esp_wifi_init(&wifi_init_cfg);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to set default wifi handlers!");
            return result;
        }

        
        
        result = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                &event_handler, NULL, &wifi_instance_any_id);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to set wifi handler: ESP_EVENT_ANY_ID");
            return result;
        }

        result = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                &event_handler, NULL, &wifi_instance_got_ip);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to set wifi handler: IP_EVENT_STA_GOT_IP");
            return result;
        }

        wifi_config_t wifi_config = {
            .sta = {
                .ssid = CONFIG_IOT_WIFI_SSID,
                .password = CONFIG_IOT_WIFI_PASSWORD,
                /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
                * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
                * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
                * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
                */
                .threshold = { .authmode = IOT_WIFI_SCAN_AUTH_MODE_THRESHOLD },
                .sae_pwe_h2e = IOT_WIFI_SAE_MODE,
                .sae_h2e_identifier = IOT_WIFI_H2E_IDENTIFIER,
            },
        };

        result = esp_wifi_set_mode(WIFI_MODE_STA);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to set wifi mode: WIFI_MODE_STA");
            return result;
        }

        result = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to configure wifi!");
            return result;
        }

        result = esp_wifi_start();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to start wifi!");
            return result;
        }

        ESP_LOGI(TAG, "WiFi init finished.");

        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
        * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
        EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                IOT_WIFI_CONNECTED_BIT | IOT_WIFI_FAIL_BIT,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY);

        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
        * happened. */
        if (bits & IOT_WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                    CONFIG_IOT_WIFI_SSID, CONFIG_IOT_WIFI_PASSWORD);
            return ESP_OK;
        } else if (bits & IOT_WIFI_FAIL_BIT) {
            ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                    CONFIG_IOT_WIFI_SSID, CONFIG_IOT_WIFI_PASSWORD);
        } else {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }

        return ESP_FAIL;
    }

    esp_err_t wifi_deinit() {
        esp_err_t result;

        result = esp_wifi_stop();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to stop wifi!");
            return result;
        }

        result = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_instance_any_id);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to unset wifi handler: ESP_EVENT_ANY_ID");
            return result;
        }

        result = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_instance_got_ip);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to unset wifi handler: IP_EVENT_STA_GOT_IP");
            return result;
        }

        result = esp_wifi_deinit();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to deinit wifi");
            return result;
        }

        result = esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif_handler);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to clear default handlers and driver");
            return result;
        }

        result = esp_event_loop_delete_default();
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to delete <defult event loop>!");
            return result;
        }

        esp_netif_destroy(wifi_netif_handler);
        
        //Note: deinitialization is not supported!
        // result = esp_netif_deinit();
        // if (result != ESP_OK) {
        //     ESP_LOGE(TAG, "Unable to deinit <netif> stack!");
        //     return result;
        // }

        return ESP_OK;
    }

    esp_err_t wifi_get_ip(uint32_t *address) {
        esp_netif_ip_info_t info;
        esp_err_t result = esp_netif_get_ip_info(wifi_netif_handler, &info);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Unable to get IP address!");
            return result;
        }
        
        *address = info.ip.addr;
        return ESP_OK;
    }
}
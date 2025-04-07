#include <stdio.h>
#include <inttypes.h>
#include "unity.h"
#include "string.h"
#include "esp_log.h"
#include "esp_system.h"
#include "iot_services/iot_services.h"
#include "iot_services/comm_api.h"
#include "mbedtls/base64.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "iot_services::comm_tests";

#ifdef __cplusplus
extern "C" {
#endif


void comm_api_test_impl() {
   
    auto rule0 = clab::iot_services::unary_rule_t<8>();
    TEST_ASSERT(rule0.parse_from("d[0]=1,12345678") == ESP_OK);
    TEST_ASSERT(rule0.port.type == clab::iot_services::port_type_t::DIGITAL);
    TEST_ASSERT(rule0.port.index == 0);
    TEST_ASSERT(rule0.value == 1.0f);
    TEST_ASSERT(strcmp(rule0.target, "12345678") == 0);

    auto rule1 = clab::iot_services::unary_rule_t<8>();
    TEST_ASSERT(rule1.parse_from("v[2]=1.33,12345678") == ESP_OK);
    TEST_ASSERT(rule1.port.type == clab::iot_services::port_type_t::VOLTAGE);
    TEST_ASSERT(rule1.port.index == 2);
    TEST_ASSERT(rule1.value == 1.33f);
    TEST_ASSERT(strcmp(rule1.target, "12345678") == 0);

    auto combined0 = clab::iot_services::combined_rule_t<4, 8>();
    TEST_ASSERT(combined0.parse_from("{d[12]=1,12345678;v[2]=1.33,12345678;;}r0") == ESP_OK);
    TEST_ASSERT(combined0.rules[0].port.type == clab::iot_services::port_type_t::DIGITAL);
    TEST_ASSERT(combined0.rules[0].port.index == 12);
    TEST_ASSERT(combined0.rules[0].value == 1.0f);
    TEST_ASSERT(strcmp(combined0.rules[0].target, "12345678") == 0);

    TEST_ASSERT(combined0.rules[1].port.type == clab::iot_services::port_type_t::VOLTAGE);
    TEST_ASSERT(combined0.rules[1].port.index == 2);
    TEST_ASSERT(combined0.rules[1].value == 1.33f);
    TEST_ASSERT(strcmp(combined0.rules[1].target, "12345678") == 0);

    TEST_ASSERT(combined0.rules[2].port.type == clab::iot_services::port_type_t::EMPTY);
    TEST_ASSERT(combined0.rules[2].port.index == 0);
    TEST_ASSERT(combined0.rules[2].value == 0);

    TEST_ASSERT(combined0.rules[3].port.type == clab::iot_services::port_type_t::EMPTY);
    TEST_ASSERT(combined0.rules[3].port.index == 0);
    TEST_ASSERT(combined0.rules[3].value == 0);

    TEST_ASSERT(combined0.action.type == clab::iot_services::port_type_t::RELAY);
    TEST_ASSERT(combined0.action.index == 0);

}

#ifdef __cplusplus
}
#endif
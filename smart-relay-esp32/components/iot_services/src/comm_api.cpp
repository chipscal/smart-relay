#include "iot_services/comm_api.h"

#include "esp_log.h"
#include "comm_api.h"

static const char *TAG = "iot_services::comm_api";


namespace clab::iot_services {
    
    dev_status_t::dev_status_t(uint8_t *buffer, size_t buffer_size) {
        _data_buffer = buffer;
        _data_size = buffer_size;
    }
}
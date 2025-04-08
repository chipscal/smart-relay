#include "iot_services/comm_api.h"

#include "esp_log.h"

static const char *TAG = "iot_services::comm_api";


namespace clab::iot_services {
    
    dev_status_t::dev_status_t(uint8_t *buffer, size_t buffer_size) {
        _data_buffer = buffer;
        _data_size = buffer_size;
    }

    esp_err_t dev_program_t::parse_from(const char *program) {

        memset(this, 0, sizeof(dev_program_t));
        
        //(e.g. "{1744047771,1751305371,14400,72000}[r0,l0]")
        size_t cnt = 1;
        size_t program_size = strlen(program);

        if (program[0] != '{')
            return ESP_ERR_INVALID_ARG;
        
        std::string_view fixed_view(program + cnt);
        auto comma_delimiter = fixed_view.find(',');
        if (comma_delimiter == fixed_view.npos) {
            return ESP_ERR_INVALID_ARG;
        }
        std::string value_string(program + cnt, comma_delimiter);
        this->start_ts = std::stoul(value_string);
        cnt += comma_delimiter + 1;

        fixed_view = std::string_view(program + cnt);
        comma_delimiter = fixed_view.find(',');
        if (comma_delimiter == fixed_view.npos) {
            return ESP_ERR_INVALID_ARG;
        }
        value_string = std::string(program + cnt, comma_delimiter);
        this->end_ts = std::stoul(value_string);
        cnt += comma_delimiter + 1;

        fixed_view = std::string_view(program + cnt);
        comma_delimiter = fixed_view.find(',');
        if (comma_delimiter == fixed_view.npos) {
            return ESP_ERR_INVALID_ARG;
        }
        value_string = std::string(program + cnt, comma_delimiter);
        this->duration = std::stoul(value_string);
        cnt += comma_delimiter + 1;

        fixed_view = std::string_view(program + cnt);
        comma_delimiter = fixed_view.find('}');
        if (comma_delimiter == fixed_view.npos) {
            return ESP_ERR_INVALID_ARG;
        }
        value_string = std::string(program + cnt, comma_delimiter);
        this->idle = std::stoul(value_string);
        cnt += comma_delimiter + 1;
        
        //[
        if (program[cnt] != '[')
            return ESP_ERR_INVALID_ARG;
        cnt += 1;
        

        while (cnt < program_size) {
            fixed_view = std::string_view(program + cnt);
            comma_delimiter = fixed_view.find(',');
            
            if (comma_delimiter == fixed_view.npos) {
                comma_delimiter = fixed_view.size() - 1;
            }
            
            if (comma_delimiter >= 2) {

                auto port_type = static_cast<port_type_t>(program[cnt]);

                value_string = std::string(program + cnt + 1, comma_delimiter - 1);
                auto port_index = std::stoul(value_string);
                

                if (port_type == port_type_t::LATCH) {
                    this->latch_mask |= (1 << port_index);
                }

                if (port_type == port_type_t::RELAY) {
                    this->relay_mask |= (1 << port_index);
                }
            }
            
            cnt += comma_delimiter + 1;
        }

        if (program[cnt - 1] != ']')
            return ESP_ERR_INVALID_ARG;

        return ESP_OK;
    }
}
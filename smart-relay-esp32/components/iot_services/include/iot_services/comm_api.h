#pragma once

#include "iot_services/iot_services.h"

#include <string.h>
#include <string>
#include <string_view>
#include <stdint.h>

namespace clab::iot_services
{
    /// @brief I/O port types.
    enum port_type_t : uint8_t {
        EMPTY       = 0x0,
        RELAY       = 'r',
        LATCH       = 'l',
        CURRENT     = 'c',
        VOLTAGE     = 'v',
        PULSE       = 'p',
        DIGITAL     = 'd'
    };

    /// @brief Port definition.
    struct port_def_t {
        /// @brief Port type.
        port_type_t type;
        /// @brief Port index.
        uint8_t     index;
    };

    /// @brief Port parameters.
    struct port_conf_t {
        /// @brief Init delay - seconds.
        uint8_t     init_d;
        /// @brief Stop delay - seconds.
        uint8_t     stop_d;
    };

    /// @brief Output ports parameters.
    template<size_t Nl, size_t Nr>
    struct ports_conf_t {
        /// @brief Latch configuration.
        port_conf_t latch_conf[Nl];
        /// @brief Relay configuration.
        port_conf_t relay_conf[Nr];

        constexpr size_t size() {
            return (Nl + Nr) * sizeof(port_conf_t);
        }

        ports_conf_t() {
            memset(this, 0, sizeof(ports_conf_t<Nl, Nr>));
        }

        esp_err_t from_buffer(uint8_t *buffer, size_t buffer_size) {

            if (buffer_size < sizeof(latch_conf) + sizeof(relay_conf)) {
                return ESP_ERR_INVALID_SIZE;
            }
            
            int offset = 0;

            memcpy(latch_conf, buffer + offset, Nl * sizeof(port_conf_t));
            offset += Nl * sizeof(port_conf_t);
            
            memcpy(relay_conf, buffer + offset, Nr * sizeof(port_conf_t));
            offset += Nr * sizeof(port_conf_t);

            return ESP_OK;
        }

    };

    template<size_t Nr>
    struct ports_conf_t<0U, Nr> {
        /// @brief Latch configuration.
        dummy_array<port_conf_t> latch_conf;
        /// @brief Relay configuration.
        port_conf_t relay_conf[Nr];

        constexpr size_t size() {
            return Nr * sizeof(port_conf_t);
        }

        esp_err_t from_buffer(uint8_t *buffer, size_t buffer_size) {

            if (buffer_size < sizeof(relay_conf)) {
                return ESP_ERR_INVALID_SIZE;
            }
            
            int offset = 0;
            
            memcpy(relay_conf, buffer + offset, Nr * sizeof(port_conf_t));
            offset += Nr * sizeof(port_conf_t);

            return ESP_OK;
        }
    };

    template<size_t Nl>
    struct ports_conf_t<Nl, 0U> {
        /// @brief Latch configuration.
        port_conf_t latch_conf[Nl];
        /// @brief Relay configuration.
        dummy_array<port_conf_t> relay_conf;

        constexpr size_t size() {
            return Nl * sizeof(port_conf_t);
        }

        esp_err_t from_buffer(uint8_t *buffer, size_t buffer_size) {

            if (buffer_size < sizeof(latch_conf)) {
                return ESP_ERR_INVALID_SIZE;
            }
            
            int offset = 0;

            memcpy(latch_conf, buffer + offset, Nl * sizeof(port_conf_t));
            offset += Nl * sizeof(port_conf_t);

            return ESP_OK;
        }
    };

    template<>
    struct ports_conf_t<0U, 0U> {
        /// @brief Latch configuration.
        dummy_array<port_conf_t> latch_conf;
        /// @brief Relay configuration.
        dummy_array<port_conf_t> relay_conf;
    };


    /// @brief I/O port types.
    enum unary_op_t : uint8_t {
        NOPE       = 0x0,
        EQ          = '=',
        NEQ         = '!',
        GT          = '>',
        GTE         = 'g',
        LT          = '<',
        LTE         = 'l'
    };

    /// @brief Unary rule definition.
    template<size_t T>
    struct unary_rule_t {
        port_def_t      port;
        unary_op_t      op;
        float           value;
        char            target[T + 1];

        /// @brief Initialize empty.
        unary_rule_t() {
            memset(this, 0, sizeof(unary_rule_t));
        };

        /// @brief Initializates from buffer.
        /// @param buffer "<port_type><port_index><operator><value>,<target0:T-1>" (e.g. "d[0]=1,XXXXXXXXXXXXXXXX")
        esp_err_t parse_from(const char *rule) {

            size_t cnt = 0;
            size_t rule_size = strlen(rule);
            
            port.type = static_cast<port_type_t>(rule[0]);
            cnt += 2;
            std::string_view rule_view(rule + cnt);
            auto index_delimiter = rule_view.find(']');
            if (index_delimiter == rule_view.npos) {
                return ESP_ERR_INVALID_ARG;
            }

            std::string index_string(rule + cnt, index_delimiter);
            port.index = std::stoul(index_string);
            cnt += index_delimiter + 1;

            op = static_cast<unary_op_t>(rule[cnt]);
            cnt++;

            rule_view = std::string_view(rule + cnt);
            auto value_delimiter = rule_view.find(',');
            if (index_delimiter == rule_view.npos) {
                return ESP_ERR_INVALID_ARG;
            }

            std::string value_string(rule + cnt, value_delimiter);
            value = std::stof(value_string);
            cnt += value_delimiter + 1;

            if (rule_size - cnt < T) {
                return ESP_ERR_INVALID_SIZE;
            }

            memcpy(target, rule + cnt, T);
            target[T] = '\0';

            return ESP_OK;
        }
    };

    template<size_t N, size_t T>
    struct combined_rule_t {
        unary_rule_t<T>     rules[N];
        port_def_t          action;

        /// @brief Initialize empty
        combined_rule_t() {
            memset(this, 0, sizeof(combined_rule_t));
        };

        /// @brief Initializates from string.
        /// @param crule "{<unary_rule(0)>;<unary_rule(1)>;...<unary_rule(N-1)>}<port_type><port_index>" 
        /// (e.g. "{d[0]=1,XXXXXXXXXXXXXXX1;v[0]=1.22,XXXXXXXXXXXXXXX2}r0")
        esp_err_t parse_from(const char *crule) {
            size_t cnt = 0;
            size_t crule_size = strlen(crule);

            cnt += 1;
            for (size_t k = 0; k < N; k++) {
                std::string_view crule_view(crule + cnt);
                auto delimiter = crule_view.find(k < N - 1 ? ';' : '}');
                if (delimiter == crule_view.npos) {
                    return ESP_ERR_INVALID_ARG;
                }

                rules[k] = unary_rule_t<T>();

                if (delimiter > 1) {
                    esp_err_t result = rules[k].parse_from(crule + cnt);
                    if (result != ESP_OK)   
                        return result;
                }
                
                cnt += delimiter + 1;
            }

            if (crule_size - cnt < 2)
                return ESP_ERR_INVALID_SIZE;

            action.type = static_cast<port_type_t>(crule[cnt]);
            cnt += 1;
            action.index = atoi(crule + cnt);

            return ESP_OK;
        }
    };

    /// @brief Device program definition.
    struct dev_program_t {
        /// @brief Active latch definition.
        uint32_t    latch_mask;
        /// @brief Active relay definition.
        uint32_t    relay_mask;

        /// @brief Start timestamp.
        uint32_t    start_ts;
        /// @brief End timestamp.
        /// @note if actual_ts > end_ts program will not activate anymore.
        uint32_t    end_ts;

        /// @brief Activation time in seconds.
        uint32_t    duration;
        /// @brief Idle time in seconds.
        /// @note if positive, the program will activate again after <idle> seconds.
        uint32_t    idle;


        /// @brief Latch output status.
        /// @param idx of the output
        /// @return true if active
        inline bool latch_status(int idx) {
            return ((latch_mask & (1 << idx)) > 0);
        }

        /// @brief Relay output status.
        /// @param idx of the output
        /// @return true if active
        inline bool relay_status(int idx){
            return ((relay_mask & (1 << idx)) > 0);
        }

        /// @brief check if program do not activate any output.
        /// @return true if no output are activated.
        inline bool is_empty() {
            return latch_mask == 0 && relay_mask == 0;
        }

        /// @brief Compute the porogram period.
        /// @return the period or UINT32_MAX if idle is 0;
        inline uint32_t period() {
            return idle > 0 ? duration + idle : UINT32_MAX;
        }

        /// @brief Initializates from string.
        /// @param program "{<start_ts>,<end_ts>,<duration>,<idle>}[<port0_type><port0_index>, ... ,<portK_type><portK_index>]" 
        /// (e.g. "{1744047771,1751305371,14400,72000}[r0,l0]")
        /// @return ESP_OK on success.
        esp_err_t parse_from(const char *program);
    };


    /// @brief Device status rapresentation.
    struct dev_status_t {

        uint8_t     *_data_buffer;

        size_t      _data_size;

        /// @brief Set internal buffer handlers.
        /// @param buffer from where to get status
        /// @param buffer_size of the status buffer
        /// @note No copy is made.
        dev_status_t(uint8_t *buffer, size_t buffer_size);

        // HREV|SREV|n_curr[0:3],n_volt[4:7]|n_pulse[0:3],n_temperature[4:7]|
        // Latch0|Latch1|Latch2|Latch3|Relay0|Relay1|Relay2|Relay3 (bit mask)
        // Digital0|Digital1|Digital2|Digital3 (bit mask)
        // Current0_LSB|Current0_MSB|...|CurrentN_LSB|CurrentN_MSB
        // Voltage0_LSB|Voltage0_MSB|...|VoltageN_LSB|VoltageN_MSB
        // Pulse0_LSB|Pulse0_MSB|...|PulseN_LSB|PulseN_MSB
        // Temperature0_LSB|Temperature0_MSB|...|TemperatureN_LSB|TemperatureN_MSB

        /// @brief Hardware revision.
        inline uint8_t     hrev() {
            return _data_buffer[0];
        }
        /// @brief Software revision.
        inline uint8_t     srev() {
            return (_data_buffer[1] & 0xF0) >> 4;
        }

        /// @brief Software minor.
        inline uint8_t     sminor() {
            return _data_buffer[1] & 0x0F;
        }

        /// @brief Number of installed current input.
        inline uint8_t     n_curr() {
            return (_data_buffer[2] & 0xF0) >> 4;
        }

        /// @brief Number of installed voltage input.
        inline uint8_t     n_volt() {
            return _data_buffer[2] & 0x0F;
        }

        /// @brief Number of installed pulse input.
        inline uint8_t     n_pulse() {
            return (_data_buffer[3] & 0xF0) >> 4;
        }
        /// @brief Number of installed temperature sensor.
        inline uint8_t     n_temperature() {
            return _data_buffer[3] & 0x0F;
        }
        
        /// @brief Latch output status.
        /// @param idx of the output
        /// @return true if active
        inline bool        latch_status(int idx) {
            uint32_t mask;
            memcpy(&mask, _data_buffer + 4, sizeof(uint32_t));
            if (!is_little_endian())
                mask = swap_uint32(mask);

            return ((mask & (1 << idx)) > 0);
        }

        /// @brief Relay output status.
        /// @param idx of the output
        /// @return true if active
        inline bool        relay_status(int idx){
            uint32_t mask;
            memcpy(&mask, _data_buffer + 8, sizeof(uint32_t));
            if (!is_little_endian())
                mask = swap_uint32(mask);

            return ((mask & (1 << idx)) > 0);
        }

        /// @brief Digital input status.
        /// @param idx of the input
        /// @return true if logically high
        inline bool        digital_status(int idx){
            uint32_t mask;
            memcpy(&mask, _data_buffer + 12, sizeof(uint32_t));
            if (!is_little_endian())
                mask = swap_uint32(mask);

            return ((mask & (1 << idx)) > 0);
        }

        /// @brief Current input value.
        /// @param idx of the input
        /// @return mA
        inline float       current_value(int idx) {
            uint8_t *base_address = _data_buffer + 16 + sizeof(uint16_t) * idx;

            uint16_t value;
            memcpy(&value, base_address, sizeof(uint16_t));
            if (!is_little_endian())
                value = swap_uint16(value);

            return static_cast<float>(value) / 1000.0f;
        }

        /// @brief Voltage input value.
        /// @param idx of the input
        /// @return mV
        inline float       voltage_value(int idx) {
            uint8_t *base_address = _data_buffer + 16 + sizeof(uint16_t) * (n_curr() + idx);

            uint16_t value;
            memcpy(&value, base_address, sizeof(uint16_t));
            if (!is_little_endian())
                value = swap_uint16(value);

            return static_cast<float>(value);
        }

        /// @brief Pulse input value.
        /// @param idx of the input
        /// @return pulse count
        inline uint16_t    pulse_value(int idx) {
            uint8_t *base_address = _data_buffer + 16 + sizeof(uint16_t) * (n_curr() + n_volt() + idx);

            uint16_t value;
            memcpy(&value, base_address, sizeof(uint16_t));
            if (!is_little_endian())
                value = swap_uint16(value);

            return value;
        }

        /// @brief Temperature input value.
        /// @param idx of the input
        /// @return K
        inline float       temperature_value(int idx) {
            uint8_t *base_address = _data_buffer + 16 + sizeof(uint16_t) * (n_curr() + n_volt() + n_pulse() + idx);

            uint16_t value;
            memcpy(&value, base_address, sizeof(uint16_t));
            if (!is_little_endian())
                value = swap_uint16(value);

            return static_cast<float>(value) / 100.0f;
        }

    };

    /// @brief Desired status rapresentation.
    struct des_status_t {
        /// @brief Latch port status - (1 << idx) => means Latch[idx] should be enabled.
        uint32_t    latch_mask;
        /// @brief Relay port status - (1 << idx) => means Relay[idx] should be enabled.
        uint32_t    relay_mask;
        /// @brief Led channel status - (1 << idx) => means Led[idx] control should be enabled.
        uint8_t     led_mask;

        inline bool latch_status(int idx) {
            return ((latch_mask & (1 << idx)) > 0);
        }

        inline void latch_status(int idx, bool status) {
            if (status)
                latch_mask |= (1 << idx);
            else
                latch_mask &= ~(1 << idx);
        }

        inline bool relay_status(int idx) {
            return ((relay_mask & (1 << idx)) > 0);
        }

        inline void relay_status(int idx, bool status) {
            if (status)
                relay_mask |= (1 << idx);
            else
                relay_mask &= ~(1 << idx);
        }

        inline bool led_status(int idx) {
            return ((led_mask & (1 << idx)) > 0);
        }

        inline void led_status(int idx, bool status) {
            if (status)
                led_mask |= (1 << idx);
            else
                led_mask &= ~(1 << idx);
        }
    };
}
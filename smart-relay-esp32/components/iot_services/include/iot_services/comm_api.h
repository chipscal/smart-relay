#pragma once

#include "iot_services/iot_services.h"

#include <cstring>
#include <string>
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
        unary_rule_t(const char *rule) {
            
            port.type = static_cast<port_type_t>(rule[0]);
            rule += 2;
            auto index_delimiter = strchr(rule, ']');
            std::string index_view(rule, static_cast<size_t>(index_delimiter - rule));
            port.index = std::stoul(index_view);
            rule = index_delimiter + 1;

            op = static_cast<unary_op_t>(*rule);
            rule++;

            auto value_delimiter = strchr(rule, ',');
            std::string value_string(rule, static_cast<size_t>(value_delimiter - rule));
            value = std::stof(value_string);
            rule = value_delimiter + 1;

            memcpy(target, rule, T);
            target[T] = '\0';
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

        /// @brief Initializates from buffer.
        /// @param buffer "{<unary_rule(0)>;<unary_rule(1)>;...<unary_rule(N-1)>}<port_type><port_index>" 
        /// (e.g. "{d[0]=1,XXXXXXXXXXXXXXX1;v[0]=1.22,XXXXXXXXXXXXXXX2}r0")
        combined_rule_t(const char *crule) {
            crule += 1;
            for (size_t k = 0; k < N; k++) {
                auto delimiter = strchr(crule, k < N - 1 ? ';' : '}');
                if (delimiter > crule + 1)
                    rules[k] = unary_rule_t<T>(crule);
                else
                    rules[k] = unary_rule_t<T>();
                
                crule = delimiter + 1;
            }

            action.type = static_cast<port_type_t>(crule[0]);
            crule += 1;
            action.index = atoi(crule);
        }
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
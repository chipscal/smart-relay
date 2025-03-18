#pragma once

#include "iot_services/io_service.h"

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
        EMPTY       = 0x0,
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
        const char      target[T];

        unary_rule_t() {};

        /// @brief Initializates from buffer.
        /// @param buffer "<port_type><port_index><operator>,<target0:T-1>" (e.g. "d0=1,XXXXXXXXXXXXXXXX")
        unary_rule_t(const char *buffer);
    };

    template<size_t N, size_t T>
    struct combined_rule_t {
        unary_rule_t<T>     rules[N];
        port_def_t          action;

        combined_rule_t() {};

        /// @brief Initializates from buffer.
        /// @param buffer "[<unary_rule(0)>;<unary_rule(1)>;...<unary_rule(N-1)>]<port_type><port_index>" 
        /// (e.g. "[d0=1,XXXXXXXXXXXXXXX1;v0=1.22,XXXXXXXXXXXXXXX2]r0")
        combined_rule_t(const char *buffer);
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

        /// @brief Hardware revision.
        uint8_t     hrev();
        /// @brief Software revision.
        uint8_t     srev();
        /// @brief Number of installed current input.
        uint8_t     n_curr();
        /// @brief Number of installed voltage input.
        uint8_t     n_volt();
        /// @brief Number of installed pulse input.
        uint8_t     n_pulse();
        /// @brief Number of installed temperature sensor.
        uint8_t     n_temperature();
        
        /// @brief Latch output status.
        /// @param idx of the output
        /// @return true if active
        bool        latch_status(int idx);

        /// @brief Relay output status.
        /// @param idx of the output
        /// @return true if active
        bool        relay_status(int idx);

        /// @brief Digital input status.
        /// @param idx of the input
        /// @return true if logically high
        bool        digital_status(int idx);

        /// @brief Current input value.
        /// @param idx of the input
        /// @return mA
        float       current_value(int idx);

        /// @brief Voltage input value.
        /// @param idx of the input
        /// @return mV
        float       voltage_value(int idx);

        /// @brief Pulse input value.
        /// @param idx of the input
        /// @return pulse count
        uint16_t    pulse_value(int idx);

        /// @brief Temperature input value.
        /// @param idx of the input
        /// @return K
        float       temperature_value(int idx);

    };

    /// @brief Desired status rapresentation.
    struct des_status_t {
        /// @brief Latch port status - (1 << idx) => means Latch[idx] should be enabled.
        uint32_t    latch_mask;
        /// @brief Relay port status - (1 << idx) => means Relay[idx] should be enabled.
        uint32_t    relay_mask;
        /// @brief Led channel status - (1 << idx) => means Led[idx] control should be enabled.
        uint8_t     led_mask;

        bool        latch_status(int idx);
        void        latch_status(int idx, bool status);

        bool        relay_status(int idx);
        void        relay_status(int idx, bool status);

        bool        led_status(int idx);
        void        led_status(int idx, bool status);
    };
}
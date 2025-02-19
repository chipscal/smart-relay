#pragma once

#include "esp_err.h"
#include <array>

#ifdef CONFIG_EDGE_SOFT_REV
    #define EDGE_SREV CONFIG_EDGE_SOFT_REV
#else
    #define EDGE_SREV 0U
#endif

#ifdef CONFIG_EDGE_SOFT_MINOR
    #define EDGE_SMINOR CONFIG_EDGE_SOFT_MINOR
#else
    #define EDGE_SMINOR 0U
#endif

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

namespace clab::iot_services {
    

    inline bool is_little_endian() {
        int x = 0x1;
        // Note that in C 'not 0 is considered true' and '0 is considered false'
        return (*(char*)&x);
    }

    //! Byte swap unsigned short
    inline uint16_t swap_uint16(uint16_t val) 
    {
        return (val << 8) | (val >> 8);
    }

    //! Byte swap short
    inline int16_t swap_int16(int16_t val) 
    {
        return (val << 8) | ((val >> 8) & 0xFF);
    }

    //! Byte swap unsigned int
    inline uint32_t swap_uint32(uint32_t val)
    {
        val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF); 
        return (val << 16) | (val >> 16);
    }

    //! Byte swap int
    inline int32_t swap_int32(int32_t val)
    {
        val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF); 
        return (val << 16) | ((val >> 16) & 0xFFFF);
    }

    inline int64_t swap_int64(int64_t val)
    {
        val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
        val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
        return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
    }

    inline uint64_t swap_uint64(uint64_t val)
    {
        val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
        val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
        return (val << 32) | (val >> 32);
    }

    template <class T>
    constexpr T &constexpr_min(T &a, T &b) {
        return a > b ? b : a;
    }

    template <class T>
    constexpr T &array_min_impl(T *begin, T *end) {
        return begin + 1 == end
            ? *begin
            : constexpr_min(*begin, array_min_impl(begin + 1, end));
    }

    template <class T, std::size_t In>
    constexpr T &array_min(T(&arr)[In]) {
        return array_min_impl(arr, arr + In);
    }

    template <class T>
    constexpr T &constexpr_max(T &a, T &b) {
        return a > b ? a : b;
    }

    template <class T>
    constexpr T &array_max_impl(T *begin, T *end) {
        return begin + 1 == end
            ? *begin
            : constexpr_max(*begin, array_max_impl(begin + 1, end));
    }

    template <class T, std::size_t In>
    constexpr T &array_max(T(&arr)[In]) {
        return array_max_impl(arr, arr + In);
    }

    constexpr size_t alligned_big_enough(size_t size) {
        size_t order = 1;
        while (size > 1) {
            size = size >> 1;
            order++;
        }

        return (1 << order);
    }

    template <class T>
    void set_mask(T &mask, int idx, bool status)
    {
        if (status)
            mask |= (1 << idx);
        else
            mask &= ~(1 << idx);
    }

    template <class T>
    bool get_mask(T &mask, int idx)
    {
        return ((mask & (1 << idx)) > 0);
    }

    template <class T>
    class dummy_array {
        public:
            constexpr int size() {return 0;}
            T& operator[](std::size_t idx)       { abort(); }
            const T& operator[](std::size_t idx) const { abort(); }
    };

    template <typename T, std::size_t N>
    constexpr std::array<T, N> filled_array(const T value) {
        std::array<T, N> to_ret;
        to_ret.fill(value);
        return to_ret;
    }

    template <typename T>
    size_t sprint_uint32_binary(char *buffer, T value) {
        uint32_t mask = value;
        size_t cnt = 0;                                                                                      
        if (!irreo::iot_services::is_little_endian())                                                               
            mask = irreo::iot_services::swap_uint32(mask);            

        for (int k = 0; k < sizeof(T); k++) {                                                                
            cnt += sprintf(buffer + cnt, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(*(((uint8_t *)&(mask)) + sizeof(T) - k - 1)));
            cnt += sprintf(buffer + cnt, " ");
        }

        return cnt;
    
    }

    template <typename T>
    size_t sprint_uint8_binary(char *buffer, T value) {
        return sprintf(buffer, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(*((uint8_t *)&(value))));
    }

    template <typename T>
    void sprint_array_hex(char *buffer, const T *array, size_t array_size) {
        for (size_t i = 0; i < array_size; i++) {
            sprintf((char *)(buffer + i * 2), "%02X", *((uint8_t *)array + i));
        }
    }

    
}

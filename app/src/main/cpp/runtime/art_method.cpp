#include "art_method.h"

#define UNLIKELY(x) __builtin_expect(!!(x), 0)

static inline uint32_t DecodeUnsignedLeb128(const uint8_t** data) {
    const uint8_t* ptr = *data;
    int result = *(ptr++);
    if (UNLIKELY(result > 0x7f)) {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur > 0x7f) {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur > 0x7f) {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur > 0x7f) {
                    // Note: We don't check to see if cur is out of range here,
                    // meaning we tolerate garbage in the four high-order bits.
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }
    *data = ptr;
    return static_cast<uint32_t>(result);
}

static inline int32_t DecodeSignedLeb128(const uint8_t** data) {
    const uint8_t* ptr = *data;
    int32_t result = *(ptr++);
    if (result <= 0x7f) {
        result = (result << 25) >> 25;
    } else {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur <= 0x7f) {
            result = (result << 18) >> 18;
        } else {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur <= 0x7f) {
                result = (result << 11) >> 11;
            } else {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur <= 0x7f) {
                    result = (result << 4) >> 4;
                } else {
                    // Note: We don't check to see if cur is out of range here,
                    // meaning we tolerate garbage in the four high-order bits.
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }
    *data = ptr;
    return result;
}

uint8_t * calc_codeitem_end(const uint8_t ** data) {
    uint32_t encoded_catch_handler_list_size = DecodeUnsignedLeb128(data);
    for (; encoded_catch_handler_list_size > 0; encoded_catch_handler_list_size --) {
        int32_t encoded_catch_handler = DecodeSignedLeb128(data);
        int encoded_catch_handler_size = encoded_catch_handler;
        if (encoded_catch_handler <= 0) {
            encoded_catch_handler_size = -encoded_catch_handler;
        }
        for (; encoded_catch_handler_size > 0; encoded_catch_handler_size --) {
            DecodeUnsignedLeb128(data);
            DecodeUnsignedLeb128(data);
        }
        if (encoded_catch_handler <= 0) {
            DecodeUnsignedLeb128(data);
        }
    }
    return (uint8_t *)(*data);
}


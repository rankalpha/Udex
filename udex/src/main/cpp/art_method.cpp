//
// Created by thehepta on 2024/2/15.
//
#include "art_method.h"
#include "runtime.h"
#include "util.h"

DexFile * ArtMethod::GetDexFile(void *artMethod) {
    uint16_t word_10 = 3;
    uint64_t v2;
    if ((*((uint32_t *)artMethod + 1) & 0x40000) != 0) {
        v2 = ((runtime::fun_art_ArtMethod_GetObsoleteDexCache)runtime::ArtMethod_GetObsoleteDexCache)(artMethod);
    } else {
        v2 = *(uint32_t *)(*(uint32_t *)artMethod + 16);
    }
    return (DexFile *)(*(uint64_t *)(v2 + 16));
}

void * ArtMethod::GetCodeItem(void * artMethod) {
    uint32_t * p1 = (uint32_t *)artMethod;
    uint32_t v1 = p1[1];
    uint32_t v2 = p1[2];
    //bool flag1 = (v1 & 0x100) != 0;
    //bool flag2 = (v1 & 0x400) != 0;
    //bool flag3 = (v1 & 0x81400400) == 0x1400400;
    //bool flag4 = v2 == -1;
    //LOGV("flag1: %b, flag2: %b, flag3: %b, flag4: %b", flag1, flag2, flag3, flag4);
    p1[1] = 0;
    p1[2] = 0;
    void * code_item = ((runtime::fun_NterpGetCodeItem)runtime::NterpGetCodeItem)(artMethod);
    p1[1] = v1;
    p1[2] = v2;
    return code_item;
}

uint32_t DecodeUnsignedLeb128(const uint8_t** data) {
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

uint8_t* codeitem_end(const uint8_t** pData) {
    uint32_t encoded_catch_handler_list_size = DecodeUnsignedLeb128(pData);
    for (; encoded_catch_handler_list_size > 0; encoded_catch_handler_list_size--) {
        int32_t encoded_catch_handler = DecodeSignedLeb128(pData);  // struct sleb128 size
        int encoded_catch_handler_size = encoded_catch_handler;
        if (encoded_catch_handler <= 0) {
            encoded_catch_handler_size = -encoded_catch_handler;
        }
        for (; encoded_catch_handler_size > 0; encoded_catch_handler_size--) {
            DecodeUnsignedLeb128(pData);  // struct uleb128 type_idx
            DecodeUnsignedLeb128(pData);  // struct uleb128 addr
        }
        if (encoded_catch_handler <= 0) {
            DecodeUnsignedLeb128(pData);
        }
    }
    return (uint8_t*)(*pData);
}

uint8_t* get_encoded_catch_handler_list(CodeItem* code_item) {
    uint16_t* insns_end_ = reinterpret_cast<uint16_t*>(&code_item->insns_[code_item->insns_size_in_code_units_]);
    uint8_t* padding_end = (uint8_t*)((reinterpret_cast<uintptr_t>(insns_end_) + 3) & ~3);
    uint8_t* encoded_catch_handler_list_data = padding_end + code_item->tries_size_ * 8;
    return encoded_catch_handler_list_data;
}
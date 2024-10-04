//
// Created by thehepta on 2024/2/15.
//

#pragma once

#include <stdint.h>
#include "runtime/dex_file.h"
#include "modifiers.h"

class ArtMethod {
    //    void * vptrAdree ;
    uint32_t declaring_class_;
    uint32_t access_flags_;
    //    uint32_t dex_code_item_offset_;
    uint32_t dex_method_index_;
    uint16_t method_index_;
    uint16_t hotness_count_;

public:
    static DexFile * GetDexFile(void * artMethod);
    static void * GetCodeItem(void * artMethod);
    static void * GetCodeItem1(void * artMethod);
    static void * GetCodeItem2(void * artMethod);

    uint32_t GetAccessFlags() const { return access_flags_; }
    bool IsNative() const {
        return IsNative(GetAccessFlags());
    }
    static bool IsNative(uint32_t access_flags) {
        return (access_flags & kAccNative) != 0;
    }
    bool IsAbstract() const {
        return IsAbstract(GetAccessFlags());
    }
    static bool IsDefaultConflicting(uint32_t access_flags) {
        static constexpr uint32_t kMask = kAccIntrinsic | kAccCopied | kAccAbstract | kAccDefault;
        static constexpr uint32_t kValue = kAccCopied | kAccAbstract | kAccDefault;
        return (access_flags & kMask) == kValue;
    }
    static bool IsAbstract(uint32_t access_flags) {
        return (access_flags & kAccAbstract) != 0 && !IsDefaultConflicting(access_flags);
    }
    uint32_t GetDexMethodIndex() { return dex_method_index_; }
};

struct CodeItem {
    uint16_t registers_size_;  // the number of registers used by this code
    //   (locals + parameters)
    uint16_t ins_size_;  // the number of words of incoming arguments to the method
    //   that this code is for
    uint16_t outs_size_;  // the number of words of outgoing argument space required
    //   by this code for method invocation
    uint16_t tries_size_;  // the number of try_items for this instance. If non-zero,
    //   then these appear as the tries array just after the
    //   insns in this instance.
    uint32_t debug_info_off_;  // Holds file offset to debug info stream.

    uint32_t insns_size_in_code_units_;  // size of the insns array, in 2 byte code units
    uint16_t insns_[1];                  // actual array of bytecode.
};

uint8_t *codeitem_end(const uint8_t **pData);
uint8_t *get_encoded_catch_handler_list(CodeItem *code_item);
#ifndef DEX_FILE_STRUCT_H
#define DEX_FILE_STRUCT_H

#include <stdint.h>
#include "art_method.h"

#define OFFSETOF_MEMBER(t, f) offsetof(t, f)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete

struct CodeItem {
protected:
    CodeItem() = default;

private:
    DISALLOW_COPY_AND_ASSIGN(CodeItem);
};

struct CompactCodeItem : public CodeItem {
    static constexpr size_t InsnsCountAndFlagsOffset() {
        return OFFSETOF_MEMBER(CompactCodeItem, insns_count_and_flags_);
    }

    static constexpr size_t InsnsOffset() {
        return OFFSETOF_MEMBER(CompactCodeItem, insns_);
    }

    uint16_t fields_;

    uint16_t insns_count_and_flags_;

    uint16_t insns_[1];

    DISALLOW_COPY_AND_ASSIGN(CompactCodeItem);
};

struct StandardCodeItem : public CodeItem {
    static constexpr size_t kAlignment = 4;

    static constexpr size_t InsSizeOffset() {
        return OFFSETOF_MEMBER(StandardCodeItem, ins_size_);
    }

    static constexpr size_t OutsSizeOffset() {
        return OFFSETOF_MEMBER(StandardCodeItem, outs_size_);
    }

    static constexpr size_t RegistersSizeOffset() {
        return OFFSETOF_MEMBER(StandardCodeItem, registers_size_);
    }

    static constexpr size_t InsnsOffset() {
        return OFFSETOF_MEMBER(StandardCodeItem, insns_);
    }

    uint8_t * get_encoded_catch_handler_list() {
        uint16_t * insns_end_ = reinterpret_cast<uint16_t *>(&insns_[insns_size_in_code_units_]);
        uint8_t * padding_end = (uint8_t *)((reinterpret_cast<uintptr_t>(insns_end_) + 3) & ~3);
        uint8_t * encoded_catch_handler_list_data = padding_end + tries_size_ * 8;
        return encoded_catch_handler_list_data;
    }

    uint32_t GetCodeItemSize() {
        if (tries_size_ > 0) {
            const uint8_t * handler_data = get_encoded_catch_handler_list();
            uint8_t * tail = calc_codeitem_end(&handler_data);
            return tail - (uint8_t *)(&registers_size_);
        } else {
            return 16 + insns_size_in_code_units_ * 2;
        }
    }
private:
    StandardCodeItem() = default;

    uint16_t registers_size_;            // the number of registers used by this code
                                         //   (locals + parameters)
    uint16_t ins_size_;                  // the number of words of incoming arguments to the method
                                         //   that this code is for
    uint16_t outs_size_;                 // the number of words of outgoing argument space required
                                         //   by this code for method invocation
    uint16_t tries_size_;                // the number of try_items for this instance. If non-zero,
                                         //   then these appear as the tries array just after the
                                         //   insns in this instance.
    uint32_t debug_info_off_;            // Holds file offset to debug info stream.

    uint32_t insns_size_in_code_units_;  // size of the insns array, in 2 byte code units
    uint16_t insns_[1];                  // actual array of bytecode.

    DISALLOW_COPY_AND_ASSIGN(StandardCodeItem);
};

#endif // DEX_FILE_STRUCT_H
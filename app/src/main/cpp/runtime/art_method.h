#ifndef ART_METHOD_H
#define ART_METHOD_H

#include <stdint.h>
#include <cstdint>
#include <atomic>
#include "modifiers.h"

#pragma once

uint8_t * calc_codeitem_end(const uint8_t ** data);

class ArtMethod {
public:
    bool IsDefaultConflicting() const {
        return IsDefaultConflicting(GetAccessFlags());
    }

    static bool IsDefaultConflicting(uint32_t access_flags) {
        // Default conflct methods are marked as copied, abstract and default.
        // We need to check the kAccIntrinsic too, see `IsCopied()`.
        static constexpr uint32_t kMask = kAccIntrinsic | kAccCopied | kAccAbstract | kAccDefault;
        static constexpr uint32_t kValue = kAccCopied | kAccAbstract | kAccDefault;
        return (access_flags & kMask) == kValue;
    }

    // Returns true if the method is abstract.
    bool IsAbstract() const {
        return IsAbstract(GetAccessFlags());
    }

    static bool IsAbstract(uint32_t access_flags) {
        // Default confliciting methods have `kAccAbstract` set but they are not actually abstract.
        return (access_flags & kAccAbstract) != 0 && !IsDefaultConflicting(access_flags);
    }

    uint32_t GetAccessFlags() const {
        return access_flags_.load(std::memory_order_relaxed);
    }

    bool IsNative() const {
        return IsNative(GetAccessFlags());
    }

    static bool IsNative(uint32_t access_flags) {
        return (access_flags & kAccNative) != 0;
    }

    uint32_t GetDexMethodIndex() const {
        return dex_method_index_;
    }
protected:
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    uint32_t declaring_class_; // GcRoot<mirror::Class>

    // Access flags; low 16 bits are defined by spec.
    // Getting and setting this flag needs to be atomic when concurrency is
    // possible, e.g. after this method's class is linked. Such as when setting
    // verifier flags and single-implementation flag.
    std::atomic<std::uint32_t> access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // interface's method array in `IfTable`s of implementing classes.
    uint16_t method_index_;

    union {
        // Non-abstract methods: The hotness we measure for this method. Not atomic,
        // as we allow missing increments: if the method is hot, we will see it eventually.
        uint16_t hotness_count_;
        // Abstract methods: IMT index.
        uint16_t imt_index_;
    };
};

#endif // ART_METHOD_H
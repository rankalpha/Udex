#ifndef DEX_FILE_H
#define DEX_FILE_H

#include <stdint.h>
#include <string>

// https://cs.android.com/android/platform/superproject/+/master:art/libdexfile/dex/dex_file.h;l=112?q=DexFile&sq=
// https://cs.android.com/android/platform/superproject/+/android-14.0.0_r25:art/libdexfile/dex/dex_file.h

static constexpr size_t kSha1DigestSize = 20;
// Raw header_item.
struct Header {
    uint8_t magic_[8] = {};
    uint32_t checksum_ = 0;  // See also location_checksum_
    uint8_t signature_[kSha1DigestSize] = {};
    uint32_t file_size_ = 0;  // size of entire file
    uint32_t header_size_ = 0;  // offset to start of next section
    uint32_t endian_tag_ = 0;
    uint32_t link_size_ = 0;  // unused
    uint32_t link_off_ = 0;  // unused
    uint32_t map_off_ = 0;  // map list offset from data_off_
    uint32_t string_ids_size_ = 0;  // number of StringIds
    uint32_t string_ids_off_ = 0;  // file offset of StringIds array
    uint32_t type_ids_size_ = 0;  // number of TypeIds, we don't support more than 65535
    uint32_t type_ids_off_ = 0;  // file offset of TypeIds array
    uint32_t proto_ids_size_ = 0;  // number of ProtoIds, we don't support more than 65535
    uint32_t proto_ids_off_ = 0;  // file offset of ProtoIds array
    uint32_t field_ids_size_ = 0;  // number of FieldIds
    uint32_t field_ids_off_ = 0;  // file offset of FieldIds array
    uint32_t method_ids_size_ = 0;  // number of MethodIds
    uint32_t method_ids_off_ = 0;  // file offset of MethodIds array
    uint32_t class_defs_size_ = 0;  // number of ClassDefs
    uint32_t class_defs_off_ = 0;  // file offset of ClassDef array
    uint32_t data_size_ = 0;  // size of data section
    uint32_t data_off_ = 0;  // file offset of data section
};

class DexFile {
public:
    void * vptrAdree ;

    // The base address of the memory mapping.
    const uint8_t* const begin_;

    // The size of the underlying memory allocation in bytes.
    const size_t size_;

    // The base address of the data section (same as Begin() for standard dex).
    const uint8_t* const data_begin_;

    // Typically the dex file name when available, alternatively some identifying string.
    //
    // The ClassLinker will use this to match DexFiles the boot class
    // path to DexCache::GetLocation when loading from an image.
    const std::string location_;

    const uint32_t location_checksum_;

    // Points to the header section.
    const Header* const header_;

    // Points to the base of the string identifier list.
    const void* const string_ids_; // const dex::StringId* const string_ids_;

    // Points to the base of the type identifier list.
    const void* const type_ids_; // const dex::TypeId* const type_ids_;

    // Points to the base of the field identifier list.
    const void* const field_ids_; // const dex::FieldId* const field_ids_;

    // Points to the base of the method identifier list.
    const void* const method_ids_; // const dex::MethodId* const method_ids_;

    // Points to the base of the prototype identifier list.
    const void* const proto_ids_; // const dex::ProtoId* const proto_ids_;

    // Points to the base of the class definition list.
    const void* const class_defs_; // const dex::ClassDef* const class_defs_;

    // Points to the base of the method handles list.
    const void* method_handles_; // const dex::MethodHandleItem* method_handles_;

    // Number of elements in the method handles list.
    size_t num_method_handles_;

    // Points to the base of the call sites id list.
    const void * call_site_ids_;

    // Number of elements in the call sites list.
    size_t num_call_site_ids_;

    // Points to the base of the hiddenapi class data item_, or nullptr if the dex
    // file does not have one.
    const void * hiddenapi_class_data_;

    // If this dex file was loaded from an oat file, oat_dex_file_ contains a
    // pointer to the OatDexFile it was loaded from. Otherwise oat_dex_file_ is
    // null.
    const void * oat_dex_file_;

    // Manages the underlying memory allocation.
    void * container_;

    // If the dex file is a compact dex file. If false then the dex file is a standard dex file.
    const bool is_compact_dex_;

    inline bool IsCompactDexFile() const {
        return is_compact_dex_;
    }

    inline bool IsStandardDexFile() const {
        return !is_compact_dex_;
    }
};

#endif // DEX_FILE_H

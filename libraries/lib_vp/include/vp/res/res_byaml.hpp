#pragma once

namespace vp::res {

    enum class ByamlDataType : u8 {
        HashArray               = 0x20,
        HashArrayWithRemap      = 0x30,
        StringIndex             = 0xa0,
        BinaryData              = 0xa1,
        BinaryDataWithAlignment = 0xa2,
        Array                   = 0xc0,
        Dictionary              = 0xc1,
        KeyTable                = 0xc2,
        DictionaryWithRemap     = 0xc4,
        RelocatedKeyTable       = 0xc5,
        MonoTypedArray          = 0xc8,
        Bool                    = 0xd0,
        S32                     = 0xd1,
        F32                     = 0xd2,
        U32                     = 0xd3,
        S64                     = 0xd4,
        U64                     = 0xd5,
        F64                     = 0xd6,
        Null                    = 0xff,
    };

    struct ResByamlContainer {
        u32 data_type : 8;
        u32 count     : 24;
    };
    static_assert(sizeof(ResByamlContainer) == 0x4);

    struct ResByamlDictionaryPair {
        u32 key_index : 24;
        u32 data_type  : 8;
        union {
            s32   s32_value;
            u32   u32_value;
            float f32_value;
        };
    };
    static_assert(sizeof(ResByamlDictionaryPair) == 0x8);

    struct ResByaml {
        u16 magic;
        u16 version;
        u32 key_table_offset;
        u32 string_table_offset;
        u32 data_offset;

        static constexpr u16 cMagic = util::TCharCode16("YB");
        static constexpr u16 cMaxVersion = 7;
        static constexpr u16 cMinVersion = 4;

        ALWAYS_INLINE void *GetBigDataOffset(u32 offset) {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + offset);
        }

        ALWAYS_INLINE bool IsValid() const {

            /* Header checks */
            if (magic != cMagic || version < cMinVersion || cMaxVersion < version) { return false; }

            /* String table checks */
            if (key_table_offset != 0 && VerifyStringTable(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(this) + key_table_offset)) == false) { return false; }
            if (string_table_offset != 0 && VerifyStringTable(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(this) + string_table_offset)) == false) { return false; }

            /* Table offset checks */
            const ResByamlContainer *string_table = reinterpret_cast<ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + string_table_offset);
            const u32 final_string_offset = (string_table_offset != 0) ? *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(string_table) + sizeof(u32) * (string_table->count + 1)) : 0;

            const ResByamlContainer *key_table = reinterpret_cast<ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + key_table_offset);
            const u32 final_key_offset = (key_table_offset != 0) ? *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(key_table) + sizeof(u32) * (key_table->count + 1)) : 0;

            if ((string_table_offset | key_table_offset) != 0 && data_offset == 0) { return false; }
            if (key_table_offset != 0) {
                if (string_table_offset != 0 && string_table_offset < final_key_offset) { return false; }
                if (data_offset != 0 && data_offset < final_key_offset) { return false; }
            }

            return (final_string_offset <= data_offset) | (string_table_offset == 0) | (data_offset == 0);
        }

        static ALWAYS_INLINE bool IsValid(void *file) {
            return reinterpret_cast<ResByaml*>(file)->IsValid();
        }

        static bool VerifyStringTable(const unsigned char *table) {

            /* Cast table and check container type */
            const ResByamlContainer *header = reinterpret_cast<const ResByamlContainer*>(table);
            if (static_cast<ByamlDataType>(header->data_type) != ByamlDataType::KeyTable) { return false; }

            const u32 string_count = header->count;

            /* Verify we have strings */
            if (string_count == 0) { return false; }

            for (u32 i = 0; i < string_count; ++i) {
                /* Find offsets at start and end of string */
                const u32 offset_start = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(header) + sizeof(ResByamlContainer) + i * sizeof(u32));
                const u32 offset_end   = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(header) + sizeof(ResByamlContainer) + i * sizeof(u32) + sizeof(u32));

                /* Verify all sucessive string indices point to after a null terminator */
                if (*reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(header) + offset_end - 1) != '\0') { return false; }

                /* Verify the sucessive offsets increase */
                if (offset_start > offset_end) { return false; }

                /* Verify the string table is sorted */
                if (i == (string_count - 1)) { break; }

                const char *first_string  = reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(header) + offset_start);
                const char *second_string = reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(header) + offset_end);
                const u32 sort_result = ::strcmp(first_string, second_string);
                if (sort_result < 1) { return false; }
            }

            return true;
        }
    };
    static_assert(sizeof(ResByaml) == 0x10);

    struct ByamlData {
        u32 key_index : 24;
        u32 data_type : 8;
        union {
            s32   s32_value;
            u32   u32_value;
            float f32_value;
        };

        constexpr ALWAYS_INLINE void SetPair(const ResByamlDictionaryPair *pair) {
            key_index = pair->key_index;
            data_type = pair->data_type;
            u32_value = pair->u32_value;
        }
        
        constexpr ALWAYS_INLINE bool IsKeyIndexValid() {
            return (key_index != 0xff'ffff);
        }
    };

    constexpr const char *ByamlDataTypeToString(ByamlDataType data_type) {
        switch (data_type) {
            case ByamlDataType::StringIndex:
                return "String";
            case ByamlDataType::BinaryData:
                return "BinaryData";
            case ByamlDataType::BinaryDataWithAlignment:
                return "BinaryDataWithAlignment";
            case ByamlDataType::Array:
                return "Array";
            case ByamlDataType::MonoTypedArray:
                return "MonoTypedArray";
            case ByamlDataType::Dictionary:
                return "Dictionary";
            case ByamlDataType::DictionaryWithRemap:
                return "DictionaryWithRemap";
            case ByamlDataType::KeyTable:
                return "KeyTable";
            case ByamlDataType::RelocatedKeyTable:
                return "RelocatedKeyTable";
            case ByamlDataType::Bool:
                return "Bool";
            case ByamlDataType::S32:
                return "S32";
            case ByamlDataType::F32:
                return "F32";
            case ByamlDataType::U32:
                return "U32";
            case ByamlDataType::S64:
                return "S64";
            case ByamlDataType::U64:
                return "U64";
            case ByamlDataType::F64:
                return "F64";
            default:
            {
                if (static_cast<ByamlDataType>(static_cast<u32>(data_type) & 0xE0) == ByamlDataType::HashArray) {
                    return "HashArray";
                }
                if (static_cast<ByamlDataType>(static_cast<u32>(data_type) & 0xF0) == ByamlDataType::HashArrayWithRemap) {
                    return "HashArrayWithRemap";
                }
            }
                break;
            case ByamlDataType::Null:
            break;
        };
        return "Null";
    }
}

/*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

namespace vp::res {

    enum class ByamlDataType : u8 {
        HashArrayU32_1           = 0x20,
        HashArrayU32_2           = 0x21,
        HashArrayU32_3           = 0x22,
        HashArrayU32_4           = 0x23,
        HashArrayU32_5           = 0x24,
        HashArrayU32_6           = 0x25,
        HashArrayU32_7           = 0x26,
        HashArrayU32_8           = 0x27,
        HashArrayU32_9           = 0x28,
        HashArrayU32_10          = 0x29,
        HashArrayU32_11          = 0x2a,
        HashArrayU32_12          = 0x2b,
        HashArrayU32_13          = 0x2c,
        HashArrayU32_14          = 0x2d,
        HashArrayU32_15          = 0x2e,
        HashArrayU32_16          = 0x2f,
        HashArrayWithRemapU32_1  = 0x30,
        HashArrayWithRemapU32_2  = 0x31,
        HashArrayWithRemapU32_3  = 0x32,
        HashArrayWithRemapU32_4  = 0x33,
        HashArrayWithRemapU32_5  = 0x34,
        HashArrayWithRemapU32_6  = 0x35,
        HashArrayWithRemapU32_7  = 0x36,
        HashArrayWithRemapU32_8  = 0x37,
        HashArrayWithRemapU32_9  = 0x38,
        HashArrayWithRemapU32_10 = 0x39,
        HashArrayWithRemapU32_11 = 0x3a,
        HashArrayWithRemapU32_12 = 0x3b,
        HashArrayWithRemapU32_13 = 0x3c,
        HashArrayWithRemapU32_14 = 0x3d,
        HashArrayWithRemapU32_15 = 0x3e,
        HashArrayWithRemapU32_16 = 0x3f,
        StringIndex              = 0xa0,
        BinaryData               = 0xa1,
        BinaryDataWithAlignment  = 0xa2,
        Array                    = 0xc0,
        Dictionary               = 0xc1,
        KeyTable                 = 0xc2,
        DictionaryWithRemap      = 0xc4,
        RelocatedKeyTable        = 0xc5,
        MonoTypedArray           = 0xc8,
        Bool                     = 0xd0,
        S32                      = 0xd1,
        F32                      = 0xd2,
        U32                      = 0xd3,
        S64                      = 0xd4,
        U64                      = 0xd5,
        F64                      = 0xd6,
        Null                     = 0xff,
    };

    template <typename T>
        requires ((std::is_same<std::add_cv_t<T>, std::add_cv_t<bool>>::value)         ||
                  (std::is_same<std::add_cv_t<T>, std::add_cv_t<s32>>::value)          ||
                  (std::is_same<std::add_cv_t<T>, std::add_cv_t<float>>::value)        ||
                  (std::is_same<std::add_cv_t<T>, std::add_cv_t<u32>>::value)          ||
                  (std::is_same<std::add_cv_t<T>, std::add_cv_t<s64>>::value)          ||
                  (std::is_same<std::add_cv_t<T>, std::add_cv_t<u64>>::value)          ||
                  (std::is_same<std::add_cv_t<T>, std::add_cv_t<double>>::value)       ||
                  (std::is_same<std::add_cv_t<T>, std::add_cv_t<const char*>>::value))
    consteval ByamlDataType TGetByamlDataTypeValue() {
        if constexpr (std::is_same<std::add_cv_t<T>, std::add_cv_t<bool>>::value)        { return ByamlDataType::Bool; }
        if constexpr (std::is_same<std::add_cv_t<T>, std::add_cv_t<s32>>::value)         { return ByamlDataType::S32; }
        if constexpr (std::is_same<std::add_cv_t<T>, std::add_cv_t<float>>::value)       { return ByamlDataType::F32; }
        if constexpr (std::is_same<std::add_cv_t<T>, std::add_cv_t<u32>>::value)         { return ByamlDataType::U32; }
        if constexpr (std::is_same<std::add_cv_t<T>, std::add_cv_t<s64>>::value)         { return ByamlDataType::S64; }
        if constexpr (std::is_same<std::add_cv_t<T>, std::add_cv_t<u64>>::value)         { return ByamlDataType::U64; }
        if constexpr (std::is_same<std::add_cv_t<T>, std::add_cv_t<double>>::value)      { return ByamlDataType::F64; }
        if constexpr (std::is_same<std::add_cv_t<T>, std::add_cv_t<const char*>>::value) { return ByamlDataType::StringIndex; }
        return ByamlDataType::Null;
    }

    constexpr ALWAYS_INLINE bool IsBigDataType(ByamlDataType data_type) {
        return (static_cast<u8>(data_type) < 0xd0) & (0xd3 < static_cast<u8>(data_type));
    }
    constexpr ALWAYS_INLINE bool IsContainerType(ByamlDataType data_type) {
        const u8 value = (static_cast<u8>(data_type) & 0xe0);
        return (value == 0xc0) | (value == 0x20);
    }

    struct ResByamlContainer {
        union {
            u32 header;
            struct {
                u32 data_type : 8;
                u32 count     : 24;
            };
        };

        constexpr void SwapEndian() {
            count = vp::util::SwapEndian24(static_cast<u32>(count));
        }
    };
    static_assert(sizeof(ResByamlContainer) == 0x4);

    struct ResByamlDictionaryPair {
        union {
            u32 header;
            struct {
                u32 key_index : 24;
                u32 data_type  : 8;
            };
        };
        union {
            s32   s32_value;
            u32   u32_value;
            float f32_value;
        };
    };
    static_assert(sizeof(ResByamlDictionaryPair) == 0x8);

    struct ByamlBigDataCache;

    struct ResByaml {
        u16 magic;
        u16 version;
        u32 key_table_offset;
        u32 string_table_offset;
        u32 data_offset;

        static constexpr u16 cMagic = util::TCharCode16("YB");
        static constexpr u16 cTargetVersion = 10;

        ALWAYS_INLINE const void *GetBigDataOffset(u32 offset) const {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + offset);
        }

        ALWAYS_INLINE bool IsValid() const {

            /* Endianess check */
            const bool is_reverse_endian = this->IsReverseEndian();

            /* Header checks */
            const u32 r_magic   = (is_reverse_endian == false) ? magic : vp::util::SwapEndian(magic);
            const u32 r_version = (is_reverse_endian == false) ? version : vp::util::SwapEndian(version);
            if (r_magic != cMagic || cTargetVersion < r_version) { return false; }

            /* String table checks */
            const u32 r_key_table_offset    = (is_reverse_endian == false) ? key_table_offset : vp::util::SwapEndian(key_table_offset);
            const u32 r_string_table_offset = (is_reverse_endian == false) ? string_table_offset : vp::util::SwapEndian(string_table_offset);
            if (r_key_table_offset != 0    && VerifyStringTable(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(this) + r_key_table_offset), is_reverse_endian) == false)       { return false; }
            if (r_string_table_offset != 0 && VerifyStringTable(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(this) + r_string_table_offset), is_reverse_endian) == false) { return false; }

            /* Table offset checks */
            const ResByamlContainer *string_table = reinterpret_cast<ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + r_string_table_offset);
            const u32 r_string_count              = (is_reverse_endian == false) ? string_table->count : vp::util::SwapEndian24(string_table->count);
            const u32 final_string_offset         = (r_string_table_offset != 0) ? *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(string_table) + sizeof(u32) * (r_string_count + 1)) : 0;
            const u32 r_final_string_offset       = (is_reverse_endian == false) ? final_string_offset : vp::util::SwapEndian(final_string_offset);

            const ResByamlContainer *key_table = reinterpret_cast<ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + r_key_table_offset);
            const u32 r_key_count              = (is_reverse_endian == false) ? key_table->count : vp::util::SwapEndian24(key_table->count);
            const u32 final_key_offset         = (r_key_table_offset != 0) ? *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(key_table) + sizeof(u32) * (r_key_count + 1)) : 0;
            const u32 r_final_key_offset       = (is_reverse_endian == false) ? final_key_offset : vp::util::SwapEndian(final_key_offset);

            if ((r_string_table_offset | r_key_table_offset) != 0 && data_offset == 0) { return false; }
            if (r_key_table_offset != 0) {
                if (r_string_table_offset != 0 && r_string_table_offset < r_final_key_offset) { return false; }
                if (data_offset != 0 && data_offset < r_final_key_offset) { return false; }
            }

            return (r_final_string_offset <= data_offset) | (r_string_table_offset == 0) | (data_offset == 0);
        }

        constexpr bool IsReverseEndian() const {
            constexpr u16 swapped_magic = vp::util::SwapEndian(cMagic);
            return magic == swapped_magic;
        }

        static ALWAYS_INLINE ResByaml *ResCast(void *file) {
            if (file == nullptr || reinterpret_cast<ResByaml*>(file)->IsValid() == false) { return nullptr; }
            return reinterpret_cast<ResByaml*>(file);
        }

        static bool VerifyStringTable(const unsigned char *table, const bool is_reverse_endian) {

            /* Cast table and check container type */
            const ResByamlContainer *header = reinterpret_cast<const ResByamlContainer*>(table);
            if (static_cast<ByamlDataType>(header->data_type) != ByamlDataType::KeyTable) { return false; }

            /* Verify we have strings */
            const u32 string_count = (is_reverse_endian == false) ? header->count : vp::util::SwapEndian24(header->count);
            if (string_count == 0) { return false; }

            for (u32 i = 0; i < string_count; ++i) {
                /* Find offsets at start and end of string */
                const u32 *start_addr  = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(header) + sizeof(ResByamlContainer) + i * sizeof(u32));
                const u32 *end_addr    = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(start_addr) + sizeof(u32));
                const u32 offset_start = (is_reverse_endian == false) ? *start_addr : vp::util::SwapEndian(*start_addr);
                const u32 offset_end   = (is_reverse_endian == false) ? *end_addr : vp::util::SwapEndian(*end_addr);;

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

        u32  SwapEndianData(ByamlDataType data_type, u32 data, ByamlBigDataCache *big_data_cache);
        void SwapEndianForBigData(ByamlDataType data_type, u32 data, ByamlBigDataCache *big_data_cache);

        void SwapEndianHashArray(ResByamlContainer *container, ByamlBigDataCache *big_data_cache);
        void SwapEndianDictionary(ResByamlContainer *container, ByamlBigDataCache *big_data_cache);
        void SwapEndianArray(ResByamlContainer *container, ByamlBigDataCache *big_data_cache);

        void SwapEndianDataContainer(ResByamlContainer *container, ByamlBigDataCache *big_data_cache);

        void SwapRootContainerEndian(u32 table_offset, ByamlBigDataCache *big_data_cache);
        void SwapStringTableEndian(u32 table_offset, ByamlBigDataCache *big_data_cache);
        void SwapKeyTableEndian(u32 table_offset, ByamlBigDataCache *big_data_cache);

        void SwapByamlEndian(vp::imem::IHeap *heap);
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

        static constexpr u32 cInvalidKeyIndex = 0xff'ffff;

        constexpr ALWAYS_INLINE void SetPair(const ResByamlDictionaryPair *pair, u32 is_reverse_endian) {
            data_type = pair->data_type;
            key_index = (is_reverse_endian == false) ? pair->key_index : vp::util::SwapEndian24(pair->key_index);
            u32_value = (is_reverse_endian == false) ? pair->u32_value : vp::util::SwapEndian(pair->u32_value);
        }

        constexpr ALWAYS_INLINE bool IsKeyIndexValid() const {
            return (key_index != cInvalidKeyIndex);
        }

        ALWAYS_INLINE const void *GetBigData(const ResByaml *byaml) const {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(byaml) + u32_value);
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
                if (static_cast<ByamlDataType>(static_cast<u32>(data_type) & 0xe0) == ByamlDataType::HashArrayU32_1) {
                    return "HashArray";
                }
                if (static_cast<ByamlDataType>(static_cast<u32>(data_type) & 0xf0) == ByamlDataType::HashArrayWithRemapU32_1) {
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

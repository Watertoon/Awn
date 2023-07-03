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

    constexpr ALWAYS_INLINE bool IsContainerType(ByamlDataType data_type) {
        return (static_cast<u32>(data_type) & 0xe0);
    }

    struct ResByamlContainer {
        union {
            struct {
                u32 data_type : 8;
                u32 count     : 24;
            };
            u32 header;
        };

        constexpr void SwapEndian() {
            count = vp::util::SwapEndian(static_cast<u32>(count)) & 0xff'ffff;
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

            /* Endianess check */
            const bool is_reverse_endian = this->IsReverseEndian();

            /* Header checks */
            const u32 r_magic = (is_reverse_endian == false) ? magic : vp::util::SwapEndian(magic);
            const u32 r_version = (is_reverse_endian == false) ? version : vp::util::SwapEndian(version);
            if (r_magic != cMagic || r_version < cMinVersion || cMaxVersion < r_version) { return false; }

            /* String table checks */
            const u32 r_key_table_offset = (is_reverse_endian == false) ? key_table_offset : vp::util::SwapEndian(key_table_offset);
            const u32 r_string_table_offset = (is_reverse_endian == false) ? string_table_offset : vp::util::SwapEndian(string_table_offset);
            if (r_key_table_offset != 0 && VerifyStringTable(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(this) + r_key_table_offset), is_reverse_endian) == false)       { return false; }
            if (r_string_table_offset != 0 && VerifyStringTable(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(this) + r_string_table_offset), is_reverse_endian) == false) { return false; }

            /* Table offset checks */
            const ResByamlContainer *string_table = reinterpret_cast<ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + r_string_table_offset);
            const u32 r_string_count = (is_reverse_endian == false) ? string_table->count : vp::util::SwapEndian(string_table->count);
            const u32 final_string_offset = (r_string_table_offset != 0) ? *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(string_table) + sizeof(u32) * (r_string_count + 1)) : 0;

            const ResByamlContainer *key_table = reinterpret_cast<ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + r_key_table_offset);
            const u32 r_key_count = (is_reverse_endian == false) ? key_table->count : vp::util::SwapEndian(key_table->count);
            const u32 final_key_offset = (r_key_table_offset != 0) ? *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(key_table) + sizeof(u32) * (r_key_count + 1)) : 0;

            if ((r_string_table_offset | r_key_table_offset) != 0 && data_offset == 0) { return false; }
            if (r_key_table_offset != 0) {
                if (r_string_table_offset != 0 && r_string_table_offset < final_key_offset) { return false; }
                if (data_offset != 0 && data_offset < final_key_offset) { return false; }
            }

            return (final_string_offset <= data_offset) | (r_string_table_offset == 0) | (data_offset == 0);
        }

        constexpr bool IsReverseEndian() const {
            return magic == vp::util::SwapEndian(cMagic);
        }

        static ALWAYS_INLINE ResByaml *ResCast(void *file) {
            if (file == nullptr || reinterpret_cast<ResByaml*>(file)->IsValid() == false) { return nullptr; }
            if (reinterpret_cast<ResByaml*>(file)->IsReverseEndian() == true) { reinterpret_cast<ResByaml*>(file)->SwapByamlEndian(); }
            return reinterpret_cast<ResByaml*>(file);
        }

        static bool VerifyStringTable(const unsigned char *table, const bool is_reverse_endian) {

            /* Cast table and check container type */
            const ResByamlContainer *header = reinterpret_cast<const ResByamlContainer*>(table);
            if (static_cast<ByamlDataType>(header->data_type) != ByamlDataType::KeyTable) { return false; }

            const u32 r_header = (is_reverse_endian == false) ? header->header : vp::util::SwapEndian(header->header);
            const u32 string_count = r_header >> 0x8;

            /* Verify we have strings */
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

        u32 SwapEndianData(ByamlDataType data_type, u32 data) {

            /* Swap data by type */
            switch(data_type) {
                case ByamlDataType::Bool:
                case ByamlDataType::S32:
                case ByamlDataType::F32:
                case ByamlDataType::U32:
                case ByamlDataType::StringIndex:
                {
                    return vp::util::SwapEndian(data);
                }
                case ByamlDataType::HashArrayU32_1:
                case ByamlDataType::HashArrayU32_2:
                case ByamlDataType::HashArrayU32_3:
                case ByamlDataType::HashArrayU32_4:
                case ByamlDataType::HashArrayU32_5:
                case ByamlDataType::HashArrayU32_6:
                case ByamlDataType::HashArrayU32_7:
                case ByamlDataType::HashArrayU32_8:
                case ByamlDataType::HashArrayU32_9:
                case ByamlDataType::HashArrayU32_10:
                case ByamlDataType::HashArrayU32_11:
                case ByamlDataType::HashArrayU32_12:
                case ByamlDataType::HashArrayU32_13:
                case ByamlDataType::HashArrayU32_14:
                case ByamlDataType::HashArrayU32_15:
                case ByamlDataType::HashArrayU32_16:
                case ByamlDataType::HashArrayWithRemapU32_1:
                case ByamlDataType::HashArrayWithRemapU32_2:
                case ByamlDataType::HashArrayWithRemapU32_3:
                case ByamlDataType::HashArrayWithRemapU32_4:
                case ByamlDataType::HashArrayWithRemapU32_5:
                case ByamlDataType::HashArrayWithRemapU32_6:
                case ByamlDataType::HashArrayWithRemapU32_7:
                case ByamlDataType::HashArrayWithRemapU32_8:
                case ByamlDataType::HashArrayWithRemapU32_9:
                case ByamlDataType::HashArrayWithRemapU32_10:
                case ByamlDataType::HashArrayWithRemapU32_11:
                case ByamlDataType::HashArrayWithRemapU32_12:
                case ByamlDataType::HashArrayWithRemapU32_13:
                case ByamlDataType::HashArrayWithRemapU32_14:
                case ByamlDataType::HashArrayWithRemapU32_15:
                case ByamlDataType::HashArrayWithRemapU32_16:
                case ByamlDataType::Dictionary:
                case ByamlDataType::DictionaryWithRemap:
                case ByamlDataType::Array:
                case ByamlDataType::MonoTypedArray:
                case ByamlDataType::S64:
                case ByamlDataType::F64:
                case ByamlDataType::U64:
                {
                    const u32 swapped_data = vp::util::SwapEndian(data);
                    SwapEndianForBigData(data_type, swapped_data);
                    return swapped_data;
                }
                default:
                    break;
            }

            return 0;
        }

        void SwapEndianForBigData(ByamlDataType data_type, u32 data) {

            void *big_data = this->GetBigDataOffset(data);
            switch (data_type) {
                case ByamlDataType::HashArrayU32_1:
                case ByamlDataType::HashArrayU32_2:
                case ByamlDataType::HashArrayU32_3:
                case ByamlDataType::HashArrayU32_4:
                case ByamlDataType::HashArrayU32_5:
                case ByamlDataType::HashArrayU32_6:
                case ByamlDataType::HashArrayU32_7:
                case ByamlDataType::HashArrayU32_8:
                case ByamlDataType::HashArrayU32_9:
                case ByamlDataType::HashArrayU32_10:
                case ByamlDataType::HashArrayU32_11:
                case ByamlDataType::HashArrayU32_12:
                case ByamlDataType::HashArrayU32_13:
                case ByamlDataType::HashArrayU32_14:
                case ByamlDataType::HashArrayU32_15:
                case ByamlDataType::HashArrayU32_16:
                case ByamlDataType::HashArrayWithRemapU32_1:
                case ByamlDataType::HashArrayWithRemapU32_2:
                case ByamlDataType::HashArrayWithRemapU32_3:
                case ByamlDataType::HashArrayWithRemapU32_4:
                case ByamlDataType::HashArrayWithRemapU32_5:
                case ByamlDataType::HashArrayWithRemapU32_6:
                case ByamlDataType::HashArrayWithRemapU32_7:
                case ByamlDataType::HashArrayWithRemapU32_8:
                case ByamlDataType::HashArrayWithRemapU32_9:
                case ByamlDataType::HashArrayWithRemapU32_10:
                case ByamlDataType::HashArrayWithRemapU32_11:
                case ByamlDataType::HashArrayWithRemapU32_12:
                case ByamlDataType::HashArrayWithRemapU32_13:
                case ByamlDataType::HashArrayWithRemapU32_14:
                case ByamlDataType::HashArrayWithRemapU32_15:
                case ByamlDataType::HashArrayWithRemapU32_16:
                case ByamlDataType::Dictionary:
                case ByamlDataType::DictionaryWithRemap:
                case ByamlDataType::Array:
                case ByamlDataType::MonoTypedArray:
                {
                    ResByamlContainer *container = reinterpret_cast<ResByamlContainer*>(big_data);
                    this->SwapEndianDataContainer(container);
                    break;
                }
                {
                case ByamlDataType::BinaryDataWithAlignment:
                    *(reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(big_data) + sizeof(u32))) = vp::util::SwapEndian(*(reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(big_data) + sizeof(u32))));
                case ByamlDataType::BinaryData:
                    *reinterpret_cast<u32*>(big_data) = vp::util::SwapEndian(*reinterpret_cast<u32*>(big_data));
                    break;
                }
                case ByamlDataType::S64:
                case ByamlDataType::F64:
                case ByamlDataType::U64:
                {
                    *reinterpret_cast<u64*>(big_data) = vp::util::SwapEndian(*reinterpret_cast<u64*>(big_data));
                    break;
                }
                default:
                    break;
            }
            return;
        }

        void SwapEndianHashArray(ResByamlContainer *container) {

            /* Larger than 64-bit keys are currently unsupported */
            const ByamlDataType container_type = static_cast<ByamlDataType>(container->data_type);
            if (static_cast<u32>(ByamlDataType::HashArrayU32_2) < (static_cast<u32>(container_type) & 0x2f)) { return; }

            /* Swap table */
            const u32 hash_size   = sizeof(u32) * ((static_cast<u32>(container_type) & 0xf) + 1);
            const u32 stride      = hash_size + sizeof(u32);
            void *content_base    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer));
            u8   *data_type_array = reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + stride * container->count);
            u32  *remap_array     = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + stride * container->count + vp::util::AlignUp(sizeof(u8) * container->count, alignof(u32)));
            for (u32 i = 0; i < container->count; ++i) {

                /* Swap hash */
                void *hash_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(content_base) + stride * i);
                if (hash_size == sizeof(u32)) {
                    *reinterpret_cast<u32*>(hash_offset) = vp::util::SwapEndian(*reinterpret_cast<u32*>(hash_offset));
                } else if (hash_size == sizeof(u64)) {
                    *reinterpret_cast<u64*>(hash_offset) = vp::util::SwapEndian(*reinterpret_cast<u64*>(hash_offset));
                }

                /* Swap data */
                u32 *data_offset = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(hash_offset) + hash_size);
                *data_offset     = this->SwapEndianData(static_cast<ByamlDataType>(data_type_array[i]), *data_offset);

                /* Swap index */
                if ((static_cast<u32>(container_type) & 0x10) != 0) {
                    remap_array[i] = vp::util::SwapEndian(remap_array[i]);
                }
            }

            return;
        }
        void SwapEndianDictionary(ResByamlContainer *container) {

            /* Swap table */
            const ByamlDataType     container_type = static_cast<ByamlDataType>(container->data_type);
            ResByamlDictionaryPair *pair_array     = reinterpret_cast<ResByamlDictionaryPair*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer));
            u32                    *remap_array    = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + sizeof(ResByamlDictionaryPair) * container->count);
            for (u32 i = 0; i < container->count; ++i) {

                /* Swap key index */
                pair_array[i].key_index = vp::util::SwapEndian(pair_array[i].header) & 0xff'ffff;

                /* Swap data */
                pair_array[i].u32_value = this->SwapEndianData(static_cast<ByamlDataType>(pair_array[i].data_type), pair_array[i].u32_value);

                /* Swap index */
                if (container_type == ByamlDataType::DictionaryWithRemap) {
                    remap_array[i] = vp::util::SwapEndian(remap_array[i]);
                }
            }

            return;
        }

        void SwapEndianArray(ResByamlContainer *container) {

            /* Swap table */
            const ByamlDataType container_type = static_cast<ByamlDataType>(container->data_type);
            u8  *data_type_array               = reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer));
            u32 *data_array                    = (container_type == ByamlDataType::Array) ? reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + vp::util::AlignUp(sizeof(u8) * container->count, alignof(u32))) : reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + vp::util::AlignUp(sizeof(u8), alignof(u32)));
            for (u32 i = 0; i < container->count; ++i) {

                /* Read data type */
                const ByamlDataType data_type = (container_type == ByamlDataType::Array) ? static_cast<ByamlDataType>(data_type_array[i]) : static_cast<ByamlDataType>(*data_type_array);

                /* Swap data */
                data_array[i] = this->SwapEndianData(data_type, data_array[i]);
            }

            return;
        }

        void SwapEndianDataContainer(ResByamlContainer *container) {

            /* Swap header endian */
            container->SwapEndian();

            /* Swap container contents  */
            switch (static_cast<ByamlDataType>(container->data_type)) {
                case ByamlDataType::HashArrayU32_1:
                case ByamlDataType::HashArrayU32_2:
                case ByamlDataType::HashArrayU32_3:
                case ByamlDataType::HashArrayU32_4:
                case ByamlDataType::HashArrayU32_5:
                case ByamlDataType::HashArrayU32_6:
                case ByamlDataType::HashArrayU32_7:
                case ByamlDataType::HashArrayU32_8:
                case ByamlDataType::HashArrayU32_9:
                case ByamlDataType::HashArrayU32_10:
                case ByamlDataType::HashArrayU32_11:
                case ByamlDataType::HashArrayU32_12:
                case ByamlDataType::HashArrayU32_13:
                case ByamlDataType::HashArrayU32_14:
                case ByamlDataType::HashArrayU32_15:
                case ByamlDataType::HashArrayU32_16:
                case ByamlDataType::HashArrayWithRemapU32_1:
                case ByamlDataType::HashArrayWithRemapU32_2:
                case ByamlDataType::HashArrayWithRemapU32_3:
                case ByamlDataType::HashArrayWithRemapU32_4:
                case ByamlDataType::HashArrayWithRemapU32_5:
                case ByamlDataType::HashArrayWithRemapU32_6:
                case ByamlDataType::HashArrayWithRemapU32_7:
                case ByamlDataType::HashArrayWithRemapU32_8:
                case ByamlDataType::HashArrayWithRemapU32_9:
                case ByamlDataType::HashArrayWithRemapU32_10:
                case ByamlDataType::HashArrayWithRemapU32_11:
                case ByamlDataType::HashArrayWithRemapU32_12:
                case ByamlDataType::HashArrayWithRemapU32_13:
                case ByamlDataType::HashArrayWithRemapU32_14:
                case ByamlDataType::HashArrayWithRemapU32_15:
                case ByamlDataType::HashArrayWithRemapU32_16:
                {
                    this->SwapEndianHashArray(container);
                    break;
                }
                case ByamlDataType::Dictionary:
                case ByamlDataType::DictionaryWithRemap:
                {
                    this->SwapEndianDictionary(container);
                    break;
                }
                case ByamlDataType::Array:
                case ByamlDataType::MonoTypedArray:
                {
                    this->SwapEndianArray(container);
                    break;
                }
                default:
                    break;
            }

            return;
        }

        void SwapRootContainerEndian() {

            /* Null table check */
            if (data_offset == 0) { return; }

            /* Get root dictionary */
            ResByamlContainer *root_dic = reinterpret_cast<ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + data_offset);
            if (IsContainerType(static_cast<ByamlDataType>(root_dic->data_type)) == false) { return; }

            /* Swap endian */
            this->SwapEndianDataContainer(root_dic);

            return;
        }

        void SwapStringTableEndian() {

            /* Null table check */
            if (string_table_offset == 0) { return; }

            /* Get table */
            ResByamlContainer *string_table = reinterpret_cast<ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + string_table_offset);
            if (static_cast<ByamlDataType>(string_table->data_type) != ByamlDataType::KeyTable) { return; }

            /* Swap endian */
            string_table->SwapEndian();
            const u32 index_count = string_table->count + 1;
            u32 *index_array = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(string_table) + sizeof(ResByamlContainer));
            for (u32 i = 0; i < index_count; ++i) {
                index_array[i] = vp::util::SwapEndian(index_array[i]);
            }

            return;
        }

        void SwapKeyTableEndian() {

            /* Null table check */
            if (key_table_offset == 0) { return; }

           /* Get table */
            ResByamlContainer *key_table = reinterpret_cast<ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + key_table_offset);
            if (static_cast<ByamlDataType>(key_table->data_type) != ByamlDataType::KeyTable) { return; }

            /* Swap endian */
            key_table->SwapEndian();
            const u32 index_count = key_table->count + 1;
            u32 *index_array = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(key_table) + sizeof(ResByamlContainer));
            for (u32 i = 0; i < index_count; ++i) {
                index_array[i] = vp::util::SwapEndian(index_array[i]);
            }

            return;
        }

        void SwapByamlEndian() {

            /* Swap header */
            magic               = vp::util::SwapEndian(magic);
            version             = vp::util::SwapEndian(version);
            key_table_offset    = vp::util::SwapEndian(key_table_offset);
            string_table_offset = vp::util::SwapEndian(string_table_offset);
            data_offset         = vp::util::SwapEndian(data_offset);
            
            /* Swap table endians */
            this->SwapKeyTableEndian();
            this->SwapStringTableEndian();
            this->SwapRootContainerEndian();

            return;
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

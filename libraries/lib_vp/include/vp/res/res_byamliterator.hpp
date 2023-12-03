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

    class ByamlIterator {
        private:
            const ResByaml *m_byaml;
            ByamlData       m_container_data;
        private:
            template <typename T>
            ALWAYS_INLINE bool TryGetValueByKeyImpl(T *out_value, const char *key) const {

                /* Get byaml data */
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                return this->TryGetValueByDataImpl(out_value, data);
            }

            template <typename T>
            ALWAYS_INLINE bool TryGetValueByIndexImpl(T *out_value, u32 index) const {

                /* Get byaml data */
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                return this->TryGetValueByDataImpl(out_value, data);
            }

            template <typename T>
            ALWAYS_INLINE bool TryGetValueByHashImpl(T *out_value, u32 hash) const {

                /* Get byaml data */
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetValueByDataImpl(out_value, data);
            }

            template <typename T>
            ALWAYS_INLINE bool TryGetValueByHashImpl(T *out_value, u64 hash) const {

                /* Get byaml data */
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetValueByDataImpl(out_value, data);
            }

            template <typename T>
            ALWAYS_INLINE bool TryGetValueByDataImpl(T *out_value, ByamlData data) const {

                /* Ensure matching data type */
                const     ByamlDataType data_type        = static_cast<ByamlDataType>(data.data_type);
                constexpr ByamlDataType target_data_type = TGetByamlDataTypeValue<T>();
                if (data_type != target_data_type) { return false; }

                /* Write out value */
                const u32 data_value = data.u32_value;
                if constexpr (IsBigDataType(target_data_type) == true) {
                    *out_value = std::bit_cast<T>(*reinterpret_cast<T*>(m_byaml->GetBigDataOffset(data_value)));
                } else if constexpr (sizeof(T) == sizeof(u32)) {
                    *out_value = std::bit_cast<T>(data_value);
                } else {
                    *out_value = data_value;
                }

                return true;
            }
        public:
            constexpr ByamlIterator() : m_byaml(nullptr), m_container_data{} {/*...*/}
            ALWAYS_INLINE ByamlIterator(const unsigned char *byaml_file) : m_byaml(reinterpret_cast<const ResByaml*>(byaml_file)), m_container_data{} {

                /* Verify valid byaml file */
                if (m_byaml->IsValid() == false) {
                    m_byaml = nullptr;
                    return;
                }

                /* Check for data container */
                if (m_byaml->data_offset == 0) { return; }

                /* Get data offset */
                const bool is_reverse_endian = m_byaml->IsReverseEndian();
                const u32 data_offset        = (is_reverse_endian == false) ? m_byaml->data_offset : vp::util::SwapEndian(m_byaml->data_offset);

                /* Get data type */
                const u8 data_type = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(byaml_file) + data_offset)->data_type;

                /* Check is single data */
                const bool is_single_data = (IsContainerType(static_cast<ByamlDataType>(data_type)) == false && data_type != static_cast<u8>(ByamlDataType::Null));

                /* Set data container */
                m_container_data.u32_value = (is_single_data == true) ? data_offset : (is_reverse_endian == false) ? *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data_offset + sizeof(u32)) : vp::util::SwapEndian(*reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data_offset + sizeof(u32)));
                m_container_data.data_type = data_type;

                return;
            }
            ALWAYS_INLINE ByamlIterator(const void *byaml_file) : m_byaml(reinterpret_cast<const ResByaml*>(byaml_file)), m_container_data{} {

                /* Verify valid byaml file */
                if (m_byaml->IsValid() == false) {
                    m_byaml = nullptr;
                    return;
                }

                /* Check for data container */
                if (m_byaml->data_offset == 0) { return; }

                /* Get data offset */
                const bool is_reverse_endian = m_byaml->IsReverseEndian();
                const u32 data_offset        = (is_reverse_endian == false) ? m_byaml->data_offset : vp::util::SwapEndian(m_byaml->data_offset);

                /* Get data type */
                const u8 data_type = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(byaml_file) + data_offset)->data_type;

                /* Check is single data */
                const bool is_single_data = (IsContainerType(static_cast<ByamlDataType>(data_type)) == false && static_cast<ByamlDataType>(data_type) != ByamlDataType::Null);

                /* Set data container */
                m_container_data.u32_value = (is_single_data == false) ? data_offset : (is_reverse_endian == false) ? *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data_offset + sizeof(u32)) : vp::util::SwapEndian(*reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data_offset + sizeof(u32)));
                m_container_data.data_type = data_type;
    
                return;
            }

            constexpr ALWAYS_INLINE ByamlIterator(const ByamlIterator& rhs) : m_byaml(rhs.m_byaml), m_container_data(rhs.m_container_data) {/*...*/}

            constexpr ~ByamlIterator() {/*...*/}

            constexpr ALWAYS_INLINE ByamlIterator &operator=(const ByamlIterator &rhs) {
                m_byaml          = rhs.m_byaml;
                m_container_data = rhs.m_container_data;
                return *this;
            }

            constexpr ALWAYS_INLINE bool IsValid()          const { return m_byaml != nullptr; }
            constexpr ALWAYS_INLINE bool IsContainerValid() const { return (IsContainerType(static_cast<ByamlDataType>(m_container_data.data_type)) == true) & (m_container_data.u32_value != 0); }

            constexpr ALWAYS_INLINE ByamlData GetData() const { return m_container_data; }

            bool TryGetKeyByData(const char **out_key, ByamlData data) const {

                /* Endianess check */
                const u32 is_reverse_endian = m_byaml->IsReverseEndian();

                /* Check key index is valid */
                if (data.IsKeyIndexValid() == false) { return false; }

                /* Get string table iter  */
                const u32                       key_table_offset = (is_reverse_endian == false) ? m_byaml->key_table_offset : vp::util::SwapEndian(m_byaml->key_table_offset);
                const ResByamlContainer        *key_container    = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + key_table_offset);
                const ByamlStringTableIterator  table_iterator(key_container);

                /* Find offset of key table */
                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(key_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 *string_table_offset = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(key_container) +  sizeof(ResByamlContainer));
                    const u32  offset_offset       = (is_reverse_endian == false) ? *string_table_offset : vp::util::SwapEndian(*string_table_offset);
                    const u64 *key_pool_data       = reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(key_container) + offset_offset);
                    key_pool_offset                = (is_reverse_endian == false) ? *key_pool_data : vp::util::SwapEndian(*key_pool_data);
                }

                /* Get key by key index */
                *out_key = table_iterator.GetStringByIndex(data.key_index, key_pool_offset, is_reverse_endian);

                return true;
            }

            bool TryGetKeyIndexByKey(u32 *out_key_index, const char *key) const {

                /* Endianess check */
                const u32 is_reverse_endian = m_byaml->IsReverseEndian();

                /* Get key index */
                const u32 key_table_offset = (is_reverse_endian == false) ? m_byaml->key_table_offset : vp::util::SwapEndian(m_byaml->key_table_offset);
                ByamlStringTableIterator table_iterator(reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) +  key_table_offset));
                const u32 index = table_iterator.TryFindKeyIndexByKey(key, is_reverse_endian);
                if (index == ByamlStringTableIterator::cInvalidKeyIndex) { return false; }

                *out_key_index = index;

                return true;
            }

            bool TryRemapIndexByKey(u32 *out_container_index, const char *key) const {

                /* Integrity checks */
                if (m_byaml == nullptr || m_byaml->key_table_offset == 0 || (static_cast<ByamlDataType>(m_container_data.data_type) != ByamlDataType::Dictionary && static_cast<ByamlDataType>(m_container_data.data_type) != ByamlDataType::DictionaryWithRemap)) { return false; }

                /* Endianess check */
                const u32 is_reverse_endian = m_byaml->IsReverseEndian();

                /* Setup container and iterators */
                const u32 r_key_table_offset = (is_reverse_endian == false) ? m_byaml->key_table_offset : vp::util::SwapEndian(m_byaml->key_table_offset);
                const ResByamlContainer        *key_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + r_key_table_offset);
                const ByamlStringTableIterator  table_iterator(key_container);

                const ResByamlContainer        *dic_container = reinterpret_cast<const ResByamlContainer*>(m_container_data.GetBigData(m_byaml));
                const ByamlDictionaryIterator   dic_iter(dic_container);

                /* Handle key table relocation */
                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(key_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 *string_table_offset = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(key_container) +  sizeof(ResByamlContainer));
                    const u32  offset_offset       = (is_reverse_endian == false) ? *string_table_offset : vp::util::SwapEndian(*string_table_offset);
                    const u64 *key_pool_data       = reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(key_container) + offset_offset);
                    key_pool_offset                = (is_reverse_endian == false) ? *key_pool_data : vp::util::SwapEndian(*key_pool_data);
                }

                /* Binary search pattern as the string table is always sorted */
                u32 size = dic_iter.GetCount(is_reverse_endian);
                u32 i = 0;
                u32 index = 0;
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    const ResByamlDictionaryPair *res_pair = dic_iter.GetDictionaryPairByIndex(index, is_reverse_endian);
                    const u32 r_key_index = (is_reverse_endian == false) ? res_pair->key_index : vp::util::SwapEndian24(res_pair->key_index);
                    const char *pair_key = table_iterator.GetStringByIndex(r_key_index, key_pool_offset, is_reverse_endian);
                    const s32 result = ::strcmp(key, pair_key);
                    
                    if (result == 0) {
                        index = dic_iter.TryRemapIndex(index, is_reverse_endian);
                        *out_container_index = index;
                        return true;
                    }
                    if (0 < result) {
                        i = index + 1;
                        index = size;
                    }
                    size = index;
                }

                return false;
            }

            bool TryGetByamlDataByKey(ByamlData *out_byaml_data, const char *key) const {

                /* Integrity checks */
                if (m_byaml == nullptr || m_byaml->key_table_offset == 0 || (static_cast<ByamlDataType>(m_container_data.data_type) != ByamlDataType::Dictionary && static_cast<ByamlDataType>(m_container_data.data_type) != ByamlDataType::DictionaryWithRemap)) { return false; }

                /* Endianess check */
                const u32 is_reverse_endian = m_byaml->IsReverseEndian();

                /* Setup container and iterators */
                const u32 r_key_table_offset = (is_reverse_endian == false) ? m_byaml->key_table_offset : vp::util::SwapEndian(m_byaml->key_table_offset);
                const ResByamlContainer        *key_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + r_key_table_offset);
                const ByamlStringTableIterator  table_iterator(key_container);

                const ResByamlContainer        *dic_container = reinterpret_cast<const ResByamlContainer*>(m_container_data.GetBigData(m_byaml));
                const ByamlDictionaryIterator   dic_iter(dic_container);

                /* Handle key table relocation */
                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(key_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 *string_table_offset = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(key_container) +  sizeof(ResByamlContainer));
                    const u32  offset_offset       = (is_reverse_endian == false) ? *string_table_offset : vp::util::SwapEndian(*string_table_offset);
                    const u64 *key_pool_data       = reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(key_container) + offset_offset);
                    key_pool_offset                = (is_reverse_endian == false) ? *key_pool_data : vp::util::SwapEndian(*key_pool_data);
                }

                /* Binary search pattern as the string table is always sorted */
                u32 size = dic_iter.GetCount(is_reverse_endian);
                u32 i = 0;
                u32 index = 0;
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    const ResByamlDictionaryPair *res_pair = dic_iter.GetDictionaryPairByIndex(index, is_reverse_endian);
                    const u32 r_key_index = (is_reverse_endian == false) ? res_pair->key_index : vp::util::SwapEndian24(res_pair->key_index);
                    const char *pair_key = table_iterator.GetStringByIndex(r_key_index, key_pool_offset, is_reverse_endian);
                    const s32 result = ::strcmp(key, pair_key);
                    
                    if (result == 0) {
                        out_byaml_data->SetPair(res_pair, is_reverse_endian);
                        return true;
                    }
                    if (0 < result) {
                        i = index + 1;
                        index = size;
                    }
                    size = index;
                }

                /* Clear data on failure */
                *out_byaml_data = ByamlData{ .key_index = ByamlData::cInvalidKeyIndex };

                return false;
            }

            bool TryGetByamlDataByHash(ByamlData *out_byaml_data, u32 hash) const {

                /* Integrity checks */
                if (m_byaml == nullptr || m_byaml->key_table_offset == 0 || (static_cast<ByamlDataType>(m_container_data.data_type & 0xe0) != ByamlDataType::HashArrayU32_1)) { return false; }

                /* Endianess check */
                const u32 is_reverse_endian = m_byaml->IsReverseEndian();

                /* Setup container and iterators */
                const u32 stride = (m_container_data.data_type & 0xf) * sizeof(u32) + sizeof(ResByamlContainer);
                const ResByamlContainer      *hash_container = reinterpret_cast<const ResByamlContainer*>(m_container_data.GetBigData(m_byaml));
                const ByamlHashArrayIterator  hash_array_iter(hash_container, stride);

                /* Binary search pattern as the string table is always sorted */
                u32 size = hash_array_iter.GetCount(is_reverse_endian);
                u32 i = 0;
                u32 index = 0;
                const u32 data_type_offset = (stride + sizeof(u32)) * size;
                const u8 *data_type_array  = reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(hash_container) + data_type_offset);
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    
                    const ByamlHashAccessor hash_accessor(hash_container, stride, index);
                    const u32 cur_hash = hash_accessor.GetHash(is_reverse_endian);
                    
                    if (hash == cur_hash) {
                        out_byaml_data->data_type = data_type_array[index];
                        out_byaml_data->key_index = index;
                        out_byaml_data->u32_value = hash_accessor.GetValue(is_reverse_endian);
                        return true;
                    }
                    if (cur_hash < hash) {
                        i = index + 1;
                        index = size;
                    }
                    size = index;
                }

                /* Clear byaml data on failure */
                *out_byaml_data = ByamlData{ .key_index = ByamlData::cInvalidKeyIndex };

                return false;
            }
            bool TryGetByamlDataByHash(ByamlData *out_byaml_data, u64 hash) const {

                /* Integrity checks */
                if (m_byaml == nullptr || m_byaml->key_table_offset == 0 || (static_cast<ByamlDataType>(m_container_data.data_type & 0xe0) != ByamlDataType::HashArrayU32_1)) { return false; }

                /* Endianess check */
                const u32 is_reverse_endian = m_byaml->IsReverseEndian();

                /* Setup container and iterators */
                const u32 stride = (m_container_data.data_type & 0xf) * sizeof(u32) + sizeof(ResByamlContainer);
                const ResByamlContainer      *hash_container = reinterpret_cast<const ResByamlContainer*>(m_container_data.GetBigData(m_byaml));
                const ByamlHashArrayIterator  hash_array_iter(hash_container, stride);

                /* Binary search pattern as the string table is always sorted */
                u32 size = hash_array_iter.GetCount(is_reverse_endian);
                u32 i = 0;
                u32 index = 0;
                const u32 data_type_offset = (stride + sizeof(u32)) * size;
                const u8 *data_type_array  = reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(hash_container) + data_type_offset);
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    
                    const ByamlHashAccessor hash_accessor(hash_container, stride, index);
                    const u64 cur_hash = hash_accessor.GetHash(is_reverse_endian);
                    
                    if (hash == cur_hash) {
                        out_byaml_data->data_type = data_type_array[index];
                        out_byaml_data->key_index = index;
                        out_byaml_data->u32_value = hash_accessor.GetValue(is_reverse_endian);
                        return true;
                    }
                    if (cur_hash < hash) {
                        i = index + 1;
                        index = size;
                    }
                    size = index;
                }

                *out_byaml_data = ByamlData{ .key_index = ByamlData::cInvalidKeyIndex };

                return false;
            }

            bool TryGetByamlDataByIndex(ByamlData *out_byaml_data, u32 index) const {

                /* Endianess check */
                const u32 is_reverse_endian = m_byaml->IsReverseEndian();

                /* Find data using respective container iter */
                u32         data_type = static_cast<u32>(m_container_data.data_type);
                const void *container = m_container_data.GetBigData(m_byaml);
                if (static_cast<ByamlDataType>(data_type) == ByamlDataType::Dictionary || static_cast<ByamlDataType>(data_type) == ByamlDataType::DictionaryWithRemap) {
                    const ByamlDictionaryIterator dic_iter(reinterpret_cast<const ResByamlContainer*>(container));
                    return dic_iter.TryGetDataByIndex(out_byaml_data, index, is_reverse_endian);
                }
                if (static_cast<ByamlDataType>(data_type) == ByamlDataType::Array || static_cast<ByamlDataType>(data_type) == ByamlDataType::MonoTypedArray) {
                    const ByamlArrayIterator array_iter(reinterpret_cast<const ResByamlContainer*>(container));
                    return array_iter.TryGetDataByIndex(out_byaml_data, index, is_reverse_endian);
                }
                if (static_cast<ByamlDataType>(data_type & 0xe0) == ByamlDataType::HashArrayU32_1) {
                    const ByamlHashArrayIterator hash_array_iter(reinterpret_cast<const ResByamlContainer*>(container));
                    return hash_array_iter.TryGetDataByIndex(out_byaml_data, index, is_reverse_endian);
                }

                /* Clear byaml data on failure */
                *out_byaml_data = ByamlData{ .key_index = ByamlData::cInvalidKeyIndex };

                return false;
            }

            ALWAYS_INLINE u32 GetDataCount() const {
                const ResByamlContainer *container = reinterpret_cast<const ResByamlContainer*>(m_container_data.GetBigData(m_byaml));
                return (m_byaml->IsReverseEndian() == false) ? container->count : vp::util::SwapEndian24(container->count);
            }

            constexpr ALWAYS_INLINE u32 GetDataType() const {
                return m_container_data.data_type;
            }

            bool TryGetBoolByKey(bool *out_bool, const char *key) const {
                return TryGetValueByKeyImpl(out_bool, key);
            }

            bool TryGetBoolByIndex(bool *out_bool, u32 index) const {
                return TryGetValueByIndexImpl(out_bool, index);
            }

            bool TryGetBoolByHash(bool *out_bool, u32 hash) const {
                return TryGetValueByHashImpl(out_bool, hash);
            }

            bool TryGetBoolByHash(bool *out_bool, u64 hash) const {
                return TryGetValueByHashImpl(out_bool, hash);
            }

            bool TryGetBoolByData(bool *out_bool, ByamlData data) const {
                return TryGetValueByDataImpl(out_bool, data);
            }

            bool TryGetS32ByKey(s32 *out_int, const char *key) const {
                return TryGetValueByKeyImpl(out_int, key);
            }

            bool TryGetS32ByIndex(s32 *out_int, u32 index) const {
                return TryGetValueByIndexImpl(out_int, index);
            }

            bool TryGetS32ByHash(s32 *out_int, u32 hash) const {
                return TryGetValueByHashImpl(out_int, hash);
            }

            bool TryGetS32ByHash(s32 *out_int, u64 hash) const {
                return TryGetValueByHashImpl(out_int, hash);
            }

            bool TryGetS32ByData(s32 *out_int, ByamlData data) const {
                return TryGetValueByDataImpl(out_int, data);
            }

            bool TryGetF32ByKey(float *out_float, const char *key) const {
                return TryGetValueByKeyImpl(out_float, key);
            }

            bool TryGetF32ByIndex(float *out_float, u32 index) const {
                return TryGetValueByIndexImpl(out_float, index);
            }

            bool TryGetF32ByHash(float *out_float, u32 hash) const {
                return TryGetValueByHashImpl(out_float, hash);
            }

            bool TryGetF32ByHash(float *out_float, u64 hash) const {
                return TryGetValueByHashImpl(out_float, hash);
            }

            bool TryGetF32ByData(float *out_float, ByamlData data) const {
                return TryGetValueByDataImpl(out_float, data);
            }

            bool TryGetU32ByKey(u32 *out_uint, const char *key) const {
                return TryGetValueByKeyImpl(out_uint, key);
            }

            bool TryGetU32ByIndex(u32 *out_uint, u32 index) const {
                return TryGetValueByIndexImpl(out_uint, index);
            }

            bool TryGetU32ByHash(u32 *out_uint, u32 hash) const {
                return TryGetValueByHashImpl(out_uint, hash);
            }

            bool TryGetU32ByHash(u32 *out_uint, u64 hash) const {
                return TryGetValueByHashImpl(out_uint, hash);
            }

            bool TryGetU32ByData(u32 *out_uint, ByamlData data) const {
                return TryGetValueByDataImpl(out_uint, data);
            }

            bool TryGetS64ByKey(s64 *out_longlong, const char *key) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                return this->TryGetS64ByData(out_longlong, data);
            }

            bool TryGetS64ByIndex(s64 *out_longlong, u32 index) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                return this->TryGetS64ByData(out_longlong, data);
            }

            bool TryGetS64ByHash(s64 *out_longlong, u32 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetS64ByData(out_longlong, data);
            }

            bool TryGetS64ByHash(s64 *out_longlong, u64 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetS64ByData(out_longlong, data);
            }

            bool TryGetS64ByData(s64 *out_longlong, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::S32) { 
                    *out_longlong = static_cast<s64>(data.s32_value);
                    return true;
                }
                if (data_type != ByamlDataType::S64) { return false; }

                const s64 *value = reinterpret_cast<s64*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_longlong = (m_byaml->IsReverseEndian() == false) ? *value : vp::util::SwapEndian(*value);

                return true;
            }

            bool TryGetU64ByKey(u64 *out_ulonglong, const char *key) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                return this->TryGetU64ByData(out_ulonglong, data);
            }

            bool TryGetU64ByIndex(u64 *out_ulonglong, u32 index) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                return this->TryGetU64ByData(out_ulonglong, data);
            }

            bool TryGetU64ByHash(u64 *out_ulonglong, u32 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetU64ByData(out_ulonglong, data);
            }

            bool TryGetU64ByHash(u64 *out_ulonglong, u64 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetU64ByData(out_ulonglong, data);
            }

            bool TryGetU64ByData(u64 *out_ulonglong, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::U32) { 
                    *out_ulonglong = static_cast<u64>(data.u32_value);
                    return true;
                }
                if (data_type != ByamlDataType::U64) { return false; }

                const u64 *value = reinterpret_cast<u64*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_ulonglong = (m_byaml->IsReverseEndian() == false) ? *value : vp::util::SwapEndian(*value);

                return true;
            }

            bool TryGetF64ByKey(double *out_double, const char *key) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                return this->TryGetF64ByData(out_double, data);
            }

            bool TryGetF64ByIndex(double *out_double, u32 index) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                return this->TryGetF64ByData(out_double, data);
            }

            bool TryGetF64ByHash(double *out_double, u32 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetF64ByData(out_double, data);
            }

            bool TryGetF64ByHash(double *out_double, u64 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetF64ByData(out_double, data);
            }

            bool TryGetF64ByData(double *out_double, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::F32) { 
                    *out_double = static_cast<double>(data.f32_value);
                    return true;
                }
                if (data_type != ByamlDataType::F64) { return false; }

                const double *value = reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_double = (m_byaml->IsReverseEndian() == false) ? *value : vp::util::SwapEndian(*value);

                return true;
            }

            bool TryGetBinaryDataByKey(void **out_binary, u32 *out_size, u32 *out_alignment, const char *key) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                return this->TryGetBinaryDataByData(out_binary, out_size, out_alignment, data);
            }

            bool TryGetBinaryDataByIndex(void **out_binary, u32 *out_size, u32 *out_alignment, u32 index) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                return this->TryGetBinaryDataByData(out_binary, out_size, out_alignment, data);
            }

            bool TryGetBinaryDataByHash(void **out_binary, u32 *out_size, u32 *out_alignment, u32 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetBinaryDataByData(out_binary, out_size, out_alignment, data);
            }

            bool TryGetBinaryDataByHash(void **out_binary, u32 *out_size, u32 *out_alignment, u64 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetBinaryDataByData(out_binary, out_size, out_alignment, data);
            }

            bool TryGetBinaryDataByData(void **out_binary, u32 *out_size, u32 *out_alignment, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::BinaryData) {
                    const u32 *size_data = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_size      = (m_byaml->IsReverseEndian() == false) ? *size_data : vp::util::SwapEndian(*size_data);
                    *out_alignment = 0;
                    *out_binary    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(size_data) + sizeof(u32));
                    return true;
                }
                if (data_type == ByamlDataType::BinaryDataWithAlignment) {
                    const u32 *size_data      = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    const u32 *alignment_data = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(size_data) + sizeof(u32));
                    *out_size      = (m_byaml->IsReverseEndian() == false) ? *size_data : vp::util::SwapEndian(*size_data);
                    *out_alignment = (m_byaml->IsReverseEndian() == false) ? *alignment_data : vp::util::SwapEndian(*alignment_data);
                    *out_binary    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(alignment_data) + sizeof(u32));
                    return true;
                }

                return false;
            }

            bool TryGetStringByKey(const char **out_string, const char *key) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                return this->TryGetStringByData(out_string, data);
            }

            bool TryGetStringByIndex(const char **out_string, u32 index) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                return this->TryGetStringByData(out_string, data);
            }

            bool TryGetStringByHash(const char **out_string, u32 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetStringByData(out_string, data);
            }

            bool TryGetStringByHash(const char **out_string, u64 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetStringByData(out_string, data);
            }

            bool TryGetStringByData(const char **out_string, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::StringIndex) { return false; }

                const u32 is_reverse_endian = m_byaml->IsReverseEndian();

                const u32 string_table_offset = (is_reverse_endian == false) ? m_byaml->string_table_offset : vp::util::SwapEndian(m_byaml->string_table_offset);
                const ResByamlContainer *key_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + string_table_offset);
                const ByamlStringTableIterator table_iter(key_container);

                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(key_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 *string_table_data = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(key_container) +  sizeof(ResByamlContainer));
                    const u32  offset_offset     = (is_reverse_endian == false) ? *string_table_data : vp::util::SwapEndian(*string_table_data);
                    const u64 *key_pool_data     = reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(key_container) + offset_offset);
                    key_pool_offset              = (is_reverse_endian == false) ? *key_pool_data : vp::util::SwapEndian(*key_pool_data);
                }

                *out_string = table_iter.GetStringByIndex(data.u32_value, key_pool_offset, is_reverse_endian);

                return true;
            }

            bool TryGetIteratorByKey(ByamlIterator *out_iterator, const char *key) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                return this->TryGetIteratorByData(out_iterator, data);
            }

            bool TryGetIteratorByIndex(ByamlIterator *out_iterator, u32 index) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                return this->TryGetIteratorByData(out_iterator, data);
            }

            bool TryGetIteratorByHash(ByamlIterator *out_iterator, u32 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetIteratorByData(out_iterator, data);
            }

            bool TryGetIteratorByHash(ByamlIterator *out_iterator, u64 hash) const {

                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                return this->TryGetIteratorByData(out_iterator, data);
            }

            bool TryGetIteratorByData(ByamlIterator *out_iterator, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                const ByamlDataType arr_type  = static_cast<ByamlDataType>(data.data_type & 0xf7);
                const ByamlDataType hash_type = static_cast<ByamlDataType>(data.data_type & 0xe0);
                if (arr_type == ByamlDataType::Array || data_type == ByamlDataType::Dictionary || data_type == ByamlDataType::DictionaryWithRemap || hash_type == ByamlDataType::HashArrayU32_1) {
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_container_data = data;
                    return true;
                }

                if (data_type == ByamlDataType::Null) { 
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_container_data = ByamlData{ .key_index = ByamlData::cInvalidKeyIndex };
                    return true;
                }

                out_iterator->m_byaml          = nullptr;
                out_iterator->m_container_data = ByamlData{ .key_index = ByamlData::cInvalidKeyIndex };

                return false;
            }

            template <typename T>
                requires (TGetByamlDataTypeValue<T>() != ByamlDataType::Null)
            bool TryGetValueByKey(T *out_value, const char *key) const {

                /* Lookup required function at compile time */
                const ByamlDataType data_type = TGetByamlDataTypeValue<T>();

                if constexpr (data_type == ByamlDataType::Bool) {
                    return this->TryGetBoolByKey(out_value, key);
                } else if constexpr (data_type == ByamlDataType::S32) {
                    return this->TryGetS32ByKey(out_value, key);
                } else if constexpr (data_type == ByamlDataType::F32) {
                    return this->TryGetF32ByKey(out_value, key);
                } else if constexpr (data_type == ByamlDataType::U32) {
                    return this->TryGetU32ByKey(out_value, key);
                } else if constexpr (data_type == ByamlDataType::S64) {
                    return this->TryGetS64ByKey(out_value, key);
                } else if constexpr (data_type == ByamlDataType::U64) {
                    return this->TryGetU64ByKey(out_value, key);
                } else if constexpr (data_type == ByamlDataType::F64) {
                    return this->TryGetF64ByKey(out_value, key);
                } else if constexpr (data_type == ByamlDataType::StringIndex) {
                    return this->TryGetStringByKey(out_value, key);
                }
            }
    
            template <typename T>
                requires (TGetByamlDataTypeValue<T>() != ByamlDataType::Null)
            bool TryGetValueByIndex(T *out_value, u32 index) const {

                /* Lookup required function at compile time */
                const ByamlDataType data_type = TGetByamlDataTypeValue<T>();

                if constexpr (data_type == ByamlDataType::Bool) {
                    return this->TryGetBoolByIndex(out_value, index);
                } else if constexpr (data_type == ByamlDataType::S32) {
                    return this->TryGetS32ByIndex(out_value, index);
                } else if constexpr (data_type == ByamlDataType::F32) {
                    return this->TryGetF32ByIndex(out_value, index);
                } else if constexpr (data_type == ByamlDataType::U32) {
                    return this->TryGetU32ByIndex(out_value, index);
                } else if constexpr (data_type == ByamlDataType::S64) {
                    return this->TryGetS64ByIndex(out_value, index);
                } else if constexpr (data_type == ByamlDataType::U64) {
                    return this->TryGetU64ByIndex(out_value, index);
                } else if constexpr (data_type == ByamlDataType::F64) {
                    return this->TryGetF64ByIndex(out_value, index);
                } else if constexpr (data_type == ByamlDataType::StringIndex) {
                    return this->TryGetStringByIndex(out_value, index);
                }
            }

            template <typename T>
                requires (TGetByamlDataTypeValue<T>() != ByamlDataType::Null)
            bool TryGetValueByHash(T *out_value, u32 hash) const {

                /* Lookup required function at compile time */
                const ByamlDataType data_type = TGetByamlDataTypeValue<T>();

                if constexpr (data_type == ByamlDataType::Bool) {
                    return this->TryGetBoolByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::S32) {
                    return this->TryGetS32ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::F32) {
                    return this->TryGetF32ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::U32) {
                    return this->TryGetU32ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::S64) {
                    return this->TryGetS64ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::U64) {
                    return this->TryGetU64ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::F64) {
                    return this->TryGetF64ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::StringIndex) {
                    return this->TryGetStringByData(out_value, hash);
                }
            }

            template <typename T>
                requires (TGetByamlDataTypeValue<T>() != ByamlDataType::Null)
            bool TryGetValueByHash(T *out_value, u64 hash) const {

                /* Lookup required function at compile time */
                const ByamlDataType data_type = TGetByamlDataTypeValue<T>();

                if constexpr (data_type == ByamlDataType::Bool) {
                    return this->TryGetBoolByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::S32) {
                    return this->TryGetS32ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::F32) {
                    return this->TryGetF32ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::U32) {
                    return this->TryGetU32ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::S64) {
                    return this->TryGetS64ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::U64) {
                    return this->TryGetU64ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::F64) {
                    return this->TryGetF64ByHash(out_value, hash);
                } else if constexpr (data_type == ByamlDataType::StringIndex) {
                    return this->TryGetStringByData(out_value, hash);
                }
            }

            template <typename T>
                requires (TGetByamlDataTypeValue<T>() != ByamlDataType::Null)
            bool TryGetValueByData(T *out_value, ByamlData data) const {

                /* Lookup required function at compile time */
                const ByamlDataType data_type = TGetByamlDataTypeValue<T>();

                if constexpr (data_type == ByamlDataType::Bool) {
                    return this->TryGetBoolByData(out_value, data);
                } else if constexpr (data_type == ByamlDataType::S32) {
                    return this->TryGetS32ByData(out_value, data);
                } else if constexpr (data_type == ByamlDataType::F32) {
                    return this->TryGetF32ByData(out_value, data);
                } else if constexpr (data_type == ByamlDataType::U32) {
                    return this->TryGetU32ByData(out_value, data);
                } else if constexpr (data_type == ByamlDataType::S64) {
                    return this->TryGetS64ByData(out_value, data);
                } else if constexpr (data_type == ByamlDataType::U64) {
                    return this->TryGetU64ByData(out_value, data);
                } else if constexpr (data_type == ByamlDataType::F64) {
                    return this->TryGetF64ByData(out_value, data);
                } else if constexpr (data_type == ByamlDataType::StringIndex) {
                    return this->TryGetStringByData(out_value, data);
                }
            }
    };
}

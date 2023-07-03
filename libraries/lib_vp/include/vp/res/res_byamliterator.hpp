#pragma once

namespace vp::res {

    class ByamlIterator {
        private:
            const ResByaml          *m_byaml;
            const ResByamlContainer *m_data_container;
        public:
            constexpr ByamlIterator() : m_byaml(nullptr), m_data_container(nullptr) {/*...*/}

            ALWAYS_INLINE ByamlIterator(const unsigned char *byaml_file) : m_byaml(reinterpret_cast<const ResByaml*>(byaml_file)) {

                /* Verify valid byaml file */
                if (m_byaml->IsValid() == false) {
                    m_byaml = nullptr;
                    return;
                }

                /* Set data container */
                m_data_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(byaml_file) +  m_byaml->data_offset);
            }
            ALWAYS_INLINE ByamlIterator(const void *byaml_file) : m_byaml(reinterpret_cast<const ResByaml*>(byaml_file)) {

                /* Verify valid byaml file */
                if (m_byaml->IsValid() == false) {
                    m_byaml = nullptr;
                    return;
                }

                /* Set data container */
                m_data_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(byaml_file) +  m_byaml->data_offset);
            }
            ALWAYS_INLINE ByamlIterator(const unsigned char *byaml_header, const unsigned char *container_header) : m_byaml(reinterpret_cast<const ResByaml*>(byaml_header)), m_data_container(reinterpret_cast<const ResByamlContainer*>(container_header)) {/*...*/}

            constexpr ALWAYS_INLINE ByamlIterator(const ByamlIterator& rhs) : m_byaml(rhs.m_byaml), m_data_container(rhs.m_data_container) {/*...*/}

            constexpr ~ByamlIterator() {/*...*/}

            constexpr ALWAYS_INLINE ByamlIterator &operator=(const ByamlIterator &rhs) {
                m_byaml = rhs.m_byaml;
                m_data_container = rhs.m_data_container;
                return *this;
            }

            constexpr ALWAYS_INLINE bool IsValid() {
                return m_byaml != nullptr;
            }

            bool TryGetKeyByData(const char **out_key, ByamlData data) {
                if (data.IsKeyIndexValid() == false) { return false; }

                const ResByamlContainer        *key_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) +  m_byaml->key_table_offset);
                const ByamlStringTableIterator  table_iterator(key_container);

                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(key_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 offset_offset = *reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(key_container) +  sizeof(ResByamlContainer));
                    key_pool_offset         = *reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(key_container) +  offset_offset);
                }

                *out_key = table_iterator.GetStringByIndex(data.key_index, key_pool_offset);
                return true;
            }

            bool TryGetKeyIndexByKey(u32 *out_key_index, const char *key) {

                ByamlStringTableIterator table_iterator(reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) +  m_byaml->key_table_offset));
                const u32 index = table_iterator.FindKeyIndexByKey(key);
                if (index == 0xffff'ffff) { return false; }

                *out_key_index = index;
                return true;
            }

            bool TryGetByamlDataByKey(ByamlData *out_byaml_data, const char *key) const {

                /* Integrity checks */
                if (m_data_container == nullptr || m_byaml == nullptr || m_byaml->key_table_offset == 0 || (static_cast<ByamlDataType>(m_data_container->data_type) != ByamlDataType::Dictionary && static_cast<ByamlDataType>(m_data_container->data_type) != ByamlDataType::DictionaryWithRemap)) { return false; }

                /* Setup container and iterators */
                const ResByamlContainer        *key_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) +  m_byaml->key_table_offset);
                const ByamlStringTableIterator  table_iterator(key_container);
                const ByamlDictionaryIterator   dic_iter(m_data_container);

                /* Handle key table relocation */
                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(key_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 offset_offset = *reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(key_container) +  sizeof(ResByamlContainer));
                    key_pool_offset         = *reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(key_container) +  offset_offset);
                }

                /* Binary search pattern as the string table is always sorted */
                u32 size = dic_iter.GetCount();
                u32 i = 0;
                u32 index = 0;
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    const ResByamlDictionaryPair *res_pair = dic_iter.GetDictionaryPairByIndex(index);
                    const char *pair_key = table_iterator.GetStringByIndex(res_pair->key_index, key_pool_offset);
                    const s32 result = ::strcmp(key, pair_key);
                    
                    if (result == 0) {
                        out_byaml_data->SetPair(res_pair);
                        return true;
                    }
                    if (0 < result) {
                        i = index + 1;
                        index = size;
                    }
                    size = index;
                }

                *out_byaml_data = ByamlData{};
                return false;
            }

            bool TryGetByamlDataByHash(ByamlData *out_byaml_data, u32 hash) const {

                /* Integrity checks */
                if (m_data_container == nullptr || m_byaml == nullptr || m_byaml->key_table_offset == 0 || (static_cast<ByamlDataType>(m_data_container->data_type & 0xe0) != ByamlDataType::HashArrayU32_1)) { return false; }

                /* Setup container and iterators */
                const u32 stride = (m_data_container->data_type & 0xf) * sizeof(u32) + sizeof(ResByamlContainer);
                const ByamlHashArrayIterator hash_array_iter(m_data_container, stride);

                /* Binary search pattern as the string table is always sorted */
                u32 size = hash_array_iter.GetCount();
                u32 i = 0;
                u32 index = 0;
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    
                    const ByamlHashAccessor hash_accessor(m_data_container, stride, index);
                    const u32 cur_hash = hash_accessor.GetHash();
                    
                    if (hash == cur_hash) {
                        out_byaml_data->data_type = hash_array_iter.GetDataType(index);
                        out_byaml_data->u32_value = hash_accessor.GetValue();
                        return true;
                    }
                    if (cur_hash < hash) {
                        i = index + 1;
                        index = size;
                    }
                    size = index;
                }

                *out_byaml_data = ByamlData{};
                return false;
            }
            bool TryGetByamlDataByHash(ByamlData *out_byaml_data, u64 hash) const {

                /* Integrity checks */
                if (m_data_container == nullptr || m_byaml == nullptr || m_byaml->key_table_offset == 0 || (static_cast<ByamlDataType>(m_data_container->data_type & 0xe0) != ByamlDataType::HashArrayU32_1)) { return false; }

                /* Setup container and iterators */
                const u32 stride = (m_data_container->data_type & 0xf) * sizeof(u32) + sizeof(ResByamlContainer);
                const ByamlHashArrayIterator hash_array_iter(m_data_container, stride);

                /* Binary search pattern as the string table is always sorted */
                u32 size = hash_array_iter.GetCount();
                u32 i = 0;
                u32 index = 0;
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    
                    const ByamlHashAccessor hash_accessor(m_data_container, stride, index);
                    const u64 cur_hash = hash_accessor.GetHash();
                    
                    if (hash == cur_hash) {
                        out_byaml_data->data_type = hash_array_iter.GetDataType(index);
                        out_byaml_data->u32_value = hash_accessor.GetValue();
                        return true;
                    }
                    if (cur_hash < hash) {
                        i = index + 1;
                        index = size;
                    }
                    size = index;
                }

                *out_byaml_data = ByamlData{};
                return false;
            }

            bool TryGetByamlDataByIndex(ByamlData *out_byaml_data, u32 index) const {
                
                if (m_data_container == nullptr) { return false; }

                u32 data_type = static_cast<u32>(m_data_container->data_type);
                if (static_cast<ByamlDataType>(data_type) == ByamlDataType::Dictionary || static_cast<ByamlDataType>(data_type) == ByamlDataType::DictionaryWithRemap) {
                    const ByamlDictionaryIterator dic_iter(m_data_container);
                    return dic_iter.TryGetDataByIndex(out_byaml_data, index);
                }
                if (static_cast<ByamlDataType>(data_type) == ByamlDataType::Array || static_cast<ByamlDataType>(data_type) == ByamlDataType::MonoTypedArray) {
                    const ByamlArrayIterator array_iter(m_data_container);
                    return array_iter.TryGetDataByIndex(out_byaml_data, index);
                }
                if (static_cast<ByamlDataType>(data_type & 0xe0) == ByamlDataType::HashArrayU32_1) {
                    const ByamlHashArrayIterator hash_array_iter(m_data_container);
                    return hash_array_iter.TryGetDataByIndex(out_byaml_data, index);
                }

                *out_byaml_data = ByamlData{};
                return false;
            }

            constexpr ALWAYS_INLINE u32 GetDataCount() const {
                if (m_data_container == nullptr) { return 0; }
                return m_data_container->count;
            }

            constexpr ALWAYS_INLINE u32 GetDataType() const {
                if (m_data_container == nullptr) { return 0; }
                return m_data_container->data_type;
            }

            bool TryGetF32ByKey(float *out_float, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::F32) { return false; }

                *out_float = data.f32_value;
                return true;
            }

            bool TryGetF32ByIndex(float *out_float, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::F32) { return false; }

                *out_float = data.f32_value;
                return true;
            }

            bool TryGetF32ByData(float *out_float, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::F32) { return false; }

                *out_float = data.f32_value;
                return true;
            }

            bool TryGetU32ByKey(u32 *out_uint, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::U32) { return false; }

                *out_uint = data.u32_value;
                return true;
            }

            bool TryGetU32ByIndex(u32 *out_uint, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::U32) { return false; }

                *out_uint = data.u32_value;
                return true;
            }

            bool TryGetU32ByData(u32 *out_uint, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::U32) { return false; }

                *out_uint = data.u32_value;
                return true;
            }

            bool TryGetS32ByKey(s32 *out_int, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::S32) { return false; }

                *out_int = data.s32_value;
                return true;
            }

            bool TryGetS32ByIndex(s32 *out_int, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::S32) { return false; }

                *out_int = data.s32_value;
                return true;
            }

            bool TryGetS32ByData(s32 *out_int, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::S32) { return false; }

                *out_int = data.s32_value;
                return true;
            }

            bool TryGetBoolByKey(bool *out_bool, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::Bool) { return false; }

                *out_bool = static_cast<bool>(data.u32_value);
                return true;
            }

            bool TryGetBoolByIndex(bool *out_bool, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::Bool) { return false; }

                *out_bool = static_cast<bool>(data.u32_value);
                return true;
            }

            bool TryGetBoolByData(bool *out_bool, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::Bool) { return false; }

                *out_bool = static_cast<bool>(data.u32_value);
                return true;
            }

            bool TryGetF64ByKey(double *out_double, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::F32) { 
                    *out_double = static_cast<double>(data.f32_value);
                    return true;
                }
                if (data_type != ByamlDataType::F64) { return false; }

                const double *value = reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_double = *value;
                return true;
            }

            bool TryGetF64ByIndex(double *out_double, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::F32) { 
                    *out_double = static_cast<double>(data.f32_value);
                    return true;
                }
                if (data_type != ByamlDataType::F64) { return false; }

                const double *value = reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_double = *value;
                return true;
            }

            bool TryGetF64ByData(double *out_double, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::F32) { 
                    *out_double = static_cast<double>(data.f32_value);
                    return true;
                }
                if (data_type != ByamlDataType::F64) { return false; }

                const double *value = reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_double = *value;
                return true;
            }

            bool TryGetU64ByKey(u64 *out_ulonglong, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::U32) { 
                    *out_ulonglong = static_cast<u64>(data.u32_value);
                    return true;
                }
                if (data_type != ByamlDataType::U64) { return false; }

                const u64 *value = reinterpret_cast<u64*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_ulonglong = *value;
                return true;
            }

            bool TryGetU64ByIndex(u64 *out_ulonglong, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::U32) { 
                    *out_ulonglong = static_cast<u64>(data.u32_value);
                    return true;
                }
                if (data_type != ByamlDataType::U64) { return false; }

                const u64 *value = reinterpret_cast<u64*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_ulonglong = *value;
                return true;
            }

            bool TryGetU64ByData(u64 *out_ulonglong, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::U32) { 
                    *out_ulonglong = static_cast<u64>(data.u32_value);
                    return true;
                }
                if (data_type != ByamlDataType::U64) { return false; }

                const u64 *value = reinterpret_cast<u64*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_ulonglong = *value;
                return true;
            }

            bool TryGetS64ByKey(s64 *out_longlong, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::S32) { 
                    *out_longlong = static_cast<s64>(data.s32_value);
                    return true;
                }
                if (data_type != ByamlDataType::S64) { return false; }

                const s64 *value = reinterpret_cast<s64*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_longlong = *value;
                return true;
            }

            bool TryGetS64ByIndex(s64 *out_longlong, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::S32) { 
                    *out_longlong = static_cast<s64>(data.s32_value);
                    return true;
                }
                if (data_type != ByamlDataType::S64) { return false; }

                const s64 *value = reinterpret_cast<s64*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_longlong = *value;
                return true;
            }
            
            bool TryGetS64ByData(s64 *out_longlong, ByamlData data) {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::S32) { 
                    *out_longlong = static_cast<s64>(data.s32_value);
                    return true;
                }
                if (data_type != ByamlDataType::S64) { return false; }

                const s64 *value = reinterpret_cast<s64*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                *out_longlong = *value;
                return true;
            }

            bool TryGetBinaryDataByKey(void **out_binary, u32 *out_size, u32 *out_alignment, const char *key) {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::BinaryData) {
                    *out_size      = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_alignment = 0;
                    *out_binary    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32));
                    return true;
                }
                if (data_type == ByamlDataType::BinaryDataWithAlignment) {
                    *out_size      = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_alignment = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32));
                    *out_binary    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32) + sizeof(u32));
                    return true;
                }

                return false;
            }

            bool TryGetBinaryDataByIndex(void **out_binary, u32 *out_size, u32 *out_alignment, u32 index) {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::BinaryData) {
                    *out_size      = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_alignment = 0;
                    *out_binary    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32));
                    return true;
                }
                if (data_type == ByamlDataType::BinaryDataWithAlignment) {
                    *out_size      = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_alignment = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32));
                    *out_binary    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32) + sizeof(u32));
                    return true;
                }

                return false;
            }

            bool TryGetBinaryDataByData(void **out_binary, u32 *out_size, u32 *out_alignment, ByamlData data) {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::BinaryData) {
                    *out_size      = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_alignment = 0;
                    *out_binary    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32));
                    return true;
                }
                if (data_type == ByamlDataType::BinaryDataWithAlignment) {
                    *out_size      = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_alignment = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32));
                    *out_binary    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32) + sizeof(u32));
                    return true;
                }

                return false;
            }

            bool TryGetStringByKey(const char **out_string, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::StringIndex) { return false; }

                const ResByamlContainer *key_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + m_byaml->string_table_offset);
                const ByamlStringTableIterator table_iter(key_container);

                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(key_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 offset_offset = *reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(key_container) +  sizeof(ResByamlContainer));
                    key_pool_offset         = *reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(key_container) +  offset_offset);
                }

                *out_string = table_iter.GetStringByIndex(data.u32_value, key_pool_offset);
                return true;
            }

            bool TryGetStringByIndex(const char **out_string, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::StringIndex) { return false; }

                const ResByamlContainer *key_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + m_byaml->string_table_offset);
                const ByamlStringTableIterator table_iter(key_container);

                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(key_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 offset_offset = *reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(key_container) +  sizeof(ResByamlContainer));
                    key_pool_offset         = *reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(key_container) +  offset_offset);
                }

                *out_string = table_iter.GetStringByIndex(data.u32_value, key_pool_offset);
                return true;
            }

            bool TryGetStringByData(const char **out_string, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::StringIndex) { return false; }

                const ResByamlContainer *key_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + m_byaml->string_table_offset);
                const ByamlStringTableIterator table_iter(key_container);

                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(key_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 offset_offset = *reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(key_container) +  sizeof(ResByamlContainer));
                    key_pool_offset         = *reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(key_container) +  offset_offset);
                }

                *out_string = table_iter.GetStringByIndex(data.u32_value, key_pool_offset);
                return true;
            }

            bool TryGetIteratorByKey(ByamlIterator *out_iterator, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                const ByamlDataType arr_type  = static_cast<ByamlDataType>(data.data_type & 0xf7);
                const ByamlDataType hash_type = static_cast<ByamlDataType>(data.data_type & 0xe0);
                if (arr_type == ByamlDataType::Array || data_type == ByamlDataType::Dictionary || data_type == ByamlDataType::DictionaryWithRemap || hash_type == ByamlDataType::HashArrayU32_1) {
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    return true;
                }

                if (data_type == ByamlDataType::Null) { 
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = nullptr;
                    return true;
                }

                out_iterator->m_byaml          = nullptr;
                out_iterator->m_data_container = nullptr;
                return false;
            }

            bool TryGetIteratorByIndex(ByamlIterator *out_iterator, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                const ByamlDataType arr_type  = static_cast<ByamlDataType>(data.data_type & 0xf7);
                const ByamlDataType hash_type = static_cast<ByamlDataType>(data.data_type & 0xe0);
                if (arr_type == ByamlDataType::Array || data_type == ByamlDataType::Dictionary || data_type == ByamlDataType::DictionaryWithRemap || hash_type == ByamlDataType::HashArrayU32_1) { 
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    return true;
                }

                if (data_type == ByamlDataType::Null) { 
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = nullptr;
                    return true;
                }

                out_iterator->m_byaml          = nullptr;
                out_iterator->m_data_container = nullptr;
                return false;
            }

            bool TryGetIteratorByHash(ByamlIterator *out_iterator, u32 hash) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                const ByamlDataType arr_type  = static_cast<ByamlDataType>(data.data_type & 0xf7);
                const ByamlDataType hash_type = static_cast<ByamlDataType>(data.data_type & 0xe0);
                if (arr_type == ByamlDataType::Array || data_type == ByamlDataType::Dictionary || data_type == ByamlDataType::DictionaryWithRemap || hash_type == ByamlDataType::HashArrayU32_1) { 
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    return true;
                }

                if (data_type == ByamlDataType::Null) { 
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = nullptr;
                    return true;
                }

                out_iterator->m_byaml          = nullptr;
                out_iterator->m_data_container = nullptr;
                return false;
            }

            bool TryGetIteratorByHash(ByamlIterator *out_iterator, u64 hash) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByHash(std::addressof(data), hash);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                const ByamlDataType arr_type  = static_cast<ByamlDataType>(data.data_type & 0xf7);
                const ByamlDataType hash_type = static_cast<ByamlDataType>(data.data_type & 0xe0);
                if (arr_type == ByamlDataType::Array || data_type == ByamlDataType::Dictionary || data_type == ByamlDataType::DictionaryWithRemap || hash_type == ByamlDataType::HashArrayU32_1) { 
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    return true;
                }

                if (data_type == ByamlDataType::Null) { 
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = nullptr;
                    return true;
                }

                out_iterator->m_byaml          = nullptr;
                out_iterator->m_data_container = nullptr;
                return false;
            }

            bool TryGetIteratorByData(ByamlIterator *out_iterator, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                const ByamlDataType arr_type  = static_cast<ByamlDataType>(data.data_type & 0xf7);
                const ByamlDataType hash_type = static_cast<ByamlDataType>(data.data_type & 0xe0);
                if (arr_type == ByamlDataType::Array || data_type == ByamlDataType::Dictionary || data_type == ByamlDataType::DictionaryWithRemap || hash_type == ByamlDataType::HashArrayU32_1) {
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = reinterpret_cast<const ResByamlContainer*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    return true;
                }

                if (data_type == ByamlDataType::Null) { 
                    out_iterator->m_byaml          = m_byaml;
                    out_iterator->m_data_container = nullptr;
                    return true;
                }

                out_iterator->m_byaml          = nullptr;
                out_iterator->m_data_container = nullptr;
                return false;
            }
    };
}

#pragma once

namespace vp::res {

    class ByamlIterator {
        private:
            const ResByaml          *m_byaml;
            const ResByamlContainer *m_data_container;
        public:
            constexpr ByamlIterator() {/*...*/}

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

                ByamlStringTableIterator table_iterator(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(m_byaml) +  m_byaml->key_table_offset));

                *out_key = table_iterator.GetStringByIndex(data.key_index);
                return true;
            }

            bool TryGetIndexByKey(u32 *out_index, const char *key) {

                ByamlStringTableIterator table_iterator(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(m_byaml) +  m_byaml->key_table_offset));
                const u32 index = table_iterator.FindIndexByString(key);
                if (index == 0xffff'ffff) { return false; }

                *out_index = index;
                return true;
            }

            bool TryGetByamlDataByKey(ByamlData *out_data, const char *key) const {

                /* Integrity checks */
                if (m_data_container == nullptr || static_cast<ByamlDataType>(m_data_container->data_type) != ByamlDataType::Dictionary || m_byaml == nullptr || m_byaml->key_table_offset == 0) { return false; }

                ByamlStringTableIterator table_iterator(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(m_byaml) +  m_byaml->key_table_offset));
                ByamlDictionaryIterator dic_iter(reinterpret_cast<const unsigned char*>(m_data_container));

                /* Binary search pattern as the string table is always sorted */
                u32 size = dic_iter.GetSize();
                u32 i = 0;
                u32 index = 0;
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    const ResByamlDictionaryPair *res_pair = dic_iter.GetDictionaryPairByIndex(index);
                    const char *pair_key = table_iterator.GetStringByIndex(res_pair->key_index);
                    const s32 result = ::strcmp(key, pair_key);
                    
                    if (result == 0) {
                        out_data->SetPair(res_pair);
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

            bool TryGetByamlDataByIndex(ByamlData *out_data, u32 index) const {
                if (m_data_container == nullptr) { return false; }

                ByamlDataType data_type = static_cast<ByamlDataType>(m_data_container->data_type);
                if (data_type == ByamlDataType::Dictionary) {
                    const ByamlDictionaryIterator dic_iter(reinterpret_cast<const unsigned char*>(m_data_container));
                    return dic_iter.TryGetDataByIndex(out_data, index);
                }
                if (data_type == ByamlDataType::Array) {
                    const ByamlArrayIterator array_iter(reinterpret_cast<const unsigned char*>(m_data_container));
                    return array_iter.TryGetDataByIndex(out_data, index);
                }
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

            bool TryGetBinaryDataByKey(void **out_binary, u32 *out_size, const char *key) {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::BinaryData) {
                    *out_size   = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_binary = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32));
                    return true;
                }
                if (data_type == ByamlDataType::BinaryDataPlus) {
                    *out_size   = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_binary = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32) + sizeof(u32));
                    return true;
                }

                return false;
            }

            bool TryGetBinaryDataByIndex(void **out_binary, u32 *out_size, u32 index) {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::BinaryData) {
                    *out_size   = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_binary = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32));
                    return true;
                }
                if (data_type == ByamlDataType::BinaryDataPlus) {
                    *out_size   = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_binary = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32) + sizeof(u32));
                    return true;
                }

                return false;
            }

            bool TryGetBinaryDataByData(void **out_binary, u32 *out_size, ByamlData data) {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::BinaryData) {
                    *out_size   = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_binary = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32));
                    return true;
                }
                if (data_type == ByamlDataType::BinaryDataPlus) {
                    *out_size   = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value);
                    *out_binary = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml) + data.u32_value + sizeof(u32) + sizeof(u32));
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

                const ByamlStringTableIterator table_iter(reinterpret_cast<unsigned char*>(reinterpret_cast<uintptr_t>(m_byaml) + m_byaml->string_table_offset));
                *out_string = table_iter.GetStringByIndex(data.u32_value);
                return true;
            }

            bool TryGetStringByIndex(const char **out_string, u32 index) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByIndex(std::addressof(data), index);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::StringIndex) { return false; }

                const ByamlStringTableIterator table_iter(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(m_byaml) + m_byaml->string_table_offset));
                *out_string = table_iter.GetStringByIndex(data.u32_value);
                return true;
            }

            bool TryGetStringByData(const char **out_string, ByamlData data) const {

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type != ByamlDataType::StringIndex) { return false; }

                const ByamlStringTableIterator table_iter(reinterpret_cast<const unsigned char*>(reinterpret_cast<uintptr_t>(m_byaml) + m_byaml->string_table_offset));
                *out_string = table_iter.GetStringByIndex(data.u32_value);
                return true;
            }

            bool TryGetIteratorByKey(ByamlIterator *out_iterator, const char *key) const {
                ByamlData data = {};
                const bool result = this->TryGetByamlDataByKey(std::addressof(data), key);
                if (result == false) { return false; }

                const ByamlDataType data_type = static_cast<ByamlDataType>(data.data_type);
                if (data_type == ByamlDataType::Array || data_type == ByamlDataType::Dictionary) { 
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
                if (data_type == ByamlDataType::Array || data_type == ByamlDataType::Dictionary) { 
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
                if (data_type == ByamlDataType::Array || data_type == ByamlDataType::Dictionary) { 
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

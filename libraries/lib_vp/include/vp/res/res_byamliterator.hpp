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
            constexpr ByamlIterator() : m_byaml(nullptr), m_container_data{ .key_index = ByamlData::cInvalidKeyIndex } {/*...*/}
            constexpr ByamlIterator(const ResByaml *res_byaml, ByamlData data) : m_byaml(res_byaml), m_container_data(data) {/*...*/}
            ByamlIterator(const unsigned char *byaml_file);
            ByamlIterator(const void *byaml_file);

            constexpr ALWAYS_INLINE ByamlIterator(const ByamlIterator& rhs) : m_byaml(rhs.m_byaml), m_container_data(rhs.m_container_data) {/*...*/}

            constexpr ~ByamlIterator() {/*...*/}

            constexpr ALWAYS_INLINE ByamlIterator &operator=(const ByamlIterator &rhs) {
                m_byaml          = rhs.m_byaml;
                m_container_data = rhs.m_container_data;
                return *this;
            }

            constexpr ALWAYS_INLINE bool IsDictionary()     const { return IsDictionaryType(static_cast<ByamlDataType>(m_container_data.data_type)); }
            constexpr ALWAYS_INLINE bool IsValid()          const { return m_byaml != nullptr; }
            constexpr ALWAYS_INLINE bool IsContainerValid() const { return (IsContainerType(static_cast<ByamlDataType>(m_container_data.data_type))) && (m_container_data.u32_value != 0); }

            constexpr ALWAYS_INLINE ByamlData GetDataByIterator() const { return m_container_data; }

            bool TryGetKeyIndexByKey(u32 *out_key_index, const char *key) const;
            bool TryGetKeyByData(const char **out_key, ByamlData data) const;
            bool TryGetKeyByIterator(const char **out_key) const;

            bool TryRemapIndexByKey(u32 *out_container_index, const char *key) const;

            bool TryGetByamlDataByKey(ByamlData *out_byaml_data, const char *key) const;
            bool TryGetByamlDataByIndex(ByamlData *out_byaml_data, u32 index) const;
            bool TryGetByamlDataByHash(ByamlData *out_byaml_data, u32 hash) const;
            bool TryGetByamlDataByHash(ByamlData *out_byaml_data, u64 hash) const;

            u32 GetDataCount() const;

            constexpr ALWAYS_INLINE u32 GetDataType() const {
                return m_container_data.data_type;
            }

            bool TryGetBoolByKey(bool *out_bool, const char *key) const;
            bool TryGetBoolByIndex(bool *out_bool, u32 index) const;
            bool TryGetBoolByHash(bool *out_bool, u32 hash) const;
            bool TryGetBoolByHash(bool *out_bool, u64 hash) const;
            bool TryGetBoolByData(bool *out_bool, ByamlData data) const;
            bool TryGetBoolByIterator(bool *out_value) const;

            bool TryGetS32ByKey(s32 *out_int, const char *key) const;
            bool TryGetS32ByIndex(s32 *out_int, u32 index) const;
            bool TryGetS32ByHash(s32 *out_int, u32 hash) const;
            bool TryGetS32ByHash(s32 *out_int, u64 hash) const;
            bool TryGetS32ByData(s32 *out_int, ByamlData data) const;
            bool TryGetS32ByIterator(s32 *out_value) const;

            bool TryGetF32ByKey(float *out_float, const char *key) const;
            bool TryGetF32ByIndex(float *out_float, u32 index) const;
            bool TryGetF32ByHash(float *out_float, u32 hash) const;
            bool TryGetF32ByHash(float *out_float, u64 hash) const;
            bool TryGetF32ByData(float *out_float, ByamlData data) const;
            bool TryGetF32ByIterator(float *out_value) const;

            bool TryGetU32ByKey(u32 *out_uint, const char *key) const;
            bool TryGetU32ByIndex(u32 *out_uint, u32 index) const;
            bool TryGetU32ByHash(u32 *out_uint, u32 hash) const;
            bool TryGetU32ByHash(u32 *out_uint, u64 hash) const;
            bool TryGetU32ByData(u32 *out_uint, ByamlData data) const;
            bool TryGetU32ByIterator(u32 *out_value) const;

            bool TryGetS64ByKey(s64 *out_longlong, const char *key) const;
            bool TryGetS64ByIndex(s64 *out_longlong, u32 index) const;
            bool TryGetS64ByHash(s64 *out_longlong, u32 hash) const;
            bool TryGetS64ByHash(s64 *out_longlong, u64 hash) const;
            bool TryGetS64ByData(s64 *out_longlong, ByamlData data) const;
            bool TryGetS64ByIterator(s64 *out_value) const;

            bool TryGetU64ByKey(u64 *out_ulonglong, const char *key) const;
            bool TryGetU64ByIndex(u64 *out_ulonglong, u32 index) const;
            bool TryGetU64ByHash(u64 *out_ulonglong, u32 hash) const;
            bool TryGetU64ByHash(u64 *out_ulonglong, u64 hash) const;
            bool TryGetU64ByData(u64 *out_ulonglong, ByamlData data) const;
            bool TryGetU64ByIterator(u64 *out_value) const;

            bool TryGetF64ByKey(double *out_double, const char *key) const;
            bool TryGetF64ByIndex(double *out_double, u32 index) const;
            bool TryGetF64ByHash(double *out_double, u32 hash) const;
            bool TryGetF64ByHash(double *out_double, u64 hash) const;
            bool TryGetF64ByData(double *out_double, ByamlData data) const;
            bool TryGetF64ByIterator(double *out_value) const;

            bool TryGetBinaryDataByKey(void **out_binary, u32 *out_size, u32 *out_alignment, const char *key) const;
            bool TryGetBinaryDataByIndex(void **out_binary, u32 *out_size, u32 *out_alignment, u32 index) const;
            bool TryGetBinaryDataByHash(void **out_binary, u32 *out_size, u32 *out_alignment, u32 hash) const;
            bool TryGetBinaryDataByHash(void **out_binary, u32 *out_size, u32 *out_alignment, u64 hash) const;
            bool TryGetBinaryDataByData(void **out_binary, u32 *out_size, u32 *out_alignment, ByamlData data) const;
            bool TryGetBinaryDataByIterator(void **out_value, u32 *out_size, u32 *out_alignment) const;

            bool TryGetStringByKey(const char **out_string, const char *key) const;
            bool TryGetStringByIndex(const char **out_string, u32 index) const;
            bool TryGetStringByHash(const char **out_string, u32 hash) const;
            bool TryGetStringByHash(const char **out_string, u64 hash) const;
            bool TryGetStringByData(const char **out_string, ByamlData data) const;
            bool TryGetStringByIterator(const char **out_value) const;

            ByamlIterator GetIteratorByIndex(u32 index) const;
            ByamlIterator GetIteratorByKey(const char *key) const;
            ByamlIterator GetIteratorByHash(u32 hash) const;
            ByamlIterator GetIteratorByHash(u64 hash) const;

            bool TryGetIteratorByKey(ByamlIterator *out_iterator, const char *key) const;
            bool TryGetIteratorByIndex(ByamlIterator *out_iterator, u32 index) const;
            bool TryGetIteratorByHash(ByamlIterator *out_iterator, u32 hash) const;
            bool TryGetIteratorByHash(ByamlIterator *out_iterator, u64 hash) const;
            bool TryGetIteratorByData(ByamlIterator *out_iterator, ByamlData data) const;
            bool TryGetIteratorByIterator(ByamlIterator *out_iter) const;

            template <typename T>
                requires (TGetByamlDataTypeValue<T>() != ByamlDataType::Null)
            ALWAYS_INLINE bool TryGetValueByKey(T *out_value, const char *key) const {

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
            ALWAYS_INLINE bool TryGetValueByIndex(T *out_value, u32 index) const {

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
            ALWAYS_INLINE bool TryGetValueByHash(T *out_value, u32 hash) const {

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
            ALWAYS_INLINE bool TryGetValueByHash(T *out_value, u64 hash) const {

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
            ALWAYS_INLINE bool TryGetValueByData(T *out_value, ByamlData data) const {

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

            template <typename T>
                requires (TGetByamlDataTypeValue<T>() != ByamlDataType::Null)
            ALWAYS_INLINE bool TryGetValueByIterator(T *out_value) const {
                return this->TryGetValueByData(out_value, m_container_data);
            }

            template <typename T>
                requires (!IsBigDataType(TGetByamlDataTypeValue<T>()))
            const T *GetMonoArray(u32 *out_count) {

                /* Fail if not array  */
                const ByamlDataType data_type = static_cast<ByamlDataType>(this->GetDataType());
                if (IsArrayType(data_type) == false) { return nullptr; }

                /* Get count */
                const ResByamlContainer *container = reinterpret_cast<const ResByamlContainer*>(m_container_data.GetBigData(m_byaml));
                const u32                count     = (m_byaml->IsReverseEndian() == false) ? container->count : vp::util::SwapEndian24(container->count);
                if (out_count != nullptr) { *out_count = count; }

                /* Get array if mono-typed */
                const ByamlDataType *data_type_array = reinterpret_cast<const ByamlDataType*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer));
                if (data_type == ByamlDataType::MonoTypedArray) {
                    if (data_type_array[0] != TGetByamlDataTypeValue<T>()) { return nullptr; }
                    return reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(container) + sizeof(u32) + sizeof(ResByamlContainer));
                }

                /* Verify array is mono-typed */
                for (u32 i = 0; i < count; ++i) {
                    if (data_type_array[i] != TGetByamlDataTypeValue<T>()) { return nullptr; }
                }

                return reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(container) + vp::util::AlignUp(count + sizeof(ResByamlContainer), alignof(u32)));
            }

            bool TryGetByamlDataAndKeyByIndex(ByamlData *out_data, const char **out_key, u32 index) {
                const bool data_result = this->TryGetByamlDataByIndex(out_data, index);
                const bool key_result  = this->TryGetKeyByData(out_key, *out_data);
                return data_result & key_result;
            }
            bool TryGetIteratorAndKeyByIndex(ByamlIterator *out_iter, const char **out_key, u32 index) {
                const bool iter_result = this->TryGetIteratorByIndex(out_iter, index);
                const bool key_result  = this->TryGetKeyByData(out_key, out_iter->m_container_data);
                return iter_result & key_result;
            }
    };
}

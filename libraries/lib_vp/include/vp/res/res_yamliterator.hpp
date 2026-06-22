#pragma once

namespace vp::res {

    class YamlIterator {
        private:
            YamlValueBase *m_value;
        public:
            bool GetYamlValueByIndex(YamlValueBase **out_value, u32 index) const {
                YamlContainerBase *container = vp::util::DynamicCast<YamlContainerBase*>(m_value);
                if (container == nullptr) { return false; }
                YamlValueBase *value;
                if (!container->GetValueByIndex(std::addressof(value), index)) { return false; }
                *out_value = value;
                return true;
            }
            bool GetYamlValueByKey(YamlValueBase **out_value, const char *key) const {
                YamlDictionary *dict = vp::util::DynamicCast<YamlDictionary*>(m_value);
                if (dict == nullptr) { return false; }
                YamlValueBase *value;
                if (!dict->GetValueByKey(std::addressof(value), key)) { return false; }
                *out_value = value;
                return true;
            }
            bool GetYamlValueByHash(YamlValueBase **out_value, const u32 *hash) const {
                YamlHashArray *h_array = vp::util::DynamicCast<YamlHashArray*>(m_value);
                if (h_array == nullptr) { return false; }
                YamlValueBase *value;
                if (!h_array->GetValueByHash(std::addressof(value), hash)) { return false; }
                *out_value = value;
                return true;
            }
            template <typename T, typename Ret>
            bool GetValueByIndex(Ret *out_value, u32 index) const {
                YamlValueBase *base;
                if (!this->GetYamlValueByIndex(std::addressof(base), index)) { return false; }
                T *value = vp::util::DynamicCast<T*>(base);
                if (value == nullptr) { return false; }
                value->GetValue(out_value);
                return true;
            }
            template <typename T, typename Ret>
            bool GetValueByKey(Ret *out_value, const char *key) const {
                YamlValueBase *base;
                if (!this->GetYamlValueByKey(std::addressof(base), key)) { return false; }
                T *value = vp::util::DynamicCast<T*>(base);
                if (value == nullptr) { return false; }
                value->GetValue(out_value);
                return true;
            }
            template <typename T, typename Ret>
            bool GetValueByHash(Ret *out_value, const u32 *hash) const {
                YamlValueBase *base;
                if (!this->GetYamlValueByHash(std::addressof(base), hash)) { return false; }
                T *value = vp::util::DynamicCast<T*>(base);
                if (value == nullptr) { return false; }
                value->GetValue(out_value);
                return true;
            }
            template <typename T, typename Ret>
            bool GetValueByIterator(Ret *out_value) const {
                YamlValueBase *base = m_value;
                if (base == nullptr) { return false; }
                T *value = vp::util::DynamicCast<T*>(base);
                if (value == nullptr) { return false; }
                value->GetValue(out_value);
                return true;
            }
        public:
            constexpr YamlIterator() : m_value() {/*...*/}
            constexpr YamlIterator(YamlValueBase *value) : m_value(value) {/*...*/}
            constexpr YamlIterator(YamlDocument *document) : m_value(document->GetRoot()) {/*...*/}
            constexpr ~YamlIterator() {/*...*/}

            constexpr bool IsContainer()  const { return IsContainerType(m_value->GetDataType()); }
            constexpr bool IsHashArray()  const { return IsHashArrayType(m_value->GetDataType()); }
            constexpr bool IsDictionary() const { return IsDictionaryType(m_value->GetDataType()); }
            constexpr bool IsArray()      const { return IsArrayType(m_value->GetDataType()); }
            constexpr bool IsValid()      const { return m_value != nullptr; }

            bool TryGetKeyIndexByKey(u32 *out_key_index, const char *key) const { VP_UNUSED(out_key_index, key); return false; }
            bool TryGetKeyByIndex(const char **out_key, u32 index) const {
                YamlDictionary *dict = vp::util::DynamicCast<YamlDictionary*>(m_value);
                return (dict == nullptr) ? false : dict->GetKeyByIndex(out_key, index);
            }
            bool TryGetKeyByIterator(const char **out_key) const {
                VP_UNUSED(out_key);
                return false;
            }
            bool TryGetHashByIndex(u32 *out_hash, u32 index) const {
                YamlHashArray *h_array = vp::util::DynamicCast<YamlHashArray*>(m_value);
                return (h_array == nullptr) ? false : h_array->GetHashByIndex(out_hash, index);
            }

            bool TryRemapIndexByKey(u32 *out_container_index, const char *key) const {
                YamlDictionary *dict = vp::util::DynamicCast<YamlDictionary*>(m_value);
                return (dict == nullptr) ? false : dict->RemapIndexByKey(out_container_index, key);
            }
            bool TryRemapIndexByHash(u32 *out_container_index, const char *key) const;

            u32 GetDataCount() const {
                YamlContainerBase *container = vp::util::DynamicCast<YamlContainerBase*>(m_value);
                if (container == nullptr) { return m_value != nullptr; }
                return container->GetDataCount();
            }

            u32 GetDataType() const {
                return (m_value == nullptr) ? static_cast<u32>(vp::res::ByamlDataType::Null) : static_cast<u32>(m_value->GetDataType());
            }

            bool TryGetBoolByKey     (bool *out_value, const char *key) const { return this->GetValueByKey     <YamlValueBool>(out_value, key); }
            bool TryGetBoolByIndex   (bool *out_value, u32 index)       const { return this->GetValueByIndex   <YamlValueBool>(out_value, index); }
            bool TryGetBoolByHash    (bool *out_value, u32 hash)        const { return this->GetValueByHash    <YamlValueBool>(out_value, std::addressof(hash)); }
            bool TryGetBoolByHash    (bool *out_value, u64 hash)        const { return this->GetValueByHash    <YamlValueBool>(out_value, reinterpret_cast<const u32*>(std::addressof(hash))); }
            bool TryGetBoolByIterator(bool *out_value)                  const { return this->GetValueByIterator<YamlValueBool>(out_value); }

            bool TryGetS32ByKey     (s32 *out_value, const char *key) const { return this->GetValueByKey     <YamlValueS32>(out_value, key); }
            bool TryGetS32ByIndex   (s32 *out_value, u32 index)       const { return this->GetValueByIndex   <YamlValueS32>(out_value, index); }
            bool TryGetS32ByHash    (s32 *out_value, u32 hash)        const { return this->GetValueByHash    <YamlValueS32>(out_value, std::addressof(hash)); }
            bool TryGetS32ByHash    (s32 *out_value, u64 hash)        const { return this->GetValueByHash    <YamlValueS32>(out_value, reinterpret_cast<const u32*>(std::addressof(hash))); }
            bool TryGetS32ByIterator(s32 *out_value)                  const { return this->GetValueByIterator<YamlValueS32>(out_value); }

            bool TryGetF32ByKey     (float *out_value, const char *key) const { return this->GetValueByKey     <YamlValueFloat>(out_value, key); }
            bool TryGetF32ByIndex   (float *out_value, u32 index)       const { return this->GetValueByIndex   <YamlValueFloat>(out_value, index); }
            bool TryGetF32ByHash    (float *out_value, u32 hash)        const { return this->GetValueByHash    <YamlValueFloat>(out_value, std::addressof(hash)); }
            bool TryGetF32ByHash    (float *out_value, u64 hash)        const { return this->GetValueByHash    <YamlValueFloat>(out_value, reinterpret_cast<const u32*>(std::addressof(hash))); }
            bool TryGetF32ByIterator(float *out_value)                  const { return this->GetValueByIterator<YamlValueFloat>(out_value); }

            bool TryGetU32ByKey     (u32 *out_value, const char *key) const { return this->GetValueByKey     <YamlValueU32>(out_value, key); }
            bool TryGetU32ByIndex   (u32 *out_value, u32 index)       const { return this->GetValueByIndex   <YamlValueU32>(out_value, index); }
            bool TryGetU32ByHash    (u32 *out_value, u32 hash)        const { return this->GetValueByHash    <YamlValueU32>(out_value, std::addressof(hash)); }
            bool TryGetU32ByHash    (u32 *out_value, u64 hash)        const { return this->GetValueByHash    <YamlValueU32>(out_value, reinterpret_cast<const u32*>(std::addressof(hash))); }
            bool TryGetU32ByIterator(u32 *out_value)                  const { return this->GetValueByIterator<YamlValueU32>(out_value); }

            bool TryGetS64ByKey     (s64 *out_value, const char *key) const { return this->GetValueByKey     <YamlValueS64>(out_value, key); }
            bool TryGetS64ByIndex   (s64 *out_value, u32 index)       const { return this->GetValueByIndex   <YamlValueS64>(out_value, index); }
            bool TryGetS64ByHash    (s64 *out_value, u32 hash)        const { return this->GetValueByHash    <YamlValueS64>(out_value, std::addressof(hash)); }
            bool TryGetS64ByHash    (s64 *out_value, u64 hash)        const { return this->GetValueByHash    <YamlValueS64>(out_value, reinterpret_cast<const u32*>(std::addressof(hash))); }
            bool TryGetS64ByIterator(s64 *out_value)                  const { return this->GetValueByIterator<YamlValueS64>(out_value); }

            bool TryGetU64ByKey     (u64 *out_value, const char *key) const { return this->GetValueByKey     <YamlValueU64>(out_value, key); }
            bool TryGetU64ByIndex   (u64 *out_value, u32 index)       const { return this->GetValueByIndex   <YamlValueU64>(out_value, index); }
            bool TryGetU64ByHash    (u64 *out_value, u32 hash)        const { return this->GetValueByHash    <YamlValueU64>(out_value, std::addressof(hash)); }
            bool TryGetU64ByHash    (u64 *out_value, u64 hash)        const { return this->GetValueByHash    <YamlValueU64>(out_value, reinterpret_cast<const u32*>(std::addressof(hash))); }
            bool TryGetU64ByIterator(u64 *out_value)                  const { return this->GetValueByIterator<YamlValueU64>(out_value); }

            bool TryGetF64ByKey     (double *out_value, const char *key) const { return this->GetValueByKey     <YamlValueDouble>(out_value, key); }
            bool TryGetF64ByIndex   (double *out_value, u32 index)       const { return this->GetValueByIndex   <YamlValueDouble>(out_value, index); }
            bool TryGetF64ByHash    (double *out_value, u32 hash)        const { return this->GetValueByHash    <YamlValueDouble>(out_value, std::addressof(hash)); }
            bool TryGetF64ByHash    (double *out_value, u64 hash)        const { return this->GetValueByHash    <YamlValueDouble>(out_value, reinterpret_cast<const u32*>(std::addressof(hash))); }
            bool TryGetF64ByIterator(double *out_value)                  const { return this->GetValueByIterator<YamlValueDouble>(out_value); }

            bool TryGetBinaryDataByKey(void **out_binary, u32 *out_size, u32 *out_alignment, const char *key) const {
                impl::YamlBinaryData bin;
                YamlValueBase *value = nullptr;
                if (!this->GetYamlValueByKey(std::addressof(value), key)) { return false; }
                YamlValueBinaryData *vb = vp::util::DynamicCast<YamlValueBinaryData*>(value);
                if (vb != nullptr) {
                    vb->GetValue(std::addressof(bin));
                    *out_alignment = 0;
                } else {
                    YamlValueBinaryDataWithAlignment *vba = vp::util::DynamicCast<YamlValueBinaryDataWithAlignment*>(value);
                    if (vba == nullptr) { return false; }
                    vba->GetValue(std::addressof(bin));
                    *out_alignment = bin.alignment;
                }
                *out_binary = bin.data;
                *out_size = bin.size;
                return true;
            }
            bool TryGetBinaryDataByIndex(void **out_binary, u32 *out_size, u32 *out_alignment, u32 index) const {
                impl::YamlBinaryData bin;
                YamlValueBase *value = nullptr;
                if (!this->GetYamlValueByIndex(std::addressof(value), index)) { return false; }
                YamlValueBinaryData *vb = vp::util::DynamicCast<YamlValueBinaryData*>(value);
                if (vb != nullptr) {
                    vb->GetValue(std::addressof(bin));
                    *out_alignment = 0;
                } else {
                    YamlValueBinaryDataWithAlignment *vba = vp::util::DynamicCast<YamlValueBinaryDataWithAlignment*>(value);
                    if (vba == nullptr) { return false; }
                    vba->GetValue(std::addressof(bin));
                    *out_alignment = bin.alignment;
                }
                *out_binary = bin.data;
                *out_size = bin.size;
                return true;
            }
            bool TryGetBinaryDataByHash(void **out_binary, u32 *out_size, u32 *out_alignment, u32 hash) const {
                impl::YamlBinaryData bin;
                YamlValueBase *value = nullptr;
                if (!this->GetYamlValueByHash(std::addressof(value), std::addressof(hash))) { return false; }
                YamlValueBinaryData *vb = vp::util::DynamicCast<YamlValueBinaryData*>(value);
                if (vb != nullptr) {
                    vb->GetValue(std::addressof(bin));
                    *out_alignment = 0;
                } else {
                    YamlValueBinaryDataWithAlignment *vba = vp::util::DynamicCast<YamlValueBinaryDataWithAlignment*>(value);
                    if (vba == nullptr) { return false; }
                    vba->GetValue(std::addressof(bin));
                    *out_alignment = bin.alignment;
                }
                *out_binary = bin.data;
                *out_size = bin.size;
                return true;
            }
            bool TryGetBinaryDataByHash(void **out_binary, u32 *out_size, u32 *out_alignment, u64 hash) const {
                impl::YamlBinaryData bin;
                YamlValueBase *value = nullptr;
                if (!this->GetYamlValueByHash(std::addressof(value), reinterpret_cast<const u32*>(std::addressof(hash)))) { return false; }
                YamlValueBinaryData *vb = vp::util::DynamicCast<YamlValueBinaryData*>(value);
                if (vb != nullptr) {
                    vb->GetValue(std::addressof(bin));
                    *out_alignment = 0;
                } else {
                    YamlValueBinaryDataWithAlignment *vba = vp::util::DynamicCast<YamlValueBinaryDataWithAlignment*>(value);
                    if (vba == nullptr) { return false; }
                    vba->GetValue(std::addressof(bin));
                    *out_alignment = bin.alignment;
                }
                *out_binary = bin.data;
                *out_size = bin.size;
                return true;
            }
            bool TryGetBinaryDataByIterator(void **out_binary, u32 *out_size, u32 *out_alignment) const {
                impl::YamlBinaryData bin;
                YamlValueBase *value = m_value;
                YamlValueBinaryData *vb = vp::util::DynamicCast<YamlValueBinaryData*>(value);
                if (vb != nullptr) {
                    vb->GetValue(std::addressof(bin));
                    *out_alignment = 0;
                } else {
                    YamlValueBinaryDataWithAlignment *vba = vp::util::DynamicCast<YamlValueBinaryDataWithAlignment*>(value);
                    if (vba == nullptr) { return false; }
                    vba->GetValue(std::addressof(bin));
                    *out_alignment = bin.alignment;
                }
                *out_binary = bin.data;
                *out_size = bin.size;
                return true;
            }

            bool TryGetStringByKey(const char **out_value, const char *key) const {
                vp::util::HeapString8 str;
                if (!this->GetValueByKey<YamlValueString>(std::addressof(str), key)) { return false; }
                *out_value = str.GetString();
                return true;
            }
            bool TryGetStringByIndex(const char **out_value, u32 index) const {
                vp::util::HeapString8 str;
                if (!this->GetValueByIndex<YamlValueString>(std::addressof(str), index)) { return false; }
                *out_value = str.GetString();
                return true;
            }
            bool TryGetStringByHash(const char **out_value, u32 hash) const {
                vp::util::HeapString8 str;
                if (!this->GetValueByHash<YamlValueString>(std::addressof(str), std::addressof(hash))) { return false; }
                *out_value = str.GetString();
                return true;
            }
            bool TryGetStringByHash(const char **out_value, u64 hash) const {
                vp::util::HeapString8 str;
                if (!this->GetValueByHash<YamlValueString>(std::addressof(str), reinterpret_cast<const u32*>(std::addressof(hash)))) { return false; }
                *out_value = str.GetString();
                return true;
            }
            bool TryGetStringByIterator(const char **out_value) const {
                vp::util::HeapString8 str;
                if (!this->GetValueByIterator<YamlValueString>(std::addressof(str))) { return false; }
                *out_value = str.GetString();
                return true;
            }

            YamlIterator GetIteratorByIndex(u32 index) const {
                YamlValueBase *value = nullptr;
                this->GetYamlValueByIndex(std::addressof(value), index);
                return YamlIterator {value,};
            }
            YamlIterator GetIteratorByKey(const char *key) const {
                YamlValueBase *value = nullptr;
                this->GetYamlValueByKey(std::addressof(value), key);
                return YamlIterator {value,};
            }
            YamlIterator GetIteratorByHash(u32 hash) const {
                YamlValueBase *value = nullptr;
                this->GetYamlValueByHash(std::addressof(value), std::addressof(hash));
                return YamlIterator {value,};
            }
            YamlIterator GetIteratorByHash(u64 hash) const {
                YamlValueBase *value = nullptr;
                this->GetYamlValueByHash(std::addressof(value), reinterpret_cast<const u32*>(std::addressof(hash)));
                return YamlIterator {value,};
            }

            bool TryGetIteratorByKey(YamlIterator *out_iterator, const char *key) const {
                YamlValueBase *value = nullptr;
                if (!this->GetYamlValueByKey(std::addressof(value), key)) { return false; }
                *out_iterator = YamlIterator{value,};
                return true;
            }
            bool TryGetIteratorByIndex(YamlIterator *out_iterator, u32 index) const {
                YamlValueBase *value = nullptr;
                if (!this->GetYamlValueByIndex(std::addressof(value), index)) { return false; }
                *out_iterator = YamlIterator{value,};
                return true;
            }
            bool TryGetIteratorByHash(YamlIterator *out_iterator, u32 hash) const {
                YamlValueBase *value = nullptr;
                if (!this->GetYamlValueByHash(std::addressof(value), std::addressof(hash))) { return false; }
                *out_iterator = YamlIterator{value,};
                return true;
            }
            bool TryGetIteratorByHash(YamlIterator *out_iterator, u64 hash) const {
                YamlValueBase *value = nullptr;
                if (!this->GetYamlValueByHash(std::addressof(value), reinterpret_cast<const u32*>(std::addressof(hash)))) { return false; }
                *out_iterator = YamlIterator{value,};
                return true;
            }
            bool TryGetIteratorByIterator(YamlIterator *out_iter) const {
                *out_iter = YamlIterator{m_value,};
                return true;
            }

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
                    return this->TryGetStringByHash(out_value, hash);
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
                    return this->TryGetStringByHash(out_value, hash);
                }
            }

            template <typename T>
                requires (TGetByamlDataTypeValue<T>() != ByamlDataType::Null)
            ALWAYS_INLINE bool TryGetValueByIterator(T *out_value) const {

                /* Lookup required function at compile time */
                const ByamlDataType data_type = TGetByamlDataTypeValue<T>();

                if constexpr (data_type == ByamlDataType::Bool) {
                    return this->TryGetBoolByIterator(out_value);
                } else if constexpr (data_type == ByamlDataType::S32) {
                    return this->TryGetS32ByIterator(out_value);
                } else if constexpr (data_type == ByamlDataType::F32) {
                    return this->TryGetF32ByIterator(out_value);
                } else if constexpr (data_type == ByamlDataType::U32) {
                    return this->TryGetU32ByIterator(out_value);
                } else if constexpr (data_type == ByamlDataType::S64) {
                    return this->TryGetS64ByIterator(out_value);
                } else if constexpr (data_type == ByamlDataType::U64) {
                    return this->TryGetU64ByIterator(out_value);
                } else if constexpr (data_type == ByamlDataType::F64) {
                    return this->TryGetF64ByIterator(out_value);
                } else if constexpr (data_type == ByamlDataType::StringIndex) {
                    return this->TryGetStringByIterator(out_value);
                }
            }

            template <typename T>
                requires (!IsBigDataType(TGetByamlDataTypeValue<T>()))
            const T *GetMonoArray(u32 *out_count) {
                YamlArray *array = vp::util::DynamicCast<YamlArray*>(m_value);
                if (array == nullptr) { return nullptr; }
                if (array->GetMonoArrayType() != TGetByamlDataTypeValue<T>()) { return nullptr; }
                *out_count = array->GetDataCount();
                return reinterpret_cast<const T*>(array->GetMonoValueArray());
            }

            bool TryGetIteratorAndKeyByIndex(YamlIterator *out_iter, const char **out_key, u32 index) {
                YamlDictionary *dict = vp::util::DynamicCast<YamlDictionary*>(m_value);
                if (dict == nullptr) { return false; }
                YamlDictionary::Node *node;
                if (!dict->GetNodeByIndex(std::addressof(node), index)) { return false; }
                if (out_iter != nullptr) {
                    *out_iter = YamlIterator{ node->value,};
                }
                if (out_key != nullptr) {
                    *out_key = node->key.GetString();
                }
                return true;
            }

            bool TryGetIteratorAndHashByIndex(YamlIterator *out_iter, u32 *out_key, u32 max_key_size, u32 index) {
                YamlHashArray *hash_array = vp::util::DynamicCast<YamlHashArray*>(m_value);
                if (hash_array == nullptr) { return false; }
                YamlHashArray::Node *node;
                if (!hash_array->GetNodeByIndex(std::addressof(node), index)) { return false; }
                if (max_key_size < node->key_array.GetCount()) { return false; }
                if (out_iter != nullptr) {
                    *out_iter = YamlIterator{ node->value,};
                }
                if (out_key != nullptr) {
                    for (u32 i = 0; i < node->key_array.GetCount(); ++i) {
                        out_key[i] = node->key_array[i];
                    }
                }
                return true;
            }
    };
}

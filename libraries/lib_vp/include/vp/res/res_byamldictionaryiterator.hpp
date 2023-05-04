#pragma once

namespace vp::res {

    namespace {

        ALWAYS_INLINE u32 RemapIndex(void *remap_table, u32 data_count, u32 index) {
            if (data_count < 0x100)   { *reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(remap_table) + index * sizeof(u8)); }
            if (data_count < 0x10000) { *reinterpret_cast<u16*>(reinterpret_cast<uintptr_t>(remap_table) + index * sizeof(u16)); }
            return *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(remap_table) + index * sizeof(u32));
        }
    }

    class ByamlDictionaryIterator {
        private:
            const ResByamlContainer *m_byaml_container;
        public:
            constexpr ALWAYS_INLINE ByamlDictionaryIterator() : m_byaml_container(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE ByamlDictionaryIterator(const ResByamlContainer *container) : m_byaml_container(container) {/*...*/}

            constexpr ALWAYS_INLINE ~ByamlDictionaryIterator() {/*...*/}

            ALWAYS_INLINE bool TryGetDataByIndex(ByamlData *out_data, u32 index) const {

                /* Integrity checks */
                if (m_byaml_container == nullptr) { return false; }

                /* Get data couunt */
                const u32 data_count = m_byaml_container->count;

                /* Bounds check index */
                if (data_count <= index) { return false; }

                /* Remap index if necessary */
                if (static_cast<ByamlDataType>(out_data->data_type) == ByamlDataType::DictionaryWithRemap) {
                    const u32 remap_table_offset = data_count * sizeof(ResByamlDictionaryPair) + sizeof(u32);
                    index = RemapIndex(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml_container) + remap_table_offset), data_count, index);
                }

                /* Get dictionary pair by index */
                const ResByamlDictionaryPair *pair = reinterpret_cast<const ResByamlDictionaryPair*>(reinterpret_cast<uintptr_t>(m_byaml_container) + sizeof(ResByamlContainer) + sizeof(ResByamlDictionaryPair) * index);
                out_data->SetPair(pair);

                return true;
            }

            ALWAYS_INLINE const ResByamlDictionaryPair *GetDictionaryPairByIndex(u32 index) const {

                /* Integrity checks */
                if (m_byaml_container == nullptr || m_byaml_container->count <= index) { return nullptr; }

                /* Get dictionary pair */
                return reinterpret_cast<const ResByamlDictionaryPair*>(reinterpret_cast<uintptr_t>(m_byaml_container) + sizeof(ResByamlContainer) + sizeof(ResByamlDictionaryPair) * index);
            }

            constexpr ALWAYS_INLINE u32 GetSize() const {
                return m_byaml_container->count;
            }
    };

    class ByamlArrayIterator {
        private:
            const ResByamlContainer *m_byaml_container;
        public:
            constexpr ALWAYS_INLINE ByamlArrayIterator() : m_byaml_container(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE ByamlArrayIterator(const ResByamlContainer *container) : m_byaml_container(container) {/*...*/}

            constexpr ALWAYS_INLINE ~ByamlArrayIterator() {/*...*/}

            ALWAYS_INLINE bool TryGetDataByIndex(ByamlData *out_data, u32 index) const {

                /* Integrity checks */
                if (m_byaml_container == nullptr || m_byaml_container->count <= index) { return false; }

                /* Select data type index */
                u32 data_type_index    = index;
                u32 value_array_offset = util::AlignUp(m_byaml_container->count, 4);
                if (static_cast<ByamlDataType>(m_byaml_container->data_type) == ByamlDataType::MonoTypedArray) {
                    data_type_index    = 1;
                    value_array_offset = sizeof(u32);
                }

                /* Calculate offsets */
                const u32 data_type_offset = sizeof(ResByamlContainer) + data_type_index;
                const u32 value_offset     = sizeof(ResByamlContainer) + value_array_offset + sizeof(u32) * index;

                /* Apply offsets */
                const u8  type  = *reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(m_byaml_container) + data_type_offset);
                const u32 value = *reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_byaml_container) + value_offset);                

                /* Set data */
                out_data->key_index = 0xff'ffff;
                out_data->data_type = type;
                out_data->u32_value = value;

                return true;
            }

            constexpr ALWAYS_INLINE u32 GetSize() const {
                return m_byaml_container->count;
            }
    };

    class ByamlHashArrayIterator {
        private:
            const ResByamlContainer *m_byaml_container;
            u32                      m_stride;
        public:
            constexpr ALWAYS_INLINE ByamlHashArrayIterator(const ResByamlContainer *container) : m_byaml_container(container), m_stride(container->data_type & 0xf * sizeof(u32) + sizeof(ResByamlContainer)) {/*...*/}

            constexpr ALWAYS_INLINE ~ByamlHashArrayIterator() {/*...*/}

            ALWAYS_INLINE bool TryGetDataByIndex(ByamlData *out_data, u32 index) const {

                /* Integrity check */
                if (m_byaml_container == nullptr) { return false; }

                /* Get array size */
                const u32 data_count = m_byaml_container->count;

                /* Bounds check index */
                if (data_count <= index) { return false; }

                /* Remap index if necessary */
                if (static_cast<ByamlDataType>(m_byaml_container->data_type & 0xf0) == ByamlDataType::HashArrayWithRemap) {
                    const u32 remap_table_offset = util::AlignUp((m_stride + sizeof(u32) + sizeof(u8)) * data_count, sizeof(u32));
                    index = RemapIndex(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_byaml_container) + remap_table_offset), data_count, index);
                }

                /* Calculate data offsets */
                const u32 data_type_offset = (m_stride + sizeof(u32)) * data_count + index * sizeof(u8);
                const u32 value_offset     = (m_stride + sizeof(u32)) * index + m_stride + sizeof(u32);

                /* Set data */
                out_data->key_index = index;
                out_data->data_type = *reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(m_byaml_container) + data_type_offset);
                out_data->u32_value = *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_byaml_container) + value_offset);

                return true;
            }
    };
}

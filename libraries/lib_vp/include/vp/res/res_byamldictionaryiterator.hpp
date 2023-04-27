#pragma once

namespace vp::res {

    class ByamlDictionaryIterator {
        private:
            const ResByamlContainer *m_dic_container;
        public:
            constexpr ALWAYS_INLINE ByamlDictionaryIterator() : m_dic_container(nullptr) {/*...*/}
            ALWAYS_INLINE ByamlDictionaryIterator(const unsigned char *container) : m_dic_container(reinterpret_cast<const ResByamlContainer*>(container)) {/*...*/}

            ALWAYS_INLINE bool TryGetDataByIndex(ByamlData *out_data, u32 index) const {

                /* Integrity checks */
                if (m_dic_container == nullptr || index < m_dic_container->count) { return false; }

                /* Get dictionary pair by index */
                const ResByamlDictionaryPair *pair = reinterpret_cast<const ResByamlDictionaryPair*>(reinterpret_cast<uintptr_t>(m_dic_container) + sizeof(ResByamlContainer) + sizeof(ResByamlDictionaryPair) * index);
                out_data->SetPair(pair);

                return true;
            }

            ALWAYS_INLINE const ResByamlDictionaryPair *GetDictionaryPairByIndex(u32 index) const {

                /* Integrity checks */
                if (m_dic_container == nullptr || m_dic_container->count <= index) { return nullptr; }

                /* Get dictionary pair */
                return reinterpret_cast<const ResByamlDictionaryPair*>(reinterpret_cast<uintptr_t>(m_dic_container) + sizeof(ResByamlContainer) + sizeof(ResByamlDictionaryPair) * index);
            }

            constexpr ALWAYS_INLINE u32 GetSize() const {
                return m_dic_container->count;
            }
    };

    class ByamlArrayIterator {
        private:
            const ResByamlContainer *m_array_container;
        public:
            constexpr ALWAYS_INLINE ByamlArrayIterator() : m_array_container(nullptr) {/*...*/}
            ALWAYS_INLINE ByamlArrayIterator(const unsigned char *container) : m_array_container(reinterpret_cast<const ResByamlContainer*>(container)) {/*...*/}

            ALWAYS_INLINE bool TryGetDataByIndex(ByamlData *out_data, u32 index) const {

                /* Integrity checks */
                if (m_array_container == nullptr || m_array_container->count <= index) { return false; }

                /* Get dictionary pair by index */
                const u32 value = *reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_array_container) + sizeof(ResByamlContainer) + util::AlignUp(m_array_container->count, 4) + sizeof(u32) * index);
                const u8  type  = *reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(m_array_container) + sizeof(ResByamlContainer) + index);
                out_data->data_type = type;
                out_data->key_index = 0xff'ffff;
                out_data->u32_value = value;

                return true;
            }

            constexpr ALWAYS_INLINE u32 GetSize() const {
                return m_array_container->count;
            }
    };
}

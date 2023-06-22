#pragma once

namespace vp::res {
    
    class ByamlStringTableIterator {
        private:
            const ResByamlContainer *m_byaml_container;
        public:
            constexpr ALWAYS_INLINE ByamlStringTableIterator() : m_byaml_container(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE ByamlStringTableIterator(const ResByamlContainer *string_table) : m_byaml_container(string_table) {/*...*/}
            constexpr ~ByamlStringTableIterator() {/*...*/}

            /* Note; binary search is only valid for the key table and not the string table */
            ALWAYS_INLINE u32 FindKeyIndexByKey(const char *key) const {

                /* Handle key table relocation */
                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(m_byaml_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 offset_offset = *reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_byaml_container) +  sizeof(ResByamlContainer));
                    key_pool_offset         = *reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(m_byaml_container) +  offset_offset);
                }

                /* Binary search for string */
                u32 size = m_byaml_container->count;
                u32 i = 0;
                u32 index = 0;
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    const char *table_string = this->GetStringByIndex(index, key_pool_offset);
                    const s32 result = ::strcmp(key, table_string);
                    
                    if (result == 0) {
                        return index;
                    }
                    if (0 < result) {
                        i = index + 1;
                        index = size;
                    }
                    size = index;
                }

                return static_cast<u32>(-1);
            }

            ALWAYS_INLINE const char *GetStringByIndex(u32 index, u64 offset) const {
                const u32 *offset_table = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_byaml_container) + sizeof(ResByamlContainer));
                return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(m_byaml_container) + offset + offset_table[index]);
            }

            ALWAYS_INLINE u32 GetStringSizeByIndex(u32 index) const {
                const u32 *offset_table = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_byaml_container) + sizeof(ResByamlContainer));
                const u32 string_offset = offset_table[index];
                const u32 next_offset   = offset_table[index + 1];
                return (next_offset - 1) - string_offset;
            }

            constexpr ALWAYS_INLINE u32 GetStringCount() const {
                return m_byaml_container->count;
            }
    };
}
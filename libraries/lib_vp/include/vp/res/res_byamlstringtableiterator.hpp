#pragma once

namespace vp::res {
    
    class ByamlStringTableIterator {
        private:
            const ResByamlContainer *m_string_table;
        public:
            constexpr ALWAYS_INLINE ByamlStringTableIterator() {/*...*/}
            ALWAYS_INLINE ByamlStringTableIterator(const unsigned char *string_table) : m_string_table(reinterpret_cast<const ResByamlContainer*>(string_table)) {/*...*/}

            ALWAYS_INLINE u32 FindIndexByString(const char *string) const {

                /* Iterate over entire table until string is found */
                const u32 *offset_table = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_string_table) + sizeof(ResByamlContainer));
                for (u32 i = 0; i < m_string_table->count; ++i) {
                    const char *table_string = reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(m_string_table) + offset_table[i]);
                    if (::strcmp(string, table_string) == 0) {
                        return i;
                    }
                }

                return static_cast<u32>(-1);
            }

            ALWAYS_INLINE const char *GetStringByIndex(u32 index) const {
                const u32 *offset_table = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_string_table) + sizeof(ResByamlContainer));
                return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(m_string_table) + offset_table[index]);
            }

            ALWAYS_INLINE u32 GetStringSizeByIndex(u32 index) const {
                const u32 *offset_table = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_string_table) + sizeof(ResByamlContainer));
                const u32 string_offset = offset_table[index];
                const u32 next_offset   = offset_table[index + 1];
                return (next_offset - 1) - string_offset;
            }

            constexpr ALWAYS_INLINE u32 GetStringCount() const {
                return m_string_table->count;
            }
    };
}
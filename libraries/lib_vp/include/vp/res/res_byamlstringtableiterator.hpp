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

    class ByamlStringTableIterator {
        public:
            static constexpr u32 cInvalidKeyIndex = 0xffff'ffff;
        private:
            const ResByamlContainer *m_byaml_container;
        public:
            constexpr ALWAYS_INLINE ByamlStringTableIterator() : m_byaml_container(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE ByamlStringTableIterator(const ResByamlContainer *string_table) : m_byaml_container(string_table) {/*...*/}
            constexpr ~ByamlStringTableIterator() {/*...*/}

            /* Note; binary search is only valid for the key table and not the string table */
            ALWAYS_INLINE u32 TryFindKeyIndexByKey(const char *key, u32 is_reverse_endian) const {

                /* Handle key table relocation */
                u64 key_pool_offset = 0;
                if (static_cast<ByamlDataType>(m_byaml_container->data_type) == ByamlDataType::RelocatedKeyTable) {
                    const u32 *string_table_offset = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_byaml_container) +  sizeof(ResByamlContainer));
                    const u32  offset_offset       = (is_reverse_endian == false) ? *string_table_offset : vp::util::SwapEndian(*string_table_offset);
                    const u64 *key_pool_data       = reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(m_byaml_container) + offset_offset);
                    key_pool_offset                = (is_reverse_endian == false) ? *key_pool_data : vp::util::SwapEndian(*key_pool_data);
                }

                /* Binary search for string */
                u32 size = this->GetStringCount(is_reverse_endian);
                u32 i = 0;
                u32 index = 0;
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    const char *table_string = this->GetStringByIndex(index, key_pool_offset, is_reverse_endian);
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

                return cInvalidKeyIndex;
            }

            ALWAYS_INLINE const char *GetStringByIndex(u32 index, u64 offset, u32 is_reverse_endian) const {
                const u32 *offset_table  = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_byaml_container) + sizeof(ResByamlContainer));
                const u32  string_offset = (is_reverse_endian == false) ? offset_table[index] : vp::util::SwapEndian(offset_table[index]);
                return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(m_byaml_container) + offset + string_offset);
            }

            ALWAYS_INLINE u32 GetStringSizeByIndex(u32 index, u32 is_reverse_endian) const {
                const u32 *offset_table = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(m_byaml_container) + sizeof(ResByamlContainer));
                const u32 string_offset = (is_reverse_endian == false) ? offset_table[index + 0] : vp::util::SwapEndian(offset_table[index + 0]);
                const u32 next_offset   = (is_reverse_endian == false) ? offset_table[index + 1] : vp::util::SwapEndian(offset_table[index + 1]);
                return (next_offset - 1) - string_offset;
            }

            constexpr ALWAYS_INLINE u32 GetStringCount(u32 is_reverse_endian) const {
                return (is_reverse_endian == false) ? static_cast<u32>(m_byaml_container->count) : vp::util::SwapEndian24(static_cast<u32>(m_byaml_container->count));
            }
    };
}
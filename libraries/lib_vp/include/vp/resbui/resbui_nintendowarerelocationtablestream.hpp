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

namespace vp::resbui {

    class NintendoWareRelocationTableStream {
        private:
            vp::res::ResNintendoWareRelocationTable             *m_header;
            vp::res::ResNintendoWareRelocationTable::ResSection *m_section_array;
            vp::res::ResNintendoWareRelocationTable::ResEntry   *m_entry_array;
            u32                                                  m_section_count;
            u32                                                  m_section_max;
            u32                                                  m_entry_count;
            u32                                                  m_entry_max;
        public:
            constexpr  NintendoWareRelocationTableStream() : m_header(), m_section_array(nullptr), m_entry_array(nullptr), m_section_count(0), m_section_max(0), m_entry_count(0), m_entry_max(0) {/*...*/}
            constexpr ~NintendoWareRelocationTableStream() {/*...*/}

            void Initialize(void *output_buffer, u32 max_sections, u32 max_entries) {

                /* Set state */
                m_header        = reinterpret_cast<vp::res::ResNintendoWareRelocationTable*>(output_buffer);
                m_section_array = reinterpret_cast<vp::res::ResNintendoWareRelocationTable::ResSection*>(reinterpret_cast<uintptr_t>(output_buffer) + sizeof(vp::res::ResNintendoWareRelocationTable));
                m_entry_array   = reinterpret_cast<vp::res::ResNintendoWareRelocationTable::ResEntry*>(reinterpret_cast<uintptr_t>(output_buffer) + sizeof(vp::res::ResNintendoWareRelocationTable) + sizeof(vp::res::ResNintendoWareRelocationTable::ResSection) * max_sections);
                m_section_count = 0;
                m_section_max   = max_sections;
                m_entry_count   = 0;
                m_entry_max     = max_entries;
            }

            Result StartSection(u32 region_offset, u32 region_size) {

                /* Integrity checks */
                RESULT_RETURN_IF(m_section_count == m_section_max, ResultSectionExhaustion);

                const u32 i = m_section_count;

                /* Update last section state */
                if (0 < i) {
                    m_section_array[i - 1].entry_count = m_entry_count - m_section_array[i - 1].base_entry_index;
                }

                /* Set new section state */
                m_section_array[i].base_pointer     = nullptr;
                m_section_array[i].region_offset    = region_offset;
                m_section_array[i].region_size      = region_size;
                m_section_array[i].base_entry_index = m_entry_count;

                /* Increment section index */
                ++m_section_count;

                RESULT_RETURN_SUCCESS;
            }
            Result AddEntry(u32 region_offset, u16 array_count, u8 relocation_count, u8 array_stride) {

                /* Integrity checks */
                RESULT_RETURN_IF(m_entry_count == m_entry_max, ResultEntryExhaustion);

                const u32 i = m_entry_count;

                /* Set new entry state */
                m_entry_array[i].region_offset    = region_offset;
                m_entry_array[i].array_count      = array_count;
                m_entry_array[i].relocation_count = relocation_count;
                m_entry_array[i].array_stride     = array_stride;

                /* Increment entry index */
                ++m_entry_count;

                RESULT_RETURN_SUCCESS;
            }

            static constexpr size_t CalculateRelocationTableSize(u32 max_sections, u32 max_entries) {
                return sizeof(vp::res::ResNintendoWareRelocationTable) + sizeof(vp::res::ResNintendoWareRelocationTable::ResSection) * max_sections + sizeof(vp::res::ResNintendoWareRelocationTable::ResEntry) * max_entries;
            }
    };
}

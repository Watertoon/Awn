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
            void                                                *m_file_head;
            vp::res::ResNintendoWareRelocationTable             *m_header;
            vp::res::ResNintendoWareRelocationTable::ResSection *m_section_array;
            vp::res::ResNintendoWareRelocationTable::ResEntry   *m_entry_array;
            u32                                                  m_section_count;
            u32                                                  m_section_max;
            u32                                                  m_entry_count;
            u32                                                  m_entry_max;
        public:
            constexpr  NintendoWareRelocationTableStream() : m_file_head(), m_header(), m_section_array(), m_entry_array(), m_section_count(), m_section_max(), m_entry_count(), m_entry_max() {/*...*/}
            constexpr ~NintendoWareRelocationTableStream() {/*...*/}

            void SetStream(void *file_head, void *table_location, u32 max_sections, u32 max_entries);
            void EndStream();

            Result StartSection(u32 region_offset, u32 region_size);
            Result AddEntry(void *region_offset_ptr, u16 array_count, u8 relocation_count, u8 array_stride);
            Result AddEntry(u32 region_offset, u16 array_count, u8 relocation_count, u8 array_stride);

            static constexpr size_t CalculateRelocationTableSize(u32 max_sections, u32 max_entries) {
                return sizeof(vp::res::ResNintendoWareRelocationTable) + sizeof(vp::res::ResNintendoWareRelocationTable::ResSection) * max_sections + sizeof(vp::res::ResNintendoWareRelocationTable::ResEntry) * max_entries;
            }
    };
}

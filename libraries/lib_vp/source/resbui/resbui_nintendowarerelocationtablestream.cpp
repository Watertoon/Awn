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
#include <vp.hpp>

namespace vp::resbui {

    void NintendoWareRelocationTableStream::SetStream(void *file_head, void *table_location, u32 max_sections, u32 max_entries) {

        /* Set state */
        m_file_head     = file_head;
        m_header        = reinterpret_cast<vp::res::ResNintendoWareRelocationTable*>(table_location);
        m_section_array = reinterpret_cast<vp::res::ResNintendoWareRelocationTable::ResSection*>(reinterpret_cast<uintptr_t>(table_location) + sizeof(vp::res::ResNintendoWareRelocationTable));
        m_entry_array   = reinterpret_cast<vp::res::ResNintendoWareRelocationTable::ResEntry*>(reinterpret_cast<uintptr_t>(table_location) + sizeof(vp::res::ResNintendoWareRelocationTable) + sizeof(vp::res::ResNintendoWareRelocationTable::ResSection) * max_sections);
        m_section_count = 0;
        m_section_max   = max_sections;
        m_entry_count   = 0;
        m_entry_max     = max_entries;

        /* Set header */
        m_header->magic = vp::res::ResNintendoWareRelocationTable::cMagic;
        m_header->offset_from_header = static_cast<u32>(reinterpret_cast<uintptr_t>(table_location) - reinterpret_cast<uintptr_t>(file_head));
        m_header->section_count = max_sections;

        return;
    }

    void NintendoWareRelocationTableStream::EndStream() {
        const u32 i = m_section_count - 1;
        m_section_array[i].entry_count = m_entry_count - m_section_array[i].base_entry_index;
    }

    Result NintendoWareRelocationTableStream::StartSection(u32 region_offset, u32 region_size) {

        /* Integrity checks */
        const u32 max_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(m_header) - reinterpret_cast<uintptr_t>(m_file_head));
        RESULT_RETURN_UNLESS(region_offset + region_size <= max_offset, ResultOutOfBoundsOffset);
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

    Result NintendoWareRelocationTableStream::AddEntry(void *region_offset_ptr, u16 array_count, u8 relocation_count, u8 array_stride) {
        const u32 region_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(region_offset_ptr) - reinterpret_cast<uintptr_t>(m_file_head));
        return this->AddEntry(region_offset, array_count, relocation_count, array_stride);
    }
    Result NintendoWareRelocationTableStream::AddEntry(u32 region_offset, u16 array_count, u8 relocation_count, u8 array_stride) {

        /* Integrity checks */
        const u32 max_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(m_header) - reinterpret_cast<uintptr_t>(m_file_head));
        RESULT_RETURN_UNLESS(region_offset <= max_offset, ResultOutOfBoundsOffset);
        RESULT_RETURN_IF(m_entry_count == m_entry_max, ResultEntryExhaustion);

        /* Set new entry state */
        const u32 i = m_entry_count;
        m_entry_array[i].region_offset    = region_offset;
        m_entry_array[i].array_count      = array_count;
        m_entry_array[i].relocation_count = relocation_count;
        m_entry_array[i].array_stride     = array_stride;

        /* Increment entry index */
        m_entry_count = i + 1;

        RESULT_RETURN_SUCCESS;
    }
}

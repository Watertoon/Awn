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

    struct ResNintendoWareFileHeader;

	struct ResNintendoWareRelocationTable {
        u32 magic;
        u32 this_table_offset;
        u32 section_count;
        u32 reserve0;

        static constexpr u32 Magic = util::TCharCode32("_RLT");

        struct ResSection {
            void *base_pointer;
            u32   region_offset;
            u32   region_size;
            u32   base_entry_index;
            u32   entry_count;
        };
        static_assert(sizeof(ResSection) == 0x18);

        struct ResEntry {
            u32 region_offset;
            u16 array_count;
            u8  relocation_count;
            u8  array_stride;
        };
        static_assert(sizeof(ResEntry) == 0x8);

        static u64 CalculateTableSize(s32 sections, s32 entries) { 
            return sections * sizeof(ResSection) + entries * sizeof(ResEntry) + sizeof(ResNintendoWareRelocationTable); 
        }

        ALWAYS_INLINE u64 GetEntryTableOffset() {
            return sizeof(ResNintendoWareRelocationTable) + section_count * sizeof(ResSection);
        }

        ResNintendoWareFileHeader *GetFileHeader();

        ResSection *GetSection(s32 section_index) { 
            return reinterpret_cast<ResSection*>(reinterpret_cast<uintptr_t>(this) + sizeof(ResNintendoWareRelocationTable) + section_index * sizeof(ResSection)); 
        }

        const ResSection *GetSection(s32 section_index) const { 
            return reinterpret_cast<const ResSection*>(reinterpret_cast<uintptr_t>(this) + sizeof(ResNintendoWareRelocationTable) + section_index * sizeof(ResSection)); 
        }

        void SetSignature() { magic = Magic; }

        void Relocate();
        void Unrelocate();
	};
}

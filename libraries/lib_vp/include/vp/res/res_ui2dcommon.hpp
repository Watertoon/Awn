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

    struct ResUi2dSection {
        u32 magic;
        u32 section_size;

        ALWAYS_INLINE ResUi2dSection *GetNextSection() {
            return reinterpret_cast<ResUi2dSection*>(reinterpret_cast<uintptr_t>(this) + section_size);
        }
    };
    static_assert(sizeof(ResUi2dSection) == 0x8);

    struct ResUi2dHeader {
        u32 magic;
        u16 endianess;
        u16 header_size;
        u16 micro_version;
        u8  minor_version;
        u8  major_version;
        u32 file_size;
        u16 section_count;
        u16 reserve0;

        ALWAYS_INLINE ResUi2dSection *GetFirstSection() {
            return reinterpret_cast<ResUi2dSection*>(reinterpret_cast<uintptr_t>(this) + header_size);
        }

        static constexpr inline u32 BflanMagic = util::TCharCode32("FLAN");
    };
    static_assert(sizeof(ResUi2dHeader) == 0x14);

    enum class Ui2dUserDataType {
        String     = 0,
        S32        = 1,
        Float      = 2,
        SystemData = 3,
    };

    struct ResUi2dUserData {
        u32 name_offset;
        u32 data_array_offset;
        u16 data_count;
        u8  data_type;
        u8  reserve0;
    
        ALWAYS_INLINE const char *GetName() const {
            return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(this) + name_offset);
        }

        ALWAYS_INLINE void *GetDataArray() {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + data_array_offset);
        }
    };
    static_assert(sizeof(ResUi2dUserData) == 0xc);

    struct ResUi2dUserDataList : public ResUi2dSection {
        u16 user_data_count;
        u16 reserve0;
        ResUi2dUserData user_data_array[];

        static constexpr inline u32 Magic = util::TCharCode32("usd1");
    };
    static_assert(sizeof(ResUi2dUserDataList) == 0xc);
}

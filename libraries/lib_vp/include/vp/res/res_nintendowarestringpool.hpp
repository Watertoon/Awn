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

    struct ResNintendoWareStringPool {
        u32   magic;
        u32   reserve0;
        u32   section_size;
        u32   reserve1;
        u32   string_count;
        u32   empty_string;
        char  string_array[];

        static constexpr u32 cMagic = util::TCharCode32("_STR");

        const char *FindString(const char *string) {

            /* Handle empty string */
            if (string == nullptr || *string == '\0') {
                return reinterpret_cast<char*>(std::addressof(empty_string));
            }

            /* Find string by string */
            const u32   string_size = ::strlen(string);
            const char *string_iter = string_array;
            for (u32 i = 0; i < string_count; ++i) {
                const char *ret = string_iter;
                const u16 table_size = *reinterpret_cast<const u16*>(string_iter);
                string_iter = string_iter + 2;
                if (table_size == string_size && ::strcmp(string_iter, string) == 0) { return ret; }
                string_iter = vp::util::AlignUp(string_iter + table_size + 1, alignof(u16));
            }

            return reinterpret_cast<char*>(std::addressof(empty_string));
        }
    };
    static_assert(sizeof(ResNintendoWareStringPool) == 0x18);
}

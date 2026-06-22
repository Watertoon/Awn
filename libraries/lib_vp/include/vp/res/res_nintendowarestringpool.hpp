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

        const NWString8 *FindString(const char *string, u32 len) const {

            /* Handle empty string */
            if (string == nullptr || *string == '\0') {
                return reinterpret_cast<const NWString8*>(std::addressof(empty_string));
            }

            /* Find string by string */
            uintptr_t string_iter = reinterpret_cast<uintptr_t>(string_array);
            for (u32 i = 0; i < string_count; ++i) {
                const NWString8 *find_string = reinterpret_cast<const NWString8*>(string_iter);
                const char *f_str = find_string->GetString();
                const u16 f_len = find_string->GetLength();
                if (f_len == len && std::memcmp(f_str, string, len) == 0) { return find_string; }
                string_iter = vp::util::AlignUp(string_iter + f_len + 3, alignof(u16));
            }

            return reinterpret_cast<const NWString8*>(std::addressof(empty_string));
        }
        template <typename T>
            requires (vp::util::IsString8OrRaw<T>)
        const NWString8 *FindString(const T *str) const {
            const char *string;
            u32 len;
            vp::util::GetUnderlyingString(std::addressof(string), std::addressof(len), str);
            return this->FindString(string, len);
        }
    };
    static_assert(sizeof(ResNintendoWareStringPool) == 0x18);
}

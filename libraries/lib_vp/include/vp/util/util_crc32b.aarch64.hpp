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

namespace vp::util {

    constexpr inline u32 cCrc32bSeed = 0xedb88320;

    consteval ALWAYS_INLINE std::array<u32, 0x100> GeneratecCrc32bTable() {
        std::array<u32, 0x100> context = {};
        
        for (u32 i = 0; i < 0x100; ++i) {
            u32 seed = (i >> 1) & 0x7fff'ffff;
            if ((i & 1) != 0) {
                seed = seed ^ cCrc32bSeed;
            }
            for (u32 u = 0; u < 7; ++u) {
                if ((seed & 1) != 0) {
                    seed = (seed >> 1) ^ cCrc32bSeed;
                } else {
                    seed = seed >> 1;
                }
            }
            context[i] = seed;
        }
        return context;
    }

    constexpr inline std::array<u32, 0x100> cCrc32bTable = GeneratecCrc32bTable();

    constexpr u32 HashCrc32bWithContext(u32 context, const char *string) {

        u32 seed = context;
        while (*string != '\0') {
            seed = cCrc32bTable[(*string ^ seed) & 0xff] ^ seed >> 8;
            string = string + 1;
        }
        
        return ~seed;
    }

    constexpr ALWAYS_INLINE u32 HashCrc32b(const char *string) {
        return HashCrc32bWithContext(0xffff'ffff, string);
    }

    consteval ALWAYS_INLINE u32 THashDataCrc32bWithContext(u32 context, u8 *data, size_t data_size) {

        u32 seed = context;
        while (data_size != 0) {
            seed = cCrc32bTable[(*data ^ seed) & 0xff] ^ seed >> 8;
            data = data + 1;
            data_size = data_size - 1;
        }

        return ~seed;
    }

    consteval ALWAYS_INLINE u32 THashDataCrc32b(u8 *data, size_t data_size) {
        return THashDataCrc32bWithContext(0xffff'ffff, data, data_size);
    }

    ALWAYS_INLINE u32 HashDataCrc32bWithContext(u32 context, void *data, size_t data_size) {

        u32 seed = context;
        const u8 *iter = reinterpret_cast<const u8*>(data);

        /* Fallback for rest of string */
        u32 count = data_size;
        while (count != 0) {
            seed = cCrc32bTable[(*iter ^ seed) & 0xff] ^ seed >> 8;

            iter = iter + 1;
            count = count - 1;
        }

        return ~seed;
    }

    ALWAYS_INLINE u32 HashDataCrc32b(void *data, size_t data_size) {
        return HashDataCrc32bWithContext(0xffff'ffff, data, data_size);
    }
}

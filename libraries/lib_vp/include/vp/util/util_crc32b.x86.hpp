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

	constexpr ALWAYS_INLINE u32 Crc32b_32(u32 value, u32 accumulate) {

        const v2sll constant = { static_cast<s64>(0xB4E5B025F7011641), static_cast<s64>(0x00000001DB710641), };
        v2sll state = {static_cast<s64>(value ^ accumulate) << 32, 0ull};

        state = CarryLessMultiply2(state, constant, CarryLessMultiply2SelectionMask(false, false));
        state = CarryLessMultiply2(state, constant, CarryLessMultiply2SelectionMask(false, true));

        return static_cast<u32>(state[1]);
    }

    constexpr ALWAYS_INLINE u32 Crc32b_64(u64 value, u32 accumulate) {

        const v2sll constant = { static_cast<s64>(0xB4E5B025F7011641), static_cast<s64>(0x00000001DB710641), };
        v2sll state = {static_cast<s64>(value ^ accumulate) << 32, 0ull};

        state = CarryLessMultiply2(state, constant, CarryLessMultiply2SelectionMask(false, false));
        state = CarryLessMultiply2(state, constant, CarryLessMultiply2SelectionMask(false, true));

        return static_cast<u32>(state[1]);
    }

    constexpr inline u32 cCrc32bSeed = 0xedb88320;

    consteval ALWAYS_INLINE std::array<u32, 0x100> GenerateCrc32bTable() {
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

    constexpr inline std::array<u32, 0x100> cCrc32bTable = GenerateCrc32bTable();

    constexpr u32 HashCrc32bWithContext(u32 context, const char *string) {
        if (std::is_constant_evaluated() == true) {

            u32 seed = context;
            while (*string != '\0') {
                seed = cCrc32bTable[(*string ^ seed) & 0xff] ^ seed >> 8;
                string = string + 1;
            }
            
            return ~seed;
        } else {

            /* Setup context */
            u32       seed           = context;
            const u32 data_size      = StringLength(string);
            const u32 unaligned_size = (data_size & 7);
            const u32 aligned_size   = (data_size >> 3);

            /* Unaligned crc32b */
            const u8 *unaligned_iter = reinterpret_cast<const u8*>(string);
            for (u32 i = 0; i < unaligned_size; ++i) {
                seed = cCrc32bTable[(unaligned_iter[i] ^ seed) & 0xff] ^ seed >> 8;
            }

            /* Aligned crc32b */
            const u64 *data_iter = reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(string) + unaligned_size);
            for (u32 i = 0; i < aligned_size; ++i) {
                const u64 val = data_iter[i];
                seed = Crc32b_64(val, seed);
            }
            
            return ~seed;
        }
    }

    constexpr ALWAYS_INLINE u32 HashCrc32b(const char *string) {
        return HashCrc32bWithContext(0xffff'ffff, string);
    }
    static_assert(vp::util::HashCrc32b("TestString") == 0x57a9fed8);

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

        /* Setup context */
        u32       seed           = context;
        const u32 unaligned_size = (data_size & 7);
        const u32 aligned_size   = (data_size >> 3);

        /* Unaligned crc32b */
        const u8 *unaligned_iter = reinterpret_cast<const u8*>(data);
        for (u32 i = 0; i < unaligned_size; ++i) {
            seed = cCrc32bTable[(unaligned_iter[i] ^ seed) & 0xff] ^ seed >> 8;
        }

        /* Aligned crc32b */
        const u64 *data_iter = reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(data) + unaligned_size);
        for (u32 i = 0; i < aligned_size; ++i) {
            const u64 val = data_iter[i];
            seed = Crc32b_64(val, seed);
        }

        return ~seed;
    }

    ALWAYS_INLINE u32 HashDataCrc32b(void *data, size_t data_size) {
        return HashDataCrc32bWithContext(0xffff'ffff, data, data_size);
    }
}

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

    consteval u16 TCharCode16(const char *string) {
        const size_t size = TStringLength(string, 3);
        if (size != 2 || std::endian::native == std::endian::big) {
            _consteval_fail();
        }

        const u32 c1 = *(string);
        const u32 c2 = *(string + 1);
        return (c1) | (c2 << 8);
    }

    consteval u32 TCharCode32(const char *string) {
        const size_t size = TStringLength(string, 5);
        if (size != 4 || std::endian::native == std::endian::big) {
            _consteval_fail();
        }

        const u32 c1 = *(string);
        const u32 c2 = *(string + 1);
        const u32 c3 = *(string + 2);
        const u32 c4 = *(string + 3);
        return (c1) | (c2 << 8) | (c3 << 16) | (c4 << 24);
    }

    consteval u64 TCharCode64(const char *string) {
        const size_t size = TStringLength(string, 9);
        if (size != 8 || std::endian::native == std::endian::big) {
            _consteval_fail();
        }

        const u64 c1 = *(string);
        const u64 c2 = *(string + 1);
        const u64 c3 = *(string + 2);
        const u64 c4 = *(string + 3);
        const u64 c5 = *(string + 4);
        const u64 c6 = *(string + 5);
        const u64 c7 = *(string + 6);
        const u64 c8 = *(string + 7);
        return (c1) | (c2 << 8) | (c3 << 16) | (c4 << 24) | (c5 << 32) | (c6 << 40) | (c7 << 48) | (c8 << 56);
    }

    static_assert(TCharCode16("BY") == 0x5942);
    static_assert(TCharCode32("BNSH") == 0x48534e42);
    static_assert(TCharCode64("FRES    ") == 0x2020202053455246);
}

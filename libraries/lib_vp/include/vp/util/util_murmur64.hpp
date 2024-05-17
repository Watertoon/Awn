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

    namespace {
        constexpr u64 TLoadU64FromString(const char *string) {
            return (static_cast<u64>(string[0]) << 0x0) | (static_cast<u64>(string[1]) << 0x8) | (static_cast<u64>(string[2]) << 0x10) | (static_cast<u64>(string[3]) << 0x18) |
                   (static_cast<u64>(string[4]) << 0x20) | (static_cast<u64>(string[5]) << 0x28) | (static_cast<u64>(string[6]) << 0x30) | (static_cast<u64>(string[7]) << 0x38);
        }
    }

    constexpr inline u64 cMurmur64Constant = 0xc6a4a7935bd1e995;
    constexpr inline u64 cMurmur64Seed     = 0xa9f63e6017234875;

    constexpr u64 HashMurmur64(const char *string) {

        /* Shift constant */
        constexpr u32 cShift = 47;

        /* Get size of string */
        const size_t size = (string != nullptr) ? StringLength(string) : 0;

        /* Compute initial hash state */
        u64 state = (size * cMurmur64Constant) ^ cMurmur64Seed;

        /* Perform 8 byte aligned version of Murmur */
        const size_t  aligned_size    = (size & (~7));
        const char   *string_leftover = string + aligned_size;
        if (aligned_size != 0) {
            while (string != string_leftover) {
                const u64 val = (std::is_constant_evaluated() == false) ? *reinterpret_cast<const u64*>(string) : TLoadU64FromString(string);
                state = (((((val * cMurmur64Constant) ^ ((val * cMurmur64Constant) >> cShift)) * cMurmur64Constant)) ^ state) * cMurmur64Constant;
                string = string + 8;
            }
        }

        /* Finish out unaligned bytes*/
        switch (size & 7) {
            case 7:
                state = state ^ (static_cast<u64>(string_leftover[6]) << 0x30);
                [[fallthrough]];
            case 6:
                state = state ^ (static_cast<u64>(string_leftover[5]) << 0x28);
                [[fallthrough]];
            case 5:
                state = state ^ (static_cast<u64>(string_leftover[4]) << 0x20);
                [[fallthrough]];
            case 4:
                state = state ^ (static_cast<u64>(string_leftover[3]) << 0x18);
                [[fallthrough]];
            case 3:
                state = state ^ (static_cast<u64>(string_leftover[2]) << 0x10);
                [[fallthrough]];
            case 2:
                state = state ^ (static_cast<u64>(string_leftover[1]) << 0x8);
                [[fallthrough]];
            case 1:
                state = (state ^ string_leftover[0]) * cMurmur64Constant;
                [[fallthrough]];
            case 0:
            default:
                break;
        };

        state = (state ^ (state >> cShift)) * cMurmur64Constant;
        return (state ^ (state >> cShift));
    }

    constexpr ALWAYS_INLINE u64 DualHashMurmur64(const char *string0, const char *string1) {
        const u64 hash0 = HashMurmur64(string0);
        const u64 hash1 = HashMurmur64(string1);
        const u64 asj_hash1 = (hash0 != hash1) ? hash1 : 0;
        return hash0 ^ asj_hash1;
    }
}

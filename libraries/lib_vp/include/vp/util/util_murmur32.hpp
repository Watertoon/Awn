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

    constexpr inline u32 cMurmur32Constant1 = 0xcc9e2d51;
    constexpr inline u32 cMurmur32Constant2 = 0x1b873593;
    constexpr inline u32 cMurmur32PreShift  = 0x16a88000;
    constexpr inline u32 cMurmur32M         = 0xe6546b64;
    constexpr inline u32 cMurmur32Finish1   = 0x85ebca6b;
    constexpr inline u32 cMurmur32Finish2   = 0xc2b2ae35;

    constexpr ALWAYS_INLINE u32 HashMurmur3WithSeed(const char *string, u32 seed) {

        u32 state = static_cast<u32>(*string);
        u32 iter = 0;
        while (state != '\0') {
            ++iter;
            /* Single byte check */
            if (string[iter] == '\0') {
                const u32 intermediate = state * cMurmur32Constant1;
                state = state * cMurmur32PreShift;
    
                seed  = (state | intermediate >> 17) * cMurmur32Constant2 ^ seed;
                break;
            }
            ++iter;
            /* Double byte check */
            if (string[iter] == '\0') {
                state = state | static_cast<u32>(string[iter - 1]) << 8;
                const u32 intermediate = state * cMurmur32Constant1;
                state = state * cMurmur32PreShift;

                seed  = (state | intermediate >> 17) * cMurmur32Constant2 ^ seed;
                break;
            }
            ++iter;
            /* Triple byte check */
            if (string[iter] == '\0') {
                state = state | static_cast<u32>(string[iter - 2]) << 8 | static_cast<u32>(string[iter - 1]) << 0x10;
                const u32 intermediate = state * cMurmur32Constant1;
                state = state * cMurmur32PreShift;

                seed  = (state | intermediate >> 17) * cMurmur32Constant2 ^ seed;
                break;
            }
            ++iter;
            /* Quad byte handler */
            state = state | static_cast<u32>(string[iter - 3]) << 8 | static_cast<u32>(string[iter - 2]) << 0x10 | static_cast<u32>(string[iter - 1]) << 0x18;
            state = (state * cMurmur32PreShift) | (state * cMurmur32Constant1) >> 17;
            state = state * cMurmur32Constant2 ^ seed;
            state = (state >> 19) | (state << 13);

            seed  = (state * 5) + cMurmur32M;
            state = static_cast<u32>(string[iter]);
        } 
        /* Finish */
        state = (iter ^ seed) ^ (iter ^ seed) >> 16;
        state = state * cMurmur32Finish1;
        state = state ^ (state >> 13);
        state = state * cMurmur32Finish2;
        return state ^ (state >> 16);
    }

    constexpr ALWAYS_INLINE u32 HashMurmur3(const char *string) {
        return HashMurmur3WithSeed(string, 0x0);
    }
}

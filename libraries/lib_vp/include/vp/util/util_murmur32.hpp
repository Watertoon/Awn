#pragma once

namespace vp::util {

    constexpr inline size_t cMurmur64Constant = 0xc6a4a7935bd1e995;
    constexpr inline size_t cMurmur64Seed     = 0xa9f63e6017234875;

    constexpr ALWAYS_INLINE size_t Murmur64Hash(const char *string) {

        /* Shift constant */
        constexpr u32 cShift = 47;

        /* Get size of string */
        const size_t size = (string != nullptr) ? ::strlen(string) : 0;

        /* Compute initial hash state */
        size_t state = (size * cMurmur64Constant) ^ cMurmur64Seed;

        /* Perform 8 byte aligned version of Murmur */
        const size_t aligned_size = (size & ~7);
        const char *string_leftover = string + aligned_size;
        if (aligned_size != 0) {
            while (string != string_leftover) {
                const size_t val = *reinterpret_cast<const size_t*>(string);
                state = ((((val * cMurmur64Constant) ^ ((val * cMurmur64Constant) >> cShift) * cMurmur64Constant)) ^ state) * cMurmur64Constant;
                string = string + 8;
            }
        }

        /* Finish out unaligned bytes*/
        switch (size & 7) {
            case 7:
                state = state ^ static_cast<size_t>(string_leftover[6]) << 48;
            case 6:
                state = state ^ static_cast<size_t>(string_leftover[5]) << 40;
            case 5:
                state = state ^ static_cast<size_t>(string_leftover[4]) << 32;
            case 4:
                state = state ^ static_cast<size_t>(string_leftover[3]) << 24;
            case 3:
                state = state ^ static_cast<size_t>(string_leftover[2]) << 16;
            case 2:
                state = state ^ static_cast<size_t>(string_leftover[1]) << 8;
            case 1:
                state = (state ^ static_cast<size_t>(string_leftover[0])) * cMurmur64Constant;
            default:
                state = (state ^ state >> cShift) * cMurmur64Constant;
        }

        return (state ^ state >> cShift);
    }
    
    constexpr inline u32 cMurmur32Constant1 = 0xcc9e2d51;
    constexpr inline u32 cMurmur32Constant2 = 0x1b873593;
    constexpr inline u32 cMurmur32PreShift  = 0x16a88000;
    constexpr inline u32 cMurmur32M         = 0xe6546b64;
    constexpr inline u32 cMurmur32Finish1   = 0x85ebca6b;
    constexpr inline u32 cMurmur32Finish2   = 0xc2b2ae35;

    constexpr ALWAYS_INLINE u32 Murmur3Hash32(const char *string, u32 seed) {

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
}

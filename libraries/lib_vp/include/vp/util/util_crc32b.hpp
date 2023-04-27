#pragma once

namespace vp::util {

	constexpr ALWAYS_INLINE u32 Crc32b_32(u32 value, u32 accumulate) {

        const sse4::v2ull constant = {0xB4E5B025F7011641, 0x00000001DB710641};
        sse4::v2ull state = {static_cast<size_t>(value ^ accumulate) << 32, 0ull};

        state = sse4::pclmulqdq(state, constant, sse4::PclmulSelectionMask(false, false));
        state = sse4::pclmulqdq(state, constant, sse4::PclmulSelectionMask(false, true));

        return static_cast<u32>(state[1]);
    }

    constexpr ALWAYS_INLINE u32 Crc32b_64(u64 value, u32 accumulate) {

        const sse4::v2ull constant = {0xB4E5B025F7011641, 0x00000001DB710641};
        sse4::v2ull state = {value ^ accumulate, 0ull};

        state = sse4::pclmulqdq(state, constant, sse4::PclmulSelectionMask(false, false));
        state = sse4::pclmulqdq(state, constant, sse4::PclmulSelectionMask(false, true));

        return static_cast<u32>(state[1]);
    }

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
        if (std::is_constant_evaluated() == true) {

            u32 seed = context;
            while (*string != '\0') {
                seed = cCrc32bTable[(*string ^ seed) & 0xff] ^ seed >> 8;
                string = string + 1;
            }
            
            return ~seed;
        } else {

            u32 seed = context;
            
            /* Perform 8 byte aligned version of crc32 */
            const size_t size = ::strlen(string);
            const size_t aligned_size = (size & ~7);
            const char *string_leftover = string + aligned_size;
            
            if (aligned_size != 0) {
                while (string != string_leftover) {
                    const size_t val = *reinterpret_cast<const size_t*>(string);
                    seed = Crc32b_64(val, seed);
                    string = string + 8;
                }
            }
            
            /* 4 byte fallback */
            if ((size & 4) != 0) {
                const u32 val = *reinterpret_cast<const u32*>(string);
                seed = Crc32b_32(val, seed);
                string_leftover = string_leftover + 4;
            }
            
            /* Fallback for rest of string */
            while (*string_leftover != '\0') {
                seed = cCrc32bTable[(*string_leftover ^ seed) & 0xff] ^ seed >> 8;
                
                string_leftover = string_leftover + 1;
            }
            
            return ~seed;
        }
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

        /* Perform 8 byte aligned version of crc32 */
        const size_t aligned_size = (data_size & ~7);
        const u64 *data_iter     = reinterpret_cast<const u64*>(data);
        const u64 *data_leftover = reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(data) + aligned_size);

        if (aligned_size != 0) {
            while (data_iter != data_leftover) {
                const size_t val = *data_iter;
                seed = Crc32b_64(val, seed);
                data_iter = data_iter + 1;
            }
        }

        /* 4 byte fallback */
        const u8 *leftover_iter = reinterpret_cast<const u8*>(data_leftover);
        if ((data_size & 4) != 0) {
            const u32 val = *reinterpret_cast<const u32*>(leftover_iter);
            seed = Crc32b_32(val, seed);
            leftover_iter = leftover_iter + 4;
        }

        /* Fallback for rest of string */
        u32 count = (data_size & 3);
        while (count != 0) {
            seed = cCrc32bTable[(*leftover_iter ^ seed) & 0xff] ^ seed >> 8;

            leftover_iter = leftover_iter + 1;
            count = count - 1;
        }

        return ~seed;
    }

    ALWAYS_INLINE u32 HashDataCrc32b(void *data, size_t data_size) {
        return HashDataCrc32bWithContext(0xffff'ffff, data, data_size);
    }
}

#pragma once

namespace vp::util {

    consteval v4si GetStore4Mask128(bool is_x, bool is_y, bool is_z, bool is_w) {
        return v4si{ is_x, is_y, is_z, is_w, };
    }

    consteval v4sll GetStore4Mask256(bool is_x, bool is_y, bool is_z, bool is_w) {
        return v4sll{ is_x, is_y, is_z, is_w, };
    }

    consteval v8si GetStore8Mask256(bool is_x0, bool is_y0, bool is_z0, bool is_w0, bool is_x1, bool is_y1, bool is_z1, bool is_w1) {
        return v8si{ is_x0, is_y0, is_z0, is_w0, is_x1, is_y1, is_z1, is_w1, };
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr void Store4Mask128(typename T::UnderlyingType *out_address, T const *address, const v4si &mask) {
        using Type = T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                _mm_maskstore_ps(out_address, reinterpret_cast<__m128 const *>(address), mask);
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 128) {
                _mm_maskstore_epi32(out_address, reinterpret_cast<__m128i const *>(address), mask);
                return;
            }
#endif
        }
        for (u32 i = 0; i < 4; ++i) {
            if (mask[i] == 0) { continue; }
            out_address[i] = (*address)[i];
        }
        return;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr void Store4Mask256(typename T::UnderlyingType *out_address, T const *address, const v4sll &mask) {
        using Type = T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, double>) {
                _mm256_maskstore_pd(out_address, reinterpret_cast<__m256 const *>(address));
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 256) {
                _mm256_maskstore_epi64(out_address, reinterpret_cast<__m256i const *>(address));
                return;
            }
#endif
        }
        for (u32 i = 0; i < 4; ++i) {
            if (mask[i] == 0) { continue; }
            out_address[i] = (*address)[i];
        }
        return;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr void Store8Mask256(typename T::UnderlyingType *out_address, T const *address, const v8si &mask) {
        using Type = T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                _mm256_maskstore_ps(out_address, reinterpret_cast<__m128 const *>(address), mask);
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 128) {
                _mm256_maskstore_epi32(out_address, reinterpret_cast<__m128i const *>(address), mask);
                return;
            }
#endif
        }
        for (u32 i = 0; i < 8; ++i) {
            if (mask[i] == 0) { continue; }
            out_address[i] = (*address)[i];
        }
        return;
    }
}

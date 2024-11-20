#pragma once

namespace vp::util {

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr void Store2(typename T::UnderlyingType *out_address, T const *address) {
        using Type = T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, double>) {
                _mm_storeu_pd(out_address, reinterpret_cast<__m128d const *>(address));
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 128) {
                _mm_storeu_si64(out_address, reinterpret_cast<__m128i const *>(address));
                return;
            }
#endif
        }
        for (u32 i = 0; i < 2; ++i) {
            out_address[i] = (*address)[i];
        }
        return;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr void Store4(typename T::UnderlyingType *out_address, T const *address) {
        using Type = T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                _mm_storeu_ps(out_address, *reinterpret_cast<__m128 const *>(address));
                return;
            } else if constexpr (std::is_same_v<Type, double>) {
                _mm256_storeu_pd(out_address, *reinterpret_cast<__m256d const *>(address));
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 128) {
                _mm_storeu_si32(out_address, *reinterpret_cast<__m128i const *>(address));
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 256) {
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(out_address), *reinterpret_cast<__m256i const *>(address));
                return;
            }
#endif
        }
        for (u32 i = 0; i < 4; ++i) {
            out_address[i] = (*address)[i];
        }
        return;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr void Store8(typename T::UnderlyingType *out_address, T const *address) {
        using Type = T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                _mm256_storeu_ps(out_address, *reinterpret_cast<__m256 const *>(address));
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 256) {
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(out_address), *reinterpret_cast<__m256i const *>(address));
                return;
            }
#endif
        }
        for (u32 i = 0; i < 8; ++i) {
            out_address[i] = (*address)[i];
        }
        return;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr void Store2Aligned(typename T::UnderlyingType *out_address, T const *address) {
        using Type = T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, double>) {
                _mm_store_pd(out_address, *reinterpret_cast<__m128d const *>(address));
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 128) {
                _mm_store_si64(out_address, *reinterpret_cast<__m128i const *>(address));
                return;
            }
#endif
        }
        for (u32 i = 0; i < 2; ++i) {
            out_address[i] = (*address)[i];
        }
        return;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr void Store4Aligned(typename T::UnderlyingType *out_address, T const *address) {
        using Type = T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                _mm_store_ps(out_address, *reinterpret_cast<__m128 const *>(address));
                return;
            } else if constexpr (std::is_same_v<Type, double>) {
                _mm256_store_pd(out_address, *reinterpret_cast<__m256d const *>(address));
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 128) {
                _mm_store_si32(out_address, *reinterpret_cast<__m128i const *>(address));
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 256) {
                _mm256_store_si256(reinterpret_cast<__m256i*>(out_address), *reinterpret_cast<__m256i const *>(address));
                return;
            }
#endif
        }
        for (u32 i = 0; i < 4; ++i) {
            out_address[i] = (*address)[i];
        }
        return;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr void Store8Aligned(typename T::UnderlyingType *out_address, T const *address) {
        using Type = T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                _mm256_store_ps(out_address, *reinterpret_cast<__m256 const *>(address));
                return;
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(T) == 256) {
                _mm256_store_si256(reinterpret_cast<__m256i*>(out_address), *reinterpret_cast<__m256i const *>(address));
                return;
            }
#endif
        }
        for (u32 i = 0; i < 8; ++i) {
            out_address[i] = (*address)[i];
        }
        return;
    }
}

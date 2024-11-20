#pragma once

namespace vp::util {

    template <typename T>
    constexpr TVectorType<T, 2> Load2(T const *address) {
        using v2t = TVectorType<T, 2>;
        using Type = v2t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, double>) {
                return v2t{ .v = reinterpret_cast<double __attribute__((vector_size(16)))>(_mm_loadu_pd(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v2t) == 128) {
                return v2t{ .v = reinterpret_cast<long long int __attribute__((vector_size(16)))>(_mm_loadu_epi64(address)), };
            }
#endif
            return *reinterpret_cast<v2t const*>(address);
        }
        v2t ret;
        for (u32 i = 0; i < 2; ++i) {
            ret[i] = address[i];
        }
        return ret;
    };

    template <typename T>
    constexpr TVectorType<T, 4> Load4(T const *address) {
        using v4t = TVectorType<T, 4>;
        using Type = v4t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v4t{ .v = reinterpret_cast<float __attribute__((vector_size(16)))>(_mm_loadu_ps(address)), };
            } else if constexpr (std::is_same_v<Type, double>) {
                return v4t{ .v = reinterpret_cast<double __attribute__((vector_size(32)))>(_mm256_loadu_pd(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v4t) == 128) {
                return v4t{ .v = reinterpret_cast<int __attribute__((vector_size(16)))>(_mm_loadu_epi32(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v4t) == 256) {
                return v4t{ .v = reinterpret_cast<long long int __attribute__((vector_size(32)))>(_mm256_loadu_epi64(address)), };
            }
#endif
            return *reinterpret_cast<v4t const*>(address);
        }
        v4t ret;
        for (u32 i = 0; i < 4; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 8> Load8(T const *address) {
        using v8t = TVectorType<T, 8>;
        using Type = v8t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v8t{ .v = reinterpret_cast<float __attribute__((vector_size(32)))>(_mm256_loadu_ps(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v8t) == 128) {
                return v8t{ .v = reinterpret_cast<short __attribute__((vector_size(16)))>(_mm_loadu_epi16(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v8t) == 256) {
                return v8t{ .v = reinterpret_cast<int __attribute__((vector_size(32)))>(_mm256_loadu_epi32(address)), };
            } else if constexpr (1 < v8t::cRegisterCount) {
                v8t ret;
                for (u32 i = 0; i < v8t::cRegisterCount; ++i) {
                    ret.v[i] = Load4(address + i * 4); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v8t const*>(address);
        }
        v8t ret;
        for (u32 i = 0; i < 8; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 16> Load16(T const *address) {
        using v16t = TVectorType<T, 16>;
        using Type = v16t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_integral_v<Type> && BITSIZEOF(v16t) == 128) {
                return v16t{ .v = reinterpret_cast<char __attribute__((vector_size(16)))>(_mm_lddqu_si128(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v16t) == 256) {
                return v16t{ .v = reinterpret_cast<short __attribute__((vector_size(32)))>(_mm256_loadu_epi16(address)), };
            } else if constexpr (1 < v16t::cRegisterCount) {
                v16t ret;
                for (u32 i = 0; i < v16t::cRegisterCount; ++i) {
                    ret.v[i] = Load8(address + i * 8); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v16t const*>(address);
        }
        v16t ret;
        for (u32 i = 0; i < 16; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 32> Load32(T const *address) {
        using v32t = TVectorType<T, 32>;
        using Type = v32t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_integral_v<Type> && BITSIZEOF(v32t) == 256) {
                return v32t{ .v = reinterpret_cast<char __attribute__((vector_size(32)))>(_mm256_lddqu_si256(reinterpret_cast<const __m256i*>(address))), };
            } else if constexpr (1 < v32t::cRegisterCount) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load8(address + i * 8); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v32t const*>(address);
        }
        v32t ret;
        for (u32 i = 0; i < 32; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 64> Load64(T const *address) {
        using v64t = TVectorType<T, 64>;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (1 < v64t::cRegisterCount) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load8(address + i * 8); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v64t const*>(address);
        }
        v64t ret;
        for (u32 i = 0; i < 64; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 128> Load128(T const *address) {
        using v128t = TVectorType<T, 128>;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (1 < v128t::cRegisterCount) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load8(address + i * 8); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v128t const*>(address);
        }
        v128t ret;
        for (u32 i = 0; i < 128; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 2> Load2Aligned(T const *address) {
        using v2t = TVectorType<T, 2>;
        using Type = v2t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, double>) {
                return v2t{ .v = reinterpret_cast<double __attribute__((vector_size(16)))>(_mm_load_pd(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v2t) == 128) {
                return v2t{ .v = reinterpret_cast<long long int __attribute__((vector_size(16)))>(_mm_load_epi64(address)), };
            }
#endif
            return *reinterpret_cast<v2t const*>(address);
        }
        v2t ret;
        for (u32 i = 0; i < 2; ++i) {
            ret[i] = address[i];
        }
        return ret;
    };

    template <typename T>
    constexpr TVectorType<T, 4> Load4Aligned(T const *address) {
        using v4t = TVectorType<T, 4>;
        using Type = v4t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v4t{ .v = reinterpret_cast<float __attribute__((vector_size(16)))>(_mm_load_ps(address)), };
            } else if constexpr (std::is_same_v<Type, double>) {
                return v4t{ .v = reinterpret_cast<double __attribute__((vector_size(32)))>(_mm256_load_pd(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v4t) == 128) {
                return v4t{ .v = reinterpret_cast<int __attribute__((vector_size(16)))>(_mm_load_epi32(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v4t) == 256) {
                return v4t{ .v = reinterpret_cast<long long int __attribute__((vector_size(32)))>(_mm256_load_epi64(address)), };
            }
#endif
            return *reinterpret_cast<v4t const*>(address);
        }
        v4t ret;
        for (u32 i = 0; i < 4; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 8> Load8Aligned(T const *address) {
        using v8t = TVectorType<T, 8>;
        using Type = v8t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v8t{ .v = reinterpret_cast<float __attribute__((vector_size(32)))>(_mm256_load_ps(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v8t) == 128) {
                return v8t{ .v = reinterpret_cast<short __attribute__((vector_size(16)))>(_mm_load_epi16(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v8t) == 256) {
                return v8t{ .v = reinterpret_cast<int __attribute__((vector_size(32)))>(_mm256_load_epi32(address)), };
            } else if constexpr (1 < v8t::cRegisterCount) {
                v8t ret;
                for (u32 i = 0; i < v8t::cRegisterCount; ++i) {
                    ret.v[i] = Load4Aligned(address + i * 4); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v8t const*>(address);
        }
        v8t ret;
        for (u32 i = 0; i < 8; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 16> Load16Aligned(T const *address) {
        using v16t = TVectorType<T, 16>;
        using Type = v16t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_integral_v<Type> && BITSIZEOF(v16t) == 128) {
                return v16t{ .v = reinterpret_cast<char __attribute__((vector_size(16)))>(_mm_load_si128(address)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v16t) == 256) {
                return v16t{ .v = reinterpret_cast<short __attribute__((vector_size(32)))>(_mm256_load_epi16(address)), };
            } else if constexpr (1 < v16t::cRegisterCount) {
                v16t ret;
                for (u32 i = 0; i < v16t::cRegisterCount; ++i) {
                    ret.v[i] = Load8Aligned(address + i * 8); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v16t const*>(address);
        }
        v16t ret;
        for (u32 i = 0; i < 16; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 32> Load32Aligned(T const *address) {
        using v32t = TVectorType<T, 32>;
        using Type = v32t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_integral_v<Type> && BITSIZEOF(v32t) == 256) {
                return v32t{ .v = reinterpret_cast<char __attribute__((vector_size(32)))>(_mm256_load_si256(reinterpret_cast<const __m256i*>(address))), };
            } else if constexpr (1 < v32t::cRegisterCount) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load8Aligned(address + i * 8); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v32t const*>(address);
        }
        v32t ret;
        for (u32 i = 0; i < 32; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 64> Load64Aligned(T const *address) {
        using v64t = TVectorType<T, 64>;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (1 < v64t::cRegisterCount) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load8Aligned(address + i * 8); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v64t const*>(address);
        }
        v64t ret;
        for (u32 i = 0; i < 64; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }

    template <typename T>
    constexpr TVectorType<T, 128> Load128Aligned(T const *address) {
        using v128t = TVectorType<T, 128>;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (1 < v128t::cRegisterCount) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load8Aligned(address + i * 8); 
                }
                return ret;
            }
#endif
            return *reinterpret_cast<v128t const*>(address);
        }
        v128t ret;
        for (u32 i = 0; i < 128; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }
}

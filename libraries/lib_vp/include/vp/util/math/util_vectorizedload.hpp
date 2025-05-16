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

    template <typename T>
    constexpr TVectorType<T, 4> Broadcast4(const T value) {
        using v4t = TVectorType<T, 4>;
        using Type = v4t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, double>) {
                return v4t{ .v = reinterpret_cast<double __attribute__((vector_size(32)))>(_mm256_set1_pd(value)), };
            } else if constexpr (std::is_same_v<Type, s32> || std::is_same_v<Type, u32>) {
                return v4t{ .v = reinterpret_cast<s32 __attribute__((vector_size(16)))>(_mm_set1_epi32(value)), };
            } else if constexpr (std::is_same_v<Type, float>) {
                return v4t{ .v = reinterpret_cast<float __attribute__((vector_size(16)))>(_mm_set1_ps(value)), };
            }
#endif
        }
        const Type v = value;
        return v4t{v,v,v,v,};
    }

    template <typename T>
    constexpr TVectorType<T, 8> Broadcast8(const T value) {
        using v8t = TVectorType<T, 8>;
        using Type = v8t::UnderlyingType;
        if (!std::is_constant_evaluated() && std::is_same_v<Type, float>) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, s32> || std::is_same_v<Type, u32>) {
                return v8t{ .v = reinterpret_cast<long long int __attribute__((vector_size(16)))>(_mm256_set1_epi32(value)), };
            } else if constexpr (std::is_same_v<Type, float>) {
                return v8t{ .v = reinterpret_cast<float __attribute__((vector_size(16)))>(_mm256_set1_ps(value)), };
            }
#endif
        }
        const Type v = value;
        return v8t{v,v,v,v,v,v,v,v,};
    }

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
            } else 
#endif
            if constexpr (v8t::cCountPerRegister == 2) {
                v8t ret;
                for (u32 i = 0; i < v8t::cRegisterCount; ++i) {
                    ret.v[i] = Load2(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v8t::cCountPerRegister == 4) {
                v8t ret;
                for (u32 i = 0; i < v8t::cRegisterCount; ++i) {
                    ret.v[i] = Load4(address + i * 4).v; 
                }
                return ret;
            }
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
            } else
#endif
            if constexpr (v16t::cCountPerRegister == 2) {
                v16t ret;
                for (u32 i = 0; i < v16t::cRegisterCount; ++i) {
                    ret.v[i] = Load2(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v16t::cCountPerRegister == 4) {
                v16t ret;
                for (u32 i = 0; i < v16t::cRegisterCount; ++i) {
                    ret.v[i] = Load4(address + i * 4).v; 
                }
                return ret;
            } else if constexpr (v16t::cCountPerRegister == 8) {
                v16t ret;
                for (u32 i = 0; i < v16t::cRegisterCount; ++i) {
                    ret.v[i] = Load8(address + i * 8).v; 
                }
                return ret;
            }
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
            } else
#endif
             if constexpr (v32t::cCountPerRegister == 2) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load2(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v32t::cCountPerRegister == 4) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load4(address + i * 4).v; 
                }
                return ret;
            } else if constexpr (v32t::cCountPerRegister == 8) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load8(address + i * 8).v; 
                }
                return ret;
            } else if constexpr (v32t::cCountPerRegister == 16) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load16(address + i * 16).v; 
                }
                return ret;
            }
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
            if constexpr (v64t::cCountPerRegister == 2) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load2(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v64t::cCountPerRegister == 4) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load4(address + i * 4).v; 
                }
                return ret;
            } else if constexpr (v64t::cCountPerRegister == 8) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load8(address + i * 8).v; 
                }
                return ret;
            } else if constexpr (v64t::cCountPerRegister == 16) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load16(address + i * 16).v; 
                }
                return ret;
            } else if constexpr (v64t::cCountPerRegister == 32) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load32(address + i * 32).v; 
                }
                return ret;
            }
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
            if constexpr (v128t::cCountPerRegister == 2) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load2(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v128t::cCountPerRegister == 4) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load4(address + i * 4).v; 
                }
                return ret;
            } else if constexpr (v128t::cCountPerRegister == 8) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load8(address + i * 8).v; 
                }
                return ret;
            } else if constexpr (v128t::cCountPerRegister == 16) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load16(address + i * 16).v; 
                }
                return ret;
            } else if constexpr (v128t::cCountPerRegister == 32) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load32(address + i * 32).v; 
                }
                return ret;
            }
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
            } else
#endif
             if constexpr (v8t::cCountPerRegister == 2) {
                v8t ret;
                for (u32 i = 0; i < v8t::cRegisterCount; ++i) {
                    ret.v[i] = Load2Aligned(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v8t::cCountPerRegister == 4) {
                v8t ret;
                for (u32 i = 0; i < v8t::cRegisterCount; ++i) {
                    ret.v[i] = Load4Aligned(address + i * 4).v; 
                }
                return ret;
            }
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
            } else
#endif
             if constexpr (v16t::cCountPerRegister == 2) {
                v16t ret;
                for (u32 i = 0; i < v16t::cRegisterCount; ++i) {
                    ret.v[i] = Load2Aligned(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v16t::cCountPerRegister == 4) {
                v16t ret;
                for (u32 i = 0; i < v16t::cRegisterCount; ++i) {
                    ret.v[i] = Load4Aligned(address + i * 4).v; 
                }
                return ret;
            } else if constexpr (v16t::cCountPerRegister == 8) {
                v16t ret;
                for (u32 i = 0; i < v16t::cRegisterCount; ++i) {
                    ret.v[i] = Load8Aligned(address + i * 8).v; 
                }
                return ret;
            }
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
            } else
#endif
             if constexpr (v32t::cCountPerRegister == 2) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load2Aligned(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v32t::cCountPerRegister == 4) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load4Aligned(address + i * 4).v; 
                }
                return ret;
            } else if constexpr (v32t::cCountPerRegister == 8) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load8Aligned(address + i * 8).v; 
                }
                return ret;
            } else if constexpr (v32t::cCountPerRegister == 16) {
                v32t ret;
                for (u32 i = 0; i < v32t::cRegisterCount; ++i) {
                    ret.v[i] = Load16Aligned(address + i * 16).v; 
                }
                return ret;
            }
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
            if constexpr (v64t::cCountPerRegister == 2) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load2Aligned(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v64t::cCountPerRegister == 4) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load4Aligned(address + i * 4).v; 
                }
                return ret;
            } else if constexpr (v64t::cCountPerRegister == 8) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load8Aligned(address + i * 8).v; 
                }
                return ret;
            } else if constexpr (v64t::cCountPerRegister == 16) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load16Aligned(address + i * 16).v; 
                }
                return ret;
            } else if constexpr (v64t::cCountPerRegister == 32) {
                v64t ret;
                for (u32 i = 0; i < v64t::cRegisterCount; ++i) {
                    ret.v[i] = Load32Aligned(address + i * 32).v; 
                }
                return ret;
            }
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
            if constexpr (v128t::cCountPerRegister == 2) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load2Aligned(address + i * 2).v; 
                }
                return ret;
            } else if constexpr (v128t::cCountPerRegister == 4) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load4Aligned(address + i * 4).v; 
                }
                return ret;
            } else if constexpr (v128t::cCountPerRegister == 8) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load8Aligned(address + i * 8).v; 
                }
                return ret;
            } else if constexpr (v128t::cCountPerRegister == 16) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load16Aligned(address + i * 16).v; 
                }
                return ret;
            } else if constexpr (v128t::cCountPerRegister == 32) {
                v128t ret;
                for (u32 i = 0; i < v128t::cRegisterCount; ++i) {
                    ret.v[i] = Load32Aligned(address + i * 32).v; 
                }
                return ret;
            }
        }
        v128t ret;
        for (u32 i = 0; i < 128; ++i) {
            ret[i] = address[i];
        }
        return ret;
    }
}

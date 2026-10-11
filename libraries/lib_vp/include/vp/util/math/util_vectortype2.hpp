#pragma once

namespace vp::util {

    #define VP_VECTOR_TRAITS(type, count, is_vectorized) \
        using UnderlyingType = type ; \
        static constexpr bool cIsVectorType = true; \
        static constexpr bool cIsVectorized = is_vectorized ; \
        static constexpr u32 cCount = count ; \
        static constexpr u32 cVectorizedCount = (is_vectorized) ? vp::util::AlignPow2(count) : count ; \
        template <class Self> \
        constexpr ALWAYS_INLINE auto &&operator[](this Self&& self, u32 index) { \
            return std::forward<Self>(self).v[index]; \
        }

    template <typename T, u32 Count>
    struct Plain {
        T v[Count];

        VP_VECTOR_TRAITS(T, Count, false);
    };
    template <typename T, u32 Count>
    struct Vector {
        typedef T VectorizedType __attribute__((vector_size(sizeof(T)*vp::util::AlignPow2(Count))));

        T v __attribute__((vector_size(sizeof(T)*vp::util::AlignPow2(Count))));

        VP_VECTOR_TRAITS(T, Count, true);
    };

    namespace impl {
        template <typename T>
            requires (!T::cIsVectorized)
        consteval std::array<T,T::cVectorizedCount> MakeUnitArray() {
            std::array<T,T::cVectorizedCount> arr;
            for (u32 i = 0; i < T::cVectorizedCount; ++i) {
                T r = T{};
                r[i] = static_cast<typename T::UnderlyingType>(1);
                arr[i] = r;
            }
            return arr;
        }
        template <typename T>
            requires (T::cIsVectorized && T::cVectorizedCount <= 4)
        consteval std::array<T,T::cVectorizedCount> MakeUnitArray() {
            std::array<T,T::cVectorizedCount> arr;
            if constexpr (0 < T::cVectorizedCount) {
                arr[0] = T{1,0,0,0,};
            }
            if constexpr (1 < T::cVectorizedCount) {
                arr[1] = T{0,1,0,0,};
            }
            if constexpr (2 < T::cVectorizedCount) {
                arr[2] = T{0,0,1,0,};
            }
            if constexpr (3 < T::cVectorizedCount) {
                arr[3] = T{0,0,0,1,};
            }
            return arr;
        }
    }

    template <typename T>
        requires (T::cIsVectorType)
    struct VectorConstant {
        static constexpr T cZero = T{};
        static constexpr T cOne = T{};
        static constexpr std::array<T,T::cVectorizedCount> cUnit = impl::MakeUnitArray<T>();
    };

    using Vector4h = vp::util::Vector<_Float16,4>;
    using Vector8h = vp::util::Vector<_Float16,8>;
    using Vector16h = vp::util::Vector<_Float16,16>;

    using Vector2f = vp::util::Vector<float,2>;
    using Vector3f = vp::util::Vector<float,3>;
    using Vector4f = vp::util::Vector<float,4>;
    using Vector8f = vp::util::Vector<float,8>;
    using Vector16f = vp::util::Vector<float,16>;
    using Float2 = vp::util::Plain<float,2>;
    using Float3 = vp::util::Plain<float,3>;
    using Float4 = vp::util::Plain<float,4>;
    using Float8 = vp::util::Plain<float,8>;
    using Float16 = vp::util::Plain<float,16>;

    using Vector2d = vp::util::Vector<double,2>;
    using Vector3d = vp::util::Vector<double,3>;
    using Vector4d = vp::util::Vector<double,4>;
    using Vector8d = vp::util::Vector<double,8>;
    using Double2 = vp::util::Plain<double,2>;
    using Double3 = vp::util::Plain<double,3>;
    using Double4 = vp::util::Plain<double,4>;
    using Double8 = vp::util::Plain<double,8>;

    using Vector16cc = vp::util::Vector<char,16>;
    using Vector32cc = vp::util::Vector<char,32>;
    using Vector64cc = vp::util::Vector<char,64>;

    using Vector16sc = vp::util::Vector<s8,16>;
    using Vector32sc = vp::util::Vector<s8,32>;
    using Vector64sc = vp::util::Vector<s8,64>;

    using Vector16uc = vp::util::Vector<u8,16>;
    using Vector32uc = vp::util::Vector<u8,32>;
    using Vector64uc = vp::util::Vector<u8,64>;

    using Vector2ss = vp::util::Vector<s16,2>;
    using Vector3ss = vp::util::Vector<s16,3>;
    using Vector4ss = vp::util::Vector<s16,4>;
    using Vector8ss = vp::util::Vector<s16,8>;
    using Vector16ss = vp::util::Vector<s16,16>;
    using Vector32ss = vp::util::Vector<s16,32>;
    using Short2 = vp::util::Plain<s16,2>;
    using Short3 = vp::util::Plain<s16,3>;
    using Short4 = vp::util::Plain<s16,4>;
    using Short8 = vp::util::Plain<s16,8>;
    using Short16 = vp::util::Plain<s16,16>;
    using Short32 = vp::util::Plain<s16,32>;

    using Vector2us = vp::util::Vector<u16,2>;
    using Vector3us = vp::util::Vector<u16,3>;
    using Vector4us = vp::util::Vector<u16,4>;
    using Vector8us = vp::util::Vector<u16,8>;
    using Vector16us = vp::util::Vector<u16,16>;
    using Vector32us = vp::util::Vector<u16,32>;
    using UShort2 = vp::util::Plain<u16,2>;
    using UShort3 = vp::util::Plain<u16,3>;
    using UShort4 = vp::util::Plain<u16,4>;
    using UShort8 = vp::util::Plain<u16,8>;
    using UShort16 = vp::util::Plain<u16,16>;
    using UShort32 = vp::util::Plain<u16,32>;

    using Vector2si = vp::util::Vector<s32,2>;
    using Vector3si = vp::util::Vector<s32,3>;
    using Vector4si = vp::util::Vector<s32,4>;
    using Vector8si = vp::util::Vector<s32,8>;
    using Vector16si = vp::util::Vector<s32,16>;
    using Int2 = vp::util::Plain<s32,2>;
    using Int3 = vp::util::Plain<s32,3>;
    using Int4 = vp::util::Plain<s32,4>;
    using Int8 = vp::util::Plain<s32,8>;
    using Int16 = vp::util::Plain<s32,16>;

    using Vector2ui = vp::util::Vector<u32,2>;
    using Vector3ui = vp::util::Vector<u32,3>;
    using Vector4ui = vp::util::Vector<u32,4>;
    using Vector8ui = vp::util::Vector<u32,8>;
    using Vector16ui = vp::util::Vector<u32,16>;
    using UInt2 = vp::util::Plain<u32,2>;
    using UInt3 = vp::util::Plain<u32,3>;
    using UInt4 = vp::util::Plain<u32,4>;
    using UInt8 = vp::util::Plain<u32,8>;
    using UInt16 = vp::util::Plain<u32,16>;

    using Vector2sll = vp::util::Vector<s64,2>;
    using Vector3sll = vp::util::Vector<s64,3>;
    using Vector4sll = vp::util::Vector<s64,4>;
    using Vector8sll = vp::util::Vector<s64,8>;
    using Long2 = vp::util::Plain<s64,2>;
    using Long3 = vp::util::Plain<s64,3>;
    using Long4 = vp::util::Plain<s64,4>;
    using Long8 = vp::util::Plain<s64,8>;

    using Vector2ull = vp::util::Vector<u64,2>;
    using Vector3ull = vp::util::Vector<u64,3>;
    using Vector4ull = vp::util::Vector<u64,4>;
    using Vector8ull = vp::util::Vector<u64,8>;
    using ULong2 = vp::util::Plain<u64,2>;
    using ULong3 = vp::util::Plain<u64,3>;
    using ULong4 = vp::util::Plain<u64,4>;
    using ULong8 = vp::util::Plain<u64,8>;

    namespace impl {

        template <typename T, u32 Count, bool IsVectorized>
        struct VectorHelper;

        template <typename T, u32 Count>
        struct VectorHelper<T,Count,true> {
            using Type = Vector<T,Count>;
        };
        template <typename T, u32 Count>
        struct VectorHelper<T,Count,false> {
            using Type = Plain<T,Count>;
        };
    }

    template <typename T, u32 Count, bool IsVectorized>
    using TVector = impl::VectorHelper<T,Count,IsVectorized>::Type;

    /* Load/Store */
    template <typename T>
        requires (T::cIsVectorType)
    constexpr void Load(T *vec, const typename T::UnderlyingType *value) {
        if constexpr (T::cIsVectorized) {
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            using Type = typename T::UnderlyingType;
            if constexpr (sizeof(T) == 0x10) {
                if constexpr (std::is_same_v<Type,float>) {
                    *vec = T { __builtin_ia32_loadups(value), };
                    return;
                } else if constexpr (std::is_same_v<Type,double>) {
                    *vec = T { __builtin_ia32_loadupd(value), };
                    return;
                } else if constexpr (std::is_integral_v<Type>) {
                    *vec = T { reinterpret_cast<typename T::VectorizedType>(__builtin_ia32_loaddqu(reinterpret_cast<const char*>(value))), };
                    return;
                }
            } else if constexpr (sizeof(T) == 0x20) {
                if constexpr (std::is_same_v<Type,float>) {
                    *vec = T { __builtin_ia32_loadups256(value), };
                    return;
                } else if constexpr (std::is_same_v<Type,double>) {
                    *vec = T { __builtin_ia32_loadupd256(value), };
                    return;
                } else if constexpr (std::is_integral_v<Type>) {
                    *vec = T { reinterpret_cast<typename T::VectorizedType>(__builtin_ia32_loaddqu256(reinterpret_cast<const char*>(value))), };
                    return;
                }
            }
#elifdef VP_TARGET_ARCHITECTURE_aarch64
            if constexpr (sizeof(T) == 0x10) {
                *vec = T { reinterpret_cast<typename T::VectorizedType>(vldrq_p128(reinterpret_cast<const poly128_t*>(value))), };
                return;
            } else if constexpr (sizeof(T) == 0x20) {
                const auto a = vldrq_p128(reinterpret_cast<const poly128_t*>(value));
                const auto b = vldrq_p128(reinterpret_cast<const poly128_t*>(reinterpret_cast<uintptr_t>(value) + 0x10));
                poly128_t *p = reinterpret_cast<poly128_t*>(vec);
                p[0] = a;
                p[1] = b;
                return;
            }
#endif
        }
        }
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            (*vec)[i] = value[i];
        }
        return;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr void Store(typename T::UnderlyingType *value, const T &vec) {
        if constexpr (T::cIsVectorized) {
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
#elifdef VP_TARGET_ARCHITECTURE_aarch64
            if constexpr (sizeof(T) == 0x10) {
                const poly128_t p = reinterpret_cast<poly128_t>(vec.v);
                vstrq_p128(reinterpret_cast<poly128_t*>(value), p);
                return;
            } else if constexpr (sizeof(T) == 0x20) {
                const poly128_t *p = reinterpret_cast<const poly128_t*>(std::addressof(vec.v));
                vstrq_p128(reinterpret_cast<poly128_t*>(value), p[0]);
                vstrq_p128(reinterpret_cast<poly128_t*>(reinterpret_cast<uintptr_t>(value) + 0x10), p[1]);
                return;
            }
#endif
        }
        }
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            value[i] = vec[i];
        }
        return;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsVectorType && T::cIsVectorType)
    constexpr Ret Convert(const T &a) {
        using RType = typename Ret::UnderlyingType;
        Ret ret = {};
        constexpr const u32 count = Min(Ret::cVectorizedCount, T::cVectorizedCount);
        for (u32 i = 0; i < count; ++i) {
            ret[i] = static_cast<RType>(a[i]);
        }
        return ret;
    } 

    /* Arithmetic operators */
    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator+(const T &left, const T &right) {
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = left[i] + right[i];
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator+(const T &left, const typename T::UnderlyingType &right) {
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = left[i] + right;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator-(const T &left, const T &right) {
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = left[i] - right[i];
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator-(const T &left, const typename T::UnderlyingType &right) {
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = left[i] - right;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator*(const T &left, const T &right) {
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = left[i] * right[i];
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator*(const T &left, const typename T::UnderlyingType &right) {
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = left[i] * right;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator/(const T &left, const typename T::UnderlyingType &right) {
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = left[i] / right;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator&(const T &left, const T &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            const MaskType r = std::bit_cast<MaskType>(left[i]) & std::bit_cast<MaskType>(right[i]);
            out[i] = std::bit_cast<Type>(r);
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator&(const T &left, const typename T::UnderlyingType &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            const MaskType r = std::bit_cast<MaskType>(left[i]) & std::bit_cast<MaskType>(right);
            out[i] = std::bit_cast<Type>(r);
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator|(const T &left, const T &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            const MaskType r = std::bit_cast<MaskType>(left[i]) | std::bit_cast<MaskType>(right[i]);
            out[i] = std::bit_cast<Type>(r);
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator|(const T &left, const typename T::UnderlyingType &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            const MaskType r = std::bit_cast<MaskType>(left[i]) | std::bit_cast<MaskType>(right);
            out[i] = std::bit_cast<Type>(r);
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator^(const T &left, const T &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        static_assert(sizeof(Type) == sizeof(MaskType));
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            const MaskType r = std::bit_cast<MaskType>(left[i]) ^ std::bit_cast<MaskType>(right[i]);
            out[i] = std::bit_cast<Type>(r);
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator^(const T &left, const typename T::UnderlyingType &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = std::bit_cast<Type>(std::bit_cast<MaskType>(left[i]) ^ std::bit_cast<MaskType>(right));
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator~(const T &left) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        if constexpr (T::cIsVectorized) {
        if (!std::is_constant_evaluated()) {
            return T { reinterpret_cast<T::VectorizedType>(~reinterpret_cast<typename TVector<MaskType,T::cCount,true>::VectorizedType>(left.v)), };
        }
        }
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            const MaskType r = ~std::bit_cast<MaskType>(left[i]);
            out[i] = std::bit_cast<Type>(r);
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator-(const T &left) {
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = -left[i];
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T MultiplyAdd(const T& a, const T& b, const T& c) {
        T temp = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            temp[i] = (a[i] * b[i]) + c[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T NegateMultiplyAdd(const T& a, const T& b, const T& c) {
        T temp = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            temp[i] = (-a[i] * b[i]) - c[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T MultiplySub(const T& a, const T& b, const T& c) {
        T temp = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            temp[i] = (-a[i] * b[i]) + c[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T NegateMultiplySub(const T& a, const T& b, const T& c) {
        T temp = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            temp[i] = (a[i] * b[i]) - c[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T MultiplySubOrAddInterleaved(const T& a, const T& b, const T& c) {
        T temp = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            if ((i & 1) == 0) {
                temp[i] = (a[i] * b[i]) - c[i];
            } else {
                temp[i] = (a[i] * b[i]) + c[i];
            }
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T SquareRoot(const T &vec) {
        T ret = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            ret[i] = SquareRoot(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Reciprocal(const T &vec) {
        using Type = typename T::UnderlyingType;
        T ret = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            ret[i] = static_cast<Type>(1.0) / vec[i];
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T ReciprocalSquareRoot(const T &vec) {
        using Type = typename T::UnderlyingType;
        T ret = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            ret[i] = static_cast<Type>(1.0) / SquareRoot(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Absolute(const T &v) {
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = Absolute(v[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Broadcast(typename T::UnderlyingType scalar) {
        T out = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            out[i] = scalar;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T::UnderlyingType Sum(const T &vec) {
        using Type = typename T::UnderlyingType;
        if constexpr (T::cIsVectorized) {
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
        if constexpr (sizeof(T) == 0x10) {
            if constexpr (std::is_same_v<Type,float>) {
                const auto t = __builtin_ia32_haddps(vec.v, vec.v);
                const auto ret = __builtin_ia32_haddps(t, t);
                return ret[0];
            } else if constexpr (std::is_same_v<Type,double>) {
                const auto t = __builtin_ia32_haddpd(vec.v, vec.v);
                const auto ret = __builtin_ia32_haddpd(t, t);
                return ret[0];
            } else if constexpr (std::is_same_v<Type,s32> || std::is_same_v<Type,u32>) {
                const auto t = __builtin_ia32_phaddd128(vec.v, vec.v);
                const auto ret = __builtin_ia32_phaddd128(t, t);
                return ret[0];
            } else if constexpr (std::is_same_v<Type,s16> || std::is_same_v<Type,u16>) {
                const auto t = __builtin_ia32_phaddw128(vec.v, vec.v);
                const auto ret = __builtin_ia32_phaddw128(t, t);
                return ret[0];
            }
        } else if constexpr (sizeof(T) == 0x20) {
            using HT = TVector<Type,T::cVectorizedCount/2,true>;
            if constexpr (std::is_same_v<Type,float>) {
                const auto t = __builtin_ia32_haddps256(vec.v, vec.v);
                const HT t1 = HT { __builtin_ia32_vextractf128_ps256(t, 1), };
                const HT t0 = HT { __builtin_ia32_vextractf128_ps256(t, 0), };
                const HT ret = t0 + t1;
                return ret[0];
            } else if constexpr (std::is_same_v<Type,double>) {
                const auto t = __builtin_ia32_haddpd256(vec.v, vec.v);
                const HT t1 = HT { __builtin_ia32_vextractf128_pd256(t, 1), };
                const HT t0 = HT { __builtin_ia32_vextractf128_pd256(t, 0), };
                const HT ret = t0 + t1;
                return ret[0];
            } else if constexpr (std::is_same_v<Type,s32> || std::is_same_v<Type,u32>) {
                const auto t = __builtin_ia32_phaddd256(vec.v, vec.v);
                const HT t1 = HT { __builtin_ia32_vextractf128_si256(t, 1), };
                const HT t0 = HT { __builtin_ia32_vextractf128_si256(t, 0), };
                const HT ret = t0 + t1;
                return ret[0];
            } else if constexpr (std::is_same_v<Type,s16> || std::is_same_v<Type,u16>) {
                const auto t = __builtin_ia32_phaddw256(vec.v, vec.v);
                const HT t1 = HT { reinterpret_cast<typename HT::VectorizedType>(__builtin_ia32_vextractf128_si256(reinterpret_cast<int __attribute((vector_size(32)))>(t), 1)), };
                const HT t0 = HT { reinterpret_cast<typename HT::VectorizedType>(__builtin_ia32_vextractf128_si256(reinterpret_cast<int __attribute((vector_size(32)))>(t), 0)), };
                const HT ret = t0 + t1;
                return ret[0];
            }
        }
#elifdef VP_TARGET_ARCHITECTURE_aarch64
        if constexpr (sizeof(T) == 0x10) {
            if constexpr (std::is_same_v<Type,float>) {
                using V = float32x4_t;
                return vaddvq_f32(reinterpret_cast<V>(vec.v));
            } else if constexpr (std::is_same_v<Type,double>) {
                using V = float64x2_t;
                return vaddvq_f64(reinterpret_cast<V>(vec.v));
            } else if constexpr (std::is_same_v<Type,s64>) {
                using V = int64x2_t;
                return vaddvq_s64(reinterpret_cast<V>(vec.v));
            } else if constexpr (std::is_same_v<Type,s32>) {
                using V = int32x4_t;
                return vaddvq_s32(reinterpret_cast<V>(vec.v));
            } else if constexpr (std::is_same_v<Type,s16>) {
                using V = int16x8_t;
                return vaddvq_s16(reinterpret_cast<V>(vec.v));
            } else if constexpr (std::is_same_v<Type,s8>) {
                using V = int8x16_t;
                return vaddvq_s8(reinterpret_cast<V>(vec.v));
            } else if constexpr (std::is_same_v<Type,u64>) {
                using V = uint64x2_t;
                return vaddvq_u64(reinterpret_cast<V>(vec.v));
            } else if constexpr (std::is_same_v<Type,u32>) {
                using V = uint32x4_t;
                return vaddvq_u32(reinterpret_cast<V>(vec.v));
            } else if constexpr (std::is_same_v<Type,u16>) {
                using V = uint16x8_t;
                return vaddvq_u16(reinterpret_cast<V>(vec.v));
            } else if constexpr (std::is_same_v<Type,u8>) {
                using V = uint8x16_t;
                return vaddvq_u8(reinterpret_cast<V>(vec.v));
            }
        } else if constexpr (sizeof(T) == 0x20) {
            if constexpr (std::is_same_v<Type,float>) {
                using V = float32x4x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_f32(v0);
                return v1;
            } else if constexpr (std::is_same_v<Type,double>) {
                using V = float64x2x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_f64(v0);
                return v1;
            } else if constexpr (std::is_same_v<Type,s64>) {
                using V = int64x2x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_s64(v0);
                return v1;
            } else if constexpr (std::is_same_v<Type,s32>) {
                using V = int32x4x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_s32(v0);
                return v1;
            } else if constexpr (std::is_same_v<Type,s16>) {
                using V = int16x8x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_s16(v0);
                return v1;
            } else if constexpr (std::is_same_v<Type,s8>) {
                using V = int8x16x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_s8(v0);
                return v1;
            } else if constexpr (std::is_same_v<Type,u64>) {
                using V = uint64x2x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_u64(v0);
                return v1;
            } else if constexpr (std::is_same_v<Type,u32>) {
                using V = uint32x4x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_u32(v0);
                return v1;
            } else if constexpr (std::is_same_v<Type,u16>) {
                using V = uint16x8x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_u16(v0);
                return v1;
            } else if constexpr (std::is_same_v<Type,u8>) {
                using V = uint8x16x2_t;
                const V v = *reinterpret_cast<const V*>(std::addressof(vec.v));
                const auto v0 = v.val[0] + v.val[1];
                const auto v1 = vaddvq_u8(v0);
                return v1;
            }
        }
#endif
        }
        }
        Type ret = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            ret += vec[i];
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Average(const T *array, u32 count) {
        T m = {};
        for (u32 i = 0; i < count; ++i) {
            m = m + array[i];
        }
        m = m / static_cast<typename T::UnderlyingType>(count);
        return m;
    }

    namespace impl {

        template <typename T, typename Ret>
        ALWAYS_INLINE Ret VectorizedOp(std::add_pointer_t<T(T,T)> op, const Ret &v0, const Ret &v1) {
            const T l = reinterpret_cast<T>(v0.v);
            const T r = reinterpret_cast<T>(v1.v);
            const T result = (op)(l,r);
            return Ret { reinterpret_cast<typename Ret::VectorizedType>(result), };
        }

        template <typename T, typename Ret>
        ALWAYS_INLINE Ret VectorizedOp(std::add_pointer_t<T(T)> op, const Ret &v0) {
            const T l = reinterpret_cast<T>(v0.v);
            const T result = (op)(l);
            return Ret { reinterpret_cast<typename Ret::VectorizedType>(result), };
        }

        template <typename T>
            requires (T::cIsVectorType && T::cIsVectorized)
        constexpr u32 MoveMask_8bit(const T &a) {

            /* Runtime intrinsic  */
            if (!std::is_constant_evaluated()) {
    #ifdef VP_TARGET_ARCHITECTURE_x86
                if constexpr (sizeof(T) == 0x10) {
                    typedef char V __attribute__((vector_size(16)));
                    const V l = reinterpret_cast<V>(a.v);
                    return __builtin_ia32_pmovmskb128(l);
                } else if (sizeof(T) == 0x20) {
                    typedef char V __attribute__((vector_size(32)));
                    const V l = reinterpret_cast<V>(a.v);
                    return __builtin_ia32_pmovmskb256(l);
                }
    #endif
            }

            /* Build mask of most significant bit in 8-bit component */
            u32 index = 0;
            u32 out   = 0;
            for (u32 i = 0; i < sizeof(T); ++i) {
                index  = i;
                out   |= ((reinterpret_cast<const u8*>(std::addressof(a))[index] >> static_cast<u8>(0x7)) << i);
            }
            return out;
        }
    }

    /* Mask comparison */
    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T CompareEqual(const T &left, const T &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T ret = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            ret[i] = (left[i] == right[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return ret;
    }
    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T CompareGreaterThan(const T &left, const T &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T ret = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            ret[i] = (left[i] > right[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return ret;
    }
    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T CompareLessThan(const T &left, const T &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T ret = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            ret[i] = (left[i] < right[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return ret;
    }
    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T CompareGreaterThanEqual(const T &left, const T &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T ret = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            ret[i] = (left[i] >= right[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return ret;
    }
    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T CompareLessThanEqual(const T &left, const T &right) {
        using Type = typename T::UnderlyingType;
        using MaskType = TMinUIntType<Type>;
        T ret = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            ret[i] = (left[i] <= right[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return ret;
    }

    /* Comparison operators */
    template <typename T>
        requires (T::cIsVectorType)
    constexpr bool operator==(const T &left, const T &right) {

        using Type = typename T::UnderlyingType;
        if constexpr (T::cIsVectorized) {
        if (!std::is_constant_evaluated()) {
        //#ifdef VP_TARGET_ARCHITECTURE_x86
        //    if constexpr (sizeof(T) == 0x10) {
        //        const auto c = CompareEqual(left, right);
        //        typedef char V __attribute__((vector_size(16)));
        //        const V l = reinterpret_cast<V>(c.v);
        //        return 0xffff == __builtin_ia32_pmovmskb128(l);
        //    } else if constexpr (sizeof(T) == 0x20) {
        //        const auto c = CompareEqual(left, right);
        //        typedef char V __attribute__((vector_size(32)));
        //        const V l = reinterpret_cast<V>(c.v);
        //        return 0xffff'ffff == __builtin_ia32_pmovmskb256(l);
        //    }
        //#elifdef VP_TARGET_ARCHITECTURE_aarch64
        //    if constexpr (sizeof(T) == 0x10) {
        //        const auto c = CompareEqual(left, right);
        //        using V = uint16x8_t;
        //        const V l = reinterpret_cast<V>(c.v);
        //        const u64 sh0 = reinterpret_cast<u64>(vshrn_n_u16(l, 4));
        //        return 0xffff'ffff'ffff'ffff == sh0;
        //    } else if constexpr (sizeof(T) == 0x20) {
        //        const auto c = CompareEqual(left, right);
        //        using V = uint16x8x2_t;
        //        const V l = *reinterpret_cast<const V*>(std::addressof(c.v));
        //        const u64 sh0 = reinterpret_cast<u64>(vshrn_n_u16(l.val[0], 4));
        //        const u64 sh1 = reinterpret_cast<u64>(vshrn_n_u16(l.val[1], 4));
        //        return 0xffff'ffff'ffff'ffff == (sh0 & sh1);
        //    }
        //#endif
        }
        }
        bool result = true;
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            const Type l = left[i]; 
            const Type r = right[i]; 
            result &= l == r;
        }
        return result;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr bool operator>(const T &left, const T &right) {
        bool result = true;
        using Type = typename T::UnderlyingType;
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            const Type l = left[i]; 
            const Type r = right[i]; 
            result &= l > r;
        }
        return result;
    }
    template <typename T>
        requires (T::cIsVectorType)
    constexpr bool operator>=(const T &left, const T &right) {
        bool result = true;
        using Type = typename T::UnderlyingType;
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            const Type l = left[i]; 
            const Type r = right[i]; 
            result &= l >= r;
        }
        return result;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr bool operator<(const T &left, const T &right) {
        return !(left >= right);
    }
    template <typename T>
        requires (T::cIsVectorType)
    constexpr bool operator<=(const T &left, const T &right) {
        return !(left > right);
    }

    /* Conditional select */
    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T InsertTrue(const T &base, const T &insert, const T &mask) {
        /* Equivalent to aarch64 bit  */
        const T result = base ^ ((base ^ insert) & mask);
        return result;
    }
    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T InsertFalse(const T &base, const T &insert, const T &mask) {
        /* Equivalent to aarch64 bif  */
        const T result = base ^ ((base ^ insert) & (~mask));
        return result;
    }
    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T Select(const T &base, const T &insert, const T &mask) {
        /* Equivalent to aarch64 bsl  */
        const T result = mask ^ ((mask ^ insert) & base);
        return result;
    }

    /* Linear algebra */
    template <typename T>
        requires (T::cIsVectorType)
    constexpr typename T::UnderlyingType Dot(const T &left, const T &right) {
        using Type = typename T::UnderlyingType;
        const T temp = left * right;
        const Type ret = Sum(temp);
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T::UnderlyingType MagnitudeSquared(const T &vector) {
        using Type = typename T::UnderlyingType;
        const Type ret = Dot(vector,vector);
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T::UnderlyingType Magnitude(const T &vector) {
        using Type = typename T::UnderlyingType;
        const Type squared_summation = MagnitudeSquared(vector);
        const Type out = SquareRoot(squared_summation);
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T MagnitudeVector(const T &vector) {
        using Type = typename T::UnderlyingType;
        const Type squared_summation = MagnitudeSquared(vector);
        const T sq_broad = Broadcast<T>(squared_summation);
        const T out = SquareRoot(sq_broad);
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, float>)
    constexpr T NormalizeByMagnitude(const T &vector, typename T::UnderlyingType mag) {
        T temp = vector;
        const float mag0 = mag;
        if (cFloatNormalizeEpsilon < mag0) {            
            if (mag0 == cFloatInfinity) {
                const float cMul = (T::cCount < 4) ? 3.129822e-20f : 2.7105054e-20f;
                temp = temp * cMul;
                const float mag1 = Magnitude(temp);
                temp = temp * mag1;
            } else {
                temp = temp / mag0;
            }
        } else {
            temp = T{};
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, double>)
    constexpr T NormalizeByMagnitude(const T &vector, typename T::UnderlyingType mag) {
        T temp = vector;
        const double mag0 = mag;
        if (cDoubleNormalizeEpsilon < mag0) {
            if (mag0 == cFloatInfinity) {
                const float cMul = (T::cCount < 4) ? 4.306075028866389e-155 : 3.7291703656001034e-155;
                temp = temp * cMul;
                const double mag1 = Magnitude(temp);
                temp = temp * mag1;
            } else {
                temp = temp / mag0;
            }
        } else {
            temp = T{};
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType && (std::is_same_v<typename T::UnderlyingType, float> || std::is_same_v<typename T::UnderlyingType, double>))
    constexpr T Normalize(const T &vector) {
        const typename T::UnderlyingType mag = Magnitude(vector);
        return NormalizeByMagnitude(vector, mag);
    }
    template <typename T>
        requires (T::cIsVectorType && T::cCount == 3)
    constexpr T Cross(const T &left, const T &right) {
        if constexpr (T::cIsVectorized) {
            const auto a = __builtin_shufflevector(left.v, left.v, 1,2,0,3);
            const auto b = __builtin_shufflevector(right.v, right.v, 2,0,1,3);
            const auto c = a * right.v;
            const auto d = a * b;
            const auto e = __builtin_shufflevector(c, c, 1, 2, 0, 3);
            return T{ d - e, };
        }
        const typename T::UnderlyingType out_x = (left[1] * right[2]) - (left[2] * right[1]);
        const typename T::UnderlyingType out_y = (left[2] * right[0]) - (left[0] * right[2]);
        const typename T::UnderlyingType out_z = (left[0] * right[1]) - (left[1] * right[0]);
        return T{ out_x, out_y, out_z, };
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Project(const T &from, const T &onto) {
        using Type = typename T::UnderlyingType;
        Type d = {};
        Type m = {};
        for (u32 i = 0; i < T::cVectorizedCount; ++i) {
            d += from[i] * onto[i];
            m += onto[i] * onto[i];
        }
        const Type k = d / m;
        const T proj = onto * k;
        return proj;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Reject(const T &from, const T &onto) {
        const T proj = Project(from, onto);
        return from - proj;
    }

    /* Special */
    template <typename T>
        requires (T::cIsVectorType)
    constexpr typename T::UnderlyingType MaxElement(const T& vec) {
        using Type = typename T::UnderlyingType;
        Type temp = vec[0];
        for (u32 i = 1; i < T::cCount; ++i) {
            temp = Max(temp, vec[i]);
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cCount == 3 && (std::is_same_v<typename T::UnderlyingType,float> || std::is_same_v<typename T::UnderlyingType,double>))
    constexpr T ModuloPi(const T &value) {
        using Type = typename T::UnderlyingType;
        constexpr const Type s_pi = (sizeof(T) == sizeof(float)) ? cFloatPi : cDoublePi;
        constexpr const Type s_one_div_pi2 = (sizeof(T) == sizeof(float)) ? cFloat1Divided2Pi : cDouble1Divided2Pi;
        constexpr const Type s_pi2 = (sizeof(T) == sizeof(float)) ? cFloat2Pi : cDouble2Pi;
        constexpr const Type s_pi_div_2 = (sizeof(T) == sizeof(float)) ? cFloatPiDivided2 : cDoublePiDivided2;
        const T v_pi = Broadcast<T>(s_pi);
        const T v_neg_pi = Broadcast<T>(-s_pi);
        const T v_one_div_pi2 = Broadcast<T>(s_one_div_pi2);
        const T v_pi2 = Broadcast<T>(s_pi2);
        const T v_pi_div_2 = Broadcast<T>(s_pi_div_2);
        const T v_neg_pi_div_2 = Broadcast<T>(-s_pi_div_2);

        /* p5 = (0 <= v) ? 0.5f : -0.5f; */
        const T point_5 = Broadcast<T>(0.5f);
        const T neg_point_5 = Broadcast<T>(-0.5f);
        const T mask = CompareLessThan(value, T{});
        const T sel0 = InsertTrue(point_5, neg_point_5, mask);

        /* r = round(p5 + (v * (1/(2pi)))) */
        const T add0 = sel0 + (value * v_one_div_pi2);
        const TVector<s32,3,T::cIsVectorized> round = Convert<TVector<s32,3,T::cIsVectorized>>(add0);
        const T adj0 = Convert<T>(round);
        /* a = v - (r * 2pi) */
        const T adj1 = value - (adj0 * v_pi2);
        /* s0 = (v > (pi/2)) ? (pi - a) : a; */
        const T mask1 = CompareLessThan(v_pi_div_2, adj1);
        const T adj2 = v_pi - adj1;
        const T ins0 = InsertTrue(adj1, adj2, mask1);
        /* ret = (-(pi/2) > s0) ? (-pi - s0) : s0; */
        const T mask2 = CompareLessThan(ins0, v_neg_pi_div_2);
        const T adj3 = v_neg_pi - adj1;
        const T ins2 = InsertTrue(ins0, adj3, mask2);
        return ins2;
    }
}

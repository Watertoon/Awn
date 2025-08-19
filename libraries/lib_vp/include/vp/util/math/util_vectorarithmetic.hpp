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
        requires (T::cIsVectorType)
    constexpr T operator+(const T &left, const T &right) {
        T out;
        if constexpr (T::cIsVectorized) {
            out = { .v = left.v + right.v, };
        } else {
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                out[i] = left[i] + right[i];
            }
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator-(const T &left, const T &right) {
        T out;
        if constexpr (T::cIsVectorized) {
            out = { .v = left.v - right.v, };
        } else {
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                out[i] = left[i] - right[i];
            }
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator-(const T &vec) {
        T out;
        if constexpr (T::cIsVectorized) {
            out = { .v = -vec.v, };
        } else {
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                out[i] = -vec[i];
            }
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator*(const T &left, const T &right) {
        T out;
        if constexpr (T::cIsVectorized) {
            out = { .v = left.v * right.v, };
        } else {
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                out[i] = left[i] * right[i];
            }
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator*(const T &left, const typename T::UnderlyingType &right) {
        T out;
        if constexpr (T::cIsVectorized) {
            out = { .v = left.v * right, };
        } else {
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                out[i] = left[i] * right;
            }
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator/(const T &left, typename T::UnderlyingType scalar) {
        T out;
        if constexpr (T::cIsVectorized) {
            out = { .v = left.v / scalar, };
        } else {
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                out[i] = left[i] / scalar;
            }
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr bool operator==(const T &left, const T &right) {
        bool result = true;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            result &= (left[i] == right[i]);
        }
        return result;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr bool operator!=(const T &left, const T &right) {
        return !(left == right);
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator&(const T &left, const T &right) {
        T out;
        if constexpr (T::cIsVectorized) {
            out = T{ .v = left.v & right.v, };
        } else {
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                out[i] = left.v[i] & right[i];
            }
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator|(const T &left, const T &right) {
        T out;
        if constexpr (T::cIsVectorized) {
            out = T{ .v = left.v | right.v, };
        } else {
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                out[i] = left.v[i] | right[i];
            }
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T operator~(const T &vector) {
        T out;
        if constexpr (T::cIsVectorized) {
            out = T{ .v = ~vector.v, };
        } else {
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                out[i] = ~vector[i];
            }
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T CompareGreaterThan(const T &lhs, const T &rhs) {
        using Type = typename T::UnderlyingType;
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            return T{ .v = CompareGreaterThan4(lhs.v, rhs.v), };
        }
        using MaskType = MinUIntType<1ull << (BITSIZEOF(Type) - 1)>;
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = (lhs[i] > rhs[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T CompareGreaterThanEqual(const T &lhs, const T &rhs) {
        using Type = typename T::UnderlyingType;
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            return T{ .v = CompareGreaterThanEqual4(lhs.v, rhs.v), };
        }
        using MaskType = MinUIntType<1ull << (BITSIZEOF(Type) - 1)>;
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = (lhs[i] >= rhs[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T CompareEqual(const T &lhs, const T &rhs) {
        using Type = typename T::UnderlyingType;
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            return T{ .v = CompareEqual4(lhs.v, rhs.v), };
        }
        using MaskType = MinUIntType<1ull << (BITSIZEOF(Type) - 1)>;
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = (lhs[i] == rhs[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4) && sizeof(typename T::UnderlyingType) == 4)
    constexpr u32 MoveMask(const T &vec) {
        return MoveMask128(vec.v);
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T InsertTrue(const T &base, const T &insert, const T &mask) {
        using Type = typename T::UnderlyingType;
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            return T{ .v = InsertTrue4(base.v, insert.v, mask.v), };
        }
        using MaskType = MinUIntType<1ull << (BITSIZEOF(Type) - 1)>;
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = std::bit_cast<Type>(std::bit_cast<MaskType>(base[i]) ^ ((std::bit_cast<MaskType>(base[i]) ^ std::bit_cast<MaskType>(insert[i])) & std::bit_cast<MaskType>(mask[i])));
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T InsertFalse(const T &base, const T &insert, const T &mask) {
        using Type = typename T::UnderlyingType;
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            return T{ .v = InsertFalse4(base.v, insert.v, mask.v), };
        }
        using MaskType = MinUIntType<1ull << (BITSIZEOF(Type) - 1)>;
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = std::bit_cast<Type>(std::bit_cast<MaskType>(base[i]) ^ ((std::bit_cast<MaskType>(base[i]) ^ std::bit_cast<MaskType>(insert[i])) & (~std::bit_cast<MaskType>(mask[i]))));
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Select(const T &base, const T &insert, const T &mask) {
        using Type = typename T::UnderlyingType;
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            return T{ .v = Select4(base.v, insert.v, mask.v), };
        }
        using MaskType = MinUIntType<1ull << (BITSIZEOF(Type) - 1)>;
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = std::bit_cast<Type>(std::bit_cast<MaskType>(mask[i]) ^ ((std::bit_cast<MaskType>(mask[i]) ^ std::bit_cast<MaskType>(insert[i])) & std::bit_cast<MaskType>(base[i])));
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T::UnderlyingType Sum(const T &vector) {
        using Type = typename T::UnderlyingType;
        Type ret = {};
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            ret += vector[i];
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T::UnderlyingType SquaredSum(const T &vector) {
        using Type = typename T::UnderlyingType;
        const T sq = vector * vector;
        const Type ret = Sum(sq);
        return ret;
    }

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
    constexpr T::UnderlyingType Magnitude(const T &vector) {
        using Type = typename T::UnderlyingType;
        const Type squared_summation = SquaredSum(vector);
        const Type out = SquareRoot(squared_summation);
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T MagnitudeVector(const T &vector) {
        using Type = typename T::UnderlyingType;
        const Type squared_summation = SquaredSum(vector);
        const T sq_broad = BroadcastVector<Type, T::cVectorCount>(squared_summation);
        const T out = SquareRoot(sq_broad);
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T ScalarMultiply(const T &vector, typename T::UnderlyingType scalar) {
        T ret;
        if constexpr (T::cIsVectorized) {
            ret.v = vector.v * scalar;
        } else {            
            for (u32 i = 0; i < T::cVectorCount; ++i) {
                ret[i] = vector[i] * scalar;
            }
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, float>)
    constexpr T Normalize(const T &vector) {
        T temp = vector;  
        const float mag0 = Magnitude(vector);
        if (cFloatNormalizeEpsilon < mag0) {            
            if (mag0 == cFloatInfinity) {
                temp = ScalarMultiply(temp, 3.129822e-20f);
                const float mag1 = Magnitude(temp);
                temp = ScalarMultiply(temp, mag1);
            } else {
                const float reciprocal_mag = 1.0f / mag0;
                temp = ScalarMultiply(temp, reciprocal_mag);
            }
        } else {
            temp = T::cZero;
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, double>)
    constexpr T Normalize(const T &vector) {
        T temp = vector;  
        const double mag0 = Magnitude(vector);
        if (cDoubleNormalizeEpsilon < mag0) {
            if (mag0 == cFloatInfinity) {
                temp = ScalarMultiply(temp, 4.306075028866389e-155);
                const double mag1 = Magnitude(temp);
                temp = ScalarMultiply(temp, mag1);
            } else {
                const double reciprocal_mag = 1.0f / mag0;
                temp = ScalarMultiply(temp, reciprocal_mag);
            }
        } else {
            temp = T::cZero;
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cVectorCount == 3)
    constexpr T Cross(const T &left, const T &right) {
#ifdef VP_TARGET_ARCHITECTURE_x86
        if constexpr (T::cIsVectorized) {            
            using v4t = TVectorType<typename T::UnderlyingType, 4>;
            const v4t a = Swizzle4(left.v, GetSwizzle4Order(1,2,0,3));
            const v4t b = Swizzle4(right.v, GetSwizzle4Order(2,0,1,3));
            const v4t c = v4t{ a.v * right.v.v, };
            const v4t d = v4t{ a.v * b.v, };
            const v4t e = Swizzle4(c, GetSwizzle4Order(1,2,0,3));
            return T{ .v = d.v - e.v, };
        }
#endif
        const typename T::UnderlyingType out_x = (left[1] * right[2]) - (left[2] * right[1]);
        const typename T::UnderlyingType out_y = (left[2] * right[0]) - (left[0] * right[2]);
        const typename T::UnderlyingType out_z = (left[0] * right[1]) - (left[1] * right[0]);
        return T{ out_x, out_y, out_z, };
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Reciprocal(const T &vec) {
        T result;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            result[i] = static_cast<typename T::UnderlyingType>(1.0) / vec[i];
        }
        return result;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Project(const T &from, const T &onto) {
        using Type = typename T::UnderlyingType;
        Type d = {};
        Type m = {};
        for (u32 i = 0; i < T::cVectorCount; ++i) {
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

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T SquareRoot(const T &vec) {
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            const T ret = { .v = SquareRoot(vec.v), };
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            ret[i] = SquareRoot(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T ReciprocalSquareRoot(const T &vec) {
        using Type = typename T::UnderlyingType;
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            const T ret = { .v = ReciprocalSquareRootNewtonRaphson(vec.v), };
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            ret[i] = static_cast<Type>(1) / SquareRoot(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Absolute(const T &vec) {
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            const T ret = { .v = Absolute(vec.v), };
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            ret[i] = Absolute(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<float, typename T::UnderlyingType>)
    constexpr T CoefficientsSin(const T &vec) {
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            const T ret = { .v = CoefficientsSin(vec.v), };
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            ret[i] = CoefficientsSin(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<float, typename T::UnderlyingType>)
    constexpr T CoefficientsCos(const T &vec) {
        if constexpr (T::cIsVectorized && (T::cVectorCount == 3 || T::cVectorCount == 4)) {
            const T ret = { .v = CoefficientsCos(vec.v), };
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            ret[i] = CoefficientsCos(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T MultiplyAdd(const T& a, const T& b, const T& c) {
        if constexpr (T::cIsVectorized) {
            return T { .v = MultiplyAdd(a.v, b.v, c.v), };
        }
        T temp;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            temp[i] = (a[i] * b[i]) + c[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr typename T::UnderlyingType MaxElement(const T& vec) {
        using Type = typename T::UnderlyingType;
        Type temp = vec[0];
        for (u32 i = 1; i < T::cVectorCount; ++i) {
            temp = Max(temp, vec[i]);
        }
        return temp;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsVectorType && T::cIsVectorType)
    constexpr Ret CastVector(const T &vector) {
        constexpr u32 cMinCount = Min(Ret::cVectorCount, T::cVectorCount);
        Ret out;
        for (u32 i = 0; i < cMinCount; ++i) {
            out[i] = static_cast<typename Ret::UnderlyingType>(vector[i]);
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector2f CastVector2f(const T &vector) {
        const Vector2f ret = CastVector<Vector2f>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector3f CastVector3f(const T &vector) {
        const Vector3f ret = CastVector<Vector3f>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector4f CastVector4f(const T &vector) {
        const Vector4f ret = CastVector<Vector4f>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector2d CastVector2d(const T &vector) {
        const Vector2d ret = CastVector<Vector2d>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector3d CastVector3d(const T &vector) {
        const Vector3d ret = CastVector<Vector3d>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector4d CastVector4d(const T &vector) {
        const Vector4d ret = CastVector<Vector4d>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector2i CastVector2i(const T &vector) {
        const Vector2i ret = CastVector<Vector2i>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector3i CastVector3i(const T &vector) {
        const Vector3i ret = CastVector<Vector3i>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector4i CastVector4i(const T &vector) {
        const Vector4i ret = CastVector<Vector4i>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Float2 CastFloat2(const T &vector) {
        const Float2 ret = CastVector<Float2>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Float3 CastFloat3(const T &vector) {
        const Float3 ret = CastVector<Float3>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Float4 CastFloat4(const T &vector) {
        const Float4 ret = CastVector<Float4>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Double2 CastDouble2(const T &vector) {
        const Double2 ret = CastVector<Double2>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Double3 CastDouble3(const T &vector) {
        const Double3 ret = CastVector<Double3>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Double4 CastDouble4(const T &vector) {
        const Double4 ret = CastVector<Double4>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Int2 CastInt2(const T &vector) {
        const Int2 ret = CastVector<Int2>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Int3 CastInt3(const T &vector) {
        const Int3 ret = CastVector<Int3>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Int4 CastInt4(const T &vector) {
        const Int4 ret = CastVector<Int4>(vector);
        return ret;
	}

    template <typename T, u32 Count, bool IsVectorized = true>
    constexpr TVector<T, Count, IsVectorized> BroadcastVector(T scalar) {
        using VectorType = TVector<T, Count, IsVectorized>;
        if constexpr (VectorType::cIsVectorized && (VectorType::cVectorCount == 3 || VectorType::cVectorCount == 4)) {
            const VectorType ret = VectorType{ .v = Broadcast4(scalar), };
            return ret;
        }
        VectorType ret;
        for (u32 i = 0; i < VectorType::cVectorCount; ++i) {
            ret[i] = scalar;
        }
        return ret;
    }

    template <typename T>
    constexpr TVector<T, 2> BroadcastVector2(T scalar) {
        return BroadcastVector<T, 2>(scalar);
    }

    template <typename T>
    constexpr TVector<T, 3> BroadcastVector3(T scalar) {
        return BroadcastVector<T, 3>(scalar);
    }

    template <typename T>
    constexpr TVector<T, 4> BroadcastVector4(T scalar) {
        return BroadcastVector<T, 4>(scalar);
    }

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized && std::is_same_v<float, typename T::UnderlyingType>)
    constexpr T ModuloPi(T value) {
        using Type = typename T::UnderlyingType;
        constexpr Type s_pi = (sizeof(T) == sizeof(float)) ? cFloatPi : cDoublePi;
        constexpr Type s_one_div_pi2 = (sizeof(T) == sizeof(float)) ? cFloat1Divided2Pi : cDouble1Divided2Pi;
        constexpr Type s_pi2 = (sizeof(T) == sizeof(float)) ? cFloat2Pi : cDouble2Pi;
        constexpr Type s_pi_div_2 = (sizeof(T) == sizeof(float)) ? cFloatPiDivided2 : cDoublePiDivided2;
        constexpr T v_pi = { s_pi, s_pi, s_pi, s_pi, };
        constexpr T v_neg_pi = { -s_pi, -s_pi, -s_pi, -s_pi, };
        constexpr T v_one_div_pi2 = { s_one_div_pi2, s_one_div_pi2, s_one_div_pi2, s_one_div_pi2, };
        constexpr T v_pi2 = { s_pi2, s_pi2, s_pi2, s_pi2, };
        constexpr T v_pi_div_2 = { s_pi_div_2, s_pi_div_2, s_pi_div_2, s_pi_div_2, };
        constexpr T v_neg_pi_div_2 = { -s_pi_div_2, -s_pi_div_2, -s_pi_div_2, -s_pi_div_2, };

        constexpr const T point_5 = { 0.5f, 0.5f, 0.5f, 0.5f, };
        constexpr const T neg_point_5 = { 0.5f, 0.5f, 0.5f, 0.5f, };
        const T mask = CompareGreaterThanEqual(value, T::cZero);
        const T sel0 = Select(point_5, neg_point_5, mask);
        const T add0 = sel0 + (value * v_one_div_pi2);
        const Vector4i round = CastVector4i(add0);
        const T adj0 = CastVector<T>(round);
        const T adj1 = value - (adj0 * v_pi2);
        const T mask1 = CompareGreaterThan(adj1, v_pi_div_2);
        const T adj2 = v_pi - adj1;
        const T ins0 = InsertTrue(adj1, adj2, mask1);
        const T mask2 = CompareGreaterThan(v_neg_pi_div_2, ins0);
        const T adj3 = v_neg_pi - adj1;
        const T ins2 = InsertTrue(ins0, adj3, mask2);
        return ins2;
    }
}

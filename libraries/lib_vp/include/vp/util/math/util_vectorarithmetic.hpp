#pragma once

namespace vp::util {

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T operator+(const T &left, const T &right) {
        const T out = { .v = left.v.v + right.v.v, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T operator-(const T &left, const T &right) {
        const T out = { .v = left.v.v - right.v.v, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T operator-(const T &vec) {
        const T out = { .v = -vec.v.v, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T operator*(const T &left, const T &right) {
        const T out = { .v = left.v.v * right.v.v, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T operator*(const T &left, const typename T::UnderlyingType &right) {
        const T out = { .v = left.v.v * right, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && T::cIsVectorized)
    constexpr T operator/(const T &left, typename T::UnderlyingType scalar) {
        const T out = { .v = left.v.v / scalar, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && !T::cIsVectorized)
    constexpr T operator+(const T &left, const T &right) {
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = left[i] + right[i];
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && !T::cIsVectorized)
    constexpr T operator-(const T &left, const T &right) {
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = left[i] - right[i];
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && !T::cIsVectorized)
    constexpr T operator-(const T &vec) {
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = -vec[i];
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && !T::cIsVectorized)
    constexpr T operator*(const T &left, const T &right) {
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = left[i] * right[i];
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && !T::cIsVectorized)
    constexpr T operator*(const T &left, const typename T::UnderlyingType &right) {
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = left[i] * right;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType && !T::cIsVectorized)
    constexpr T operator/(const T &left, typename T::UnderlyingType scalar) {
        T out;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            out[i] = left[i] / scalar;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T::UnderlyingType Magnitude(const T &vector) {
        typename T::UnderlyingType squared_summation = {};
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            squared_summation += vector[i] * vector[i];
        }
        const typename T::UnderlyingType out = SquareRoot(squared_summation);
        return out;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T ScalarMultiply(const T &vector, typename T::UnderlyingType scalar) {
        T ret;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            ret[i] = vector[i] * scalar;
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, float>)
    constexpr T Normalize(const T &vector) {
        T temp = vector;  
        const float mag0 = Magnitude(vector);
        if (mag0 == cFloatInfinity) {
            temp = ScalarMultiply(temp, 3.129822e-20f);
            const float mag1 = Magnitude(temp);
            temp = ScalarMultiply(temp, mag1);
        } else if (cFloatNormalizeEpsilon < mag0) {
            const float reciprocal_mag = 1.0f / mag0;
            temp = ScalarMultiply(temp, reciprocal_mag);
        } else {
            temp = T{};
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
            const v4t c = v4f{ a.v * right.v.v, };
            const v4t d = v4f{ a.v * b.v, };
            const v4t e = Swizzle4(c, GetSwizzle4Order(1,2,0,3));
            return T{ .v = d.v - e.v, };
        }
#endif
        const typename T::UnderlyingType out_x = (left[1] * right[2]) - (left[2] * right[1]);
        const typename T::UnderlyingType out_y = -((left[0] * right[2]) - (left[2] * right[0]));
        const typename T::UnderlyingType out_z = (left[0] * right[1]) - (left[1] * right[0]);
        return T{ out_x, out_y, out_z, };
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr typename T::UnderlyingType Dot(const T &left, const T &right) {
        const T temp = left * right;
        typename T::UnderlyingType dot = {};
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            dot += temp[i];
        }
        return dot;
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

    namespace {

        template <typename Ret, typename T>
            requires (Ret::cIsVectorType && T::cIsVectorType)
        constexpr Ret CastVectorImpl(const T &vector) {
            constexpr u32 cMinCount = Min(Ret::cVectorCount, T::cVectorCount);
            Ret out;
            for (u32 i = 0; i < cMinCount; ++i) {
                out[i] = static_cast<typename Ret::UnderlyingType>(vector[i]);
            }
            return out;
        }
    }

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector2f CastVector2f(const T &vector) {
        const Vector2f ret = CastVectorImpl<Vector2f>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector3f CastVector3f(const T &vector) {
        const Vector3f ret = CastVectorImpl<Vector3f>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector4f CastVector4f(const T &vector) {
        const Vector4f ret = CastVectorImpl<Vector4f>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector2d CastVector2d(const T &vector) {
        const Vector2d ret = CastVectorImpl<Vector2d>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector3d CastVector3d(const T &vector) {
        const Vector3d ret = CastVectorImpl<Vector3d>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector4d CastVector4d(const T &vector) {
        const Vector4d ret = CastVectorImpl<Vector4d>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector2i CastVector2i(const T &vector) {
        const Vector2i ret = CastVectorImpl<Vector2i>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector3i CastVector3i(const T &vector) {
        const Vector3i ret = CastVectorImpl<Vector3i>(vector);
        return ret;
	}

    template <typename T>
        requires (T::cIsVectorType)
	constexpr Vector4i CastVector4i(const T &vector) {
        const Vector4i ret = CastVectorImpl<Vector4i>(vector);
        return ret;
	}

}

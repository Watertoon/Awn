#pragma once

namespace vp::util {

    template <typename Ret, typename R>
        requires (Ret::cIsMatrixType && R::cIsVectorType && R::cCount == 3)
    constexpr Ret EulerXYZToMatrix(const R &rotation_euler) {

        const R mod_eu = ModuloPi(rotation_euler);
        const R sin_eu = CoefficientsSin(mod_eu);
        const R cos_eu = CoefficientsCos(mod_eu);
        Ret ret = {};
        const R row0 = R{  cos_eu[0] * cos_eu[1],                                         sin_eu[2] * cos_eu[1],                                        -sin_eu[1], };
        const R row1 = R{ (cos_eu[0] * sin_eu[0] * sin_eu[1]) - (sin_eu[2] * cos_eu[2]), (sin_eu[0] * sin_eu[1] * sin_eu[2]) + (cos_eu[0] * cos_eu[2]),  sin_eu[0] * cos_eu[1], };
        const R row2 = R{ (sin_eu[1] * cos_eu[0] * cos_eu[2]) + (sin_eu[2] * sin_eu[1]), (sin_eu[1] * sin_eu[2] * cos_eu[2]) - (cos_eu[0] * sin_eu[1]),  cos_eu[1] * cos_eu[2], };
        StoreRow(std::addressof(ret), row0, 0);
        StoreRow(std::addressof(ret), row1, 1);
        StoreRow(std::addressof(ret), row2, 2);

        return ret;
    }

    template <typename Ret, typename S, typename R>
        requires (Ret::cIsMatrixType && S::cIsVectorType && S::cCount == 3 && R::cIsVectorType && R::cCount == 3)
    constexpr Ret ScaleEulerXYZToMatrix(const S &scale, const R &rotation_euler) {

        const R mod_eu = ModuloPi(rotation_euler);
        const R sin_eu = CoefficientsSin(mod_eu);
        const R cos_eu = CoefficientsCos(mod_eu);
        Ret ret = {};
        const R row0 = R{  cos_eu[0] * cos_eu[1],                                         sin_eu[2] * cos_eu[1],                                        -sin_eu[1], } * scale[0];
        const R row1 = R{ (cos_eu[0] * sin_eu[0] * sin_eu[1]) - (sin_eu[2] * cos_eu[2]), (sin_eu[0] * sin_eu[1] * sin_eu[2]) + (cos_eu[0] * cos_eu[2]),  sin_eu[0] * cos_eu[1], } * scale[1];
        const R row2 = R{ (sin_eu[1] * cos_eu[0] * cos_eu[2]) + (sin_eu[2] * sin_eu[1]), (sin_eu[1] * sin_eu[2] * cos_eu[2]) - (cos_eu[0] * sin_eu[1]),  cos_eu[1] * cos_eu[2], } * scale[2];
        StoreRow(std::addressof(ret), row0, 0);
        StoreRow(std::addressof(ret), row1, 1);
        StoreRow(std::addressof(ret), row2, 2);

        return ret;
    }

    template <typename Ret, typename S, typename R, typename T>
        requires (Ret::cIsMatrixType && S::cIsVectorType && S::cCount == 3 && R::cIsVectorType && R::cCount == 3 && T::cIsVectorType && T::cCount == 3)
    constexpr Ret SRTEulerXYZToMatrix(const S &scale, const R &rotation_euler, const T &translation) {
        Ret ret = ScaleEulerXYZToMatrix<Ret>(scale, rotation_euler);
        StoreRow(std::addressof(ret), translation, 3);
        return ret;
    }

    template <typename V>
        requires (V::cIsVectorType && V::cCount == 3)
    constexpr void EulerXYZToAxes(V *out_axes, const V &rotation_euler) {
        const V mod_eu = ModuloPi(rotation_euler);
        const V sin_eu = CoefficientsSin(mod_eu);
        const V cos_eu = CoefficientsCos(mod_eu);
        const V ax0 = {
            cos_eu[1] * cos_eu[2],
            (sin_eu[0] * sin_eu[1] * cos_eu[2]) - (cos_eu[0] * sin_eu[2]),
            cos_eu[0] * cos_eu[2] + sin_eu[0] * sin_eu[1] * sin_eu[2],
            0.0f,
        };
        const V ax1 = {
            cos_eu[1] * sin_eu[2],
            (sin_eu[2] * cos_eu[0] * cos_eu[2]) + (sin_eu[0] * sin_eu[1]),
            cos_eu[0] * sin_eu[2] + sin_eu[1] * -(sin_eu[0] * cos_eu[2]),
            0.0f,
        };
        out_axes[0] = ax0;
        out_axes[1] = ax1;
        return;
    }

    template <typename V>
        requires (V::cIsVectorType && V::cCount == 3 && std::is_floating_point_v<typename V::UnderlyingType>)
    constexpr void LinearInterpolateAxes(V *from, const V *to, typename V::UnderlyingType factor) {
        const V b_ax0 = from[0];
        const V b_ax1 = from[1];
        const V a_ax0 = to[0];
        const V a_ax1 = to[1];
        const V r_ax0 = b_ax0 + a_ax0 * factor;
        const V r_ax1 = b_ax1 + a_ax1 * factor;
        from[0] = r_ax0;
        from[1] = r_ax1;
        return;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsVectorType && T::cIsMatrixType)
    constexpr Ret GetScale(const T &trans_matrix) {
        using Type = typename T::UnderlyingType;
        using V = TVector<Type, T::cColumnCount, T::cIsVectorized>;
        const V row0 = LoadRow(trans_matrix, 0);
        const V row1 = LoadRow(trans_matrix, 1);
        const V row2 = LoadRow(trans_matrix, 2);
        const Type m_row0 = Magnitude(row0);
        const Type m_row1 = Magnitude(row1);
        const Type m_row2 = Magnitude(row2);
        const Ret ret = { m_row0, m_row1, m_row2, };
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Parallelize(const T &base, const T &to_parallelize) {
        const float dot = Dot(base, to_parallelize);
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = to_parallelize[i] * dot;
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, float> && (T::cCount == 3 || T::cCount == 4))
    constexpr T RotateEulerAxisX(const T &vector, float angle) {
        const Float2 sin_cos = MethodSinCos(TrigMethod::Sample, angle);
        /*  | x                       |
         *  | y*sin(ang) - z*cos(ang) |
         *  | y*cos(ang) + z*sin(ang) |
         */
        const T ret = {
            vector[0],
            (vector[1] * sin_cos[0]) - (sin_cos[1] * vector[2]),
            (vector[1] * sin_cos[1]) + (sin_cos[0] * vector[2]),
        };
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, float> && (T::cCount == 3 || T::cCount == 4))
    constexpr T RotateEulerAxisY(const T &vector, float angle) {
        const Float2 sin_cos = MethodSinCos(TrigMethod::Sample, angle);
        /*  | z*cos(ang) + x*sin(ang) |
         *  | y                       |
         *  | z*sin(ang) - x*cos(ang) |
         */
        const T ret = {
            (vector[2] * sin_cos[1]) + (sin_cos[0] * vector[0]),
            vector[1],
            (vector[2] * sin_cos[0]) - (sin_cos[1] * vector[0]),
        };
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, float> && (T::cCount == 3 || T::cCount == 4))
    constexpr T RotateEulerAxisZ(const T &vector, float angle) {
        const Float2 sin_cos = MethodSinCos(TrigMethod::Sample, angle);
        /*  | x*sin(ang) - y*cos(ang) |
         *  | x*cos(ang) + y*sin(ang) |
         *  | z                       |
         */
        const T ret = {
            (vector[0] * sin_cos[0]) - (sin_cos[1] * vector[1]),
            (vector[0] * sin_cos[1]) + (sin_cos[0] * vector[1]),
            vector[2],
        };
        return ret;
    }

    template <typename T>
        requires (T::cIsMatrix && 3 <= T::cRowCount && 3 <= T::cColumnCount)
    constexpr void NormalizeRotation(T *rot_matrix) {
        using Type = typename T::UnderlyingType;
        TVector<Type, T::cColumnCount, T::cIsVectorized> v[3];
        v[0] = LoadRow(*rot_matrix,0);
        v[1] = LoadRow(*rot_matrix,1);
        v[2] = LoadRow(*rot_matrix,2);
        v[0] = Normalize(v[0]);
        v[1] = Normalize(v[1]);
        v[2] = Normalize(v[2]);
        StoreRow(rot_matrix,v[0],0);
        StoreRow(rot_matrix,v[1],1);
        StoreRow(rot_matrix,v[2],2);
    }

    template <typename T>
        requires (T::cIsMatrix && 3 <= T::cRowCount && 3 <= T::cColumnCount)
    constexpr void RotateLocalX(T *rot_matrix, float theta) {
        const Float2 sin_cos = MethodSinCos(TrigMethod::Sample, theta);
        const float m10 = (*rot_matrix)[1,0];
        const float m11 = (*rot_matrix)[1,1];
        const float m12 = (*rot_matrix)[1,2];
        (*rot_matrix)[1,0] = (m10 * sin_cos[1])                + ((*rot_matrix)[2,0] * sin_cos[0]);
        (*rot_matrix)[2,0] = ((*rot_matrix)[2,0] * sin_cos[1]) - (m10 * sin_cos[0]);
        (*rot_matrix)[1,1] = (m11 * sin_cos[1])                + ((*rot_matrix)[2,1] * sin);
        (*rot_matrix)[2,1] = ((*rot_matrix)[2,1] * sin_cos[1]) - (m11 * sin_cos[0]);
        (*rot_matrix)[1,2] = (m12 * sin_cos[1])                + ((*rot_matrix)[2,2] * sin_cos[0]);
        (*rot_matrix)[2,2] = ((*rot_matrix)[2,2] * sin_cos[1]) - (m12 * sin_cos[0]);
    }

    template <typename T>
        requires (T::cIsMatrix && 3 <= T::cRowCount && 3 <= T::cColumnCount)
    constexpr void RotateLocalY(T *rot_matrix, float theta) {
        const Float2 sin_cos = MethodSinCos(TrigMethod::Sample, theta);
        const float m00 = (*rot_matrix)[0,0];
        const float m01 = (*rot_matrix)[0,1];
        const float m02 = (*rot_matrix)[0,2];
        (*rot_matrix)[0,0] = (m00 * sin_cos[1])                - ((*rot_matrix)[2,0] * sin_cos[0]);
        (*rot_matrix)[2,0] = ((*rot_matrix)[2,0] * sin_cos[1]) + (m00 * sin_cos[0]);
        (*rot_matrix)[0,1] = (m01 * sin_cos[1])                - ((*rot_matrix)[2,1] * sin_cos[0]);
        (*rot_matrix)[2,1] = ((*rot_matrix)[2,1] * sin_cos[1]) + (m01 * sin_cos[0]);
        (*rot_matrix)[0,2] = (m02 * sin_cos[1])                - ((*rot_matrix)[2,2] * sin_cos[0]);
        (*rot_matrix)[2,2] = ((*rot_matrix)[2,2] * sin_cos[1]) + (m02 * sin_cos[0]);
    }

    template <typename T>
        requires (T::cIsMatrix && 3 <= T::cRowCount && 3 <= T::cColumnCount)
    constexpr void RotateLocalZ(T *rot_matrix, float theta) {
        const Float2 sin_cos = MethodSinCos(TrigMethod::Sample, theta);
        const float m00 = (*rot_matrix)[0,0];
        const float m01 = (*rot_matrix)[0,1];
        const float m02 = (*rot_matrix)[0,2];
        (*rot_matrix)[0,0] = (m00 * sin_cos[1])                + ((*rot_matrix)[1,0] * sin_cos[0]);
        (*rot_matrix)[1,0] = ((*rot_matrix)[1,0] * sin_cos[1]) - (m00 * sin_cos[0]);
        (*rot_matrix)[0,1] = (m01 * sin_cos[1])                + ((*rot_matrix)[1,1] * sin_cos[0]);
        (*rot_matrix)[1,1] = ((*rot_matrix)[1,1] * sin_cos[1]) - (m01 * sin_cos[0]);
        (*rot_matrix)[0,2] = (m02 * sin_cos[1])                + ((*rot_matrix)[1,2] * sin_cos[0]);
        (*rot_matrix)[1,2] = ((*rot_matrix)[1,2] * sin_cos[1]) - (m02 * sin_cos[0]);
    }
}

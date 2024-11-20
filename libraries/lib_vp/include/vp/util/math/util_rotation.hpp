#pragma once

namespace vp::util {

    template <typename T>
        requires (T::cIsVectorType)
    constexpr T Parallelize(const T &base, const T &to_parallelize) {
        const float dot = Dot(base, to_parallelize);
        T ret;
        for (u32 i = 0; i < T::cVectorCount; ++i) {
            ret[i] = to_parallelize[i] * dot;
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, float> && (T::cVectorCount == 3 || T::cVectorCount == 4))
    constexpr T RotateVectorAxisX(const T &vector, float angle) {
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
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, float> && (T::cVectorCount == 3 || T::cVectorCount == 4))
    constexpr T RotateVectorAxisY(const T &vector, float angle) {
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
        requires (T::cIsVectorType && std::is_same_v<typename T::UnderlyingType, float> && (T::cVectorCount == 3 || T::cVectorCount == 4))
    constexpr T RotateVectorAxisZ(const T &vector, float angle) {
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
        requires (T::cIsMatrix && 3 <= T::cRowCount, 3 <= T::cColumnCount)
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
        requires (T::cIsMatrix && 3 <= T::cRowCount, 3 <= T::cColumnCount)
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
        requires (T::cIsMatrix && 3 <= T::cRowCount, 3 <= T::cColumnCount)
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

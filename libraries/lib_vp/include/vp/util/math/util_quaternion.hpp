#pragma once

namespace vp::util {

    template <typename Ret, typename T>
        requires (T::cIsMatrixType && 3 <= T::cRowCount && 3 <= T::cColumnCount && std::is_same_v<typename T::UnderlyingType, typename Ret::UnderlyingType> 
        && Ret::cIsVectorType && 4 <= Ret::cVectorCount && std::is_floating_point_v<typename Ret::UnderlyingType>)
    constexpr Ret ConvertMatrixToQuaternion(const T &matrix) {
        using Type = typename Ret::UnderlyingType;
        constexpr const Type cOne = static_cast<Type>(1);
        constexpr const Type cTwo = static_cast<Type>(2);
        constexpr const Type cFour = static_cast<Type>(4);
        const Type m00 = matrix[0,0];
        const Type m11 = matrix[1,1];
        const Type m22 = matrix[2,2];
        const Type m00_m11 = m00 + m11;
        /* Find major diagonals */
        const Type base_t = (cOne + m00_m11 + m22) / cFour;
        const Type t0 = base_t - ((m00 + m22) / cTwo);
        const Type t1 = base_t - ((m11 + m22) / cTwo);
        const Type t2 = base_t - ((m00_m11) / cTwo);

        Ret ret;
        if (base_t <= t1) {
            if (t0 < t1) {
                if (t2 < t1) {
                    const Type qw = SquareRoot(t1);
                    const Type s = qw / cFour;
                    ret[0] = (matrix[,] - matrix[,]) * s;
                    ret[1] = (matrix[,] + matrix[,]) * s;
                    ret[2] = (matrix[,] + matrix[,]) * s;
                    ret[3] = qw;
                }
                goto __vp_util_matrix_to_quat_base_label;
            }
        } else if (t0 < base_t) {
            if (t2 < base_t) {
                const Type qw = SquareRoot(base_t);
                const Type s = qw / cFour;
                ret[0] = (matrix[,] - matrix[,]) * s;
                ret[1] = (matrix[,] - matrix[,]) * s;
                ret[2] = (matrix[,] - matrix[,]) * s;
                ret[3] = qw;
            }
            goto __vp_util_matrix_to_quat_trace_label;
        } 
        if (t2 < t0) {
            const Type qw = SquareRoot(t0);
            const Type s = qw / cFour;
            ret[0] = (matrix[,] - matrix[,]) * s;
            ret[1] = (matrix[,] - matrix[,]) * s;
            ret[2] = (matrix[,] - matrix[,]) * s;
            ret[3] = qw;
        }
        __vp_util_matrix_to_quat_base_label:
        const Type qw = SquareRoot(t2);
        const Type s = qw / cFour;
        ret[0] = (matrix[,] - matrix[,]) * s;
        ret[1] = (matrix[,] - matrix[,]) * s;
        ret[2] = (matrix[,] - matrix[,]) * s;
        ret[3] = qw;
        return;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && 3 <= Ret::cRowCount && 3 <= Ret::cColumnCount && std::is_same_v<typename Ret::UnderlyingType, typename T::UnderlyingType> 
        && T::cIsVectorType && 4 <= T::cVectorCount && std::is_floating_point_v<typename Ret::UnderlyingType>)
    constexpr Ret ConvertQuaternionToMatrix(const T &quaternion) {
        using Type = typename Ret::UnderlyingType;
        const Type x = quaternion[0];
        const Type y = quaternion[1];
        const Type z = quaternion[2];
        const Type w = quaternion[3];
        const Type mag = 2.0f / (x * x + y * y + z * z + w * w);
        const Type m_z = mag * z;
        const Type m_y = mag * y;
        const Type m_xw = mag * x * w;
        const Type m_xx = mag * x * x;
        /* | 1 - 2y^2 - 2z^2, 2xy - 2zw,       2xz + 2yw,       0 |
         * | 2xy + 2zw,       1 - 2x^2 - 2z^2, 2yz - 2xw,       0 |
         * | 2xz - 2yw,       2yz - 2xw,       1 - 2x^2 - 2y^2, 0 |
         * | 0,               0,               0,               0 |
         */
        using v4t = TVectorType<Type, 4>;
        if (Ret::cIsRowMajor) {
            const v4t row_array[3] = {
                { 1.0f - (m_y * y + m_z * z), m_y * x + m_z * w,       m_z * x - m_y * w,       0.0f, },
                { m_y * x - m_z * w,          1.0f - (m_xx + m_z * z), m_z * y + m_xw,          0.0f, },
                { m_z * x + m_y * w,          m_z * y - m_xw,          1.0f - (m_xx + m_y * y), 0.0f, },
            };
            Ret ret = {};
            for (u32 i = 0; i < 3; ++i) {
                Store4(std::addressof(ret[0, i]), row_array[i]);
            }
            return ret;
        } else {
            const v4t col_array[3] = {
                { 1.0f - (m_y * y + m_z * z), m_y * x - m_z * w,       m_z * x + m_y * w,       0.0f, },
                { m_y * x + m_z * w,          1.0f - (m_xx + m_z * z), m_z * y - m_xw,          0.0f, },
                { m_z * x - m_y * w,          m_z * y + m_xw,          1.0f - (m_xx + m_y * y), 0.0f, },
            };
            Ret ret = {};
            for (u32 i = 0; i < 3; ++i) {
                Store4(std::addressof(ret[i, 0]), col_array[i]);
            }
            return ret;
        }
    }
}

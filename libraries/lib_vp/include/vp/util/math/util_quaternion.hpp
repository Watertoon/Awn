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

    template <typename Ret, typename T>
        requires (T::cIsMatrixType && 3 <= T::cRowCount && 3 <= T::cColumnCount && std::is_same_v<typename T::UnderlyingType, typename Ret::UnderlyingType> 
        && Ret::cIsVectorType && 4 <= Ret::cVectorCount && std::is_floating_point_v<typename Ret::UnderlyingType>)
    constexpr Ret MatrixToQuat(const T &matrix) {
        using Type = typename Ret::UnderlyingType;
        constexpr const Type cOne = static_cast<Type>(1);
        constexpr const Type cOneDivTwo = static_cast<Type>(1) / static_cast<Type>(2);
        constexpr const Type cNegOneDivTwo = -cOneDivTwo;
        constexpr const Type cOneDivFour = static_cast<Type>(1) / static_cast<Type>(4);
        const Type m00 = matrix[0,0];
        const Type m11 = matrix[1,1];
        const Type m22 = matrix[2,2];
        const Type m00_m11 = m00 + m11;
        /* Find major diagonals */
        const Type base_t = (cOne + m00_m11 + m22) * cOneDivFour;
        const Type t0 = MultiplyAdd(m00 + m22, cNegOneDivTwo, base_t);
        const Type t1 = MultiplyAdd(m11 + m22, cNegOneDivTwo, base_t);
        const Type t2 = MultiplyAdd(m00_m11, cNegOneDivTwo, base_t);

        Ret ret;
        if (base_t <= t1) {
            if (t0 < t1) {
                if (t2 < t1) {
                    const Type qx = SquareRoot(t1);
                    const Type s = cOneDivFour / qx;
                    ret[0] = qx;
                    ret[1] = (matrix[1,0] + matrix[0,1]) * s;
                    ret[2] = (matrix[2,0] + matrix[0,2]) * s;
                    ret[3] = (matrix[1,2] - matrix[2,1]) * s;
                    return;
                }
                goto __vp_util_matrix_to_quat_z_label;
            }
        } else if (t0 < base_t) {
            if (t2 < base_t) {
                const Type qw = SquareRoot(base_t);
                const Type s = qw * cOneDivFour;
                ret[0] = (matrix[1,2] - matrix[2,1]) * s;
                ret[1] = (matrix[2,0] - matrix[0,2]) * s;
                ret[2] = (matrix[0,1] - matrix[1,0]) * s;
                ret[3] = qw;
                return;
            }
            goto __vp_util_matrix_to_quat_z_label;
        }
        if (t2 < t0) {
            const Type qy = SquareRoot(t0);
            const Type s = cOneDivFour / qy;
            ret[0] = (matrix[0,1] + matrix[1,0]) * s;
            ret[1] = qy;
            ret[2] = (matrix[2,1] + matrix[1,2]) * s;
            ret[3] = (matrix[2,0] - matrix[0,2]) * s;
            return;
        }
        __vp_util_matrix_to_quat_z_label:
        const Type qz = SquareRoot(t2);
        const Type s = qz * cOneDivFour;
        ret[0] = (matrix[0,2] + matrix[2,0]) * s;
        ret[1] = (matrix[1,2] + matrix[2,1]) * s;
        ret[2] = qz;
        ret[3] = (matrix[0,1] - matrix[1,0]) * s;
        return;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && 3 <= Ret::cRowCount && 3 <= Ret::cColumnCount && std::is_same_v<typename Ret::UnderlyingType, typename T::UnderlyingType> 
        && T::cIsVectorType && 4 <= T::cVectorCount && std::is_floating_point_v<typename Ret::UnderlyingType>)
    constexpr Ret QuatToMatrix(const T &quaternion) {
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
        if (Ret::cIsColumnMajor) {
            const v4t row_array[3] = {
                { 1.0f - (m_y * y + m_z * z), m_y * x + m_z * w,       m_z * x - m_y * w,       0.0f, },
                { m_y * x - m_z * w,          1.0f - (m_xx + m_z * z), m_z * y + m_xw,          0.0f, },
                { m_z * x + m_y * w,          m_z * y - m_xw,          1.0f - (m_xx + m_y * y), 0.0f, },
            };
            Ret ret = {};
            for (u32 i = 0; i < 3; ++i) {
                StoreColumn(std::addressof(ret), row_array[i], i);
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
                StoreRow(std::addressof(ret), col_array[i], i);
            }
            return ret;
        }
    }

    constexpr Vector4f EulerXYZToQuat(const Vector3f &euler) {
        const Vector3f scale = euler * 0.5f;
        const float roll_mod = ModuloPi(scale[0]);
        const float pitch_mod = ModuloPi(scale[1]);
        const float yaw_mod = ModuloPi(scale[2]);
        const Vector3f euler_mod = { roll_mod, pitch_mod, yaw_mod, 0.0f, };
        const Vector3f sin = CoefficientsSin(euler_mod);
        const Vector3f cos = CoefficientsCos(euler_mod);
        /*
         *  .x = sin(r/2)cos(p/2)cos*(y/2)-cos(r/2)sin(p/2)sin(y/2)
         *  .y = cos(r/2)sin(p/2)cos*(y/2)+sin(r/2)cos(p/2)sin(y/2)
         *  .z = cos(r/2)cos(p/2)sin*(y/2)-sin(r/2)sin(p/2)cos(y/2)
         *  .w = cos(r/2)cos(p/2)cos*(y/2)+sin(r/2)sin(p/2)sin(y/2)
         */
        const Vector4f ret = {
            sin[0] * cos[1] * cos[2] - cos[0] * sin[1] * sin[2],
            cos[0] * sin[1] * cos[2] + sin[0] * cos[1] * sin[2],
            cos[0] * cos[1] * sin[2] - sin[0] * sin[1] * cos[2],
            cos[0] * cos[1] * cos[2] + sin[0] * sin[1] * sin[2],
        };
        return ret;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsVectorType && 3 <= Ret::cVectorCount && T::cIsVectorType && 4 <= T::cVectorCount)
    constexpr Ret QuatToEulerXYZ(const T &quat) {
        
        const float x = quat[0];
        const float y = quat[1];
        const float z = quat[2];
        const float w = quat[3];
        /* Roll */
        const float sinr_cosp = 2 * (x * w + y * z);
        const float cosr_cosp = ((w * w - x * x) - y * y) + z * z;
        const float roll = ApproximateArcTan2(sinr_cosp, cosr_cosp);
        /* Pitch */
        const float p = 2 * (w * y - x * z);
        const float pitch = ApproximateArcCos(p);
        /* Yaw */
        const float siny_cosp = 2 * (x * y + z * w);
        const float cosy_cosp = ((w * w + x * x) - y * y) - z * z;
        const float yaw = ApproximateArcTan2(siny_cosp, cosy_cosp);
        return Ret{ roll, pitch, yaw, };
    }

    //constexpr Vector4f AxesToQuat(const Vector3f &axis0, const Vector3f &axis0) {
    //    /*
    //     *  .x = sin(r/2)cos(p/2)cos*(y/2)-cos(r/2)sin(p/2)sin(y/2)
    //     *  .y = cos(r/2)sin(p/2)cos*(y/2)+sin(r/2)cos(p/2)sin(y/2)
    //     *  .z = cos(r/2)cos(p/2)sin*(y/2)-sin(r/2)sin(p/2)cos(y/2)
    //     *  .w = cos(r/2)cos(p/2)cos*(y/2)+sin(r/2)sin(p/2)sin(y/2)
    //     */
    //    using Type = typename T::UnderlyingType;
    //    const Type q_x = ;
    //    const Type q_y = ;
    //    const Type q_z = ;
    //    const Type q_w2 = ;
    //    const Type q_x2 = ;
    //    const Vector4f axis0 = {
    //        ,
    //        ,
    //        ,
    //        ,
    //    };
    //    return ret;
    //}

    constexpr void QuatToAxes(Vector3f *out_axes, const Vector4f &quat) {
        const float x = quat[0];
        const float y = quat[1];
        const float z = quat[2];
        const float w = quat[3];
        const float x2 = x + x;
        const float z2_sq = z * (z + z);
        const float w2 = w + w;
        const Vector3f axis0 = {
            (1.0f - (y * (y + y))) - z2_sq,
            y * x2 - z * w2,
            z * x2 + y * w2,
            0.0f,
        };
        const Vector3f axis1 = {
            y * x2 + z * w2,
            (1.0f - (x * x2)) - (x * w2),
            z * (y + y) - x * w2,
            0.0f,
        };
        out_axes[0] = axis0;
        out_axes[1] = axis1;
        return;
    }

    //constexpr Vector3f QuatToEuler(const Vector4f &quat) {
    //}

    template <typename T>
        requires (T::cIsVectorType && 4 <= T::cVectorCount && std::is_floating_point_v<typename T::UnderlyingType>)
    constexpr T QuatMultiply(const T &quat_lhs, const T &quat_rhs) {
        /*
         *  (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y)
         *  (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x)
         *  (q1.w * q2.z) + (q1.x * q2.w) - (q1.y * q2.z) + (q1.z * q2.y)
         *  (q1.w * q2.w) - (q1.x * q2.w) - (q1.y * q2.z) - (q1.z * q2.y)
         */
        using Type = typename T::UnderlyingType;
        const Type ql_x = quat_lhs[0];
        const Type ql_y = quat_lhs[1];
        const Type ql_z = quat_lhs[2];
        const Type ql_w = quat_lhs[3];
        const Type qr_x = quat_rhs[0];
        const Type qr_y = quat_rhs[1];
        const Type qr_z = quat_rhs[2];
        const Type qr_w = quat_rhs[3];
        const Type x = (ql_w * qr_x) + (ql_x * qr_w) + (ql_y * qr_z) - (ql_z * qr_y);
        const Type y = (ql_w * qr_y) - (ql_x * qr_z) + (ql_y * qr_w) + (ql_z * qr_x);
        const Type z = (ql_w * qr_z) + (ql_x * qr_y) - (ql_y * qr_x) + (ql_z * qr_w);
        const Type w = (ql_w * qr_w) - (ql_x * qr_x) - (ql_y * qr_y) - (ql_z * qr_z);
        const T ret = {
            x,
            y,
            z,
            w,
        };
        return ret;
    }

    template <typename T, typename V>
        requires (T::cIsVectorType && 4 <= T::cVectorCount && std::is_floating_point_v<typename T::UnderlyingType>)
    constexpr V QuatRotate(const T &quat, const V &point) {
        using Type = typename T::UnderlyingType;
        const Type q_x = quat[0];
        const Type q_y = quat[1];
        const Type q_z = quat[2];
        const Type q_w = quat[3];
        const Type p_x = point[0];
        const Type p_y = point[1];
        const Type p_z = point[2];
        const Type m_x = ((q_w * p_x) + (p_z * q_y)) - (p_y * q_z);
        const Type m_y = ((q_w * p_y) + (p_x * q_z)) - (p_z * q_x);
        const Type m_z = ((q_w * p_z) + (p_y * q_x)) - (p_x * q_y);
        const Type m_w = (p_x * q_x) + (p_y * q_y) + (p_z * q_z);
        const Type x = (q_w * m_x) + (q_x * m_w) + ((q_y * m_z) - (q_z * m_y));
        const Type y = (q_w * m_y) + (q_y * m_w) + ((q_z * m_x) - (q_x * m_z));
        const Type z = (q_w * m_z) + (q_z * m_w) + ((q_x * m_y) - (q_y * m_x));
        const V ret = {
            x,
            y,
            z,
        };
        return ret;
    }

    constexpr Vector4f ApproximateQuatNormalizedLinearInterpolation(const Vector4f &from, const Vector4f &to, float time) {

        /* Invert quat for neg  */
        const float dot = Dot(from, to);
        const Vector4f dot_v = BroadcastVector4(dot);
        const Vector4f b_mask = CompareGreaterThan(Vector4f::cZero, dot_v);
        const Vector4f neg_to = -to;
        const Vector4f abs_to = InsertTrue(to, neg_to, b_mask);

        /* Linearly interpolate */
        const Vector4f lin = from + (abs_to - from) * time;
        const Vector4f lin_sq = lin * lin;
        float lin_sum = Sum(lin_sq);
        const Vector4f lin_sum_b = BroadcastVector4(lin_sum);
        const Vector4f inv_mag = ReciprocalSquareRoot(lin_sum_b);
        const Vector4f value = lin * inv_mag;

        /* Epsilon mask result */
        const Vector4f epsilon = BroadcastVector4(cFloatQuaternionEpsilon);
        const Vector4f ep_mask = CompareGreaterThan(epsilon, lin_sum_b);
        const Vector4f ret = Vector4f{ .v = AndNot(value.v, ep_mask.v), };
        return ret;
    }

    constexpr Vector4f ApproximateQuatSphericalLinearInterpolation(const Vector4f &from, const Vector4f &to, float time, s32 dir) {

        /* Select  */
        const float dot = Dot(from, to);
        const float abs_dot = (0.0f <= dot) ? dot : -dot;
        const float flip_sel = (0.0f <= dot) ? 1.0f : -1.0f;

        /* Fallback nlerp */
        const float ep_1 = 1.0f - cFloatQuaternionEpsilon;
        if (ep_1 < abs_dot) {
            
            /* Blend linear  */
            const float t_from = 1.0f - time;
            const float t_to = flip_sel * time;
            const float t_x = t_from * from[0] + t_to * to[0];
            const float t_y = t_from * from[1] + t_to * to[1];
            const float t_z = t_from * from[2] + t_to * to[2];
            const float t_w = t_from * from[3] + t_to * to[3];

            /* Normalize */
            const Vector4f lin = Vector4f{ t_x, t_y, t_z, t_w, };
            const Vector4f lin_sq = lin * lin;
            float lin_sum = Sum(lin_sq);
            const Vector4f lin_sum_b = BroadcastVector4(lin_sum);
            const Vector4f inv_mag = ReciprocalSquareRoot(lin_sum_b);
            const Vector4f value = lin * inv_mag;

            /* Epsilon mask result */
            const Vector4f epsilon = BroadcastVector4(cFloatQuaternionEpsilon);
            const Vector4f ep_mask = CompareGreaterThan(epsilon, lin_sum_b);
            const Vector4f ret = Vector4f{ .v = AndNot(value.v, ep_mask.v), };
            return ret;
        }

        /* Calc slerp */
        const float ang_acos = ApproximateArcCos(abs_dot);
        const float ang_pi = ModuloPi(ang_acos);
        const float ang_sin = CoefficientsSin(ang_pi);
        float t_from_sin;
        float t_to_sin;
        if (dir == 0) {
            const float t_from = (1.0f - time) * ang_acos;
            const float t_from_mod = ModuloPi(t_from);
            t_from_sin = CoefficientsSin(t_from_mod);
            const float t_to = time * ang_acos;
            const float t_to_mod = ModuloPi(t_to);
            t_to_sin = CoefficientsSin(t_to_mod);
        } else {
            const float t_to = (static_cast<float>(dir) * cFloatPi + ang_acos) * time;
            const float t_from = ang_acos - t_to;
            const float t_from_mod = ModuloPi(t_from);
            t_from_sin = CoefficientsSin(t_from_mod);
            //const float t_to = time * ang_acos;
            const float t_to_mod = ModuloPi(t_to);
            t_to_sin = CoefficientsSin(t_to_mod);
        }

        /* Blend */
        const float from_scale = t_from_sin / ang_sin;
        const float to_scale = flip_sel * (t_to_sin / ang_sin);
        const Vector4f v_from = to * to_scale;
        const Vector4f v_to = from * from_scale;
        const Vector4f result = v_from + v_to;
        return result;
    }

    constexpr void ApproximateQuatSlerpBlend(vp::util::Vector4f *from, vp::util::Vector4f *to, float from_factor, float to_factor) {

        if (from_factor == 0.0f) {
            *from = *to;
            return;
        }

        const vp::util::Vector4f res = *from;
        const vp::util::Vector4f rot = *to;
        const float fact_ratio = to_factor / (from_factor + to_factor);
        const float sum = vp::util::Sum(res * rot);
        const float abs_sum = (sum < 0.0f) ? -sum : sum;
        const float sum_sign = (sum < 0.0f) ? -1.0f : 1.0f;
        const float arc = vp::util::ApproximateArcCos(abs_sum);
        const float inv_sin = 1.0f / vp::util::CoefficientsSin(vp::util::ModuloPi(arc));

        const float at_fact = fact_ratio * arc;
        const float rev_at_fact = (1.0f - fact_ratio) * arc;
        const float at_fact_div = at_fact * vp::util::cFloat1Divided2Pi;
        const float rev_at_fact_div = rev_at_fact * vp::util::cFloat1Divided2Pi;
        const float sel = (0.0f <= at_fact_div) ? 0.5f : -0.5f;
        const float rev_sel = (0.0f <= rev_at_fact_div) ? 0.5f : -0.5f;
        const float rnd = static_cast<float>(static_cast<s32>(sel));
        const float rev_rnd = static_cast<float>(static_cast<s32>(rev_sel));
        const float at_off = at_fact - rnd * vp::util::cFloat2Pi;
        const float rev_at_off = rev_at_fact - rev_rnd * vp::util::cFloat2Pi;
        const float at_flip0 = (vp::util::cFloatPiDivided2 < at_off) ? vp::util::cFloatPi - at_off : at_off;
        const float rev_at_flip0 = (vp::util::cFloatPiDivided2 < rev_at_off) ? vp::util::cFloatPi - rev_at_off : rev_at_off;
        const float at_flip1 = (at_flip0 < vp::util::cFloatPiDivided2) ? -vp::util::cFloatPi - at_flip0 : at_flip0;
        const float rev_at_flip1 = (rev_at_flip0 < vp::util::cFloatPiDivided2) ? -vp::util::cFloatPi - rev_at_flip0 : rev_at_flip0;

        float a_fact = fact_ratio * sum_sign;
        float b_fact = 1.0f - fact_ratio;
        if (abs_sum <= (1.0f - vp::util::cFloatQuaternionEpsilon)) {
            a_fact = inv_sin * sum_sign * vp::util::CoefficientsSin(at_flip1);
            b_fact = inv_sin * vp::util::CoefficientsSin(rev_at_flip1);
        }
        const vp::util::Vector4f ret = res * b_fact + rot * a_fact;
        *from = ret;
        return;
    }
}

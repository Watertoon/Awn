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

namespace awn::para {

    enum class CurveType : u8 {
        Linear           = 0x0,
        Hermit           = 0x1,
        Step             = 0x2,
        Sin              = 0x3,
        Cos              = 0x4,
        SinPow2          = 0x5,
        Linear2D         = 0x6,
        Hermit2D         = 0x7,
        Step2D           = 0x8,
        NonuniformSpline = 0x9,
        Hermit2DSmooth   = 0xa,
        Hermit2DSplit    = 0xb,
    };

    struct CurveInfo {
        u8 curve_type;
        s32 data_count;
    };

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveLinear(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        if (curve_info.data_count <= 0) { return 0.0; }

        /* Calculate index (1 float per entry) */
        const s32 end_id  = curve_info.data_count - 1;
        const T offset = static_cast<T>(end_id) * x;
        const s32 c_id = static_cast<s32>(offset);

        /* Bounds check */
        if (end_id <= c_id) { return curve_data[end_id]; }

        /* Interpolate linear curve */
        const T value = curve_data[c_id];
        const T result = value + (offset - static_cast<T>(c_id)) * (curve_data[c_id + 1] - value);

        return result;
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveHermit(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        if (curve_info.data_count <= 0) { return 0.0; }

        /* x below 0 */
        if (x < 0.0) { return curve_data[0]; }

        /* Calculate index (2 floats per entry) */
        const s32 end_id  = (curve_info.data_count / 2) - 1;
        const T   offset = static_cast<T>(end_id) * x;
        const s32 c_id = static_cast<s32>(offset);

        /* Bounds check */
        if (end_id <= c_id) { return curve_data[end_id * 2]; }

        /* Interpolate hermite curve */
        const T *data_offset = std::addressof(curve_data[c_id * 2]);
        const T round  = offset - static_cast<T>(static_cast<s32>(offset));
        const T _2a_sq = round * (round + round);
        const T _3a_sq = 3.0 * round * round;
        const T _a_sq3 = round * round * round;
        const T result = (((round * _2a_sq) - _3a_sq) + 1.0) * data_offset[0] + (_3a_sq - (round * _2a_sq)) * data_offset[2] + (round + (_a_sq3 - _2a_sq)) * data_offset[1] + (_a_sq3 - (round * round)) * data_offset[0];

        return result;
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveStep(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        VP_ASSERT(0 < curve_info.data_count);

        /* Calculate index */
        const T min_x = vp::util::Min(x, 1.0);
        const T adj_x = (0.0 <= x) ? min_x : 0.0;
        const s32   index = adj_x * (curve_info.data_count - 1);

        /* Get step */
        return curve_data[index];
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveSin(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        VP_ASSERT(0 < curve_info.data_count);

        /* Get sin */
        const T pi2 = (std::is_same_v<T, float>) ? vp::util::cFloat2Pi : vp::util::cDouble2Pi;
        const T sin_x  = vp::util::Sin(curve_data[0] * x * pi2);
        const T result = sin_x * curve_data[1];

        return result;
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveCos(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        VP_ASSERT(0 < curve_info.data_count);

        /* Get sin */
        const T pi2 = (std::is_same_v<T, float>) ? vp::util::cFloat2Pi : vp::util::cDouble2Pi;
        const T cos_x  = vp::util::Cos(curve_data[0] * x * pi2);
        const T result = cos_x * curve_data[1];

        return result;
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveSinPow2(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        VP_ASSERT(0 < curve_info.data_count);

        /* Get sin */
        const T pi2 = (std::is_same_v<T, float>) ? vp::util::cFloat2Pi : vp::util::cDouble2Pi;
        const T sin_x  = vp::util::Sin(curve_data[0] * x * pi2);
        const T result = sin_x * sin_x * curve_data[1];

        return result;
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveLinear2D(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        VP_ASSERT(0 < curve_info.data_count);

        /* Under curve key */
        if (x <= curve_data[0]) { return curve_data[1]; }

        /* Calculate index (2 float per entry) */
        const s32 real_count  = curve_info.data_count / 2;

        /* Check if over real count */
        if (curve_data[curve_info.data_count - 2] <= x) { return curve_data[curve_info.data_count - 1]; }

        /* No value */
        if (curve_info.data_count < 2) { return 0.0; }

        /* Find frame */
        T *data_offset = std::addressof(curve_data[2]);
        for (s32 frame_i = (real_count < 2) ? 1 : real_count; frame_i != 0; --frame_i) {

            /* Search frame */
            if (data_offset[0] <= x) { data_offset = std::addressof(data_offset[2]); continue; }

            /* Calculate */
            const T result = data_offset[-1] + ((x - data_offset[-2]) / (data_offset[0] - data_offset[-2])) * (data_offset[1] - data_offset[-1]);

            return result;
        }

        return 0.0;
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveHermit2D(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        VP_ASSERT(0 < curve_info.data_count);

        /* Under curve key */
        if (x <= curve_data[0]) { return curve_data[1]; }

        /* Calculate index (3 float per entry) */
        const s32 real_count  = curve_info.data_count / 3;

        /* Over curve key */
        if (curve_data[curve_info.data_count - 3] <= x) { return curve_data[curve_info.data_count - 2]; }

        /* No value */
        if (curve_info.data_count < 3) { return 0.0; }

        /* Find frame */
        T *data_offset = std::addressof(curve_data[3]);
        for (s32 frame_i = (real_count < 2) ? 1 : real_count; frame_i != 0; --frame_i) {

            /* Search frame */
            if (data_offset[0] <= x) { data_offset = std::addressof(data_offset[3]); continue; }

            /* Calculate */
            const T delta   = (x - data_offset[-3]) / (data_offset[0] - data_offset[-3]);
            const T _2a_sq  = delta * (delta + delta);
            const T _3a_sq  = 3.0 * delta * delta;
            const T _2a_sq3 = _2a_sq * delta;
            const T _a_sq3  = delta * delta * delta;
            const T result = data_offset[2] * (_a_sq3 - (delta * delta)) + data_offset[-1] * (delta + (_a_sq3 - _2a_sq)) + data_offset[1] * (_3a_sq - _2a_sq3) + data_offset[-2] * ((_2a_sq3 - _3a_sq) + 1.0);

            return result;
        }

        return 0.0;
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveStep2D(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        VP_ASSERT(0 < curve_info.data_count);

        /* Under curve key */
        if (x <= curve_data[0]) { return curve_data[1]; }

        /* Calculate index (2 float per entry) */
        const s32 real_count  = curve_info.data_count / 2;

        /* Over curve key */
        if (curve_data[curve_info.data_count - 2] <= x) { return curve_data[curve_info.data_count - 1]; }

        /* No value */
        if (curve_info.data_count < 2) { return 0.0; }

        /* Find frame */
        T *data_offset = std::addressof(curve_data[2]);
        for (s32 frame_i = (real_count < 2) ? 1 : real_count; frame_i != 0; --frame_i) {

            /* Search frame */
            if (data_offset[0] <= x) { data_offset = std::addressof(data_offset[2]); continue; }

            /* Get step */
            return data_offset[1];
        }

        return 0.0;
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveHermit2DSmooth(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        VP_ASSERT(0 < curve_info.data_count);

        /* Under curve key */
        if (x <= curve_data[0]) { return curve_data[1]; }

        /* Calculate index (3 float per entry) */
        const s32 real_count  = curve_info.data_count / 3;

        /* Over curve key */
        if (curve_data[curve_info.data_count - 3] <= x) { return curve_data[curve_info.data_count - 2]; }

        /* No value */
        if (curve_info.data_count < 3) { return 0.0; }

        /* Find frame */
        T *data_offset = std::addressof(curve_data[3]);
        for (s32 frame_i = (real_count < 2) ? 1 : real_count; frame_i != 0; --frame_i) {

            /* Search frame */
            if (data_offset[0] <= x) { data_offset = std::addressof(data_offset[3]); continue; }

            /* Calculate */
            const T smooth  = (data_offset[0] - data_offset[-3]);
            const T delta   = (x - data_offset[-3]) / smooth;
            const T _2a_sq  = delta * (delta + delta);
            const T _3a_sq  = 3.0 * delta * delta;
            const T _2a_sq3 = _2a_sq * delta;
            const T _a_sq3  = delta * delta * delta;
            const T result = data_offset[2] * smooth * (_a_sq3 - (delta * delta)) + data_offset[-1] * smooth * (delta + (_a_sq3 - _2a_sq)) + data_offset[1] * (_3a_sq - _2a_sq3) + data_offset[-2] * ((_2a_sq3 - _3a_sq) + 1.0);

            return result;
        }

        return 0.0;
    }

    template <typename T>
        requires std::is_same_v<T, float> || std::is_same_v<T, double>
    constexpr T EvaluateCurveHermit2DSplit(T x, const CurveInfo curve_info, const T *curve_data) {

        /* No data */
        VP_ASSERT(0 < curve_info.data_count);

        /* Under curve key */
        if (x <= curve_data[0]) { return curve_data[1]; }

        /* Calculate index (4 float per entry) */
        const s32 real_count  = curve_info.data_count / 4;

        /* Over curve key */
        if (curve_data[curve_info.data_count - 4] <= x) { return curve_data[curve_info.data_count - 3]; }

        /* No value */
        if (curve_info.data_count < 4) { return 0.0; }

        /* Find frame (note base offset is 3 after) */
        T *data_offset = std::addressof(curve_data[3]);
        for (s32 frame_i = (real_count < 2) ? 1 : real_count; frame_i != 0; --frame_i) {

            /* Search frame */
            if (data_offset[1] <= x) { data_offset = std::addressof(data_offset[4]); continue; }

            /* Calculate */
            const T smooth  = (data_offset[1] - data_offset[-3]);
            const T delta   = (x - data_offset[-3]) / smooth;
            const T _2a_sq  = delta * (delta + delta);
            const T _3a_sq  = 3.0 * delta * delta;
            const T _2a_sq3 = _2a_sq * delta;
            const T _a_sq3  = delta * delta * delta;
            const T result = data_offset[3] * smooth * (_a_sq3 - (delta * delta)) + data_offset[0] * smooth * (delta + (_a_sq3 - _2a_sq)) + data_offset[1] * (_3a_sq - _2a_sq3) + data_offset[-2] * ((_2a_sq3 - _3a_sq) + 1.0);

            return result;
        }

        return 0.0;
    }

    template <typename T>
    class Curve {
        public:
            static constexpr bool cIsCustomTypeForPropertyAccessor = true;
        private:
            CurveType  m_curve_type;
            u32        m_data_count;
            T          m_max_x;
            T         *m_array;
        public:
            constexpr Curve() : m_curve_type(), m_data_count(), m_max_x(), m_array() {/*...*/}
            constexpr ~Curve() {/*...*/}

            bool LoadByamlIterator(vp::res::ByamlIterator iter) {

                /* Parse type */
                {
                    const char *curve_type_name = "";
                    const bool result0 = iter.TryGetStringByKey(std::addressof(curve_type_name), "Type");
                    if (!result0 || !iter.TryGetStringByKey(std::addressof(curve_type_name), "type")) { return false; }
                    const bool result1 = vp::util::TEnumTraits<CurveType>::GetValueByString(std::addressof(m_curve_type), curve_type_name);
                    if (!result1) { return false; }
                }

                /* Get max x */
                m_max_x = 1.0;
                iter.TryGetValueByKey<T>(std::addressof(m_max_x), "MaxX");

                /* Get data iter */
                vp::res::ByamlIterator data_iter;
                {
                    const bool result1 = iter.TryGetIteratorByKey(std::addressof(data_iter), "Data");
                    if (!result1 || !iter.TryGetIteratorByKey(std::addressof(data_iter), "data")) { return false; }
                }

                /* Get array */
                m_array = iter.GetMonoArray<T>(std::addressof(m_data_count));

                return m_array != nullptr;
            }
#ifdef VP_ENABLE_LIBYAML
            bool LoadYamlIterator(vp::res::YamlIterator iter) {

                /* Parse type */
                {
                    const char *curve_type_name = "";
                    const bool result0 = iter.TryGetStringByKey(std::addressof(curve_type_name), "Type");
                    if (!result0 || !iter.TryGetStringByKey(std::addressof(curve_type_name), "type")) { return false; }
                    const bool result1 = vp::util::TEnumTraits<CurveType>::GetValueByString(std::addressof(m_curve_type), curve_type_name);
                    if (!result1) { return false; }
                }

                /* Get max x */
                m_max_x = 1.0;
                iter.TryGetValueByKey<T>(std::addressof(m_max_x), "MaxX");

                /* Get data iter */
                vp::res::YamlIterator data_iter;
                {
                    const bool result1 = iter.TryGetIteratorByKey(std::addressof(data_iter), "Data");
                    if (!result1 || !iter.TryGetIteratorByKey(std::addressof(data_iter), "data")) { return false; }
                }

                /* Get array */
                m_array = iter.GetMonoArray<T>(std::addressof(m_data_count));

                return m_array != nullptr;
            }
#endif
            constexpr T Evaluate(T x) {

                /* Setup curve info */
                const CurveInfo curve_info = {
                    .curve_type = static_cast<u8>(m_curve_type),
                    .data_count = m_data_count,
                };

                /* Adjust for max x if Linear, Hermit, or Step */
                if (curve_info.curve_type < 3) {
                    x = x / m_max_x;
                }

                /* Evaluate curve */
                switch (m_curve_type) {
                    case CurveType::Linear:
                        return EvaluateCurveLinear<T>(x, curve_info, m_array);
                    case CurveType::Hermit:
                        return EvaluateCurveHermit<T>(x, curve_info, m_array);
                    case CurveType::Step:
                        return EvaluateCurveStep<T>(x, curve_info, m_array);
                    case CurveType::Sin:
                        return EvaluateCurveSin<T>(x, curve_info, m_array);
                    case CurveType::Cos:
                        return EvaluateCurveCos<T>(x, curve_info, m_array);
                    case CurveType::SinPow2:
                        return EvaluateCurveSinPow2<T>(x, curve_info, m_array);
                    case CurveType::Linear2D:
                        return EvaluateCurveLinear2D<T>(x, curve_info, m_array);
                    case CurveType::Hermit2D:
                        return EvaluateCurveHermit2D<T>(x, curve_info, m_array);
                    case CurveType::Step2D:
                        return EvaluateCurveStep2D<T>(x, curve_info, m_array);
                    case CurveType::Hermit2DSmooth:
                        return EvaluateCurveHermit2DSmooth<T>(x, curve_info, m_array);
                    case CurveType::Hermit2DSplit:
                        return EvaluateCurveHermit2DSplit<T>(x, curve_info, m_array);
                    case CurveType::NonuniformSpline:
                    default:
                        break;
                };

                return 0.0;
            }
    };

    using Curvef = Curve<float>;
    using Curved = Curve<double>;
}

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
    struct BoundingBox {
        TVector<T, 3, false> min;
        TVector<T, 3, false> max;

        using UnderlyingType = T;
        static constexpr bool cIsBoundingBoxType = true;
        static constexpr bool cIsVectorized = false;
    };

    template <typename T>
    struct BoundingBoxV {
        TVector<T, 3, true> min;
        TVector<T, 3, true> max;

        using UnderlyingType = T;
        static constexpr bool cIsBoundingBoxType = true;
        static constexpr bool cIsVectorized = true;
    };

    using BoundingBoxf = BoundingBox<float>;
    using BoundingBoxVf = BoundingBoxV<float>;


    template <typename B>
        requires (B::cIsBoundingBoxType)
    constexpr bool IsBoundingBoxValid(const B &box) {
        if constexpr (B::cIsVectorized) {
            const vp::util::Vector3f compare_mask = vp::util::CompareGreaterThan(box.min, box.max);
            const u32 mask = vp::util::MoveMask(compare_mask);
            return (mask == 0);
        } else {
            if (box.max[0] < box.min[0]) { return false; }
            if (box.max[1] < box.min[1]) { return false; }
            if (box.max[2] < box.min[2]) { return false; }
            return true;
        }
    }

    template <typename Ret, typename M, typename B>
        requires (Ret::cIsBoundingBoxType && M::cIsMatrixType && B::cIsBoundingBoxType)
    constexpr Ret BoundingBoxTransform(const M &matrix, const B &box) {
        using Type = typename Ret::UnderlyingType;
        using PositionType = TVector<Type, M::cColumnCount, Ret::cIsVectorized>;
        const PositionType min_trans = MatrixTransform<PositionType>(matrix, box.min);
        const M abs_matrix = Absolute(matrix);
        const PositionType max_trans = MatrixTransform<PositionType>(abs_matrix, box.max);
        const PositionType new_min = min_trans - max_trans;
        const PositionType new_max = min_trans + max_trans;
        const Ret ret = Ret{ .min = new_min, .max = new_max, };
        return ret;
    }

    template <typename Ret, typename B0, typename B1>
        requires (Ret::cIsBoundingBoxType && B0::cIsBoundingBoxType && B1::cIsBoundingBoxType)
    constexpr Ret BoundingBoxMerge(const B0 &l_box, const B1 &r_box) {
        Ret ret = {};
        if constexpr (Ret::cIsVectorized && B0::cIsVectorized && B1::cIsVectorized) {
            ret.min = Min(l_box.min, r_box.min);
            ret.max = Max(l_box.max, r_box.max);
            return ret;
        }
        ret.min[0] = Min(l_box.min[0], r_box.min[0]);
        ret.min[1] = Min(l_box.min[1], r_box.min[1]);
        ret.min[2] = Min(l_box.min[2], r_box.min[2]);
        ret.max[0] = Max(l_box.max[0], r_box.max[0]);
        ret.max[1] = Max(l_box.max[1], r_box.max[1]);
        ret.max[2] = Max(l_box.max[2], r_box.max[2]);
        return ret;
    }

    template <typename B>
        requires (B::cIsBoundingBoxType)
    constexpr typename B::UnderlyingType BoundingBoxToVolume(const B &box) {
        using Type = typename B::UnderlyingType;
        using VectorType = TVector<Type, 3, B::cIsVectorized>;
        const VectorType size = box.max - box.min;
        const Type ret = size[0] * size[1] * size[2];
        return ret;
    }

    template <typename B>
        requires (B::cIsBoundingBoxType)
    constexpr TVector<typename B::UnderlyingType,3,B::cIsVectorized> BoundingBoxToCenter(const B &box) {
        using Type = typename B::UnderlyingType;
        using VectorType = TVector<Type, 3, B::cIsVectorized>;
        const VectorType size = box.max + box.min;
        const VectorType ret = size * 0.5f;
        return ret;
    }
}

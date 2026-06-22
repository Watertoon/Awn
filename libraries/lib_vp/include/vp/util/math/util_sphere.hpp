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
    struct Sphere {
        TVector<T, 3, false> position;
        float radius;

        using UnderlyingType = T;
        static constexpr bool cIsSphereType = true;
        static constexpr bool cIsVectorized = false;
    };
    template <typename T>
    struct SphereV {
        TVector<T, 3, true> position;
        float radius;

        using UnderlyingType = T;
        static constexpr bool cIsSphereType = true;
        static constexpr bool cIsVectorized = true;
    };

    using Spheref = Sphere<float>;
    using SphereVf = SphereV<float>;
    static_assert(sizeof(Spheref) == 0x10);
    static_assert(sizeof(SphereVf) == 0x20);

    template <typename T>
    constexpr SphereV<T> CastSphereV(const Sphere<T> &sphere) {
        const TVector<T, 3, true> position = vp::util::Convert<TVector<T,3,true>>(sphere.position);
        const float radius = sphere.radius;
        const SphereV<T> ret = SphereV<T> { position, radius, };
        return ret;
    }

    template <typename Ret, typename S, typename M>
        requires (Ret::cIsSphereType && S::cIsSphereType && std::is_same_v<typename Ret::UnderlyingType, typename S::UnderlyingType> && M::cIsMatrixType && 4 <= M::cRowCount && 3 <= M::cColumnCount && std::is_floating_point_v<typename M::UnderlyingType>)
    Ret SphereTransform(const M &matrix, const S &sphere) {
        using Type = typename Ret::UnderlyingType;
        using PositionType = TVector<Type, M::cColumnCount, Ret::cIsVectorized>;
        const PositionType position = MatrixTransform<PositionType>(matrix, sphere.position);
        const PositionType scale = GetScale<PositionType>(matrix);
        const Type max_scale = MaxElement(scale);
        const Type radius = max_scale * sphere.radius;
        return Ret { position, radius, };
    }

    static constexpr float cSpherefMergeMin = 0.0001;

    template <typename S>
        requires (S::cIsSphereType && std::is_floating_point_v<typename S::UnderlyingType>)
    S SphereMerge(const S &l_sphere, const S &r_sphere) {
        using Type = typename S::UnderlyingType;
        using PositionType = TVector<Type, 3, S::cIsVectorized>;

        /* Create new sphere encompassing if no overlap */
        const PositionType diff_position = r_sphere.position - l_sphere.position;
        const Type pos_sum = MagnitudeSquared(diff_position);
        const Type diff_radius = r_sphere.radius - l_sphere.radius;
        const Type diff_r2 = diff_radius * diff_radius;
        const Type r_sum = Max(cSpherefMergeMin, diff_r2);
        if (r_sum <= pos_sum) {
            const Type pos_mag = pos_sum * (static_cast<Type>(1.0) / vp::util::SquareRoot(pos_sum));
            const Type rad = (pos_mag + l_sphere.radius + r_sphere.radius) / static_cast<Type>(2.0);
            const Type scale_r = (rad - l_sphere.radius) / pos_mag;
            const PositionType scale_pos = diff_position * scale_r;
            const PositionType pos = l_sphere.position + scale_pos;
            const S result = S{ pos, rad, };
            return result;
        }

        /* Select larger sphere for they overlap */
        const S *sel = (l_sphere.radius <= r_sphere.radius) ? std::addressof(r_sphere) : std::addressof(l_sphere);
        const S result = S{ sel->position, sel->radius, };
        return result;
    }
}

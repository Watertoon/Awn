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
        requires (T::cIsVectorizedType && std::is_same_v<float, typename T::UnderlyingType>)
    constexpr T CoefficientsSin(const T &vec) {
        if constexpr (T::cIsVectorizedType && T::cCount == 4) {
            const v4f coef0 = Broadcast4(cSinCoefficients[0]);
            const v4f coef1 = Broadcast4(cSinCoefficients[1]);
            const v4f coef2 = Broadcast4(cSinCoefficients[2]);
            const v4f coef3 = Broadcast4(cSinCoefficients[3]);
            const v4f coef4 = Broadcast4(cSinCoefficients[4]);
            const v4f t0 = MultiplySub(vec, coef1, coef0);
            const v4f t1 = NegateMultiplySub(vec, coef2, t0);
            const v4f t2 = MultiplyAdd(vec, coef3, t1);
            const v4f t3 = NegateMultiplySub(vec, coef4, t2);
            const v4f one = v4f{ 1.0f, 1.0f, 1.0f, 1.0f, };
            const v4f ret = MultiplyAdd(vec, one, t3);
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = CoefficientsSin(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorizedType && std::is_same_v<float, typename T::UnderlyingType>)
    constexpr T CoefficientsCos(const T &vec) {
        if constexpr (T::cIsVectorizedType && T::cCount == 4) {
            const v4f coef0 = Broadcast4(cCosCoefficients[0]);
            const v4f coef1 = Broadcast4(cCosCoefficients[1]);
            const v4f coef2 = Broadcast4(cCosCoefficients[2]);
            const v4f coef3 = Broadcast4(cCosCoefficients[3]);
            const v4f coef4 = Broadcast4(cCosCoefficients[4]);
            const v4f t0 = MultiplySub(vec, coef1, coef0);
            const v4f t1 = NegateMultiplySub(vec, coef2, t0);
            const v4f t2 = MultiplyAdd(vec, coef3, t1);
            const v4f t3 = NegateMultiplySub(vec, coef4, t2);
            const v4f one = v4f{ 1.0f, 1.0f, 1.0f, 1.0f, };
            const v4f ret = MultiplyAdd(vec, one, t3);
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = CoefficientsCos(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorizedType && std::is_same_v<float, typename T::UnderlyingType>)
    constexpr T CoefficientsArcTan(const T &vec) {
        if constexpr (T::cIsVectorizedType && T::cCount == 4) {
            const v4f coef0 = Broadcast4(cArcTanCoefficients[0]);
            const v4f coef1 = Broadcast4(cArcTanCoefficients[1]);
            const v4f coef2 = Broadcast4(cArcTanCoefficients[2]);
            const v4f coef3 = Broadcast4(cArcTanCoefficients[3]);
            const v4f coef4 = Broadcast4(cArcTanCoefficients[4]);
            const v4f coef5 = Broadcast4(cArcTanCoefficients[5]);
            const v4f coef6 = Broadcast4(cArcTanCoefficients[6]);
            const v4f coef7 = Broadcast4(cArcTanCoefficients[7]);
            const v4f t0 = NegateMultiplySub(coef0, vec, coef1);
            const v4f t1 = MultiplyAdd(t0, vec, coef2);
            const v4f t2 = NegateMultiplySub(t1, vec, coef3);
            const v4f t3 = MultiplyAdd(t2, vec, coef4);
            const v4f t4 = NegateMultiplySub(t3, vec, coef5);
            const v4f t5 = MultiplyAdd(t4, vec, coef6);
            const v4f t6 = NegateMultiplySub(t5, vec, coef7);
            const v4f one = v4f{ 1.0f, 1.0f, 1.0f, 1.0f, };
            const v4f result = MultiplyAdd(t6, vec, one);
            return result;
        }
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = CoefficientsArcTan(vec[i]);
        }
        return ret;
    }
}

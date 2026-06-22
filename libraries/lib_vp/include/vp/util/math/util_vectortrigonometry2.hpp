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
        requires (T::cIsVectorType && std::is_same_v<float, typename T::UnderlyingType>)
    constexpr T CoefficientsSin(const T &vec) {
        if constexpr (T::cIsVectorized) {
            const T coef0 = Broadcast<T>(cSinCoefficients[0]);
            const T coef1 = Broadcast<T>(cSinCoefficients[1]);
            const T coef2 = Broadcast<T>(cSinCoefficients[2]);
            const T coef3 = Broadcast<T>(cSinCoefficients[3]);
            const T coef4 = Broadcast<T>(cSinCoefficients[4]);
            const T v_sq = vec * vec;
            const T t0 = MultiplySub(coef0, v_sq, coef1);
            const T t1 = NegateMultiplySub(t0, v_sq, coef2);
            const T t2 = MultiplyAdd(t1, v_sq, coef3);
            const T t3 = NegateMultiplySub(t2, v_sq, coef4);
            const T one = Broadcast<T>(1.0f);
            const T t4 = MultiplyAdd(t3, v_sq, one);
            const T ret = vec * t4;
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = CoefficientsSin(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<float, typename T::UnderlyingType>)
    constexpr T CoefficientsCos(const T &vec) {
        if constexpr (T::cIsVectorized) {
            const T coef0 = Broadcast<T>(cCosCoefficients[0]);
            const T coef1 = Broadcast<T>(cCosCoefficients[1]);
            const T coef2 = Broadcast<T>(cCosCoefficients[2]);
            const T coef3 = Broadcast<T>(cCosCoefficients[3]);
            const T coef4 = Broadcast<T>(cCosCoefficients[4]);
            const T v_sq = vec * vec;
            const T t0 = MultiplySub(coef0, v_sq, coef1);
            const T t1 = NegateMultiplySub(t0, v_sq, coef2);
            const T t2 = MultiplyAdd(t1, v_sq, coef3);
            const T t3 = NegateMultiplySub(t2, v_sq, coef4);
            const T one = Broadcast<T>(1.0f);
            const T ret = MultiplyAdd(t3, v_sq, one);
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = CoefficientsCos(vec[i]);
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsVectorType && std::is_same_v<float, typename T::UnderlyingType>)
    constexpr T CoefficientsArcTan(const T &vec) {
        if constexpr (T::cIsVectorized) {
            const T coef0 = Broadcast<T>(cArcTanCoefficients[0]);
            const T coef1 = Broadcast<T>(cArcTanCoefficients[1]);
            const T coef2 = Broadcast<T>(cArcTanCoefficients[2]);
            const T coef3 = Broadcast<T>(cArcTanCoefficients[3]);
            const T coef4 = Broadcast<T>(cArcTanCoefficients[4]);
            const T coef5 = Broadcast<T>(cArcTanCoefficients[5]);
            const T coef6 = Broadcast<T>(cArcTanCoefficients[6]);
            const T coef7 = Broadcast<T>(cArcTanCoefficients[7]);
            const T v_sq = vec * vec;
            const T t0 = NegateMultiplySub(coef0, v_sq, coef1);
            const T t1 = MultiplyAdd(t0, v_sq, coef2);
            const T t2 = NegateMultiplySub(t1, v_sq, coef3);
            const T t3 = MultiplyAdd(t2, v_sq, coef4);
            const T t4 = NegateMultiplySub(t3, v_sq, coef5);
            const T t5 = MultiplyAdd(t4, v_sq, coef6);
            const T t6 = NegateMultiplySub(t5, v_sq, coef7);
            const T one = Broadcast<T>(1.0f);
            const T t7 = MultiplyAdd(t6, v_sq, one);
            const T ret = vec * t7;
            return ret;
        }
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = CoefficientsArcTan(vec[i]);
        }
        return ret;
    }
}

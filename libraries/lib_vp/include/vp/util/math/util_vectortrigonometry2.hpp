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
            const T t0 = MultiplySub(vec, coef1, coef0);
            const T t1 = NegateMultiplySub(vec, coef2, t0);
            const T t2 = MultiplyAdd(vec, coef3, t1);
            const T t3 = NegateMultiplySub(vec, coef4, t2);
            const T one = Broadcast<T>(1.0f);
            const T ret = MultiplyAdd(vec, one, t3);
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
            const T t0 = MultiplySub(vec, coef1, coef0);
            const T t1 = NegateMultiplySub(vec, coef2, t0);
            const T t2 = MultiplyAdd(vec, coef3, t1);
            const T t3 = NegateMultiplySub(vec, coef4, t2);
            const T one = Broadcast<T>(1.0f);
            const T ret = MultiplyAdd(vec, one, t3);
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
            const T t0 = NegateMultiplySub(coef0, vec, coef1);
            const T t1 = MultiplyAdd(t0, vec, coef2);
            const T t2 = NegateMultiplySub(t1, vec, coef3);
            const T t3 = MultiplyAdd(t2, vec, coef4);
            const T t4 = NegateMultiplySub(t3, vec, coef5);
            const T t5 = MultiplyAdd(t4, vec, coef6);
            const T t6 = NegateMultiplySub(t5, vec, coef7);
            const T one = Broadcast<T>(1.0f);
            const T result = MultiplyAdd(t6, vec, one);
            return result;
        }
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = CoefficientsArcTan(vec[i]);
        }
        return ret;
    }
}

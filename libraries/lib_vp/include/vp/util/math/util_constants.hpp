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

    constexpr inline float cFloatNan                = NAN;
    constexpr inline float cFloatInfinity           = INFINITY;
    constexpr inline float cFloatNegativeInfinity   = -INFINITY;
    constexpr inline float cFloatPi                 = 3.1415927;
    constexpr inline float cFloat2Pi                = 6.2831855;
    constexpr inline float cFloatPiDivided2         = 1.5707964;
    constexpr inline float cFloat3PiDivided2        = 4.712389;
    constexpr inline float cFloat1Divided2Pi        = 0.15915494;
    constexpr inline float cFloatDegree180          = 180.0;
    constexpr inline float cFloatNormalizeEpsilon   = 9.094947e-14;
    constexpr inline float cFloatQuaternionEpsilon  = 0.00001;
    constexpr inline float cFloatSquareroot0Point5  = 0.70710678;
    constexpr inline float cFloatUlp                = 1.1920929E-7;
    constexpr inline float cFloatMax                = FLT_MAX;

    constexpr inline double cDoublePi          = 3.1415927;
    constexpr inline double cDouble2Pi         = 6.2831855;
    constexpr inline double cDoublePiDivided2  = 1.5707964;
    constexpr inline double cDouble3PiDivided2 = 4.712389;
    constexpr inline double cDouble1Divided2Pi = 0.15915494;
    constexpr inline double cDoubleDegree180   = 180.0;
    constexpr inline double cDoubleMax         = DBL_MAX;


    template <typename T>
        requires (std::is_floating_point_v<T>)
    constexpr bool IsInf(T value) { return std::isinf(value); }

    template <typename T>
        requires (std::is_floating_point_v<T>)
    constexpr bool IsNan(T value) { return std::isnan(value); }
    
    template <typename T>
        requires (std::is_floating_point_v<T>)
    constexpr T Absolute(T value) { return std::abs(value); }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T Modulo(T value0, T value1) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::fmod(value0, value1);
        }
        return value0 % value1;
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T MultiplyAdd(T value0, T value1, T addend) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::fma(value0, value1, addend);
        }
        return (value0 * value1) + addend;
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T NegateMultiplyAdd(T value0, T value1, T addend) {
        return -(value0 * value1) + addend;
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T MultiplySub(T value0, T value1, T addend) {
        return (value0 * value1) - addend;
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T NegateMultiplySub(T value0, T value1, T addend) {
        return -(value0 * value1) - addend;
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T SquareRoot(T value) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::sqrt(value);
        }
        return static_cast<T>(std::sqrt(static_cast<double>(value)));
    }

    template <typename T>
        requires (std::is_floating_point_v<T>)
    constexpr T Pow(T value, s32 exponent) { return std::pow(value, exponent); }

    template<typename T, T Degrees> requires std::is_floating_point<T>::value
    constexpr inline T TRadians = (Degrees * cFloatPi / cFloatDegree180);

    template<typename T, T Radians> requires std::is_floating_point<T>::value
    constexpr inline T TDegrees = (Radians * cFloatDegree180 / cFloatPi);

    static_assert(TRadians<float, 45.0f> == 0.7853982f);
    static_assert(TDegrees<float, TRadians<float, 90.0f>> == 90.0f);
    static_assert(TDegrees<float, TRadians<float, 180.0f>> == 180.0f);
    static_assert(TDegrees<float, TRadians<float, 270.0f>> == 270.0f);

    template<typename T> requires std::is_floating_point<T>::value
    constexpr T ToRadians(T degrees) {
        return (degrees * (cFloatPi / cFloatDegree180));
    }

    template<typename T> requires std::is_floating_point<T>::value
    constexpr T ToDegrees(T radians) {
        return (radians * (cFloatDegree180 / cFloatPi));
    }

    constexpr ALWAYS_INLINE bool IsCacheLineSafe128(uintptr_t address) {
        const uintptr_t page_size_left = vp::util::cPageSize - (address & (vp::util::cPageSize - 1));
        const bool      result = 0x10 <= page_size_left;
        return result;
    }
    constexpr ALWAYS_INLINE bool IsCacheLineSafe256(uintptr_t address) {
        const uintptr_t page_size_left = vp::util::cPageSize - (address & (vp::util::cPageSize - 1));
        const bool      result = 0x20 <= page_size_left;
        return result;
    }
}

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

    constexpr inline float cInfinity = INFINITY;

    constexpr inline float cEpsilon = 9.094947e-14;

    constexpr inline float cFloatPi = 3.1415927;

    constexpr inline float cFloat2Pi = 6.2831855;

    constexpr inline float cFloatPiDivided2 = 1.5707964;

    constexpr inline float cFloat3PiDivided2 = 4.712389;

    constexpr inline float cFloat1Divided2Pi = 0.15915494;

    constexpr inline float cFloatDegree180 = 180.0;

    constexpr inline float cFloatQuarternionEpsilon = 1.0E-5;

    /* Ulp stands for "Unit of least precision" as in '1 / 2^23' */
    constexpr inline float cFloatUlp = 1.1920929E-7;

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

    template<typename T, T V, size_t N>
    constexpr inline T TPow = V * TPow<T, V, N-1>;

    template<typename T, T V>
    constexpr inline T TPow<T,V,0> = 1;

    template<typename T, size_t V>
    constexpr inline T TFactorial = V * TFactorial<T, V - 1>;

    template<typename T>
    constexpr inline T TFactorial<T,0> = 1;

    template<double V>
    constexpr inline double TSin = V - (TPow<double, V, 3> / TFactorial<double, 3>) + (TPow<double, V, 5> / TFactorial<double, 5>) - (TPow<double, V, 7> / TFactorial<double, 7>) + (TPow<double, V, 9> / TFactorial<double, 9>) - (TPow<double, V, 11> / TFactorial<double, 11>) + (TPow<double, V, 13> / TFactorial<double, 13>) - (TPow<double, V, 15> / TFactorial<double, 15>);

    template<double V>
    constexpr inline double TCos = 1 - (TPow<double, V, 2> / TFactorial<double, 2>) + (TPow<double, V, 4> / TFactorial<double, 4>) - (TPow<double, V, 6> / TFactorial<double, 6>) + (TPow<double, V, 8> / TFactorial<double, 8>) - (TPow<double, V, 10> / TFactorial<double, 10>) + (TPow<double, V, 12> / TFactorial<double, 12>) - (TPow<double, V, 14> / TFactorial<double, 14>);

    constexpr float cSinCoefficients[5] = {(TPow<double, 1.0, 3> / TFactorial<double,3>), (TPow<double, 1.0, 5> / TFactorial<double,5>), (TPow<double, 1.0, 7> / TFactorial<double,7>), (TPow<double, 1.0, 9> / TFactorial<double,9>), (TPow<double, 1.0, 11> / TFactorial<double,11>)};
    constexpr float cCosCoefficients[5] = {(TPow<double, 1.0, 2> / TFactorial<double,2>), (TPow<double, 1.0, 4> / TFactorial<double,4>), (TPow<double, 1.0, 6> / TFactorial<double,6>), (TPow<double, 1.0, 8> / TFactorial<double,8>), (TPow<double, 1.0, 10> / TFactorial<double,10>)};

    /* Values used to calculate an angle index for the sincos sample table */
    constexpr inline u32 cAngleIndexHalfRound = 0x80000000;
    constexpr inline u32 cAngleIndexQuarterRound = 0x40000000;
    constexpr inline u32 cAngleIndexThreeQuarterRound = 0xC0000000;

    constexpr long double PreciseSin(long double V) {
        return V - (std::pow(V, 3) / TFactorial<long double, 3>) + (std::pow(V, 5) / TFactorial<long double, 5>) - (std::pow(V, 7) / TFactorial<long double, 7>) + (std::pow(V, 9) / TFactorial<long double, 9>) - (std::pow(V, 11) / TFactorial<long double, 11>) + (std::pow(V, 13) / TFactorial<long double, 13>) - (std::pow(V, 15) / TFactorial<long double, 15>) + (std::pow(V, 17) / TFactorial<long double, 17>) - (std::pow(V, 19) / TFactorial<long double, 19>) + (std::pow(V, 21) / TFactorial<long double, 21>) - (std::pow(V, 23) / TFactorial<long double, 23>) + (std::pow(V, 25) / TFactorial<long double, 25>) - (std::pow(V, 27) / TFactorial<long double, 27>) + (std::pow(V, 29) / TFactorial<long double, 29>) - (std::pow(V, 31) / TFactorial<long double, 31>) + (std::pow(V, 33) / TFactorial<long double, 33>) - (std::pow(V, 35) / TFactorial<long double, 35>);
    }

    constexpr long double PreciseCos(long double V) {
        return  1.0l - (std::pow(V, 2) / TFactorial<long double, 2>) + (std::pow(V, 4) / TFactorial<long double, 4>) - (std::pow(V, 6) / TFactorial<long double, 6>) + (std::pow(V, 8) / TFactorial<long double, 8>) - (std::pow(V, 10) / TFactorial<long double, 10>) + (std::pow(V, 12) / TFactorial<long double, 12>) - (std::pow(V, 14) / TFactorial<long double, 14>) + (std::pow(V, 16) / TFactorial<long double, 16>) - (std::pow(V, 18) / TFactorial<long double, 18>) + (std::pow(V, 20) / TFactorial<long double, 20>) - (std::pow(V, 22) / TFactorial<long double, 22>) + (std::pow(V, 24) / TFactorial<long double, 24>) - (std::pow(V, 26) / TFactorial<long double, 26>) + (std::pow(V, 28) / TFactorial<long double, 28>) - (std::pow(V, 30) / TFactorial<long double, 30>) + (std::pow(V, 32) / TFactorial<long double, 32>) - (std::pow(V, 34) / TFactorial<long double, 34>);
    }

    consteval std::array<float, 1024> GenerateSinCosTable() {
        std::array<float, 1024>  values{}; 
        values[0] = 1.0l;
        values[1] = 0.0l;

        long double last_cos = 1.0l;
        long double last_sin = 0.0l;

        for(int i = 0; i < 255; ++i) {
            const int index = i * 4;
            const long double n = ((2.0l * 3.141592653589l) * (static_cast<long double>(i) + 1.00l) / 256.0l);
            long double cos_value = PreciseCos(n);
            long double sin_value = PreciseSin(n);
            long double cos_diff =  cos_value - last_cos;
            long double sin_diff =  sin_value - last_sin;

            if (-cFloatUlp < cos_value && cos_value < cFloatUlp) {
                cos_value = 0;
            }
            if (-cFloatUlp < sin_value && sin_value < cFloatUlp) {
                sin_value = 0;
            }

            values[index + 2] = cos_diff;
            values[index + 3] = sin_diff;
            
            values[index + 4] = cos_value;
            values[index + 5] = sin_value;
            last_cos = cos_value;
            last_sin = sin_value;
        }
        const long double cos_value = 1.0l;
        const long double sin_value = 0.0l;
        const long double cos_diff =  cos_value - last_cos;
        const long double sin_diff =  sin_value - last_sin;
        values[1022] = cos_diff;
        values[1023] = sin_diff;

        return values;
    }

    /* Size 1024 float table of cos and sin values interpolated from a period of 2Pi with differences calculated between as implemented above */
    /* Following Cos value, Sin Value, Next Cos Diff, Next Sin Diff, Next Cos value, Next Sin value ... N */
    /* Each method has 256 values and 256 differences */
    constexpr std::array<float, 1024> SinCosSampleTable = GenerateSinCosTable();
    
    static_assert(SinCosSampleTable[0] == 1.0f);
    
    constexpr ALWAYS_INLINE float AngleHalfRound(float rad) {
        return rad * (static_cast<float>(cAngleIndexHalfRound) / cFloatPi);
    }

    constexpr ALWAYS_INLINE float SampleSin(float value_from_angle_index) {
        const unsigned long long angle_index = static_cast<unsigned long long>(value_from_angle_index);
        const unsigned int       index       = (angle_index >> 24) & 0xff;
        const float variance = static_cast<float>(angle_index & 0xff'ffff) * 5.9604644775390625e-8;
        return SinCosSampleTable[(index * 4) + 1] + (SinCosSampleTable[(index * 4) + 3] * variance);
    }

    constexpr ALWAYS_INLINE float SampleCos(float value_from_angle_index) {
        const unsigned long long angle_index = static_cast<unsigned long long>(value_from_angle_index);
        const unsigned int       index       = (angle_index >> 24) & 0xff;
        const float variance = static_cast<float>(angle_index & 0xff'ffff) * 5.9604644775390625e-8;
        return SinCosSampleTable[(index * 4)] + (SinCosSampleTable[(index * 4) + 2] * variance);
    }

    constexpr ALWAYS_INLINE Vector2f SampleSinCos(float value_from_angle_index) {
        const unsigned long long angle_index = static_cast<unsigned long long>(value_from_angle_index);
        const unsigned int       index       = (angle_index >> 24) & 0xff;
        const float variance = static_cast<float>(angle_index & 0xff'ffff) * 5.9604644775390625e-8;
        return { SinCosSampleTable[(index * 4) + 1] + (SinCosSampleTable[(index * 4) + 3] * variance), SinCosSampleTable[(index * 4)] + (SinCosSampleTable[(index * 4) + 2] * variance) };
    }

    /* Interpolate a Sin value from one period  */
    constexpr ALWAYS_INLINE float GetSinPeriodStep(int t, int max) {
        const float value = ((static_cast<float>(cAngleIndexHalfRound) / cFloatPi) * ((static_cast<float>(t) * cFloat2Pi) / static_cast<float>(max)));
        return SampleSin(value);
    }

    /* Interpolate a Cos value from one period  */
    constexpr ALWAYS_INLINE float GetCosPeriodStep(int t, int max) {
        const float value = ((static_cast<float>(cAngleIndexHalfRound) / cFloatPi) * ((static_cast<float>(t) * cFloat2Pi) / static_cast<float>(max)));
        return SampleCos(value);
    }

    /* Interpolate a Sin and Cos value from one period  */
    constexpr ALWAYS_INLINE Vector2f GetSinCosPeriodStep(int t, int max) {
        const float value = ((static_cast<float>(cAngleIndexHalfRound) / cFloatPi) * ((static_cast<float>(t) * cFloat2Pi) / static_cast<float>(max)));
        return SampleSinCos(value);
    }
}

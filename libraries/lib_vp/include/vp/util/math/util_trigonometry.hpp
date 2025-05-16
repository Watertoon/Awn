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
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T Sin(T value) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::sin(value);
        }
        return static_cast<T>(std::sin(static_cast<double>(value)));
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T Cos(T value) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::cos(value);
        }
        return static_cast<T>(std::cos(static_cast<double>(value)));
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T Tan(T value) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::tan(value);
        }
        return static_cast<T>(std::tan(static_cast<double>(value)));
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T ArcSin(T value) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::asin(value);
        }
        return static_cast<T>(std::asin(static_cast<double>(value)));
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T ArcCos(T value) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::acos(value);
        }
        return static_cast<T>(std::acos(static_cast<double>(value)));
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T ArcTan(T value) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::atan(value);
        }
        return static_cast<T>(std::atan(static_cast<double>(value)));
    }

    template <typename T>
        requires (std::is_floating_point_v<T> || std::is_integral_v<T>)
    constexpr T ArcTan2(T y, T x) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::atan2(y, x);
        }
        return static_cast<T>(std::atan2(static_cast<double>(y), static_cast<double>(x)));
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

    constexpr float cSinCoefficients[5] = {
        2.06962649e-08,
        2.70857196e-06,
        1.98172518e-04,
        8.33277685e-03,
        1.66666217e-01,
    };
    constexpr float cCosCoefficients[5] = {
        2.22696745e-07,
        2.42727578e-05,
        1.38645759e-03,
        4.16617756e-02,
        4.99996803e-01,
    };
    constexpr float cArcTanCoefficients[8] = {
        0.00273138,
        0.01555041,
        0.04176995,
        0.07418835,
        0.10597013,
        0.14191457,
        0.19991017,
        0.33333011,
    };

    constexpr float CoefficientsSin(float value) {
        const float coef0 = cSinCoefficients[0];
        const float coef1 = cSinCoefficients[1];
        const float coef2 = cSinCoefficients[2];
        const float coef3 = cSinCoefficients[3];
        const float coef4 = cSinCoefficients[4];
        const float value_sq = value * value;
        const float n4 = MultiplySub(coef0, value_sq, coef1);
        const float n3 = NegateMultiplySub(n4, value_sq, coef2);
        const float n2 = MultiplyAdd(n3, value_sq, coef3);
        const float n1 = NegateMultiplySub(n2, value_sq, coef4);
        const float n0 = MultiplyAdd(n1, value_sq, 1.0f);
        const float result = value * n0;
        return result;
    }

    constexpr float CoefficientsCos(float value) {
        const float coef0 = cCosCoefficients[0];
        const float coef1 = cCosCoefficients[1];
        const float coef2 = cCosCoefficients[2];
        const float coef3 = cCosCoefficients[3];
        const float coef4 = cCosCoefficients[4];
        const float value_sq = value * value;
        const float n4 = MultiplySub(coef0, value_sq, coef1);
        const float n3 = NegateMultiplySub(n4, value_sq, coef2);
        const float n2 = MultiplyAdd(n3, value_sq, coef3);
        const float n1 = NegateMultiplySub(n2, value_sq, coef4);
        const float n0 = MultiplyAdd(n1, value_sq, 1.0f);
        const float result = n0;
        return result;
    }

    constexpr float CoefficientsArcTan(float value) {
        const float coef0 = cArcTanCoefficients[0];
        const float coef1 = cArcTanCoefficients[1];
        const float coef2 = cArcTanCoefficients[2];
        const float coef3 = cArcTanCoefficients[3];
        const float coef4 = cArcTanCoefficients[4];
        const float coef5 = cArcTanCoefficients[5];
        const float coef6 = cArcTanCoefficients[6];
        const float coef7 = cArcTanCoefficients[7];
        const float value_sq = value * value;
        const float n7 = NegateMultiplySub(coef0, value_sq, coef1);
        const float n6 = MultiplyAdd(n7, value_sq, coef2);
        const float n5 = NegateMultiplySub(n6, value_sq, coef3);
        const float n4 = MultiplyAdd(n5, value_sq, coef4);
        const float n3 = NegateMultiplySub(n4, value_sq, coef5);
        const float n2 = MultiplyAdd(n3, value_sq, coef6);
        const float n1 = NegateMultiplySub(n2, value_sq, coef7);
        const float n0 = MultiplyAdd(n1, value_sq, 1.0f);
        const float result = value * n0;
        return result;
    }

    constexpr float ApproximateArcCos(float angle_rag) {
        const float one_min_ang_sq = 1.0f - (angle_rag * angle_rag);
        if (0.0f <= angle_rag) {
            if (angle_rag <= cFloatSquareRootOneHalf) {
                /* {0 <= ang <= cFloatSqrtOneHalf} */
                const float ang_div = angle_rag / vp::util::SquareRoot(one_min_ang_sq);
                const float atan = CoefficientsArcTan(ang_div);
                const float result = cFloatPiDivided2 - atan;
                return result;
            }
            /* {cFloatSqrtOneHalf < ang} */
            const float ang_div = vp::util::SquareRoot(one_min_ang_sq) / angle_rag;
            const float atan = CoefficientsArcTan(ang_div);
            const float result = atan;
            return result;
        }
        if (-cFloatSquareRootOneHalf <= angle_rag) {
            /* {-cFloatSqrtOneHalf <= angle_rag < 0} */
            const float ang_div = -angle_rag / vp::util::SquareRoot(one_min_ang_sq);
            const float atan = CoefficientsArcTan(ang_div);
            const float result = cFloatPiDivided2 + atan;
            return result;
        }
        /* {ang < -cFloatSqrtOneHalf} */
        const float ang_div = (-vp::util::SquareRoot(one_min_ang_sq)) / angle_rag;
        const float atan = CoefficientsArcTan(ang_div);
        const float result = cFloatPiDivided2 - atan;
        return result;
    }

    constexpr float ApproximateArcTan2(float y, float x) {
        if (y == 0.0f && x == 0.0f) { return 0.0f; }
        if (0.0f <= x) {
            if (0.0f <= y) {
                if (y <= x) {
                    const float y_div_x = y / x;
                    const float atan = CoefficientsArcTan(y_div_x);
                    return atan;
                } else {
                    const float x_div_y = x / y;
                    const float atan = CoefficientsArcTan(x_div_y);
                    return vp::util::cFloatPiDivided2 - atan;
                }
            } else {
                const float neg_y = -y;
                if (neg_y <= x) {
                    const float y_div_x = neg_y / x;
                    const float atan = CoefficientsArcTan(y_div_x);
                    return -atan;
                } else {
                    const float x_div_y = x / neg_y;
                    const float atan = CoefficientsArcTan(x_div_y);
                    return atan - vp::util::cFloatPiDivided2;
                }
            }
        }
        else {
            if (0.0f <= y) {
                const float neg_x = -x;
                if (y <= x) {
                    const float y_div_x = y / neg_x;
                    const float atan = CoefficientsArcTan(y_div_x);
                    return vp::util::cFloatPi - atan;
                } else {
                    const float x_div_y = neg_x / y;
                    const float atan = CoefficientsArcTan(x_div_y);
                    return vp::util::cFloatPiDivided2 + atan;
                }
            } else {
                if (y < x) {
                    const float x_div_y = x / y;
                    const float atan = CoefficientsArcTan(x_div_y);
                    return vp::util::cFloatPiDivided2 - atan;
                } else {
                    const float y_div_x = y / x;
                    const float atan = CoefficientsArcTan(y_div_x);
                    return atan - vp::util::cFloatPi;
                }
            }
        }
    }

    /* Values used to calculate an angle index for the sincos sample table */
    constexpr inline u32 cAngleIndexHalfRound = 0x80000000;
    constexpr inline u32 cAngleIndexQuarterRound = 0x40000000;
    constexpr inline u32 cAngleIndexThreeQuarterRound = 0xc0000000;

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
    static constexpr const std::array<float, 1024> cSinCosSampleTable = GenerateSinCosTable();
    
    static_assert(cSinCosSampleTable[0] == 1.0f);
    
    constexpr ALWAYS_INLINE float AngleHalfRound(float rad) {
        return rad * (static_cast<float>(cAngleIndexHalfRound) / cFloatPi);
    }

    constexpr ALWAYS_INLINE float SampleSin(float value_from_angle_index) {
        const unsigned long long angle_index = static_cast<unsigned long long>(value_from_angle_index);
        const unsigned int       index       = (angle_index >> 24) & 0xff;
        const float variance = static_cast<float>(angle_index & 0xff'ffff) * 5.9604644775390625e-8;
        return cSinCosSampleTable[(index * 4) + 1] + (cSinCosSampleTable[(index * 4) + 3] * variance);
    }

    constexpr ALWAYS_INLINE float SampleCos(float value_from_angle_index) {
        const unsigned long long angle_index = static_cast<unsigned long long>(value_from_angle_index);
        const unsigned int       index       = (angle_index >> 24) & 0xff;
        const float variance = static_cast<float>(angle_index & 0xff'ffff) * 5.9604644775390625e-8;
        return cSinCosSampleTable[(index * 4)] + (cSinCosSampleTable[(index * 4) + 2] * variance);
    }

    constexpr ALWAYS_INLINE Float2 SampleSinCos(float value_from_angle_index) {
        const unsigned long long angle_index = static_cast<unsigned long long>(value_from_angle_index);
        const unsigned int       index       = (angle_index >> 24) & 0xff;
        const float variance = static_cast<float>(angle_index & 0xff'ffff) * 5.9604644775390625e-8;
        return { cSinCosSampleTable[(index * 4) + 1] + (cSinCosSampleTable[(index * 4) + 3] * variance), cSinCosSampleTable[(index * 4)] + (cSinCosSampleTable[(index * 4) + 2] * variance) };
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
    constexpr ALWAYS_INLINE Float2 GetSinCosPeriodStep(int t, int max) {
        const float value = ((static_cast<float>(cAngleIndexHalfRound) / cFloatPi) * ((static_cast<float>(t) * cFloat2Pi) / static_cast<float>(max)));
        return SampleSinCos(value);
    }

    enum class TrigMethod {
        Standard = 0,
        Sample = 1,
        Coefficients = 2,
    };

    constexpr float MethodSin(TrigMethod method, float value) {
        if (method == TrigMethod::Sample) {
            return SampleSin(AngleHalfRound(value));
        } else if (method == TrigMethod::Coefficients) {
            return CoefficientsSin(value);
        }
        return Sin(value);
    }

    constexpr float MethodCos(TrigMethod method, float value) {
        if (method == TrigMethod::Sample) {
            return SampleCos(AngleHalfRound(value));
        } else if (method == TrigMethod::Coefficients) {
            return CoefficientsCos(value);
        }
        return Cos(value);
    }

    constexpr Float2 MethodSinCos(TrigMethod method, float value) {
        if (method == TrigMethod::Sample) {
            return SampleSinCos(AngleHalfRound(value));
        } else if (method == TrigMethod::Coefficients) {
            return { CoefficientsSin(value), CoefficientsCos(value), };
        }
        return { Sin(value), Cos(value), };
    }
}

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

namespace vp::res {

    enum class BfresAnimCurveFrameDataType : u8 {
        Float = 0,
        S16   = 1,
        U8    = 2,
    };
    constexpr u32 GetFrameDataTypeSize(BfresAnimCurveFrameDataType data_type) {
        if (data_type == BfresAnimCurveFrameDataType::Float) {
            return sizeof(float);
        } else if (data_type == BfresAnimCurveFrameDataType::S16) {
            return sizeof(s16);
        } else if (data_type == BfresAnimCurveFrameDataType::U8) {
            return sizeof(u8);
        }
        VP_UNEXPECTED_DEFAULT();
    }
    enum class BfresAnimCurveValueDataType : u8 {
        Float = 0,
        S32   = 0,
        S16   = 1,
        S8    = 2,
    };
    enum class BfresAnimCurveCurveType : u8 {
        CubicHermiteFloat = 0,
        LinearFloat  = 1,
        BakedFloat   = 2,
        StepFloat    = 3,
        StepInteger  = 4,
        BakedInteger = 5,
        StepBoolean  = 6,
        BakedBoolean = 7,
        SphericalLinearQuaternion = 8,
        NormalizedLinearQuaternion = 9,
        SphericalLinearQuaternion2 = 10,
    };
    constexpr bool IsQuaternionType(BfresAnimCurveCurveType curve_type) {
        const u8 type = static_cast<u8>(curve_type);
        return (type & 8) != 0;
    }
    constexpr bool IsFloatType(BfresAnimCurveCurveType curve_type) {
        const u8 type = static_cast<u8>(curve_type);
        return (type & (~3)) == 0;
    }
    constexpr bool IsIntegerType(BfresAnimCurveCurveType curve_type) {
        const u8 type = static_cast<u8>(curve_type);
        return (type & 4) != 0;
    }
    constexpr bool IsTangentScaleCompatible(BfresAnimCurveCurveType curve_type, BfresAnimCurveValueDataType value_type) {
        const u8 v_type = static_cast<u8>(value_type);
        return curve_type == BfresAnimCurveCurveType::CubicHermiteFloat && v_type != 0;
    }

    enum class BfresAnimCurveWrapMode : u8 {
        Clamp  = 0,
        Repeat = 1,
        Mirror = 2,
        RelativeRepeat = 3,
    };

    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    struct ResBfresAnimCurveCubic {
        T point0;
        T tangent0;
        T point1;
        T tangent1;
    };
    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    struct ResBfresAnimCurveLinear {
        T point;
        T delta;
    };

    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    struct ResBfresAnimCurveCubicSlim {
        T point;
        T tangent;
    };
    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    struct ResBfresAnimCurveCubicTangentScale {
        float tangent_scale;
        float tangent_addend;
        ResBfresAnimCurveCubicSlim<T> curve_array[];
    };

    constexpr u32 GetCurveDataTypeSize(BfresAnimCurveCurveType curve_type, BfresAnimCurveValueDataType data_type, bool is_ext) {
        switch (curve_type) {
            case BfresAnimCurveCurveType::CubicHermiteFloat:
            {
                if (is_ext) {
                    switch (data_type) {
                        case BfresAnimCurveValueDataType::Float:
                            return sizeof(ResBfresAnimCurveCubicSlim<float>);
                        case BfresAnimCurveValueDataType::S16:
                            return sizeof(ResBfresAnimCurveCubicSlim<s16>);
                        case BfresAnimCurveValueDataType::S8:
                            return sizeof(ResBfresAnimCurveCubicSlim<s8>);
                        default:
                            VP_UNEXPECTED_DEFAULT();
                    };
                }
                switch (data_type) {
                    case BfresAnimCurveValueDataType::Float:
                        return sizeof(ResBfresAnimCurveCubic<float>);
                    case BfresAnimCurveValueDataType::S16:
                        return sizeof(ResBfresAnimCurveCubic<s16>);
                    case BfresAnimCurveValueDataType::S8:
                        return sizeof(ResBfresAnimCurveCubic<s8>);
                    default:
                        VP_UNEXPECTED_DEFAULT();
                };
            }
            case BfresAnimCurveCurveType::LinearFloat:
            {
                if (is_ext) {
                    switch (data_type) {
                        case BfresAnimCurveValueDataType::Float:
                            return sizeof(float);
                        case BfresAnimCurveValueDataType::S16:
                            return sizeof(s16);
                        case BfresAnimCurveValueDataType::S8:
                            return sizeof(s8);
                        default:
                            VP_UNEXPECTED_DEFAULT();
                    };
                }
                switch (data_type) {
                    case BfresAnimCurveValueDataType::Float:
                        return sizeof(ResBfresAnimCurveLinear<float>);
                    case BfresAnimCurveValueDataType::S16:
                        return sizeof(ResBfresAnimCurveLinear<s16>);
                    case BfresAnimCurveValueDataType::S8:
                        return sizeof(ResBfresAnimCurveLinear<s8>);
                    default:
                        VP_UNEXPECTED_DEFAULT();
                };
            }
            case BfresAnimCurveCurveType::StepFloat:
            case BfresAnimCurveCurveType::StepInteger:
            {
                switch (data_type) {
                    case BfresAnimCurveValueDataType::Float:
                        return sizeof(float);
                    case BfresAnimCurveValueDataType::S16:
                        return sizeof(s16);
                    case BfresAnimCurveValueDataType::S8:
                        return sizeof(s8);
                    default:
                        VP_UNEXPECTED_DEFAULT();
                };
            }
            case BfresAnimCurveCurveType::SphericalLinearQuaternion:
            case BfresAnimCurveCurveType::NormalizedLinearQuaternion:
            case BfresAnimCurveCurveType::SphericalLinearQuaternion2:
            {
                return sizeof(vp::util::Float4);
            }
            default:
                VP_UNEXPECTED_DEFAULT();
        };
        return 0;
    }

    struct BfresAnimFrameCache {
        float start_key;
        float end_key;
        u32   key_frame_index;
        union {
            ResBfresAnimCurveCubic<float> cache_value_cubic;
            ResBfresAnimCurveLinear<float> cache_value_linear;
            float cache_value_step;
        };
        bool is_cache_value;

        constexpr void SetDefaults() {
            this->start_key = vp::util::cFloatInfinity;
            this->is_cache_value = false;
        }
    };

    template <typename T>
        requires (std::is_same_v<T,s32> || std::is_same_v<T,float> || std::is_same_v<T,vp::util::Vector4f>)
    struct BfresFrameWrap {
        float frame;
        T addend;
    };

    struct ResBfresAnimCurve {
        void *key_frame_array;
        void *value_array;
        union {
            u16 flags;
            struct {
                u16 frame_data_type   : 2;
                u16 value_data_type   : 2;
                u16 curve_type        : 4;
                u16 pre_wrap_mode     : 2;
                u16 is_interval_scale : 1;
                u16 is_tangent_scale  : 1;
                u16 post_wrap_mode    : 2;
                u16 is_ext_curve      : 1;
                u16 reserve2          : 1;
            };
        };
        u16 key_frame_count;
        u32 base_result_offset;
        float start_frame;
        float end_frame;
        union {
            float eval_scale; /* See EvaluateFloat */
            float interval_scale; /* See FindFrameExtIntervalScaleImpl s16 u8 */
        };
        union {
            float eval_addend_float; /* See WrapFrame */
            s32   eval_addend_int;
            float value_scale; /* See EvaluateCubicExt EvaluateLinearExt EvaluateStepExt s16 s8 */
        };
        union {
            float repeat_float; /* See WrapFrame RelativeRepeat */
            s32   repeat_int;
            struct {
                float value_addend;
                float repeat_ext; /* See WrapFrameExt RelativeRepeat */
            };
            vp::util::Float4 *quat_repeat;
        };

        constexpr bool IsQuaternion() const { return (this->flags >> 7) & 1; }
        constexpr bool IsExt() const { return this->is_ext_curve; }

        constexpr s32 CalculateBakedSizeFloat() const {
            return vp::util::AlignUp((static_cast<s32>(static_cast<float>(static_cast<s32>(this->end_frame) + 1.0f)) - static_cast<s32>(this->start_frame) + 1) * sizeof(float), alignof(size_t));
        }
    
        /* Find frame */
        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,u8>)
        void FindFrameImpl(BfresAnimFrameCache *cache, float frame) const;

        void FindFrameFloat(BfresAnimFrameCache *cache, float frame) const { return this->FindFrameImpl<float>(cache, frame); }
        void FindFrameS16(BfresAnimFrameCache *cache, float frame) const { return this->FindFrameImpl<s16>(cache, frame); }
        void FindFrameU8(BfresAnimFrameCache *cache, float frame) const { return this->FindFrameImpl<u8>(cache, frame); }

        using FindFrameFunction = void(ResBfresAnimCurve::*)(BfresAnimFrameCache *cache, float frame) const;
        static constexpr const FindFrameFunction cFindFrameArray[3] = {
            &ResBfresAnimCurve::FindFrameFloat,
            &ResBfresAnimCurve::FindFrameS16,
            &ResBfresAnimCurve::FindFrameU8,
        };

        void FindFrame(BfresAnimFrameCache *cache, float frame) const;
        void UpdateFrame(BfresAnimFrameCache *cache, float frame) const;

        /* Wrap frame */
        template <typename T>
            requires (std::is_same_v<T,s32> || std::is_same_v<T,float> || std::is_same_v<T,vp::util::Vector4f>)
        constexpr BfresFrameWrap<T> WrapFrameImpl(float frame) const;

        constexpr BfresFrameWrap<float> WrapFrameFloat(float frame) const { return this->WrapFrameImpl<float>(frame); }
        constexpr BfresFrameWrap<s32> WrapFrameInt(float frame) const { return this->WrapFrameImpl<s32>(frame); }
        constexpr BfresFrameWrap<vp::util::Vector4f> WrapFrameQuaternion(float frame) const { return this->WrapFrameImpl<vp::util::Vector4f>(frame); }

        /* Find frame ext */
        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,u8>)
        void FindFrameExtImpl(BfresAnimFrameCache *cache, float frame) const;
        
        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,u8>)
        void FindFrameExtIntervalScaleImpl(BfresAnimFrameCache *cache, float frame) const;

        void FindFrameExtFloat(BfresAnimFrameCache *cache, float frame) const { this->FindFrameExtImpl<float>(cache, frame); }
        void FindFrameExtS16(BfresAnimFrameCache *cache, float frame) const { this->FindFrameExtImpl<s16>(cache, frame); }
        void FindFrameExtU8(BfresAnimFrameCache *cache, float frame) const { this->FindFrameExtImpl<u8>(cache, frame); }

        void FindFrameExtIntervalScaleS16(BfresAnimFrameCache *cache, float frame) const { return this->FindFrameExtIntervalScaleImpl<s16>(cache, frame); }
        void FindFrameExtIntervalScaleU8(BfresAnimFrameCache *cache, float frame) const { return this->FindFrameExtIntervalScaleImpl<u8>(cache, frame); }

        static constexpr const FindFrameFunction cFindFrameArrayExt[6] = {
           &ResBfresAnimCurve::FindFrameExtFloat,
           &ResBfresAnimCurve::FindFrameExtS16,
           &ResBfresAnimCurve::FindFrameExtU8,
           &ResBfresAnimCurve::FindFrameExtFloat,
           &ResBfresAnimCurve::FindFrameExtIntervalScaleS16,
           &ResBfresAnimCurve::FindFrameExtIntervalScaleU8,
        };

        /* Wrap frame ext */
        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,vp::util::Vector4f>)
        float WrapFrameExtImpl(T *out_addend, BfresAnimFrameCache *cache, float frame) const;

        float WrapFrameExt(float *out_addend, BfresAnimFrameCache *cache, float frame) const { return this->WrapFrameExtImpl<float>(out_addend, cache, frame); }
        float WrapFrameExtQuaternion(vp::util::Vector4f *out_addend, BfresAnimFrameCache *cache, float frame) const { return this->WrapFrameExtImpl<vp::util::Vector4f>(out_addend, cache, frame); }

        /* Evaluate constants */
        static constexpr u32 cCurveTypeCount = 11;
        static constexpr u32 cCurveTypeCountFloat = 3;
        static constexpr u32 cBaseIntCurveType = 4;
        static constexpr u32 cCurveTypeCountInt = 4;
        static constexpr u32 cCurveTypeCountQuaternion = 3;

        static constexpr u32 cValueDataTypeCount  = 3;

        /* Float evaluation */
        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
        float EvaluateFloatCubicImpl(BfresAnimFrameCache *cache, float frame) const;

        float EvaluateFloatCubicFloat(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatCubicImpl<float>(cache, frame); }
        float EvaluateFloatCubicS16(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatCubicImpl<s16>(cache, frame); }
        float EvaluateFloatCubicS8(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatCubicImpl<s8>(cache, frame); }

        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
        float EvaluateFloatLinearImpl(BfresAnimFrameCache *cache, float frame) const;

        float EvaluateFloatLinearFloat(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatLinearImpl<float>(cache, frame); }
        float EvaluateFloatLinearS16(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatLinearImpl<s16>(cache, frame); }
        float EvaluateFloatLinearS8(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatLinearImpl<s8>(cache, frame); }

        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
        float EvaluateFloatBakedImpl(BfresAnimFrameCache *cache, float frame) const;

        float EvaluateFloatBakedFloat(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatBakedImpl<float>(cache, frame); }
        float EvaluateFloatBakedS16(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatBakedImpl<s16>(cache, frame); }
        float EvaluateFloatBakedS8(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatBakedImpl<s8>(cache, frame); }

        using EvaluateFunctionFloat = float (ResBfresAnimCurve::*)(BfresAnimFrameCache *cache, float frame) const;
        static constexpr const EvaluateFunctionFloat cEvaluateArrayFloat[cCurveTypeCountFloat][cValueDataTypeCount] = {
            &ResBfresAnimCurve::EvaluateFloatCubicFloat,
            &ResBfresAnimCurve::EvaluateFloatCubicS16,
            &ResBfresAnimCurve::EvaluateFloatCubicS8,
            &ResBfresAnimCurve::EvaluateFloatLinearFloat,
            &ResBfresAnimCurve::EvaluateFloatLinearS16,
            &ResBfresAnimCurve::EvaluateFloatLinearS8,
            &ResBfresAnimCurve::EvaluateFloatBakedFloat,
            &ResBfresAnimCurve::EvaluateFloatBakedS16,
            &ResBfresAnimCurve::EvaluateFloatBakedS8,
        };

        float EvaluateFloat(BfresAnimFrameCache *cache, float frame) const;

        /* Int evaluation */
        template <typename T>
            requires (std::is_same_v<T,s32> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
        s32 EvaluateIntStepImpl(BfresAnimFrameCache *cache, float frame) const;

        s32 EvaluateIntStepS32(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateIntStepImpl<s32>(cache, frame); }
        s32 EvaluateIntStepS16(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateIntStepImpl<s16>(cache, frame); }
        s32 EvaluateIntStepS8(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateIntStepImpl<s8>(cache, frame); }

        template <typename T>
            requires (std::is_same_v<T,s32> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
        s32 EvaluateIntBakedImpl(BfresAnimFrameCache *cache, float frame) const;

        s32 EvaluateIntBakedS32(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateIntBakedImpl<s32>(cache, frame); }
        s32 EvaluateIntBakedS16(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateIntBakedImpl<s16>(cache, frame); }
        s32 EvaluateIntBakedS8(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateIntBakedImpl<s8>(cache, frame); }

        s32 EvaluateIntStepBool(BfresAnimFrameCache *cache, float frame) const;

        s32 EvaluateIntBakedBool(BfresAnimFrameCache *cache, float frame) const;

        using EvaluateFunctionInt = s32 (ResBfresAnimCurve::*)(BfresAnimFrameCache *cache, float frame) const;
        static constexpr const EvaluateFunctionInt cEvaluateArrayInt[cCurveTypeCountInt][cValueDataTypeCount] = {
            &ResBfresAnimCurve::EvaluateIntStepS32,
            &ResBfresAnimCurve::EvaluateIntStepS16,
            &ResBfresAnimCurve::EvaluateIntStepS8,
            &ResBfresAnimCurve::EvaluateIntBakedS32,
            &ResBfresAnimCurve::EvaluateIntBakedS16,
            &ResBfresAnimCurve::EvaluateIntBakedS8,
            &ResBfresAnimCurve::EvaluateIntStepBool,
            &ResBfresAnimCurve::EvaluateIntStepBool,
            &ResBfresAnimCurve::EvaluateIntStepBool,
            &ResBfresAnimCurve::EvaluateIntBakedBool,
            &ResBfresAnimCurve::EvaluateIntBakedBool,
            &ResBfresAnimCurve::EvaluateIntBakedBool,
        };

        s32 EvaluateInt(BfresAnimFrameCache *cache, float frame) const;

        /* Quaternion evaluation */
        vp::util::Vector4f EvaluateQuaternionSlerp(BfresAnimFrameCache *cache, float frame) const;
        vp::util::Vector4f EvaluateQuaternionNlerp(BfresAnimFrameCache *cache, float frame) const;
        vp::util::Vector4f EvaluateQuaternionSlerp2(BfresAnimFrameCache *cache, float frame) const;

        using EvaluateFunctionQuaternion = vp::util::Vector4f (ResBfresAnimCurve::*)(BfresAnimFrameCache *cache, float frame) const;
        static constexpr const EvaluateFunctionQuaternion cEvaluateArrayQuaternion[cCurveTypeCountQuaternion] = {
            &ResBfresAnimCurve::EvaluateQuaternionSlerp,
            &ResBfresAnimCurve::EvaluateQuaternionNlerp,
            &ResBfresAnimCurve::EvaluateQuaternionSlerp2,
        };

        vp::util::Vector4f EvaluateQuaternion(BfresAnimFrameCache *cache, float frame) const;

        /* Evaluate float ext */
        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
        float EvaluateFloatExtCubicImpl(BfresAnimFrameCache *cache, float frame) const;
        template <typename T>
            requires (std::is_same_v<T,s16> || std::is_same_v<T,s8>)
        float EvaluateFloatExtCubicTangentScaleImpl(BfresAnimFrameCache *cache, float frame) const;

        float EvaluateFloatExtCubicFloat(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtCubicImpl<float>(cache, frame); }
        float EvaluateFloatExtCubicS16(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtCubicImpl<s16>(cache, frame); }
        float EvaluateFloatExtCubicS8(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtCubicImpl<s8>(cache, frame); }
        float EvaluateFloatExtCubicTangentScaleS16(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtCubicImpl<s16>(cache, frame); }
        float EvaluateFloatExtCubicTangentScaleS8(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtCubicImpl<s8>(cache, frame); }

        static constexpr const EvaluateFunctionFloat cEvaluateArrayFloatExtCubic[cValueDataTypeCount] = {
            &ResBfresAnimCurve::EvaluateFloatExtCubicFloat,
            &ResBfresAnimCurve::EvaluateFloatExtCubicS16,
            &ResBfresAnimCurve::EvaluateFloatExtCubicS8,
        };
        static constexpr const EvaluateFunctionFloat cEvaluateArrayFloatExtCubicTangentScale[cValueDataTypeCount] = {
            &ResBfresAnimCurve::EvaluateFloatExtCubicFloat,
            &ResBfresAnimCurve::EvaluateFloatExtCubicTangentScaleS16,
            &ResBfresAnimCurve::EvaluateFloatExtCubicTangentScaleS8,
        };

        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
        float EvaluateFloatExtLinearImpl(BfresAnimFrameCache *cache, float frame) const;

        float EvaluateFloatExtLinearFloat(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtLinearImpl<float>(cache, frame); }
        float EvaluateFloatExtLinearS16(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtLinearImpl<s16>(cache, frame); }
        float EvaluateFloatExtLinearS8(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtLinearImpl<s8>(cache, frame); }

        static constexpr const EvaluateFunctionFloat cEvaluateArrayFloatExtLinear[cValueDataTypeCount] = {
            &ResBfresAnimCurve::EvaluateFloatExtLinearFloat,
            &ResBfresAnimCurve::EvaluateFloatExtLinearS16,
            &ResBfresAnimCurve::EvaluateFloatExtLinearS8,
        };

        float EvaluateFloatExtBaked(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtLinearFloat(cache, frame); }

        template <typename T>
            requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
        float EvaluateFloatExtStepImpl(BfresAnimFrameCache *cache, float frame) const;

        float EvaluateFloatExtStepFloat(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtStepImpl<float>(cache, frame); }
        float EvaluateFloatExtStepS16(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtStepImpl<s16>(cache, frame); }
        float EvaluateFloatExtStepS8(BfresAnimFrameCache *cache, float frame) const { return this->EvaluateFloatExtStepImpl<s8>(cache, frame); }

        static constexpr const EvaluateFunctionFloat cEvaluateArrayFloatExtStep[cValueDataTypeCount] = {
            &ResBfresAnimCurve::EvaluateFloatExtStepFloat,
            &ResBfresAnimCurve::EvaluateFloatExtStepS16,
            &ResBfresAnimCurve::EvaluateFloatExtStepS8,
        };

        float EvaluateFloatExt(BfresAnimFrameCache *cache, float frame) const;

        /* Evaluate int ext*/
        float EvaluateIntExtStepBool(BfresAnimFrameCache *cache, float frame) const;

        s32 EvaluateIntExt(BfresAnimFrameCache *cache, float frame) const;

        /* Evaluate quat ext */
        vp::util::Vector4f EvaluateQuaternionExt(BfresAnimFrameCache *cache, float frame) const;
    };
    static_assert(sizeof(ResBfresAnimCurve) == 0x30);

    /* Find frame impl */
    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,u8>)
    void ResBfresAnimCurve::FindFrameImpl(BfresAnimFrameCache *cache, float frame) const {

        /* Setup frame and bounds */
        const u32 max_key = this->key_frame_count - 1;

        /* Handle overflow */
        const T *frame_array = reinterpret_cast<const T*>(this->key_frame_array);
        T conv_frame;
        if constexpr (!std::is_same_v<T, s16>) {
            conv_frame = static_cast<T>(frame);
        } else {
            conv_frame = static_cast<T>(frame * 32.0);
        }
        if (frame_array[max_key] <= conv_frame) {
            const T start = frame_array[max_key];
            cache->key_frame_index = max_key;
            cache->start_key = static_cast<float>(start);
            cache->end_key = static_cast<float>(start) + 1.0f;
            return;
        }

        /* Quick increment or decrement by 1 */
        if (cache->start_key != vp::util::cFloatInfinity) {
            const u32 index = cache->key_frame_index;
            if (conv_frame <= static_cast<float>(frame_array[index])) {
                if (0 < index) {
                    const u32 back_index = index - 1;
                    const float back_frame = static_cast<float>(frame_array[back_index]);
                    if (back_frame <= conv_frame) {
                        cache->key_frame_index = back_index;
                        cache->start_key = back_frame;
                        cache->end_key = cache->start_key;
                        return;
                    }
                }
            } else if (index < max_key) {
                const u32 forward_index = index + 2;
                const float forward_frame = static_cast<float>(frame_array[forward_index]);
                if (conv_frame < forward_frame) {
                    cache->key_frame_index = forward_index;
                    cache->start_key = cache->end_key;
                    cache->end_key = forward_frame;
                    return;
                }
            }
        }

        /* Find next frame index from base */
        u32 base_index = static_cast<u32>(((frame - this->start_frame) * static_cast<float>(this->key_frame_count)) / (this->end_frame - this->start_frame));
        u32 index_iter = (base_index < this->key_frame_count) ? base_index : max_key;
        if (conv_frame < static_cast<float>(frame_array[index_iter])) {
            do { --index_iter; } while (conv_frame < frame_array[index_iter]);
        } else {
            do { ++index_iter; } while (frame_array[index_iter + 1] <= conv_frame);
        }
        cache->key_frame_index = index_iter;
        cache->start_key = (!std::is_same_v<T, s16>) ? static_cast<float>(frame_array[index_iter]) : static_cast<float>(frame_array[index_iter]) / 32.0f;
        cache->end_key = (!std::is_same_v<T, s16>) ? static_cast<float>(frame_array[index_iter + 1]) : static_cast<float>(frame_array[index_iter + 1]) / 32.0f;

        return;
    }

    /* Wrap frame impl */
    template <typename T>
        requires (std::is_same_v<T,s32> || std::is_same_v<T,float> || std::is_same_v<T,vp::util::Vector4f>)
    constexpr BfresFrameWrap<T> ResBfresAnimCurve::WrapFrameImpl(float frame) const {

        /* Get anim duration */
        const float start_extent = this->start_frame;
        const float end_extent   = this->end_frame;

        /* No wrap if in bounds */
        T addend;
        if constexpr (!std::is_same_v<T,vp::util::Vector4f>) {
            addend = std::bit_cast<T>(this->eval_addend_int);
        } else {
            addend = vp::util::VectorConstant<vp::util::Vector4f>::cUnit[3];
        }
        if (start_extent <= frame && frame <= end_extent) { return { frame, addend }; }

        /* Handle wrap */
        float adjust_frame = start_extent - frame;
        const bool is_post_wrap = (adjust_frame <= 0.0f);
        u32 wrap_mode;
        if (is_post_wrap) {
            wrap_mode = static_cast<u32>(this->post_wrap_mode);
            /* If our post wrap mode is clamp, clamp frame to end */
            if (wrap_mode == static_cast<u32>(BfresAnimCurveWrapMode::Clamp)) { return { end_extent, addend, }; }
        } else {
            wrap_mode = static_cast<u32>(this->pre_wrap_mode);
            /* If our pre wrap mode is clamp, clamp frame to start */
            if (wrap_mode == static_cast<u32>(BfresAnimCurveWrapMode::Clamp)) { return { start_extent, addend, }; }
            adjust_frame = frame - end_extent;
        }

        /* Calc loop count from absolute frame */
        const float frame_count = end_extent - start_extent;
        const float loop_frame = adjust_frame / frame_count;
        const u32 loop_count = static_cast<u32>(loop_frame);

        /* Calc looped offset into animation range */
        const float wrap_offset = adjust_frame - (frame_count * loop_frame);

        /* Flip wrap mode if mirror wrap */
        bool mirror = true;
        if (wrap_mode == static_cast<u32>(BfresAnimCurveWrapMode::Mirror)) { mirror = loop_count & 1; }
        /* Scale frame addend by loop count if relative repeat */
        else if (wrap_mode == static_cast<u32>(BfresAnimCurveWrapMode::RelativeRepeat)) {
           if constexpr (std::is_same_v<T,vp::util::Vector4f>) {
                const vp::util::Vector4f rot = vp::util::Convert<vp::util::Vector4f>(*reinterpret_cast<const vp::util::Float4*>(this->quat_repeat));
                if (!is_post_wrap) {
                    addend[0] = -addend[0];
                    addend[1] = -addend[1];
                    addend[2] = -addend[2];
                }
                for (u32 i = 0; i < loop_count; ++i) {
                    addend = vp::util::QuatMultiply(addend, rot);
                }
           } else {               
               const T delta = (std::is_same_v<T,float>) ? this->repeat_float * static_cast<float>(loop_count + 1) : this->repeat_int + (this->repeat_int * loop_count);
               const T flip_delta = (is_post_wrap) ? delta : -delta;
               addend = addend + flip_delta;
           }
        }

        /* Wrap frame */
        const float wrapped_frame = (mirror != is_post_wrap) ? (end_extent - wrap_offset) : (start_extent + wrap_offset);
        return { wrapped_frame, addend, };
    }

    /* Find frame ext impl */
    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,u8>)
    void ResBfresAnimCurve::FindFrameExtImpl(BfresAnimFrameCache *cache, float frame) const {

        /* Handle key frame overflow */
        const u32 max_key = this->key_frame_count - 1;
        const T *frame_array = reinterpret_cast<const T*>(this->key_frame_array);
        const T conv_frame = (!std::is_same_v<T, s16>) ? static_cast<T>(frame) : static_cast<T>(frame * 32.0);
        if (frame_array[max_key] <= conv_frame) {
            const T end = frame_array[max_key];
            cache->key_frame_index = max_key;
            cache->start_key = static_cast<float>(end);
            cache->end_key = static_cast<float>(end) + 1.0f;
            cache->is_cache_value = false;
            return;
        }

        /* Quick increment or decrement by 1 */
        if (cache->is_cache_value) {
            const u32 index = cache->key_frame_index;
            if (conv_frame <= frame_array[index]) {
                if (0 < index) {
                    const u32 back_index = index - 1;
                    const float back_frame = frame_array[back_index];
                    if (back_frame <= conv_frame) {
                        cache->key_frame_index = back_index;
                        cache->start_key = back_frame;
                        cache->end_key = cache->start_key;
                        cache->is_cache_value = false;
                        return;
                    }
                }
            } else if (index < max_key) {
                const u32 forward_index = index + 2;
                const float forward_frame = frame_array[forward_index];
                if (conv_frame < forward_frame) {
                    cache->key_frame_index = forward_index;
                    cache->start_key = cache->end_key;
                    cache->end_key = forward_frame;
                    cache->is_cache_value = false;
                    return;
                }
            }
        }

        /* Handle frame overflow */
        cache->is_cache_value = false;
        u32 base_index = static_cast<u32>(((frame - this->start_frame) * static_cast<float>(this->key_frame_count)) / (this->end_frame - this->start_frame));
        if (this->key_frame_count <= base_index) {
            cache->key_frame_index = max_key;
            cache->start_key = this->end_frame;
            cache->end_key = this->end_frame + 1.0f;
        }

        /* Find next frame index from base */
        u32 index_iter = (base_index < this->key_frame_count) ? base_index : max_key;
        if (conv_frame < frame_array[index_iter]) {
            do { --index_iter; } while (conv_frame < frame_array[index_iter]);
        } else {
            do { ++index_iter; } while (frame_array[index_iter + 1] <= conv_frame);
        }
        cache->key_frame_index = index_iter;
        cache->start_key = (!std::is_same_v<T, s16>) ? static_cast<float>(frame_array[index_iter]) : static_cast<float>(frame_array[index_iter]) / 32.0f;
        cache->end_key = (!std::is_same_v<T, s16>) ? static_cast<float>(frame_array[index_iter + 1]) : static_cast<float>(frame_array[index_iter + 1]) / 32.0f;

        return;
    }

    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,u8>)
    void ResBfresAnimCurve::FindFrameExtIntervalScaleImpl(BfresAnimFrameCache *cache, float frame) const {

        /* Handle key frame overflow */
        const float interval_scale = this->interval_scale;
        const u32 max_key = this->key_frame_count - 1;
        const T *frame_array = reinterpret_cast<const T*>(this->key_frame_array);
        const T conv_frame = static_cast<T>((frame - this->start_frame) / interval_scale);
        if (frame_array[max_key] <= conv_frame) {
            const T end = frame_array[max_key];
            cache->key_frame_index = max_key;
            cache->start_key = static_cast<float>(end);
            cache->end_key = static_cast<float>(end) + 1.0f;
            cache->is_cache_value = false;
            return;
        }

        /* Quick increment or decrement by 1 */
        if (cache->is_cache_value) {
            const u32 index = cache->key_frame_index;
            if (conv_frame <= frame_array[index]) {
                if (0 < index) {
                    const u32 back_index = index - 1;
                    const T back_frame = frame_array[back_index];
                    if (back_frame <= conv_frame) {
                        cache->key_frame_index = back_index;
                        cache->start_key = static_cast<float>(back_frame) * interval_scale + this->start_frame;
                        cache->end_key = cache->start_key;
                        cache->is_cache_value = false;
                        return;
                    }
                }
            } else if (index < max_key) {
                const u32 forward_index = index + 2;
                const T forward_frame = frame_array[forward_index];
                if (conv_frame < forward_frame) {
                    cache->key_frame_index = forward_index;
                    cache->start_key = cache->end_key;
                    cache->end_key = static_cast<float>(forward_frame) * interval_scale + this->start_frame;
                    cache->is_cache_value = false;
                    return;
                }
            }
        }

        /* Handle frame overflow */
        cache->is_cache_value = false;
        u32 base_index = static_cast<u32>(((frame - this->start_frame) * static_cast<float>(this->key_frame_count)) / (this->end_frame - this->start_frame));
        if (this->key_frame_count <= base_index) {
            cache->key_frame_index = max_key;
            cache->start_key = this->end_frame;
            cache->end_key = this->end_frame + 1.0f;
        }

        /* Find next frame index from base */
        u32 index_iter = (base_index < this->key_frame_count) ? base_index : max_key;
        if (conv_frame < frame_array[index_iter]) {
            do { --index_iter; } while (conv_frame < frame_array[index_iter]);
        } else {
            do { ++index_iter; } while (frame_array[index_iter + 1] <= conv_frame);
        }
        cache->key_frame_index = index_iter;
        cache->start_key = static_cast<float>(frame_array[index_iter]) * interval_scale + this->start_frame;
        cache->end_key = static_cast<float>(frame_array[index_iter + 1]) * interval_scale + this->start_frame;

        return;
    }

    /* Wrap frame ext impl */
    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,vp::util::Vector4f>)
    float ResBfresAnimCurve::WrapFrameExtImpl(T *out_addend, BfresAnimFrameCache *cache, float frame) const {
        
        /* Wrap frame if necessary */
        float wrapped_frame = frame;
        if (this->end_frame < wrapped_frame) {

            /* Post wrap impl */
            const float frame_offset = frame - this->start_frame;
            const float frame_range = this->end_frame - this->start_frame;
            const float loop_offset = frame_offset / frame_range;
            const u32 loop_count = static_cast<u32>(loop_offset);
            const float local_frame_offset = (frame_range * static_cast<float>(loop_count)) - frame_offset;
            switch (static_cast<BfresAnimCurveWrapMode>(this->post_wrap_mode)) {
                case BfresAnimCurveWrapMode::Clamp:
                {
                    wrapped_frame = this->end_frame;
                    break;
                }
                case BfresAnimCurveWrapMode::Repeat:
                {
                    wrapped_frame = this->start_frame + local_frame_offset;
                    break;
                }
                case BfresAnimCurveWrapMode::Mirror:
                {
                    wrapped_frame = (loop_count & 1) ? this->end_frame - local_frame_offset : this->start_frame + local_frame_offset;
                    break;
                }
                case BfresAnimCurveWrapMode::RelativeRepeat:
                {
                    wrapped_frame = this->start_frame + local_frame_offset;
                    if constexpr (std::is_same_v<T,vp::util::Vector4f>) {
                        vp::util::Vector4f repeat = vp::util::Convert<vp::util::Vector4f>(*reinterpret_cast<const vp::util::Float4*>(this->quat_repeat));
                        vp::util::Vector4f addend = *out_addend;
                        for (u32 i = 0; i < loop_count; ++i) {
                            addend = vp::util::QuatMultiply(addend, repeat);
                        }
                        *out_addend = addend;
                    } else {
                        *out_addend += this->repeat_ext * static_cast<float>(loop_count);
                    }
                    break;
                }
            };            
        } else if (wrapped_frame < this->start_frame) {

            /* Pre wrap impl */
            const float frame_offset = this->start_frame - frame;
            const float frame_range = this->end_frame - this->start_frame;
            const float loop_offset = frame_offset / frame_range;
            const u32 loop_count = static_cast<u32>(loop_offset);
            const float local_frame_offset = (frame_range * static_cast<float>(loop_count)) - frame_offset;
            switch (static_cast<BfresAnimCurveWrapMode>(this->pre_wrap_mode)) {
                case BfresAnimCurveWrapMode::Clamp:
                {
                    wrapped_frame = this->start_frame;
                    break;
                }
                case BfresAnimCurveWrapMode::Repeat:
                {
                    wrapped_frame = this->end_frame - local_frame_offset;
                    break;
                }
                case BfresAnimCurveWrapMode::Mirror:
                {
                    wrapped_frame = (loop_count & 1) ? this->end_frame - local_frame_offset : this->start_frame + local_frame_offset;
                    break;
                }
                case BfresAnimCurveWrapMode::RelativeRepeat:
                {
                    wrapped_frame = this->end_frame - local_frame_offset;
                    if constexpr (std::is_same_v<T,vp::util::Vector4f>) {
                        vp::util::Vector4f repeat = vp::util::Convert<vp::util::Vector4f>(*reinterpret_cast<const vp::util::Float4*>(this->quat_repeat));
                        vp::util::Vector4f addend = *out_addend;
                        addend[0] = -addend[0];
                        addend[1] = -addend[1];
                        addend[2] = -addend[2];
                        for (u32 i = 0; i < loop_count; ++i) {
                            addend = vp::util::QuatMultiply(addend, repeat);
                        }
                        *out_addend = addend;
                    } else {
                        *out_addend += this->repeat_ext * static_cast<float>(~loop_count);
                    }
                    break;
                }
            };
        }

        /* Frame in cache */
        if (wrapped_frame < cache->end_key && cache->start_key < wrapped_frame) {
            return wrapped_frame;
        }

        /* Find frame ext */
        if (static_cast<BfresAnimCurveCurveType>(this->curve_type) == BfresAnimCurveCurveType::BakedFloat) {
            const u32 offset = static_cast<u32>(wrapped_frame - this->start_frame);
            cache->key_frame_index = offset;
            const float base = this->start_frame + static_cast<float>(offset);
            cache->start_key = base;
            cache->end_key = base + 1.0f;
            cache->is_cache_value = false;
            return wrapped_frame;
        }
        const u32 frame_data_type_index = static_cast<u32>(this->frame_data_type);
        if (VP_UNLIKELY(2 < frame_data_type_index)) { VP_UNEXPECTED_DEFAULT(); }
        const u32 base_index = (!this->is_interval_scale) ?  frame_data_type_index : frame_data_type_index + 3;
        (this->*cFindFrameArrayExt[base_index])(cache, frame);
        return wrapped_frame;
    }

    /* Evalute float */
    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    float ResBfresAnimCurve::EvaluateFloatCubicImpl(BfresAnimFrameCache *cache, float frame) const {

        /* Update frame */
        this->UpdateFrame(cache, frame);

        /* Perform cubic hermite interpolation */
        const ResBfresAnimCurveCubic<T> *curve_array = reinterpret_cast<const ResBfresAnimCurveCubic<T>*>(this->value_array);
        const u32   i     = cache->key_frame_index;
        const float t     = (frame - this->start_frame) / (this->end_frame - this->start_frame);
        const float value = static_cast<float>(curve_array[i].point0) + t * static_cast<float>(curve_array[i].tangent0) + t * t * (static_cast<float>(curve_array[i].point1) + t * static_cast<float>(curve_array[i].tangent1));

        return value;
    }

    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    float ResBfresAnimCurve::EvaluateFloatLinearImpl(BfresAnimFrameCache *cache, float frame) const {

        /* Update frame */
        this->UpdateFrame(cache, frame);

        /* Perform linear interpolation */
        const ResBfresAnimCurveLinear<T> *curve_array = reinterpret_cast<const ResBfresAnimCurveLinear<T>*>(this->value_array);
        const u32   i     = cache->key_frame_index;
        const float t     = (frame - this->start_frame) / (this->end_frame - this->start_frame);
        const float value = static_cast<float>(curve_array[i].point) + t * static_cast<float>(curve_array[i].delta);

        return value;
    }

    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    float ResBfresAnimCurve::EvaluateFloatBakedImpl(BfresAnimFrameCache *cache, float frame) const {
        VP_UNUSED(cache);
        
        /* Find value */
        const T *value_location = reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(this->value_array) + sizeof(s16) * (static_cast<u32>(frame) - static_cast<u32>(this->start_frame)));

        /* Interpolate */
        const float round = (frame - static_cast<float>(static_cast<u32>(frame)));
        const float value = static_cast<float>(value_location[0]) * (1.0f - round) + static_cast<float>(value_location[1]) * round;

        return value;
    }

    /* Evaluate int */
    template <typename T>
        requires (std::is_same_v<T,s32> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    s32 ResBfresAnimCurve::EvaluateIntStepImpl(BfresAnimFrameCache *cache, float frame) const {

        /* Update frame */
        this->UpdateFrame(cache, frame);

        /* Evaluate */
        const T *curve_array = reinterpret_cast<const T*>(this->value_array);
        const s32 value = static_cast<s32>(curve_array[cache->key_frame_index]);

        return value;
    }

    template <typename T>
        requires (std::is_same_v<T,s32> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    s32 ResBfresAnimCurve::EvaluateIntBakedImpl(BfresAnimFrameCache *cache, float frame) const {
        VP_UNUSED(cache);

        /* Find value */
        const T *value_location = reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(this->value_array) + sizeof(T) * (static_cast<u32>(frame) - static_cast<u32>(this->start_frame)));

        /* Evaluate */
        const s32 value = static_cast<s32>(*value_location);

        return value;
    }

    /* Evaluate float ext */
    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    float ResBfresAnimCurve::EvaluateFloatExtCubicImpl(BfresAnimFrameCache *cache, float frame) const {

        /* Get key frame range */
        const float frame_range = cache->end_key - cache->start_key;

        /* Get from curve and to curve */
        float point0;
        float tangent0;
        float point1;
        float tangent1;
        if (!cache->is_cache_value) {

            /* Unpack points and tangents for from and to */
            const ResBfresAnimCurveCubicSlim<T> *curve_array = reinterpret_cast<const ResBfresAnimCurveCubicSlim<T>*>(this->value_array);
            const s32 base_index = cache->key_frame_index;
            point0 = (std::is_same_v<T,float>) ? curve_array[base_index].point : static_cast<float>(curve_array[base_index].point) * this->value_scale + this->value_addend;
            if (base_index == this->key_frame_count - 1) {
                return point0;
            }
            const float t0 = (std::is_same_v<T,float>) ? curve_array[base_index].tangent : static_cast<float>(curve_array[base_index].tangent) * this->value_scale + this->value_addend;
            const float p1 = (std::is_same_v<T,float>) ? curve_array[base_index + 1].point : static_cast<float>(curve_array[base_index + 1].point) * this->value_scale + this->value_addend;
            const float t1 = (std::is_same_v<T,float>) ? curve_array[base_index + 1].tangent : static_cast<float>(curve_array[base_index + 1].tangent) * this->value_scale + this->value_addend;
            tangent0 = frame_range * t0;
            point1 = (p1 - point0) * 3.0f - (t0 * 2.0f + t1) * frame_range;
            tangent1 = (p1 - point0) * -2.0f + (t0 + t1) * frame_range;
            cache->is_cache_value = true;
            cache->cache_value_cubic.point0 = point0;
            cache->cache_value_cubic.tangent0 = tangent0;
            cache->cache_value_cubic.point1 = point1;
            cache->cache_value_cubic.tangent1 = tangent1;

        } else {
            point0 = cache->cache_value_cubic.point0;
            tangent0 = cache->cache_value_cubic.tangent0;
            point1 = cache->cache_value_cubic.point1;
            tangent1 = cache->cache_value_cubic.tangent1;
        }

        const float t = (frame - cache->start_key) / frame_range;
        const float pt0 = point0 + tangent0 * t;
        const float pt1 = point1 + tangent1 * t;
        const float value = pt0 + t * t * pt1;
        return value;
    }

    template <typename T>
        requires (std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    float ResBfresAnimCurve::EvaluateFloatExtCubicTangentScaleImpl(BfresAnimFrameCache *cache, float frame) const {

        /* Get key frame range */
        const float frame_range = cache->end_key - cache->start_key;

        /* Get from curve and to curve */
        float point0;
        float tangent0;
        float point1;
        float tangent1;
        if (!cache->is_cache_value) {

            /* Unpack points and tangents for from and to with separate tangent scale */
            const ResBfresAnimCurveCubicTangentScale<T> *slim_head = reinterpret_cast<const ResBfresAnimCurveCubicTangentScale<T>*>(this->value_array);
            const ResBfresAnimCurveCubicSlim<T> *curve_array = slim_head->curve_array;
            const s32 base_index = cache->key_frame_index * 2;
            const float p0 = static_cast<float>(curve_array[base_index]);
            point0 = p0 * this->value_scale + this->value_addend;
            if (base_index == this->key_frame_count - 1) {
                return point0;
            }
            const float t0_base = static_cast<float>(curve_array[base_index].tangent);
            const float p1_base = static_cast<float>(curve_array[base_index + 1].point);
            const float t1_base = static_cast<float>(curve_array[base_index + 1].tangent);
            const float t0 = t0_base * slim_head->tangent_scale + slim_head->tangent_addend;
            const float p1 = p1_base * this->value_scale + this->value_addend;
            const float t1 = t1_base * slim_head->tangent_scale + slim_head->tangent_addend;
            tangent0 = frame_range * t0;
            point1 = (p1 - point0) * 3.0f - (t0 * 2.0f + t1) * frame_range;
            tangent1 = (p1 - point0) * -2.0f + (t0 + t1) * frame_range;
            cache->is_cache_value = true;
            cache->cache_value_cubic.point0 = point0;
            cache->cache_value_cubic.tangent0 = tangent0;
            cache->cache_value_cubic.point1 = point1;
            cache->cache_value_cubic.tangent1 = tangent1;

        } else {
            point0 = cache->cache_value_cubic.point0;
            tangent0 = cache->cache_value_cubic.tangent0;
            point1 = cache->cache_value_cubic.point1;
            tangent1 = cache->cache_value_cubic.tangent1;
        }

        const float t = (frame - cache->start_key) / frame_range;
        const float pt0 = point0 + tangent0 * t;
        const float pt1 = point1 + tangent1 * t;
        const float value = pt0 + t * t * pt1;
        return value;
    }

    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    float ResBfresAnimCurve::EvaluateFloatExtLinearImpl(BfresAnimFrameCache *cache, float frame) const {

        /* Get from curve and to curve */
        float point;
        float delta;
        if (!cache->is_cache_value) {

            /* Unpack points for from and to */
            const T *curve_array = reinterpret_cast<const T*>(this->value_array);
            const s32 base_index = cache->key_frame_index;
            point = (std::is_same_v<T,float>) ? curve_array[base_index] : static_cast<float>(curve_array[base_index]) * this->value_scale + this->value_addend;
            if (base_index == this->key_frame_count - 1) {
                return point;
            }
            const float p1 = (std::is_same_v<T,float>) ? curve_array[base_index + 1] : static_cast<float>(curve_array[base_index + 1]) * this->value_scale + this->value_addend;
            delta = (p1 - point);
            cache->is_cache_value = true;
            cache->cache_value_linear.point = point;
            cache->cache_value_linear.delta = delta;

        } else {
            point = cache->cache_value_linear.point;
            delta = cache->cache_value_linear.delta;
        }

        const float frame_range = cache->end_key - cache->start_key;
        const float t = (frame - cache->start_key) / frame_range;
        const float value = point + delta * t;
        return value;
    }

    template <typename T>
        requires (std::is_same_v<T,float> || std::is_same_v<T,s16> || std::is_same_v<T,s8>)
    float ResBfresAnimCurve::EvaluateFloatExtStepImpl(BfresAnimFrameCache *cache, float frame) const {
        VP_UNUSED(frame);

        /* Cache value */
        if (cache->is_cache_value) {
            return cache->cache_value_step;
        }

        /* Load step */
        const T *curve_array = reinterpret_cast<const T*>(this->value_array);
        const u32 base_index = cache->key_frame_index;
        const float value = (std::is_same_v<T,float>) ? curve_array[base_index] : static_cast<float>(curve_array[base_index]) * this->value_scale + this->value_addend;
        cache->is_cache_value = true;
        cache->cache_value_step = value;
        return value;
    }
}

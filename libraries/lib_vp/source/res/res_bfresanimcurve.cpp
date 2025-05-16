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
#include <vp.hpp>

namespace vp::res {

    void ResBfresAnimCurve::FindFrame(BfresAnimFrameCache *cache, float frame) const {

        /* Find frame by data type */
        const u32 frame_data_type_index = static_cast<u32>(this->frame_data_type);
        if (VP_UNLIKELY(2 < frame_data_type_index)) { VP_UNEXPECTED_DEFAULT(); }
        (this->*cFindFrameArray[frame_data_type_index])(cache, frame);

        return;
    }

    void ResBfresAnimCurve::UpdateFrame(BfresAnimFrameCache *cache, float frame) const {

        /* Find frame if necessary */
        if (cache->start_key <= frame && frame <= cache->end_key) { return; }

        /* Estimate next frame */
        const float frames_per_key    = static_cast<float>(key_frame_count) * (1.0f / (this->end_frame - this->start_frame));
        const u32 estimated_key_index = static_cast<u32>((frame - this->start_frame) * frames_per_key);
        cache->key_frame_index        = (key_frame_count <= estimated_key_index) ? key_frame_count - 1 : estimated_key_index;

        /* Fixup */
        this->FindFrame(cache, frame);

        return;
    }

    float ResBfresAnimCurve::EvaluateFloat(BfresAnimFrameCache *cache, float frame) const {

        /* Check ext curve */
        if (this->IsExt()) {
            return this->EvaluateFloatExt(cache, frame);
        }

        /* Handle wrap */
        const BfresFrameWrap<float> wrap_frame = this->WrapFrameFloat(frame);

        /* Evaluate */
        const u32 value_data_type_index = static_cast<u32>(this->value_data_type);
        if (VP_UNLIKELY(2 < value_data_type_index)) { VP_UNEXPECTED_DEFAULT(); }
        const u32 curve_type_index = static_cast<u32>(this->curve_type);
        if (VP_UNLIKELY(2 < curve_type_index)) { VP_UNEXPECTED_DEFAULT(); }
        const float eval = (this->*cEvaluateArrayFloat[curve_type_index][value_data_type_index])(cache, wrap_frame.frame);

        /* Fixup output */
        return eval * this->eval_scale + wrap_frame.addend;
    }

    s32 ResBfresAnimCurve::EvaluateIntStepBool(BfresAnimFrameCache *cache, float frame) const {

        /* Update frame */
        this->UpdateFrame(cache, frame);

        const u32 *curve_array = reinterpret_cast<u32*>(this->value_array);

        /* Evaluate */
        const s32 value = static_cast<s32>((curve_array[cache->key_frame_index >> 5] >> cache->key_frame_index) & 1);

        return value;
    }

    s32 ResBfresAnimCurve::EvaluateIntBakedBool(BfresAnimFrameCache *cache, float frame) const {
        VP_UNUSED(cache);

        /* Find value */
        const u32  frame_index = static_cast<u32>(frame) - static_cast<u32>(this->start_frame);
        const u32 *value_location = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(this->value_array) + sizeof(u32) * (frame_index >> 5));

        /* Evaluate */
        const s32 value = static_cast<s32>((*value_location >> frame_index) & 1);

        return value;
    }

    s32 ResBfresAnimCurve::EvaluateInt(BfresAnimFrameCache *cache, float frame) const {

        /* Check ext curve */
        if (this->IsExt()) {
            return this->EvaluateIntExt(cache, frame);
        }

        /* Handle wrap */
        const BfresFrameWrap<s32> wrap_frame = this->WrapFrameInt(frame);

        /* Evaluate */
        const u32 value_data_type_index = static_cast<u32>(value_data_type);
        const u32 curve_type_index = static_cast<u32>(curve_type) - cBaseIntCurveType;
        const s32 eval = (this->*cEvaluateArrayInt[curve_type_index][value_data_type_index])(cache, wrap_frame.frame);

        /* Fixup output */
        return wrap_frame.addend + eval;
    }

    vp::util::Vector4f ResBfresAnimCurve::EvaluateQuaternionSlerp(BfresAnimFrameCache *cache, float frame) const {

        /* Get curve keyframe location */
        const s32 index = cache->key_frame_index;
        cache->is_cache_value = true;

        /* Clamp to end key */
        const vp::util::Float4 *curve_array = reinterpret_cast<const vp::util::Float4*>(this->value_array);
        if (index == this->key_frame_count - 1) { return vp::util::CastVector4f(curve_array[index]); }

        /* Interpolate key frames */
        vp::util::Vector4f from = vp::util::CastVector4f(curve_array[index]);
        vp::util::Vector4f to = vp::util::CastVector4f(curve_array[index + 1]);
        const float t = (frame - cache->start_key) / (cache->end_key - cache->start_key);
        return vp::util::ApproximateQuatSphericalLinearInterpolation(from, to, t, 0);
    }

    vp::util::Vector4f ResBfresAnimCurve::EvaluateQuaternionNlerp(BfresAnimFrameCache *cache, float frame) const {
        
        /* Get curve keyframe location */
        const s32 index = cache->key_frame_index;
        cache->is_cache_value = true;

        /* Clamp to end key */
        const vp::util::Float4 *curve_array = reinterpret_cast<const vp::util::Float4*>(this->value_array);
        if (index == this->key_frame_count - 1) { return vp::util::CastVector4f(curve_array[index]); }

        /* Interpolate key frames */
        vp::util::Vector4f from = vp::util::CastVector4f(curve_array[index]);
        vp::util::Vector4f to = vp::util::CastVector4f(curve_array[index + 1]);
        const float t = (frame - cache->start_key) / (cache->end_key - cache->start_key);
        return vp::util::ApproximateQuatNormalizedLinearInterpolation(from, to, t);
    }

    vp::util::Vector4f ResBfresAnimCurve::EvaluateQuaternionSlerp2(BfresAnimFrameCache *cache, float frame) const {
        VP_UNUSED(cache, frame);
        VP_UNEXPECTED_DEFAULT();
        return vp::util::Vector4f::cZero;
    }

    vp::util::Vector4f ResBfresAnimCurve::EvaluateQuaternion(BfresAnimFrameCache *cache, float frame) const {

        /* Check ext curve */
        if (this->IsExt()) {
            return this->EvaluateQuaternionExt(cache, frame);
        }

        /* Handle wrap */
        const BfresFrameWrap<vp::util::Vector4f> wrap_frame = this->WrapFrameQuaternion(frame);

        /* Find current frame */
        if (frame < cache->start_key || cache->end_key < frame) {            
            this->FindFrame(cache, frame);
        }

        /* Evaluate */
        const u32 curve_type_index = static_cast<u32>(this->curve_type) - 8;
        if (VP_UNLIKELY(2 < curve_type_index)) { VP_UNEXPECTED_DEFAULT(); }
        const vp::util::Vector4f value = (this->*cEvaluateArrayQuaternion[curve_type_index])(cache, frame);
        return vp::util::QuatMultiply(value, wrap_frame.addend);
    }

    vp::util::Vector4f ResBfresAnimCurve::EvaluateQuaternionExt(BfresAnimFrameCache *cache, float frame) const {

        /* Wrap frame */
        BfresFrameWrap<vp::util::Vector4f> wrap_frame;
        wrap_frame.frame = this->WrapFrameExtQuaternion(std::addressof(wrap_frame.addend), cache, frame);

        /* Evaluate */
        const u32 curve_type_index = static_cast<u32>(this->curve_type) - 8;
        if (VP_UNLIKELY(2 < curve_type_index)) { VP_UNEXPECTED_DEFAULT(); }
        const vp::util::Vector4f value = (this->*cEvaluateArrayQuaternion[curve_type_index])(cache, frame);
        return vp::util::QuatMultiply(value, wrap_frame.addend);
    }

    float ResBfresAnimCurve::EvaluateFloatExt(BfresAnimFrameCache *cache, float frame) const {

        /* Handle wrap */
        BfresFrameWrap<float> wrap_frame;
        wrap_frame.frame = this->WrapFrameExt(std::addressof(wrap_frame.addend), cache, frame);

        /* Evaluate */
        float eval;
        const u32 value_data_type_index = static_cast<u32>(this->value_data_type);
        if (VP_UNLIKELY(2 < value_data_type_index)) { VP_UNEXPECTED_DEFAULT(); }
        const BfresAnimCurveCurveType curve_data_type = static_cast<BfresAnimCurveCurveType>(this->curve_type);
        switch (curve_data_type) {
            case BfresAnimCurveCurveType::CubicHermiteFloat:
            {
                if (this->is_tangent_scale) {
                    eval = (this->*cEvaluateArrayFloatExtCubicTangentScale[value_data_type_index])(cache, wrap_frame.frame);
                } else {
                    eval = (this->*cEvaluateArrayFloatExtCubic[value_data_type_index])(cache, wrap_frame.frame);
                }
                break;
            }
            case BfresAnimCurveCurveType::LinearFloat:
            {
                eval = (this->*cEvaluateArrayFloatExtLinear[value_data_type_index])(cache, wrap_frame.frame);
                break;
            }
            case BfresAnimCurveCurveType::BakedFloat:
            {
                eval = this->EvaluateFloatExtBaked(cache, wrap_frame.frame);
                break;
            }
            case BfresAnimCurveCurveType::StepFloat:
            {
                eval = (this->*cEvaluateArrayFloatExtStep[value_data_type_index])(cache, wrap_frame.frame);
                break;
            }
            default:
                VP_UNEXPECTED_DEFAULT();
        };

        /* Fixup output */
        return eval + wrap_frame.addend;
    }

    float ResBfresAnimCurve::EvaluateIntExtStepBool(BfresAnimFrameCache *cache, float frame) const {
        VP_UNUSED(frame);

        /* Cache value */
        if (cache->is_cache_value) {
            return cache->cache_value_step;
        }

        /* Load step */
        const u8 *curve_array = reinterpret_cast<const u8*>(this->value_array);
        const u32 base_index = cache->key_frame_index;
        const float value = static_cast<float>((curve_array[base_index >> 3] >> (base_index & 7)) & 1);
        cache->is_cache_value = true;
        cache->cache_value_step = value;
        return value;
    }

    s32 ResBfresAnimCurve::EvaluateIntExt(BfresAnimFrameCache *cache, float frame) const {

        /* Handle wrap */
        BfresFrameWrap<float> wrap_frame;
        wrap_frame.frame = this->WrapFrameExt(std::addressof(wrap_frame.addend), cache, frame);

        /* Evaluate */
        float eval;
        const u32 value_data_type_index = static_cast<u32>(this->value_data_type);
        if (VP_UNLIKELY(2 < value_data_type_index)) { VP_UNEXPECTED_DEFAULT(); }
        const BfresAnimCurveCurveType curve_data_type = static_cast<BfresAnimCurveCurveType>(this->curve_type);
        switch (curve_data_type) {
            case BfresAnimCurveCurveType::StepInteger:
            {
                eval = (this->*cEvaluateArrayFloatExtStep[value_data_type_index])(cache, wrap_frame.frame);
                break;
            }
            case BfresAnimCurveCurveType::BakedInteger:
            case BfresAnimCurveCurveType::BakedBoolean:
            {
                eval = this->EvaluateFloatExtBaked(cache, wrap_frame.frame);
                break;
            }
            case BfresAnimCurveCurveType::StepBoolean:
            {
                eval = this->EvaluateIntExtStepBool(cache, wrap_frame.frame);
                break;
            }
            default:
                VP_UNEXPECTED_DEFAULT();
        };
        return static_cast<s32>(eval + wrap_frame.addend);
    }
}

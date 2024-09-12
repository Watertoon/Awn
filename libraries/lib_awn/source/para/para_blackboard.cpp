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
#include <awn.hpp>

namespace awn::para {

    void Blackboard::Initialize(mem::Heap *heap, const BlackboardInitInfo *blackboard_info) {
        m_state_string.Initialize(heap, blackboard_info);
        m_state_s32.Initialize(heap, blackboard_info);
        m_state_float.Initialize(heap, blackboard_info);
        m_state_bool.Initialize(heap, blackboard_info);
        m_state_vector3f.Initialize(heap, blackboard_info);
        m_state_pointer.Initialize(heap, blackboard_info);
        m_state_s8.Initialize(heap, blackboard_info);
        m_state_s16.Initialize(heap, blackboard_info);
        m_state_s64.Initialize(heap, blackboard_info);
        m_state_u8.Initialize(heap, blackboard_info);
        m_state_u16.Initialize(heap, blackboard_info);
        m_state_u32.Initialize(heap, blackboard_info);
        m_state_u64.Initialize(heap, blackboard_info);
        m_state_double.Initialize(heap, blackboard_info);
        m_state_vector3i.Initialize(heap, blackboard_info);
        m_state_color4f.Initialize(heap, blackboard_info);
        m_state_matrix33f.Initialize(heap, blackboard_info);
        m_state_matrix34f.Initialize(heap, blackboard_info);
        m_state_quaternionf.Initialize(heap, blackboard_info);
        m_state_custom_type.Initialize(heap, blackboard_info);
    }

    void Blackboard::Finalize() {
        m_state_string.Finalize();
        m_state_s32.Finalize();
        m_state_float.Finalize();
        m_state_bool.Finalize();
        m_state_vector3f.Finalize();
        m_state_pointer.Finalize();
        m_state_s8.Finalize();
        m_state_s16.Finalize();
        m_state_s64.Finalize();
        m_state_u8.Finalize();
        m_state_u16.Finalize();
        m_state_u32.Finalize();
        m_state_u64.Finalize();
        m_state_double.Finalize();
        m_state_vector3i.Finalize();
        m_state_color4f.Finalize();
        m_state_matrix33f.Finalize();
        m_state_matrix34f.Finalize();
        m_state_quaternionf.Finalize();
        m_state_custom_type.Finalize();
    }

    bool Blackboard::ParseBlackboardApplyInfoByamlDictionary(u32 *out_apply_count, vp::res::ByamlIterator iter_dynamic, u32 base_byaml_index, BlackboardApplyInfo *apply_array, u32 array_count) {

        /* Ensure dictionary */
        if (!iter_dynamic.IsDictionary()) { return false; }

        /* Apply */
        const u32 byaml_count = iter_dynamic.GetDataCount();
        u32 apply_count = 0;
        for (u32 i = 0; i < byaml_count; ++i) {

            /* Find */
            vp::res::ByamlIterator value_iter;
            const u32 base_index = base_byaml_index + i;
            const bool is_iter = iter_dynamic.TryGetIteratorByIndex(std::addressof(value_iter), base_index);
            if (!is_iter) { continue; }
            const char *key;
            const bool is_key = value_iter.TryGetKeyByIterator(std::addressof(key));
            if (!is_key) { continue; }
            BlackboardValueType value_type;
            u32 index;
            const bool is_in_bb = this->TryFindEntryByKey(std::addressof(value_type), std::addressof(index), key);
            if (!is_in_bb) { continue; }
        
            BlackboardApplyInfo *apply_info = std::addressof(apply_array[apply_count]);
            apply_info->key = key;
            switch (value_type) {
                case BlackboardValueType::String:
                {
                    const bool result = value_iter.TryGetStringByIterator(std::addressof(apply_info->value_string));
                    if (!result) { apply_info->value_string = ""; }
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::S32: 
                {
                    s32 value = 0;
                    value_iter.TryGetS32ByIterator(std::addressof(value));
                    apply_info->value_s32 = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::Float: 
                {
                    float value = 0.0f;
                    value_iter.TryGetF32ByIterator(std::addressof(value));
                    apply_info->value_float = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::Bool: 
                {
                    bool value = 0;
                    value_iter.TryGetBoolByIterator(std::addressof(value));
                    apply_info->value_bool = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::Vector3f: 
                {
                    vp::util::Float3 value = {};
                    value_iter.TryGetF32ByIndex(std::addressof(value.x), 0);
                    value_iter.TryGetF32ByIndex(std::addressof(value.y), 1);
                    value_iter.TryGetF32ByIndex(std::addressof(value.z), 2);
                    apply_info->value_vector3f = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::S8: 
                {
                    s32 value = 0;
                    value_iter.TryGetS32ByIterator(std::addressof(value));
                    apply_info->value_s8 = static_cast<s8>(value);
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::S16: 
                {
                    s32 value = 0;
                    value_iter.TryGetS32ByIterator(std::addressof(value));
                    apply_info->value_s16 = static_cast<s16>(value);
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::S64: 
                {
                    s64 value = 0;
                    value_iter.TryGetS64ByIterator(std::addressof(value));
                    apply_info->value_s64 = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::U8: 
                {
                    u32 value = 0;
                    value_iter.TryGetU32ByIterator(std::addressof(value));
                    apply_info->value_u8 = static_cast<u8>(value);
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::U16: 
                {
                    u32 value = 0;
                    value_iter.TryGetU32ByIterator(std::addressof(value));
                    apply_info->value_u16 = static_cast<u16>(value);
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::U32: 
                {
                    u32 value = 0;
                    value_iter.TryGetU32ByIterator(std::addressof(value));
                    apply_info->value_u32 = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::U64: 
                {
                    u64 value = 0;
                    value_iter.TryGetU64ByIterator(std::addressof(value));
                    apply_info->value_u64 = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::Double: 
                {
                    double value = 0;
                    value_iter.TryGetF64ByIterator(std::addressof(value));
                    apply_info->value_double = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::Vector3i: 
                {
                    vp::util::Int3 value = {};
                    value_iter.TryGetS32ByIndex(std::addressof(value.x), 0);
                    value_iter.TryGetS32ByIndex(std::addressof(value.y), 1);
                    value_iter.TryGetS32ByIndex(std::addressof(value.z), 2);
                    apply_info->value_vector3i = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                case BlackboardValueType::Color4f: 
                {
                    vp::util::Color4f value = vp::util::cBlack4f;
                    value_iter.TryGetF32ByIndex(std::addressof(value.r), 0);
                    value_iter.TryGetF32ByIndex(std::addressof(value.g), 1);
                    value_iter.TryGetF32ByIndex(std::addressof(value.b), 2);
                    value_iter.TryGetF32ByIndex(std::addressof(value.a), 3);
                    apply_info->value_color4f = value;
                    ++apply_count;
                    apply_info->value_type = value_type;
                    break;
                }
                default:
                    break;
            };
            if (array_count <= apply_count) { break; }
        }

        *out_apply_count = apply_count;

        return true;
    }
}

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

namespace awn::para::impl {

    using BBValueFlagSet = vp::util::FixedConnection<u8, 4>;

    class BlackboardStateBase {
        public:
            using KeyArray             = vp::util::HeapArray<const char*>;
            using FlagsArray           = vp::util::HeapArray<BBValueFlagSet>;
            using ValueSetCounterArray = vp::util::HeapArray<u8>;
            using ValueIndexMap        = vp::util::KeyIndexMap<u32, u8>;
        public:
            static constexpr u32 cInvalidEntryIndex = ValueIndexMap::cInvalidEntryIndex;
        protected:
            KeyArray             m_key_array;
            FlagsArray           m_flags_array;
            ValueSetCounterArray m_value_set_counter_array;
            ValueIndexMap        m_index_map;
        protected:
            void InitializeBase(mem::Heap *heap, const BlackboardStateInfo *state_info);
            void FinalizeBase();
        public:
            constexpr  BlackboardStateBase() : m_key_array(), m_flags_array(), m_value_set_counter_array(), m_index_map() {/*...*/}
            constexpr ~BlackboardStateBase() {/*...*/}

            constexpr u32 TryGetIndexByKey(const char *key) const {
                for (u32 i = 0; i < m_key_array.GetCount(); ++i) {
                    const char *string = m_key_array[i];
                    if (vp::util::StringCompare(key, string)) { return i; }
                }
                return cInvalidEntryIndex;
            }
            constexpr u32 TryGetIndexByHash(u32 hash) const {
                return m_index_map.TryGetIndexByKey(hash);
            }

            constexpr const char *GetKeyByIndex(u32 index) const {
                VP_ASSERT(index != cInvalidEntryIndex);
                return m_key_array[index];
            }
            constexpr const char *GetKeyByHash(u32 hash) const {
                const u32 index = this->TryGetIndexByHash(hash);
                return this->GetKeyByIndex(index);
            }

            constexpr bool IsInstanceByKey(const char *key) {
                const u32 index = this->TryGetIndexByKey(key);
                return this->IsInstanceByIndex(index);
            }
            constexpr bool IsInstanceByIndex(u32 index) {
                VP_ASSERT(index != cInvalidEntryIndex);
                return m_flags_array[index].GetBit(static_cast<u32>(BlackboardValueFlag::IsInstanceParam));
            }
            constexpr bool IsInstanceByHash(u32 hash) {
                const u32 index = this->TryGetIndexByHash(hash);
                return this->IsInstanceByIndex(index);
            }

            constexpr void SetIsInstanceByKey(const char *key) {
                const u32 index = this->TryGetIndexByKey(key);
                this->SetIsInstanceByIndex(index);
            }
            constexpr void SetIsInstanceByIndex(u32 index) {
                VP_ASSERT(index != cInvalidEntryIndex);
                m_flags_array[index].SetBit(static_cast<u32>(BlackboardValueFlag::IsInstanceParam));
            }
            constexpr void SetIsInstanceByHash(u32 hash) {
                const u32 index = this->TryGetIndexByHash(hash);
                this->SetIsInstanceByIndex(index);
            }

            constexpr void ClearIsInstanceByKey(const char *key) {
                const u32 index = this->TryGetIndexByKey(key);
                this->SetIsInstanceByIndex(index);
            }
            constexpr void ClearIsInstanceByIndex(u32 index) {
                VP_ASSERT(index != cInvalidEntryIndex);
                m_flags_array[index].ClearBit(static_cast<u32>(BlackboardValueFlag::IsInstanceParam));
            }
            constexpr void ClearIsInstanceByHash(u32 hash) {
                const u32 index = this->TryGetIndexByHash(hash);
                this->ClearIsInstanceByIndex(index);
            }
    };

    template <typename T>
        requires (TIsBlackboardCompatible<T>())
    class BlackboardState : public BlackboardStateBase {
        public:
            using ValueArray = vp::util::HeapArray<T>;
        private:
            ValueArray m_value_array;
            ValueArray m_default_value_array;
        private:
            static void GetBBParamAuto(BlackboardParamTable *table, u32 index, const char **out_bb_key, T *out_init_val, BBValueFlagSet *out_flags) requires (TIsBlackboardCompatible<T>()) && (!TIsBlackboardCustomType<T>()) {
                
                const BlackboardParam *param_base;
                if constexpr (std::is_same<T, const char*>::value) {
                    const TypedParamArray<BlackboardParamString> *array = table->GetBlackboardParamStringArray();
                    const BlackboardParamString *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = *param->GetInitValConverted();
                } else if constexpr (std::is_same<T, bool>::value) {
                    const TypedParamArray<BlackboardParamBool> *array = table->GetBlackboardParamBoolArray();
                    const BlackboardParamBool *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = *param->GetInitVal();
                } else if constexpr (std::is_same<T, float>::value) {
                    const TypedParamArray<BlackboardParamF32> *array = table->GetBlackboardParamF32Array();
                    const BlackboardParamF32 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<float>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, double>::value) {
                    const TypedParamArray<BlackboardParamF64> *array = table->GetBlackboardParamF64Array();
                    const BlackboardParamF64 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<double>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, s8>::value) {
                    const TypedParamArray<BlackboardParamS8> *array = table->GetBlackboardParamS8Array();
                    const BlackboardParamS8 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<s8>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, s16>::value) {
                    const TypedParamArray<BlackboardParamS16> *array = table->GetBlackboardParamS16Array();
                    const BlackboardParamS16 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<s16>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, s32>::value) {
                    const TypedParamArray<BlackboardParamS32> *array = table->GetBlackboardParamS32Array();
                    const BlackboardParamS32 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<s32>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, s64>::value) {
                    const TypedParamArray<BlackboardParamS64> *array = table->GetBlackboardParamS64Array();
                    const BlackboardParamS64 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<s64>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, u8>::value) {
                    const TypedParamArray<BlackboardParamU8> *array = table->GetBlackboardParamU8Array();
                    const BlackboardParamU8 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<u8>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, u16>::value) {
                    const TypedParamArray<BlackboardParamU16> *array = table->GetBlackboardParamU16Array();
                    const BlackboardParamU16 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<u16>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, u32>::value) {
                    const TypedParamArray<BlackboardParamU32> *array = table->GetBlackboardParamU32Array();
                    const BlackboardParamU32 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<u32>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, u64>::value) {
                    const TypedParamArray<BlackboardParamU64> *array = table->GetBlackboardParamU64Array();
                    const BlackboardParamU64 *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = static_cast<u64>(*param->GetInitVal());
                } else if constexpr (std::is_same<T, vp::util::Float3>::value) {
                    const TypedParamArray<BlackboardParamVec3f> *array = table->GetBlackboardParamVec3fArray();
                    const BlackboardParamVec3f *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = *param->GetInitVal();
                } else if constexpr (std::is_same<T, vp::util::Int3>::value) {
                    const TypedParamArray<BlackboardParamVec3i> *array = table->GetBlackboardParamVec3iArray();
                    const BlackboardParamVec3i *param = array->GetTypedParamByIndex(index);
                    param_base = param;
                    *out_init_val = *param->GetInitVal();
                } else if constexpr (std::is_same<T, vp::util::FloatRowMajor3x3>::value) {
                    const TypedParamArray<BlackboardParamMtx33f> *array = table->GetBlackboardParamMtx33fArray();
                    param_base = array->GetTypedParamByIndex(index);
                    *out_init_val = vp::util::FloatRowMajor3x3::cIdentity;
                } else if constexpr (std::is_same<T, vp::util::FloatRowMajor4x3>::value) {
                    const TypedParamArray<BlackboardParamMtx34f> *array = table->GetBlackboardParamMtx34fArray();
                    param_base = array->GetTypedParamByIndex(index);
                    *out_init_val = vp::util::FloatRowMajor4x3::cIdentity;
                } else if constexpr (std::is_same<T, vp::util::Quaternionf>::value) {
                    const TypedParamArray<BlackboardParamQuatf> *array = table->GetBlackboardParamQuatfArray();
                    param_base = array->GetTypedParamByIndex(index);
                    *out_init_val = {0.0f, 0.0f, 0.0f, 1.0f };
                } else if constexpr (std::is_same<T, vp::util::Color4f>::value) {
                    const TypedParamArray<BlackboardParamClr4f> *array = table->GetBlackboardParamClr4fArray();
                    param_base = array->GetTypedParamByIndex(index);
                    *out_init_val = vp::util::cBlack4f;
                } else if constexpr (std::is_same<T, void*>::value) {
                    const TypedParamArray<BlackboardParamPtr> *array = table->GetBlackboardParamPtrArray();
                    param_base = array->GetTypedParamByIndex(index);
                    *out_init_val = nullptr;
                }

                /* Common */
                *out_bb_key = *param_base->GetBBKey();
                if (*param_base->GetIsInstanceParam()) {
                    out_flags->SetBit(static_cast<u32>(BlackboardValueFlag::IsInstanceParam));
                }
                if (*param_base->GetIsRecord()) {
                    out_flags->SetBit(static_cast<u32>(BlackboardValueFlag::IsRecord));
                }
                if (*param_base->GetIsSafeParamForExternalTools()) {
                    out_flags->SetBit(static_cast<u32>(BlackboardValueFlag::IsSafeParamForExternalTools));
                }
                if (*param_base->GetIsVisibleForMapUnit()) {
                    out_flags->SetBit(static_cast<u32>(BlackboardValueFlag::IsVisibleForMapUnit));
                }
                
                return;
            }
            static void GetBBParamAuto(BlackboardParamTable *table, u32 index, const char **out_bb_key, T *out_init_val, BBValueFlagSet *out_flags) {
                VP_UNUSED(table, index, out_bb_key, out_init_val, out_flags);
            }

            static void GetBBParamForCustomType(BlackboardParamTable *table, u32 index, const char **out_bb_key, BBValueFlagSet *out_flags) {
                TypedParamArray<BlackboardParamCustomType> *array = table->GetBlackboardParamCustomTypeArray();
                BlackboardParam *param_base = array->GetTypedParamByIndex(index);
                *out_bb_key = *param_base->GetBBKey();
                if (*param_base->GetIsInstanceParam()) {
                    out_flags->SetBit(static_cast<u32>(BlackboardValueFlag::IsInstanceParam));
                }
                if (*param_base->GetIsRecord()) {
                    out_flags->SetBit(static_cast<u32>(BlackboardValueFlag::IsRecord));
                }
                if (*param_base->GetIsSafeParamForExternalTools()) {
                    out_flags->SetBit(static_cast<u32>(BlackboardValueFlag::IsSafeParamForExternalTools));
                }
                if (*param_base->GetIsVisibleForMapUnit()) {
                    out_flags->SetBit(static_cast<u32>(BlackboardValueFlag::IsVisibleForMapUnit));
                }
                return;
            }
        public:
            constexpr  BlackboardState() : BlackboardStateBase(), m_value_array(), m_default_value_array() {/*...*/}
            constexpr ~BlackboardState() {/*...*/}

            void Initialize(mem::Heap *heap, const BlackboardInitInfo *bb_info) {

                /* Initialize arrays */
                const BlackboardStateInfo *state_info = bb_info->TGetBlackboardStateInfo<T>();
                const u32 entry_count = state_info->entry_count;
                this->InitializeBase(heap, state_info);
                m_value_array.Initialize(heap, entry_count);
                m_default_value_array.Initialize(heap, entry_count);

                /* Initialize names, flags, lookup table, and values */
                for (u32 i = 0; i < entry_count; ++i) {
                    Document *doc_param_table = (*bb_info->blackboard_param_table_document_array)[state_info->entry_array[i].table_index];
                    BlackboardParamTable *param_table = doc_param_table->GetTypedParam<BlackboardParamTable>();
                    T init_val = {};
                    const char *key = "";
                    GetBBParamAuto(param_table, i, std::addressof(key), std::addressof(init_val), std::addressof(m_flags_array[i]));
                    m_key_array[i] = key;
                    const u32 hash = vp::util::HashMurmur3(key);
                    m_index_map.AddEntry(hash, i);
                    m_default_value_array[i] = init_val;
                    m_value_array[i] = init_val;
                }

                return;
            }

            void Finalize() {
                this->FinalizeBase();
                m_value_array.Finalize();
                m_default_value_array.Finalize();
            }

            constexpr void Reset() {
                const u32 value_count = m_value_array.GetCount();
                for (u32 i = 0; i < value_count; ++i) {
                    m_value_array[i] = m_default_value_array[i];
                }
                return;
            }

            constexpr T *GetValuePointerByKey(const char *key) {
                const u32 index = this->TryGetIndexByKey(key);
                return this->GetValuePointerByIndex(index);
            }
            constexpr T *GetValuePointerByIndex(u32 index) {
                VP_ASSERT(index != cInvalidEntryIndex);
                return std::addressof(m_value_array[index]);
            }
            constexpr T *GetValuePointerByHash(u32 hash) {
                const u32 index = this->TryGetIndexByHash(hash);
                return this->GetValuePointerByIndex(index);
            }

            constexpr void SetValueByKey(const char *key, const T *value) {
                const u32 index = this->TryGetIndexByKey(key);
                return this->SetValueByIndex(index, value);
            }
            constexpr void SetValueByIndex(u32 index, const T *value) {
                if (m_flags_array[index].GetBit(static_cast<u32>(BlackboardValueFlag::IsInstanceParam))) { return; }
                m_value_array[index] = *value;
                m_value_set_counter_array[index] += 1;
            }
            constexpr void SetValueByHash(u32 hash, const T *value) {
                const u32 index = this->TryGetIndexByHash(hash);
                return this->SetValueByIndex(index, value);
            }

            constexpr void SetValueByKeyInstance(const char *key, const T *value) {
                const u32 index = this->TryGetIndexByKey(key);
                return this->SetValueByIndexInstance(index, value);
            }
            constexpr void SetValueByIndexInstance(u32 index, const T *value) {
                m_value_array[index] = *value;
                m_value_set_counter_array[index] += 1;
            }
            constexpr void SetValueByHashInstance(u32 hash, const T *value) {
                const u32 index = this->TryGetIndexByHash(hash);
                return this->SetValueByIndexInstance(index, value);
            }
    };
}

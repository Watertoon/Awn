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

#define AWN_PARA_DECLARE_BB_ACCESSORS(func_type_name, type_name) \
    constexpr const char *GetKeyByIndex##func_type_name(u32 index) { \
        return this->GetKeyByIndex<type_name>(index); \
    } \
    constexpr const char *GetKeyByHash##func_type_name(u32 hash) { \
        return this->GetKeyByHash<type_name>(hash); \
    } \
\
    constexpr u32 TryGetIndexByKey##func_type_name(const char *key) { \
        return this->TryGetIndexByKey<type_name>(key); \
    } \
    constexpr u32 TryGetIndexByHash##func_type_name(u32 hash) { \
        return this->TryGetIndexByHash<type_name>(hash); \
    } \
\
    constexpr type_name Get##func_type_name##ByKey(const char *key) { \
        return *this->GetValuePointerByKey<type_name>(key); \
    } \
    constexpr type_name Get##func_type_name##ByIndex(u32 index) { \
        return *this->GetValuePointerByIndex<type_name>(index); \
    } \
    constexpr type_name Get##func_type_name##ByHash(u32 hash) { \
        return *this->GetValuePointerByHash<type_name>(hash); \
    } \
\
    constexpr void Set##func_type_name##ByKey(const char *key, type_name value) { \
        this->SetValueByKey<type_name>(key, std::addressof(value)); \
    } \
    constexpr void Set##func_type_name##ByIndex(u32 index, type_name value) { \
        this->SetValueByIndex<type_name>(index, std::addressof(value)); \
    } \
    constexpr void Set##func_type_name##ByHash(u32 hash, type_name value) { \
        this->SetValueByHash<type_name>(hash, std::addressof(value)); \
    } \
\
    constexpr void Set##func_type_name##ByKeyInstance(const char *key, type_name value) { \
        this->SetValueByKeyInstance<type_name>(key, std::addressof(value)); \
    } \
    constexpr void Set##func_type_name##ByIndexInstance(u32 index, type_name value) { \
        this->SetValueByIndexInstance<type_name>(index, std::addressof(value)); \
    } \
    constexpr void Set##func_type_name##ByHashInstance(u32 hash, type_name value) { \
        this->SetValueByHashInstance<type_name>(hash, std::addressof(value)); \
    }

namespace awn::para {

    class Blackboard {
        private:
            template <typename T>
                requires (TIsBlackboardCompatible<T>()) && (!TIsBlackboardCustomType<T>())
            constexpr impl::BlackboardState<T> *TGetBlackboardState() {

                /* Return proper state */
                if constexpr (std::is_same<T, const char*>::value) {
                    return std::addressof(m_state_string);
                } else if constexpr (std::is_same<T, bool>::value) {
                    return std::addressof(m_state_bool);
                } else if constexpr (std::is_same<T, float>::value) {
                    return std::addressof(m_state_float);
                } else if constexpr (std::is_same<T, double>::value) {
                    return std::addressof(m_state_double);
                } else if constexpr (std::is_same<T, s8>::value) {
                    return std::addressof(m_state_s8);
                } else if constexpr (std::is_same<T, s16>::value) {
                    return std::addressof(m_state_s16);
                } else if constexpr (std::is_same<T, s32>::value) {
                    return std::addressof(m_state_s32);
                } else if constexpr (std::is_same<T, s64>::value) {
                    return std::addressof(m_state_s64);
                } else if constexpr (std::is_same<T, u8>::value) {
                    return std::addressof(m_state_u8);
                } else if constexpr (std::is_same<T, u16>::value) {
                    return std::addressof(m_state_u16);
                } else if constexpr (std::is_same<T, u32>::value) {
                    return std::addressof(m_state_u32);
                } else if constexpr (std::is_same<T, u64>::value) {
                    return std::addressof(m_state_u64);
                } else if constexpr (std::is_same<T, vp::util::Float3>::value) {
                    return std::addressof(m_state_vector3f);
                } else if constexpr (std::is_same<T, vp::util::Int3>::value) {
                    return std::addressof(m_state_vector3i);
                } else if constexpr (std::is_same<T, vp::util::FloatColumnMajor3x3>::value) {
                    return std::addressof(m_state_matrix33f);
                } else if constexpr (std::is_same<T, vp::util::FloatColumnMajor4x3>::value) {
                    return std::addressof(m_state_matrix34f);
                } else if constexpr (std::is_same<T, vp::util::Float4>::value) {
                    return std::addressof(m_state_quaternionf);
                } else if constexpr (std::is_same<T, vp::util::Color4f>::value) {
                    return std::addressof(m_state_color4f);
                } else if constexpr (std::is_same<T, void*>::value) {
                    return std::addressof(m_state_pointer);
                }
                return nullptr;
            }
            template <typename T>
                requires (TIsBlackboardCompatible<T>()) && (TIsBlackboardCustomType<T>())
            constexpr impl::BlackboardState<T> *TGetBlackboardState() {
                return std::addressof(m_state_custom_type);
            }
        private:
            impl::BlackboardState<const char*>                   m_state_string;
            impl::BlackboardState<s32>                           m_state_s32;
            impl::BlackboardState<float>                         m_state_float;
            impl::BlackboardState<bool>                          m_state_bool;
            impl::BlackboardState<vp::util::Float3>              m_state_vector3f;
            impl::BlackboardState<void*>                         m_state_pointer;
            impl::BlackboardState<s8>                            m_state_s8;
            impl::BlackboardState<s16>                           m_state_s16;
            impl::BlackboardState<s64>                           m_state_s64;
            impl::BlackboardState<u8>                            m_state_u8;
            impl::BlackboardState<u16>                           m_state_u16;
            impl::BlackboardState<u32>                           m_state_u32;
            impl::BlackboardState<u64>                           m_state_u64;
            impl::BlackboardState<double>                        m_state_double;
            impl::BlackboardState<vp::util::Int3>                m_state_vector3i;
            impl::BlackboardState<vp::util::Color4f>             m_state_color4f;
            impl::BlackboardState<vp::util::FloatColumnMajor3x3> m_state_matrix33f;
            impl::BlackboardState<vp::util::FloatColumnMajor4x3> m_state_matrix34f;
            impl::BlackboardState<vp::util::Float4>              m_state_quaternionf;
            impl::BlackboardState<BlackboardCustomTypeBase*>     m_state_custom_type;
        public:
            constexpr  Blackboard() : m_state_string(), m_state_s32(), m_state_float(), m_state_bool(), m_state_vector3f(), m_state_pointer(), m_state_s8(), m_state_s16(), m_state_s64(), m_state_u8(), m_state_u16(), m_state_u32(), m_state_u64(), m_state_double(), m_state_vector3i(), m_state_color4f(), m_state_matrix33f(), m_state_matrix34f(), m_state_quaternionf(), m_state_custom_type() {/*...*/}
            constexpr ~Blackboard() {/*...*/}

            void Initialize(mem::Heap *heap, const BlackboardInitInfo *blackboard_info);
            void Finalize();

            bool ParseBlackboardApplyInfoByamlDictionary(u32 *out_apply_count, vp::res::ByamlIterator iter_dynamic, u32 base_byaml_index, BlackboardApplyInfo *apply_array, u32 array_count);

            constexpr void ApplyValues(const BlackboardApplyInfo *apply_array, u32 array_count) {
                for (u32 i = 0; i < array_count; ++i) {
                    const BlackboardApplyInfo *apply_info = std::addressof(apply_array[i]);
                    const char *key = apply_info->key.GetString();
                    switch(apply_info->value_type) {
                        case BlackboardValueType::String:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_string));
                            break;
                        }
                        case BlackboardValueType::S32:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_s32));
                            break;
                        }
                        case BlackboardValueType::Float:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_float));
                            break;
                        }
                        case BlackboardValueType::Bool:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_bool));
                            break;
                        }
                        case BlackboardValueType::Vector3f:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_vector3f));
                            break;
                        }
                        case BlackboardValueType::Pointer:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_pointer));
                            break;
                        }
                        case BlackboardValueType::S8:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_s8));
                            break;
                        }
                        case BlackboardValueType::S16:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_s16));
                            break;
                        }
                        case BlackboardValueType::S64:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_s64));
                            break;
                        }
                        case BlackboardValueType::U8:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_u8));
                            break;
                        }
                        case BlackboardValueType::U16:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_u16));
                            break;
                        }
                        case BlackboardValueType::U32:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_u32));
                            break;
                        }
                        case BlackboardValueType::U64:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_u64));
                            break;
                        }
                        case BlackboardValueType::Double:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_double));
                            break;
                        }
                        case BlackboardValueType::Vector3i:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_vector3i));
                            break;
                        }
                        case BlackboardValueType::Color4f:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_color4f));
                            break;
                        }
                        case BlackboardValueType::Matrix33f:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_matrix33f));
                            break;
                        }
                        case BlackboardValueType::Matrix34f:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_matrix34f));
                            break;
                        }
                        case BlackboardValueType::Quaternionf:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_quaternionf));
                            break;
                        }
                        case BlackboardValueType::CustomType:
                        {
                            this->SetValueByKey(key, std::addressof(apply_info->value_custom_type));
                            break;
                        }
                        default:
                            break;
                    };
                }
            }

            constexpr void ApplyValuesInstance(const BlackboardApplyInfo *apply_array, u32 array_count) {
                for (u32 i = 0; i < array_count; ++i) {
                    const BlackboardApplyInfo *apply_info = std::addressof(apply_array[i]);
                    const char *key = apply_info->key.GetString();
                    switch(apply_info->value_type) {
                        case BlackboardValueType::String:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_string));
                            break;
                        }
                        case BlackboardValueType::S32:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_s32));
                            break;
                        }
                        case BlackboardValueType::Float:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_float));
                            break;
                        }
                        case BlackboardValueType::Bool:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_bool));
                            break;
                        }
                        case BlackboardValueType::Vector3f:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_vector3f));
                            break;
                        }
                        case BlackboardValueType::Pointer:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_pointer));
                            break;
                        }
                        case BlackboardValueType::S8:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_s8));
                            break;
                        }
                        case BlackboardValueType::S16:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_s16));
                            break;
                        }
                        case BlackboardValueType::S64:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_s64));
                            break;
                        }
                        case BlackboardValueType::U8:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_u8));
                            break;
                        }
                        case BlackboardValueType::U16:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_u16));
                            break;
                        }
                        case BlackboardValueType::U32:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_u32));
                            break;
                        }
                        case BlackboardValueType::U64:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_u64));
                            break;
                        }
                        case BlackboardValueType::Double:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_double));
                            break;
                        }
                        case BlackboardValueType::Vector3i:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_vector3i));
                            break;
                        }
                        case BlackboardValueType::Color4f:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_color4f));
                            break;
                        }
                        case BlackboardValueType::Matrix33f:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_matrix33f));
                            break;
                        }
                        case BlackboardValueType::Matrix34f:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_matrix34f));
                            break;
                        }
                        case BlackboardValueType::Quaternionf:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_quaternionf));
                            break;
                        }
                        case BlackboardValueType::CustomType:
                        {
                            this->SetValueByKeyInstance(key, std::addressof(apply_info->value_custom_type));
                            break;
                        }
                        default:
                            break;
                    };
                }
            }
        protected:
            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr const char *GetKeyByIndex(u32 index) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                return state->GetKeyByIndex(index);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr const char *GetKeyByHash(u32 hash) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                return state->GetKeyByHash(hash);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr u32 TryGetIndexByKey(const char *key) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                return state->TryGetIndexByKey(key);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr u32 TryGetIndexByHash(u32 hash) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                return state->TryGetIndexByHash(hash);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr T *GetValuePointerByKey(const char *key) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                return state->GetValuePointerByKey(key);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr T *GetValuePointerByIndex(u32 index) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                return state->GetValuePointerByIndex(index);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr T *GetValuePointerByHash(u32 hash) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                return state->GetValuePointerByHash(hash);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr void SetValueByKey(const char *key, const T *value) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                state->SetValueByKey(key, value);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr void SetValueByIndex(u32 index, const T *value) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                state->SetValueByIndex(index, value);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr void SetValueByHash(u32 hash, const T *value) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                state->SetValueByHash(hash, value);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr void SetValueByKeyInstance(const char *key, const T *value) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                state->SetValueByKeyInstance(key, value);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr void SetValueByIndexInstance(u32 index, const T *value) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                state->SetValueByIndexInstance(index, value);
            }

            template <typename T>
                requires (TIsBlackboardCompatible<T>())
            constexpr void SetValueByHashInstance(u32 hash, const T *value) {
                impl::BlackboardState<T> *state = TGetBlackboardState<T>();
                state->SetValueByHashInstance(hash, value);
            }
        public:
            constexpr bool TryFindEntryByKey(BlackboardValueType *out_value_type, u32 *out_index, const char *key) {
                u32 index;
                if (index = m_state_string.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)      { *out_index = index; *out_value_type = BlackboardValueType::String; return true; }
                if (index = m_state_s32.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)         { *out_index = index; *out_value_type = BlackboardValueType::S32; return true; }
                if (index = m_state_float.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)       { *out_index = index; *out_value_type = BlackboardValueType::Float; return true; }
                if (index = m_state_bool.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)        { *out_index = index; *out_value_type = BlackboardValueType::Bool; return true; }
                if (index = m_state_vector3f.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)    { *out_index = index; *out_value_type = BlackboardValueType::Vector3f; return true; }
                if (index = m_state_pointer.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)     { *out_index = index; *out_value_type = BlackboardValueType::Pointer; return true; }
                if (index = m_state_s8.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)          { *out_index = index; *out_value_type = BlackboardValueType::S8; return true; }
                if (index = m_state_s16.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)         { *out_index = index; *out_value_type = BlackboardValueType::S16; return true; }
                if (index = m_state_s64.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)         { *out_index = index; *out_value_type = BlackboardValueType::S64; return true; }
                if (index = m_state_u8.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)          { *out_index = index; *out_value_type = BlackboardValueType::U8; return true; }
                if (index = m_state_u16.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)         { *out_index = index; *out_value_type = BlackboardValueType::U16; return true; }
                if (index = m_state_u32.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)         { *out_index = index; *out_value_type = BlackboardValueType::U32; return true; }
                if (index = m_state_u64.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)         { *out_index = index; *out_value_type = BlackboardValueType::U64; return true; }
                if (index = m_state_double.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)      { *out_index = index; *out_value_type = BlackboardValueType::Double; return true; }
                if (index = m_state_vector3i.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)    { *out_index = index; *out_value_type = BlackboardValueType::Vector3i; return true; }
                if (index = m_state_color4f.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)     { *out_index = index; *out_value_type = BlackboardValueType::Color4f; return true; }
                if (index = m_state_matrix33f.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)   { *out_index = index; *out_value_type = BlackboardValueType::Matrix33f; return true; }
                if (index = m_state_matrix34f.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex)   { *out_index = index; *out_value_type = BlackboardValueType::Matrix34f; return true; }
                if (index = m_state_quaternionf.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex) { *out_index = index; *out_value_type = BlackboardValueType::Quaternionf; return true; }
                if (index = m_state_custom_type.TryGetIndexByKey(key), index != impl::BlackboardStateBase::cInvalidEntryIndex) { *out_index = index; *out_value_type = BlackboardValueType::CustomType; return true; }
                return false;
            }

            constexpr void Reset() {
                m_state_string.Reset();
                m_state_s32.Reset();
                m_state_float.Reset();
                m_state_bool.Reset();
                m_state_vector3f.Reset();
                m_state_pointer.Reset();
                m_state_s8.Reset();
                m_state_s16.Reset();
                m_state_s64.Reset();
                m_state_u8.Reset();
                m_state_u16.Reset();
                m_state_u32.Reset();
                m_state_u64.Reset();
                m_state_double.Reset();
                m_state_vector3i.Reset();
                m_state_color4f.Reset();
                m_state_matrix33f.Reset();
                m_state_matrix34f.Reset();
                m_state_quaternionf.Reset();
                m_state_custom_type.Reset();
            }

            AWN_PARA_DECLARE_BB_ACCESSORS(String, const char *);
            AWN_PARA_DECLARE_BB_ACCESSORS(S32, s32);
            AWN_PARA_DECLARE_BB_ACCESSORS(Bool, bool);
            AWN_PARA_DECLARE_BB_ACCESSORS(Float, float);
            AWN_PARA_DECLARE_BB_ACCESSORS(Vector3f, vp::util::Float3);
            AWN_PARA_DECLARE_BB_ACCESSORS(Pointer, void*);
            AWN_PARA_DECLARE_BB_ACCESSORS(S8, s8);
            AWN_PARA_DECLARE_BB_ACCESSORS(S16, s16);
            AWN_PARA_DECLARE_BB_ACCESSORS(S64, s64);
            AWN_PARA_DECLARE_BB_ACCESSORS(U8, u8);
            AWN_PARA_DECLARE_BB_ACCESSORS(U16, u16);
            AWN_PARA_DECLARE_BB_ACCESSORS(U32, u32);
            AWN_PARA_DECLARE_BB_ACCESSORS(U64, u64);
            AWN_PARA_DECLARE_BB_ACCESSORS(Double, double);
            AWN_PARA_DECLARE_BB_ACCESSORS(Vector3i, vp::util::Int3);
            AWN_PARA_DECLARE_BB_ACCESSORS(Color4f, vp::util::Color4f);
            AWN_PARA_DECLARE_BB_ACCESSORS(Matrix33f, vp::util::FloatColumnMajor3x3);
            AWN_PARA_DECLARE_BB_ACCESSORS(Matrix44f, vp::util::FloatColumnMajor4x3);
            AWN_PARA_DECLARE_BB_ACCESSORS(Quaternionf, vp::util::Float4);
    };
}

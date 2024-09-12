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

namespace awn::para {
	
    struct BlackboardCustomTypeBase {
        VP_STATIC_RTTI_BASE(BlackboardCustomTypeBase);
    };

    template <typename T>
    constexpr bool TIsBlackboardCustomType() {
        return false;
    }
    template <typename T>
        requires (std::is_pointer_v<T> && !std::is_same_v<T, const char*> && BlackboardCustomTypeBase::CheckRuntimeTypeInfoStatic(std::remove_pointer_t<T>::GetRuntimeTypeInfoStatic()))
    constexpr bool TIsBlackboardCustomType() {
        return true;
    }

    template <typename T>
    constexpr bool TIsBlackboardCompatible() {
        return false;
    }
    template <typename T>
        requires (std::is_same<T, const char*>::value) || (std::is_same<T, bool>::value) || (std::is_same<T, float>::value) || (std::is_same<T, double>::value)
        || (std::is_same<T, s8>::value) || (std::is_same<T, s16>::value) || (std::is_same<T, s32>::value) || (std::is_same<T, s64>::value)
        || (std::is_same<T, u8>::value) || (std::is_same<T, u16>::value) || (std::is_same<T, u32>::value) || (std::is_same<T, u64>::value)
        || (std::is_same<T, vp::util::Float3>::value) || (std::is_same<T, vp::util::Int3>::value) || (std::is_same<T, vp::util::FloatRowMajor3x3>::value) || (std::is_same<T, vp::util::FloatRowMajor4x3>::value)
        || (std::is_same<T, vp::util::Quaternionf>::value) || (std::is_same<T, vp::util::Color4f>::value) || (std::is_same<T, void*>::value)
    constexpr bool TIsBlackboardCompatible() {
        return true;
    }
    template <typename T>
        requires (std::is_pointer_v<T> && !std::is_same_v<T, const char*> && BlackboardCustomTypeBase::CheckRuntimeTypeInfoStatic(std::remove_pointer_t<T>::GetRuntimeTypeInfoStatic()))
    constexpr bool TIsBlackboardCompatible() {
        return true;
    }


    static constexpr size_t cMaxBlackboardEntriesPerTypeCount = 0x100;

    struct BlackboardEntryInfo {
        u32 table_index;
        u32 entry_index;
    };

    struct BlackboardStateInfo {
        BlackboardEntryInfo entry_array[cMaxBlackboardEntriesPerTypeCount];
        u32 entry_count;
    };

    using BlackboardParamTableArray = vp::util::HeapArray<Document*>;

    struct BlackboardInitInfo {
        Document *blackboard_info_document;
        BlackboardParamTableArray *blackboard_param_table_document_array;
        BlackboardStateInfo info_string;
        BlackboardStateInfo info_s32;
        BlackboardStateInfo info_float;
        BlackboardStateInfo info_bool;
        BlackboardStateInfo info_vector3f;
        BlackboardStateInfo info_pointer;
        BlackboardStateInfo info_s8;
        BlackboardStateInfo info_s16;
        BlackboardStateInfo info_s64;
        BlackboardStateInfo info_u8;
        BlackboardStateInfo info_u16;
        BlackboardStateInfo info_u32;
        BlackboardStateInfo info_u64;
        BlackboardStateInfo info_double;
        BlackboardStateInfo info_vector3i;
        BlackboardStateInfo info_color4f;
        BlackboardStateInfo info_matrix33f;
        BlackboardStateInfo info_matrix34f;
        BlackboardStateInfo info_quaternionf;
        BlackboardStateInfo info_custom_type;

        template <typename T>
            requires (TIsBlackboardCompatible<T>()) && (!TIsBlackboardCustomType<T>())
        constexpr const BlackboardStateInfo *TGetBlackboardStateInfo() const {

            /* Return proper state */
            if constexpr (std::is_same<T, const char*>::value) {
                return std::addressof(this->info_string);
            } else if constexpr (std::is_same<T, bool>::value) {
                return std::addressof(this->info_bool);
            } else if constexpr (std::is_same<T, float>::value) {
                return std::addressof(this->info_float);
            } else if constexpr (std::is_same<T, double>::value) {
                return std::addressof(this->info_double);
            } else if constexpr (std::is_same<T, s8>::value) {
                return std::addressof(this->info_s8);
            } else if constexpr (std::is_same<T, s16>::value) {
                return std::addressof(this->info_s16);
            } else if constexpr (std::is_same<T, s32>::value) {
                return std::addressof(this->info_s32);
            } else if constexpr (std::is_same<T, s64>::value) {
                return std::addressof(this->info_s64);
            } else if constexpr (std::is_same<T, u8>::value) {
                return std::addressof(this->info_u8);
            } else if constexpr (std::is_same<T, u16>::value) {
                return std::addressof(this->info_u16);
            } else if constexpr (std::is_same<T, u32>::value) {
                return std::addressof(this->info_u32);
            } else if constexpr (std::is_same<T, u64>::value) {
                return std::addressof(this->info_u64);
            } else if constexpr (std::is_same<T, vp::util::Float3>::value) {
                return std::addressof(this->info_vector3f);
            } else if constexpr (std::is_same<T, vp::util::Int3>::value) {
                return std::addressof(this->info_vector3i);
            } else if constexpr (std::is_same<T, vp::util::FloatRowMajor3x3>::value) {
                return std::addressof(this->info_matrix33f);
            } else if constexpr (std::is_same<T, vp::util::FloatRowMajor4x3>::value) {
                return std::addressof(this->info_matrix34f);
            } else if constexpr (std::is_same<T, vp::util::Quaternionf>::value) {
                return std::addressof(this->info_quaternionf);
            } else if constexpr (std::is_same<T, vp::util::Color4f>::value) {
                return std::addressof(this->info_color4f);
            } else if constexpr (std::is_same<T, void*>::value) {
                return std::addressof(this->info_pointer);
            }
            return nullptr;
        }
        template <typename T>
            requires (TIsBlackboardCompatible<T>()) && (TIsBlackboardCustomType<T>())
        constexpr const BlackboardStateInfo *TGetBlackboardStateInfo() const {
            return std::addressof(this->info_custom_type);
        }
    };

    enum class BlackboardValueFlag : u32 {
        IsInstanceParam = 0,
        IsRecord = 1,
        IsSafeParamForExternalTools = 2,
        IsVisibleForMapUnit = 3,
    };

    enum class BlackboardValueType : u32 {
        Invalid = 0x0,
        String = 0x1,
        S32 = 0x2,
        Float = 0x3,
        Bool = 0x4,
        Vector3f = 0x5,
        Pointer = 0x6,
        S8 = 0x7,
        S16 = 0x8,
        S64 = 0x9,
        U8 = 0xa,
        U16 = 0xb,
        U32 = 0xc,
        U64 = 0xd,
        Double = 0xe,
        Vector3i = 0xf,
        Color4f = 0x10,
        Matrix33f = 0x11,
        Matrix34f = 0x12,
        Quaternionf = 0x13,
        CustomType = 0x14,
    };

    struct BlackboardApplyInfo {
        BlackboardValueType value_type;
        vp::util::FixedString<0x80> key;
        union {
            const char *value_string;
            s32 value_s32;
            float value_float;
            bool value_bool;
            vp::util::Float3 value_vector3f;
            void *value_pointer;
            s8 value_s8;
            s16 value_s16;
            s64 value_s64;
            u8 value_u8;
            u16 value_u16;
            u32 value_u32;
            u64 value_u64;
            double value_double;
            vp::util::Int3 value_vector3i;
            vp::util::Color4f value_color4f;
            vp::util::FloatRowMajor3x3 value_matrix33f;
            vp::util::FloatRowMajor4x3 value_matrix34f;
            vp::util::Quaternionf value_quaternionf;
            BlackboardCustomTypeBase *value_custom_type;
        };
    };

}
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

    class RuntimeTypeInfo {
        public:
            const RuntimeTypeInfo *m_next;
        public:
            constexpr ALWAYS_INLINE RuntimeTypeInfo() : m_next(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE RuntimeTypeInfo(const RuntimeTypeInfo *base_class) : m_next(base_class) {/*...*/}

            constexpr ALWAYS_INLINE bool IsSameTypeInfo(const RuntimeTypeInfo *other_info) const { return this == other_info; }
    };

    #define VP_RTTI_BASE(class_name) \
        protected: \
            using RootType = class_name; \
            static constexpr inline const vp::util::RuntimeTypeInfo sRootRTTI = {}; \
        public: \
            static constexpr ALWAYS_INLINE const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfoStatic() { \
                return std::addressof(sRootRTTI); \
            } \
            virtual constexpr ALWAYS_INLINE const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfo() const { \
                return GetRuntimeTypeInfoStatic(); \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(RootType *other_obj) { \
                const vp::util::RuntimeTypeInfo *class_info = other_obj->GetRuntimeTypeInfo(); \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(vp::util::RuntimeTypeInfo *other_obj) { \
                const vp::util::RuntimeTypeInfo *class_info = other_obj; \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            } \
            static constexpr ALWAYS_INLINE bool IsSameTypeInfo(RootType *other_obj) { \
                return GetRuntimeTypeInfoStatic() == other_obj->GetRuntimeTypeInfo(); \
            }

    #define VP_RTTI_DERIVED(derived_class, base_class) \
        protected: \
            static constexpr inline const vp::util::RuntimeTypeInfo sDerivedRTTI = base_class::GetRuntimeTypeInfoStatic(); \
        public: \
            static constexpr ALWAYS_INLINE const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfoStatic() { \
                return std::addressof(sDerivedRTTI); \
            } \
            virtual constexpr ALWAYS_INLINE const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfo() const override { \
                return GetRuntimeTypeInfoStatic(); \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(RootType *other_obj) { \
                const vp::util::RuntimeTypeInfo *class_info = other_obj->GetRuntimeTypeInfo(); \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(vp::util::RuntimeTypeInfo *other_obj) { \
                const vp::util::RuntimeTypeInfo *class_info = other_obj; \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            } \
            static constexpr ALWAYS_INLINE bool IsSameTypeInfo(RootType *other_obj) { \
                return GetRuntimeTypeInfoStatic() == other_obj->GetRuntimeTypeInfo(); \
            }
}

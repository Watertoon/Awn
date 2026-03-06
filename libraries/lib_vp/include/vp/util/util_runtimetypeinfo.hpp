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
            constexpr RuntimeTypeInfo() : m_next(nullptr) {/*...*/}
            constexpr RuntimeTypeInfo(const RuntimeTypeInfo *base_class) : m_next(base_class) {/*...*/}

            constexpr bool IsSameTypeInfo(const RuntimeTypeInfo *other_info) const { return this == other_info; }
    };

    constexpr bool CheckRuntimeTypeInfo(const vp::util::RuntimeTypeInfo *target_obj, const vp::util::RuntimeTypeInfo *other_obj) {
        const vp::util::RuntimeTypeInfo *class_info = other_obj;
        const vp::util::RuntimeTypeInfo *target     = target_obj;
        do {
            if (target->IsSameTypeInfo(class_info)) { return true; }
            class_info = class_info->m_next;
        } while (class_info != nullptr);
        return false;
    }

    template <typename Ret, typename T>
        requires (std::is_pointer_v<Ret> && T::CheckRuntimeTypeInfoStatic(std::remove_pointer_t<Ret>::GetRuntimeTypeInfoStatic()))
    Ret DynamicCast(T *base_obj) {
        if (base_obj == nullptr) { return nullptr; }
        const RuntimeTypeInfo *obj_rtti = base_obj->GetRuntimeTypeInfo();
        Ret ptr = (std::remove_pointer_t<Ret>::CheckRuntimeTypeInfoStatic(obj_rtti)) ? reinterpret_cast<Ret>(base_obj) : nullptr;
        return ptr;
    }

    #define VP_RTTI_BASE(class_name) \
        protected: \
            using RootType = class_name; \
            static constexpr inline const vp::util::RuntimeTypeInfo sRootRTTI = {}; \
        public: \
            static constexpr const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfoStatic() { \
                return std::addressof(sRootRTTI); \
            } \
            virtual constexpr const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfo() const { \
                return GetRuntimeTypeInfoStatic(); \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(const RootType *other_obj) { \
                if (other_obj == nullptr) { return false; } \
                const vp::util::RuntimeTypeInfo *class_info = other_obj->GetRuntimeTypeInfo(); \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(const vp::util::RuntimeTypeInfo *other_obj) { \
                const vp::util::RuntimeTypeInfo *class_info = other_obj; \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            } \
            static constexpr bool IsSameTypeInfo(RootType *other_obj) { \
                return GetRuntimeTypeInfoStatic() == other_obj->GetRuntimeTypeInfo(); \
            }

    #define VP_RTTI_DERIVED(derived_class, base_class) \
        protected: \
            static constexpr inline const vp::util::RuntimeTypeInfo sDerivedRTTI = base_class::GetRuntimeTypeInfoStatic(); \
        public: \
            static constexpr const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfoStatic() { \
                return std::addressof(sDerivedRTTI); \
            } \
            virtual constexpr const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfo() const override { \
                return GetRuntimeTypeInfoStatic(); \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(const RootType *other_obj) { \
                if (other_obj == nullptr) { return false; } \
                const vp::util::RuntimeTypeInfo *class_info = other_obj->GetRuntimeTypeInfo(); \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(const vp::util::RuntimeTypeInfo *other_obj) { \
                const vp::util::RuntimeTypeInfo *class_info = other_obj; \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            } \
            static constexpr bool IsSameTypeInfo(RootType *other_obj) { \
                return GetRuntimeTypeInfoStatic() == other_obj->GetRuntimeTypeInfo(); \
            }

    #define VP_STATIC_RTTI_BASE(class_name) \
        protected: \
            using RootType = class_name; \
            static constexpr inline const vp::util::RuntimeTypeInfo sRootRTTI = {}; \
        public: \
            static constexpr const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfoStatic() { \
                return std::addressof(sRootRTTI); \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(const vp::util::RuntimeTypeInfo *other_obj) { \
                const vp::util::RuntimeTypeInfo *class_info = other_obj; \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            }

    #define VP_STATIC_RTTI_DERIVED(derived_class, base_class) \
        protected: \
            static constexpr inline const vp::util::RuntimeTypeInfo sDerivedRTTI = base_class::GetRuntimeTypeInfoStatic(); \
        public: \
            static constexpr const vp::util::RuntimeTypeInfo *GetRuntimeTypeInfoStatic() { \
                return std::addressof(sDerivedRTTI); \
            } \
            static constexpr bool CheckRuntimeTypeInfoStatic(const vp::util::RuntimeTypeInfo *other_obj) { \
                const vp::util::RuntimeTypeInfo *class_info = other_obj; \
                const vp::util::RuntimeTypeInfo *target     = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (target->IsSameTypeInfo(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != nullptr); \
                return false; \
            }
}

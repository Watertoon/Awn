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

/* For implementation files */
#define AWN_SINGLETON_TRAITS_IMPL(class_name) \
    constinit class_name *class_name::sInstance = nullptr; \

/* For class definitions */
#define AWN_SINGLETON_TRAITS(class_name) \
    private: \
        static class_name *sInstance; \
    public: \
        static ALWAYS_INLINE class_name *CreateInstance(awn::mem::Heap *allocate_heap) { \
            if (sInstance != nullptr) { \
                return sInstance; \
            } \
            sInstance = reinterpret_cast<class_name*>(::operator new(sizeof(class_name), allocate_heap, alignof(class_name))); \
            std::construct_at(sInstance); \
            return sInstance; \
        } \
        static ALWAYS_INLINE void DeleteInstance() { \
            if (sInstance != nullptr) { \
                std::destroy_at(sInstance); \
                ::operator delete(sInstance); \
            } \
            sInstance = nullptr; \
        } \
        constexpr static ALWAYS_INLINE class_name *GetInstance() { return (std::is_constant_evaluated() == true) ? nullptr : sInstance; }

/* For implementation files */
#define AWN_SINGLETON_TRAITS_DISPOSER_IMPL(class_name) \
    constinit class_name *class_name::sInstance = nullptr; \
    constinit class_name::SingletonDisposer *class_name::sDisposer = nullptr

/* For class definitions */
#define AWN_SINGLETON_TRAITS_DISPOSER(class_name) \
    public: \
        class SingletonDisposer; \
    private: \
        static class_name *sInstance; \
        static SingletonDisposer *sDisposer; \
    public: \
        class SingletonDisposer : public awn::mem::IDisposer { \
            public: \
                SingletonDisposer(awn::mem::Heap *heap) : IDisposer(heap) { \
                    sDisposer = this; \
                } \
                ~SingletonDisposer() { \
                    if (sDisposer == this) { \
                        sDisposer = nullptr; \
                        std::destroy_at(sInstance); \
                        sInstance = nullptr; \
                    } \
                } \
        }; \
    private: \
        vp::util::TypeStorage<SingletonDisposer> m_singleton_disposer; \
    public: \
        static ALWAYS_INLINE class_name *CreateInstance(awn::mem::Heap *allocate_heap) { \
            if (sInstance != nullptr) { \
                return sInstance; \
            } \
            sInstance = reinterpret_cast<class_name*>(::operator new(sizeof(class_name), allocate_heap, alignof(class_name))); \
            std::construct_at(sInstance); \
            vp::util::ConstructAt(sInstance->m_singleton_disposer, allocate_heap); \
            return sInstance; \
        } \
        static ALWAYS_INLINE void DeleteInstance() { \
            SingletonDisposer *disposer = sDisposer; \
            if (sDisposer == nullptr) { \
                return; \
            } \
            sDisposer = nullptr; \
            std::destroy_at(disposer); \
        } \
        constexpr static ALWAYS_INLINE class_name *GetInstance() { return (std::is_constant_evaluated() == true) ? nullptr : sInstance; }

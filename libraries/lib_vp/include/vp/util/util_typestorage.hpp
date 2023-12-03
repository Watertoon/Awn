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

/* While the initial idea of some elements were inspired by Atmosphère-NX (https://github.com/Atmosphere-NX/Atmosphere), for this implementation I only used cppreference and vague mental notes from months ago */
/* If you can make a case for infringement please let me know and I'll correct it */

namespace vp::util {

    template<typename T, size_t S = sizeof(T), size_t A = alignof(T)>
    struct TypeStorage {
        using UnderlyingType = T;
        std::aligned_storage<S, A>::type _storage;
    };

    template<typename T>
    ALWAYS_INLINE T::UnderlyingType &GetReference(T& storage) {
        return *reinterpret_cast<T::UnderlyingType*>(std::addressof(storage._storage));
    }

    template<typename T>
    ALWAYS_INLINE const T::UnderlyingType &GetReference(const T& storage) {
        return *reinterpret_cast<const T::UnderlyingType*>(std::addressof(storage._storage));
    }

    template<typename T>
    ALWAYS_INLINE T::UnderlyingType *GetPointer(T& storage) {
        return reinterpret_cast<T::UnderlyingType*>(std::addressof(storage._storage));
    }

    template<typename T>
    ALWAYS_INLINE const T::UnderlyingType *GetPointer(const T& storage) {
        return reinterpret_cast<const T::UnderlyingType*>(std::addressof(storage._storage));
    }

    template<typename T, class... Args>
    ALWAYS_INLINE void ConstructAt(T &storage, Args... args) {
        std::construct_at(GetPointer(storage), std::forward<Args>(args)...);
    }

    template<typename T>
    ALWAYS_INLINE void ConstructAt(T &storage) {
        std::construct_at(GetPointer(storage));
    }

    template<typename T, class... Args>
    ALWAYS_INLINE void DestructAt(T &storage, Args... args) {
        std::destroy_at(GetPointer(storage), std::forward<Args>(args)...);
    }

    template<typename T>
    ALWAYS_INLINE void DestructAt(T &storage) {
        std::destroy_at(GetPointer(storage));
    }
}

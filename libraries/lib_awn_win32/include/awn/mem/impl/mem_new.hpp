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

namespace awn::mem::impl {

    constexpr inline size_t cDefaultNewAlignment = 8;
    constexpr inline bool   cForceUseHeapAllocator  = false;

    ALWAYS_INLINE void *NewImpl(size_t size, u32 alignment) {

        /* Attempt to fallback to malloc if the heap manager is not initialized */
        if constexpr (cForceUseHeapAllocator == false) {
            if (awn::mem::IsHeapManagerInitialized() == false) {
                return ::_aligned_malloc(size, alignment);
            }
        }

        /* Try to allocate from current heap if able */
        vp::imem::IHeap *current_heap = awn::mem::GetCurrentThreadHeap();
        VP_ASSERT(current_heap != nullptr);

        return current_heap->TryAllocate(size, alignment);
    }

    ALWAYS_INLINE void *NewImpl(size_t size, u32 alignment, vp::imem::IHeap *heap) {

        /* Attempt to fallback to malloc if the heap manager is not initialized */
        if constexpr (cForceUseHeapAllocator == false) {
            if (awn::mem::IsHeapManagerInitialized() == false) {
                return ::_aligned_malloc(size, alignment);
            }
        }

        /* Try to allocate from specified heap if able, or current heap */
        vp::imem::IHeap *current_heap = (heap != nullptr) ? heap : awn::mem::GetCurrentThreadHeap();
        VP_ASSERT(current_heap != nullptr);

        return current_heap->TryAllocate(size, alignment);
    }

    ALWAYS_INLINE void DeleteImpl(void *address) {
        if constexpr (cForceUseHeapAllocator == false) {
            if (awn::mem::IsHeapManagerInitialized() == false) {
                return ::_aligned_free(address);
            }
        }
        vp::imem::IHeap *address_heap = awn::mem::FindHeapFromAddress(address);
        VP_ASSERT(address_heap != nullptr);

        address_heap->Free(address);
    }
}

/* Default overloads */

ALWAYS_INLINE void *operator new(size_t size) {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

ALWAYS_INLINE void *operator new(size_t size, std::align_val_t alignment) {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

ALWAYS_INLINE void *operator new(size_t size, const std::nothrow_t&) {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

ALWAYS_INLINE void *operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

ALWAYS_INLINE void *operator new[](size_t size) {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

ALWAYS_INLINE void *operator new[](size_t size, std::align_val_t alignment) {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

ALWAYS_INLINE void *operator new[](size_t size, const std::nothrow_t&) {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

ALWAYS_INLINE void *operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

/* Custom */

ALWAYS_INLINE void *operator new(size_t size, vp::imem::IHeap *heap, u32 alignment) {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

ALWAYS_INLINE void *operator new(size_t size, vp::imem::IHeap *heap, const std::nothrow_t&) {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment, heap);
}

ALWAYS_INLINE void *operator new(size_t size, vp::imem::IHeap *heap, u32 alignment, const std::nothrow_t&) {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

ALWAYS_INLINE void *operator new[](size_t size, vp::imem::IHeap *heap, u32 alignment) {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

ALWAYS_INLINE void *operator new[](size_t size, vp::imem::IHeap *heap, const std::nothrow_t&) {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment, heap);
}

ALWAYS_INLINE void *operator new[](size_t size, vp::imem::IHeap *heap, u32 alignment, const std::nothrow_t&) {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

/* Default overloads */

ALWAYS_INLINE void operator delete(void *address) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, std::align_val_t) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, size_t) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, size_t, std::align_val_t) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, const std::nothrow_t&) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, std::align_val_t, const std::nothrow_t&) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, std::align_val_t) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, size_t) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, size_t, std::align_val_t) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, const std::nothrow_t&) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, std::align_val_t, const std::nothrow_t&) {
    awn::mem::impl::DeleteImpl(address);
}

/* Custom */

ALWAYS_INLINE void operator delete(void *address, vp::imem::IHeap *, u32) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, vp::imem::IHeap *, const std::nothrow_t&) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, vp::imem::IHeap *, u32, const std::nothrow_t&) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, vp::imem::IHeap *, u32) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, vp::imem::IHeap *, const std::nothrow_t&) {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, vp::imem::IHeap *, u32, const std::nothrow_t&) {
    awn::mem::impl::DeleteImpl(address);
}

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

    void *NewImpl(size_t size, u32 alignment);

    void *NewImpl(size_t size, u32 alignment, vp::imem::IHeap *heap);

    void DeleteImpl(void *address);
}

/* Default overloads */

ALWAYS_INLINE void *operator new(size_t size) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

ALWAYS_INLINE void *operator new(size_t size, std::align_val_t alignment) noexcept {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

ALWAYS_INLINE void *operator new(size_t size, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

ALWAYS_INLINE void *operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

ALWAYS_INLINE void *operator new[](size_t size) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

ALWAYS_INLINE void *operator new[](size_t size, std::align_val_t alignment) noexcept {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

ALWAYS_INLINE void *operator new[](size_t size, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

ALWAYS_INLINE void *operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

/* Custom */

ALWAYS_INLINE void *operator new(size_t size, vp::imem::IHeap *heap, u32 alignment) noexcept {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

ALWAYS_INLINE void *operator new(size_t size, vp::imem::IHeap *heap, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment, heap);
}

ALWAYS_INLINE void *operator new(size_t size, vp::imem::IHeap *heap, u32 alignment, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

ALWAYS_INLINE void *operator new[](size_t size, vp::imem::IHeap *heap, u32 alignment) noexcept {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

ALWAYS_INLINE void *operator new[](size_t size, vp::imem::IHeap *heap, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment, heap);
}

ALWAYS_INLINE void *operator new[](size_t size, vp::imem::IHeap *heap, u32 alignment, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

/* Default overloads */

ALWAYS_INLINE void operator delete(void *address) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, std::align_val_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, size_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, size_t, std::align_val_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, std::align_val_t, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, std::align_val_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, size_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, size_t, std::align_val_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, std::align_val_t, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

/* Custom */

ALWAYS_INLINE void operator delete(void *address, vp::imem::IHeap *, u32) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, vp::imem::IHeap *, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, vp::imem::IHeap *, u32, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, vp::imem::IHeap *, u32) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, vp::imem::IHeap *, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, vp::imem::IHeap *, u32, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

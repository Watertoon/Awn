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
#include <vp/util/util_includes.h>
#include <vp/util/util_defines.h>
#include <vp/util/util_typedefs.h>

namespace vp::imem {
    class IHeap;
}
namespace awn::mem {
    class Heap;
}
namespace awn::mem::impl {

    static constexpr inline size_t cDefaultNewAlignment = 8;

    void *NewImpl(size_t size, u32 alignment);
    void *NewImpl(size_t size, u32 alignment, vp::imem::IHeap *heap);
    void DeleteImpl(void *address);
}

/* Default overloads */

void *operator new(size_t size) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

void *operator new(size_t size, std::align_val_t alignment) noexcept {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

void *operator new(size_t size, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

void *operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

void *operator new[](size_t size) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

void *operator new[](size_t size, std::align_val_t alignment) noexcept {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

void *operator new[](size_t size, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment);
}

void *operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

/* Custom */

void *operator new(size_t size, vp::imem::IHeap *heap, u32 alignment) noexcept {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

void *operator new(size_t size, vp::imem::IHeap *heap, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment, heap);
}

void *operator new(size_t size, vp::imem::IHeap *heap, u32 alignment, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

void *operator new[](size_t size, vp::imem::IHeap *heap, u32 alignment) noexcept {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

void *operator new[](size_t size, vp::imem::IHeap *heap, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, awn::mem::impl::cDefaultNewAlignment, heap);
}

void *operator new[](size_t size, vp::imem::IHeap *heap, u32 alignment, const std::nothrow_t&) noexcept {
    return awn::mem::impl::NewImpl(size, alignment, heap);
}

/* Default overloads */

void operator delete(void *address) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete(void *address, std::align_val_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete(void *address, size_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete(void *address, size_t, std::align_val_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete(void *address, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete(void *address, std::align_val_t, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete[](void *address) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete[](void *address, std::align_val_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete[](void *address, size_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete[](void *address, size_t, std::align_val_t) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete[](void *address, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete[](void *address, std::align_val_t, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

/* Custom */

void operator delete(void *address, vp::imem::IHeap *, u32) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete(void *address, vp::imem::IHeap *, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete(void *address, vp::imem::IHeap *, u32, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete[](void *address, vp::imem::IHeap *, u32) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete[](void *address, vp::imem::IHeap *, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

void operator delete[](void *address, vp::imem::IHeap *, u32, const std::nothrow_t&) noexcept {
    awn::mem::impl::DeleteImpl(address);
}

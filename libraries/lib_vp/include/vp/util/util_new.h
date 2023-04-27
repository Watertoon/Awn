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

namespace vp::imem {
    class IHeap;
}

void *operator new(size_t size);
void *operator new(size_t size, std::align_val_t alignment);
void *operator new(size_t size, const std::nothrow_t&);
void *operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&);

void *operator new[](size_t size);
void *operator new[](size_t size, std::align_val_t alignment);
void *operator new[](size_t size, const std::nothrow_t&);
void *operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&);

/* Custom */
void *operator new(size_t size, vp::imem::IHeap *heap, u32 alignment);
void *operator new(size_t size, vp::imem::IHeap *heap, const std::nothrow_t&);
void *operator new(size_t size, vp::imem::IHeap *heap, u32 alignment, const std::nothrow_t&);
void *operator new[](size_t size, vp::imem::IHeap *heap, u32 alignment);
void *operator new[](size_t size, vp::imem::IHeap *heap, const std::nothrow_t&);
void *operator new[](size_t size, vp::imem::IHeap *heap, u32 alignment, const std::nothrow_t&);

/* Default overloads */
void operator delete(void *address);
void operator delete(void *address, std::align_val_t);
void operator delete(void *address, size_t);
void operator delete(void *address, size_t, std::align_val_t);
void operator delete(void *address, const std::nothrow_t&);
void operator delete(void *address, std::align_val_t, const std::nothrow_t&);

void operator delete[](void *address);
void operator delete[](void *address, std::align_val_t);
void operator delete[](void *address, size_t);
void operator delete[](void *address, size_t, std::align_val_t);
void operator delete[](void *address, const std::nothrow_t&);
void operator delete[](void *address, std::align_val_t, const std::nothrow_t&);

/* Custom */
void operator delete(void *address, vp::imem::IHeap *heap, u32);
void operator delete(void *address, vp::imem::IHeap *heap, const std::nothrow_t&);
void operator delete(void *address, vp::imem::IHeap *heap, u32, const std::nothrow_t&);

void operator delete[](void *address, vp::imem::IHeap *heap, u32);
void operator delete[](void *address, vp::imem::IHeap *heap, const std::nothrow_t&);
void operator delete[](void *address, vp::imem::IHeap *heap, u32, const std::nothrow_t&);

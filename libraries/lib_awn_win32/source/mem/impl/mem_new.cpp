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
#include <awn.hpp>

namespace awn::mem::impl {

    void *NewImpl(size_t size, u32 alignment) {

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

    void *NewImpl(size_t size, u32 alignment, vp::imem::IHeap *heap) {

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

    void DeleteImpl(void *address) {
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

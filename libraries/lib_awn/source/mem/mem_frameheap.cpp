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

namespace awn::mem {

    FrameHeap *FrameHeap::TryCreate(const char *name, mem::Heap *parent_heap, size_t size, s32 alignment, bool is_thread_safe) {

        /* Get size */
        const size_t max_size = (size != 0) ? vp::util::AlignUp(size, alignof(size_t)) : parent_heap->GetMaximumAllocatableSize(alignment);
        if (max_size < sizeof(FrameHeap)) { return nullptr; }

        /* Allocate new frameheap */
        void *memory = parent_heap->TryAllocate(max_size, alignment);
        if (memory == nullptr) { return nullptr; }

        /* Construct frame heap */
        FrameHeap *new_frameheap = reinterpret_cast<FrameHeap*>(memory);
        std::construct_at(new_frameheap, name, parent_heap, memory, max_size, is_thread_safe);
        new_frameheap->m_iter = reinterpret_cast<uintptr_t>(memory) + sizeof(FrameHeap);
        new_frameheap->m_end = reinterpret_cast<uintptr_t>(new_frameheap->m_end_address);

        /* Add to parent heap child list */
        parent_heap->PushBackChild(new_frameheap);

        return new_frameheap;
    }

    MemoryRange FrameHeap::AdjustHeap() {

        /* Ensure parent to free to */
        if (m_parent_heap == nullptr) {
            return {m_end_address, 0, };
        }

        /* Lock heap */
        ScopedHeapLock l0(this);
        ScopedHeapLock l1(reinterpret_cast<mem::Heap*>(m_parent_heap));

        /* Adjust parent allocation */
        const size_t adjusted_size = m_iter - reinterpret_cast<uintptr_t>(m_start_address);
        const size_t new_size = m_parent_heap->AdjustAllocation(this, adjusted_size);

        /* Calculate new end address */
        const size_t freed_size = m_end - m_iter;
        m_end_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_start_address) + new_size);;
        return { m_end_address, freed_size, };
    }

    void *FrameHeap::TryAllocate(size_t size, s32 alignment) {

        /* Adjust size */
        const size_t adjusted_size = vp::util::AlignUp(vp::util::Max(size, cMinimumAllocationGranularity), alignment);
        auto fail_guard = SCOPE_GUARD {
            /* Attempt out of memory callback */
            OutOfMemoryInfo out_of_memory = {
                .out_of_memory_heap      = this,
                .allocation_size         = size,
                .aligned_allocation_size = adjusted_size,
                .alignment               = alignment,
            };
            mem::OutOfMemoryImpl(std::addressof(out_of_memory));
        };
        if (!vp::util::IsAligned(alignment, alignment)) { return nullptr; }

        /* Lock heap */
        ScopedHeapLock l(this);

        /* Calculate new start offset */
        const uintptr_t address = vp::util::AlignUp(m_iter, alignment);
        const uintptr_t next_iter = address + adjusted_size;
        /* Ensure space */
        if (next_iter < address || m_end < next_iter) { return nullptr; }
        m_iter = next_iter;

        fail_guard.Cancel();
        return reinterpret_cast<void*>(address);
    }

    void FrameHeap::FreeAll() {

        /* Lock heap */
        ScopedHeapLock l(this);

        /* Dispose */
        this->DisposeAll();

        /* Reset start offset */
        m_iter = reinterpret_cast<uintptr_t>(m_start_address) + sizeof(FrameHeap);

        return;
    }

    bool FrameHeap::IsEmpty() const {
        const bool is_start = (m_iter == reinterpret_cast<uintptr_t>(m_start_address) + sizeof(FrameHeap));
        const bool is_end = (m_end == reinterpret_cast<uintptr_t>(m_end_address));
        return is_start && is_end;
    }

    size_t FrameHeap::GetTotalFreeSize() {
        ScopedHeapLock l(this);
        return m_end - m_iter;
    }

    size_t FrameHeap::GetMaximumAllocatableSize(s32 alignment) {
        if (vp::util::IsAligned(alignment, alignment)) { return 0; }
        ScopedHeapLock l(this);
        const uintptr_t address = vp::util::AlignUp(m_iter, alignment);
        const uintptr_t end_address = m_end;
        const uintptr_t size = (address < end_address) ? end_address - address : 0;
        return size;
    }
}
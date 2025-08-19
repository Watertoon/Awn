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

    UnitHeap *UnitHeap::TryCreate(const char *name, mem::Heap *parent_heap, size_t size, u32 block_size, s32 alignment, bool is_thread_safe) {

        /* Integrity checks */
        if (block_size < sizeof(UnitHeapBlock)) { return nullptr; }
        alignment = vp::util::Max(alignment, static_cast<s32>(alignof(size_t)));
        if (!vp::util::IsAligned(alignment, alignment)) { return nullptr; }

        /* Parent to current heap if one is not provided */
        if (parent_heap == nullptr) {
            parent_heap = mem::GetCurrentThreadHeap();
        }

        /* Respect whole size */
        if (size == mem::Heap::cWholeSize) {
            size = parent_heap->GetMaximumAllocatableSize(alignment);
            size = vp::util::AlignDown(size, alignof(size_t));
        } else {
            size = vp::util::AlignUp(size, alignof(size_t));
        }
        if (size < vp::util::AlignUp(block_size, alignment) + sizeof(UnitHeap)) { return nullptr; }

        /* Allocate unit heap memory */
        void *address = parent_heap->TryAllocate(size, alignment);
        if (address == nullptr) { return nullptr; }

        /* Construct unit heap */
        UnitHeap *unit_heap = reinterpret_cast<UnitHeap*>(address);
        std::construct_at(unit_heap, name, parent_heap, address, size, is_thread_safe);

        /* Calc free clock region */
        const uintptr_t list_start = vp::util::AlignUp(reinterpret_cast<uintptr_t>(address) + sizeof(UnitHeap), alignment);
        block_size = vp::util::AlignUp(block_size, alignment);
        unit_heap->m_block_size = block_size;
        const uintptr_t list_size = reinterpret_cast<uintptr_t>(unit_heap->m_end_address) - list_start;
        unit_heap->m_free_start = list_start;
        const u32 block_count = list_size / block_size;
        unit_heap->m_block_count = block_count;
        unit_heap->m_free_size = block_count * block_size;

        /* Add blocks to free list */
        uintptr_t iter = list_start;
        for (u32 i = 0; i < block_count; ++i) {

            /* Construct unit heap block */
            UnitHeapBlock *block = reinterpret_cast<UnitHeapBlock*>(iter);
            std::construct_at(block);
            unit_heap->m_free_list.PushBack(*block);
            iter += block_size;
        }

        return unit_heap;
    }

    void *UnitHeap::TryAllocate(size_t size, s32 alignment) {

        /* Integrity checks */
        auto fail_guard = SCOPE_GUARD {
            /* Attempt out of memory callback */
            OutOfMemoryInfo out_of_memory = {
                .out_of_memory_heap = this,
                .allocation_size    = size,
                .alignment          = alignment,
            };
            mem::OutOfMemoryImpl(std::addressof(out_of_memory));
        };
        if (m_block_size < size) { return nullptr; }
        if (!vp::util::IsAligned(m_block_size, alignment)) { return nullptr; }
        ScopedHeapLock l(this);
        if (m_free_list.IsEmpty()) { return nullptr; }

        /* Pop a block */
        m_free_size -= m_block_size;
        void *block = reinterpret_cast<void*>(std::addressof(m_free_list.PopBack()));

        fail_guard.Cancel();
        return block;
    }

    void UnitHeap::Free(void *address) {

        /* Ensure valid address */
        VP_ASSERT(address != nullptr);
        VP_ASSERT(this->IsAddressInHeap(address));

        /* Construct free block */
        ScopedHeapLock l(this);
        UnitHeapBlock *block = reinterpret_cast<UnitHeapBlock*>(address);
        std::construct_at(block);
        m_free_list.PushBack(*block);
        m_free_size += m_block_size;

        return;
    }

    void UnitHeap::FreeAll() {

        /* Dispose */
        ScopedHeapLock l(this);
        this->DisposeAll();

        /* Rebuild free list */
        m_free_list.ForceClear();
        const u32 block_size = m_block_size;
        const u32 block_count = m_block_count;
        uintptr_t iter = m_free_start;
        for (u32 i = 0; i < block_count; ++i) {

            /* Construct unit heap block */
            UnitHeapBlock *block = reinterpret_cast<UnitHeapBlock*>(iter);
            std::construct_at(block);
            m_free_list.PushBack(*block);
            iter += block_size;
        }

        return;
    }

    size_t UnitHeap::GetTotalFreeSize() {
        return m_free_size;
    }

    size_t UnitHeap::GetMaximumAllocatableSize(s32 alignment) {
        if (!vp::util::IsAligned(m_block_size, alignment)) { return 0; }
        return (!m_free_list.IsEmpty()) ? m_block_size : 0;
    }
}

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

    SeparateHeap *SeparateHeap::Create(const char *name, void *management_area, size_t management_size, void *start_address, size_t size, bool is_thread_safe) {

        /* Construct new seperate heap */
        SeparateHeap *heap = reinterpret_cast<SeparateHeap*>(management_area);
        std::construct_at(heap, name, start_address, size, is_thread_safe);

        /* Construct free list */
        SeparateMemoryBlock *block_start = reinterpret_cast<SeparateMemoryBlock*>(reinterpret_cast<uintptr_t>(management_area) + sizeof(SeparateHeap));
        const u32 block_count = (management_size - sizeof(SeparateHeap)) / sizeof(SeparateMemoryBlock);
        heap->m_block_allocator.SetMemoryAndClear(block_start, block_count);
        heap->m_total_count = block_count;

        return heap;
    }

    void *SeparateHeap::TryAllocate(size_t size, s32 alignment) {

        /* Integrity check existing free block */
        auto fail_guard = SCOPE_GUARD {
            /* Fail callback */
            OutOfMemoryInfo out_of_memory = {
                .out_of_memory_heap      = this,
                .allocation_size         = size,
                .alignment               = alignment,
            };
            mem::OutOfMemoryImpl(std::addressof(out_of_memory));
        };
        if (!vp::util::IsAligned(alignment, alignment)) { return nullptr; }

        /* Granularity */
        const size_t gran_size = vp::util::Max(size, cMinimumAllocationGranularity);
        alignment = vp::util::Max(alignment, static_cast<s32>(alignof(size_t)));

        /* Ensure block */
        ScopedHeapLock lock(this);
        if (m_block_allocator.IsEmpty()) { return nullptr; }

        /* Find free range */
        const uintptr_t start_address = reinterpret_cast<uintptr_t>(m_start_address);
        uintptr_t addr = start_address;
        uintptr_t iter = 0;
        BlockList::iterator b_iter = m_block_list.begin();
        BlockList::iterator b_end = m_block_list.end();
        while (b_iter != b_end) {
            SeparateMemoryBlock &block = *b_iter;
            iter = vp::util::AlignUp(addr, alignment);
            if (gran_size <= block.address - iter) { break; }
            ++b_iter;
            addr = block.address + block.size;
        }
        if (iter == 0 || b_iter == b_end) {
            iter = vp::util::AlignUp(addr, alignment);
            if (iter == 0 || reinterpret_cast<uintptr_t>(m_end_address) - iter < gran_size) { return nullptr; }
        }

        /* Add used block */
        const uintptr_t base_address = iter;
        SeparateMemoryBlock *block = m_block_allocator.Allocate();
        (*b_iter).list_node.LinkPrev(std::addressof(block->list_node));
        ++m_used_count;
        block->address = base_address;
        block->size = gran_size;

        fail_guard.Cancel();
        return reinterpret_cast<void*>(base_address);
    }

    void SeparateHeap::Free(void *address) {

        /* Ensure valid address */
        VP_ASSERT(address != nullptr);

        /* Free block */
        ScopedHeapLock lock(this);
        for (SeparateMemoryBlock &block : m_block_list) {
            if (block.address != reinterpret_cast<uintptr_t>(address)) { continue; }
            block.list_node.Unlink();
            m_block_allocator.Free(std::addressof(block));
            --m_used_count;
            return;
        }

        return;
    }

    void SeparateHeap::FreeAll() {

        /* Dispose */
        ScopedHeapLock lock(this);
        this->DisposeAll();

        /* Free block */
        m_block_list.ForceClear();
        m_block_allocator.Clear(m_total_count);

        return;
    }

    size_t SeparateHeap::GetTotalFreeSize() {

        /* Tally */
        ScopedHeapLock lock(this);
        uintptr_t used_size = 0;
        for (SeparateMemoryBlock &block : m_block_list) {
            used_size += block.size;
        }

        return (reinterpret_cast<uintptr_t>(m_end_address) - reinterpret_cast<uintptr_t>(m_start_address)) - used_size;
    }

    size_t SeparateHeap::GetMaximumAllocatableSize(s32 alignment) {

        /* Integrity checks */
        if (!vp::util::IsAligned(alignment, alignment)) { return 0; }
        /* Granularity */
        alignment = vp::util::Max(alignment, static_cast<s32>(alignof(size_t)));

        const uintptr_t start_address = reinterpret_cast<uintptr_t>(m_start_address);
        uintptr_t addr = start_address;
        uintptr_t iter = 0;
        uintptr_t max = 0;
        uintptr_t prev_max = 0;
        for (SeparateMemoryBlock &block : m_block_list) {
            iter = vp::util::AlignUp(addr, alignment);
            const uintptr_t free = block.address - iter;
            max = vp::util::Max(prev_max, free);
            max = (block.address <= iter) ? prev_max : max;
            addr = block.address + block.size;
        }
        iter = vp::util::AlignUp(addr, alignment);
        const uintptr_t end = reinterpret_cast<uintptr_t>(m_end_address);
        max = vp::util::Max(prev_max, end - iter);
        max = (end <= iter) ? prev_max : max;
        return max;
    }
}

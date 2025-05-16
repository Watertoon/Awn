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

    SeparateHeap::SeparateHeap(const char *name, void *heap_start, u64 heap_size, void *management_start, u64 management_size, bool is_thread_safe) : Heap(name, nullptr, reinterpret_cast<void*>(heap_start), heap_size, is_thread_safe), m_used_block_list(), m_management_free_list_start(reinterpret_cast<SeparateFreeListHelper*>(management_start)), m_used_management_block_count(0), m_total_management_block_count(management_size/ sizeof(SeparateMemoryBlock)), m_management_area_size(management_size), m_management_area_start(management_start) {

        /* Initialize managment free list */
        SeparateFreeListHelper *block_list = reinterpret_cast<SeparateFreeListHelper*>(management_start);
        for (u32 i = 0; i < m_total_management_block_count - 1; ++i) {
            block_list[i].next = std::addressof(block_list[i + 1]);
        }
    }
    SeparateHeap::~SeparateHeap() {/*...*/}

    void SeparateHeap::Finalize() {/*...*/}

    MemoryRange SeparateHeap::AdjustHeap() {
        const uintptr_t size = reinterpret_cast<uintptr_t>(m_end_address) - reinterpret_cast<uintptr_t>(m_start_address);
        return MemoryRange{ m_start_address, size, };
    }
    size_t SeparateHeap::AdjustAllocation([[maybe_unused]] void *byte_offset_of_allocation, [[maybe_unused]] size_t new_size) { return 0; }

    void *SeparateHeap::TryAllocate(size_t size, s32 alignment) {

        /* Integrity checks */
        VP_ASSERT(m_management_free_list_start != nullptr);

        /* Conditional heap lock */
        alignment = vp::util::Max(alignment, cMinimumAlignment);
        ScopedHeapLock l(this);
        const size_t aligned_size = (size != Heap::cWholeSize) ? vp::util::AlignUp(size, alignment) : this->GetMaximumAllocatableSize(alignment);

        /* Find new offset */
        SeparateMemoryBlock *prev_block = std::addressof(m_used_block_list.Back());
        uintptr_t free_start = vp::util::AlignUp(reinterpret_cast<uintptr_t>(m_start_address), alignment);
        for (SeparateMemoryBlock &block : m_used_block_list) {
            const size_t free_size = static_cast<size_t>(block.block_offset - free_start);
            if (aligned_size <= free_size) {
                prev_block = std::addressof(block);
                break;
            }
            free_start = vp::util::AlignUp(block.block_size + block.block_offset, alignment);
        }
        if (prev_block == std::addressof(m_used_block_list.Back())) {
            const size_t end_free_size = reinterpret_cast<uintptr_t>(m_end_address) - free_start;
            if (end_free_size < aligned_size) {
                return nullptr;
            }
        }

        /* Create new separate heap block with offset */
        SeparateMemoryBlock *new_block = reinterpret_cast<SeparateMemoryBlock*>(m_management_free_list_start);
        std::construct_at(new_block);
        m_management_free_list_start = m_management_free_list_start->next;

        /* Ordered insert to used list */
        if (prev_block == std::addressof(m_used_block_list.Back())) {
            m_used_block_list.PushBack(*new_block);
        } else {
            prev_block->used_list_node.LinkNext(std::addressof(new_block->used_list_node));
        }

        /* Set block state */
        new_block->block_offset = free_start;
        new_block->block_size   = aligned_size;

        return reinterpret_cast<void*>(free_start);
    }

    void SeparateHeap::Free(void *byte_offset_of_allocation) {

        /* Conditional heap lock */
        ScopedHeapLock l(this);

        /* Find block */
        for (SeparateMemoryBlock &used_block : m_used_block_list) {
            if (used_block.block_offset != reinterpret_cast<size_t>(byte_offset_of_allocation)) { continue; }
            /* Free used block to free list */
            used_block.used_list_node.Unlink();
            reinterpret_cast<SeparateFreeListHelper&>(used_block).next = m_management_free_list_start;
            m_management_free_list_start = reinterpret_cast<SeparateFreeListHelper*>(std::addressof(used_block));
            break;
        }

        return;
    }

    size_t SeparateHeap::GetTotalFreeSize() {

        ScopedHeapLock l(this);

        /* Add up used block sizes */
        size_t used_size = reinterpret_cast<size_t>(m_start_address);
        for (SeparateMemoryBlock &used_block : m_used_block_list) {
            used_size = used_size + used_block.block_size;
        }

        /* Return difference of total memory */
        const size_t diff_size = reinterpret_cast<size_t>(m_end_address) - used_size;
        return diff_size;
    }

    size_t SeparateHeap::GetMaximumAllocatableSize(s32 alignment) {

        alignment = vp::util::Max(alignment, cMinimumAlignment);
        ScopedHeapLock l(this);
        if (m_management_free_list_start == nullptr) { return 0; }

        size_t max_free_size = 0;
        uintptr_t free_start = vp::util::AlignUp(reinterpret_cast<uintptr_t>(m_start_address), alignment);
        for (const SeparateMemoryBlock &block : m_used_block_list) {

            const uintptr_t block_start = block.block_offset;
            const uintptr_t block_end   = block_start + block.block_size;
            const uintptr_t align_start = vp::util::AlignUp(free_start, alignment);
            const size_t    size        = static_cast<size_t>(block_start - align_start);
            free_start = block_end;
            max_free_size = vp::util::Max(size, max_free_size);
        }
        const uintptr_t align_start = vp::util::AlignUp(free_start, alignment);
        const size_t end_free_size = reinterpret_cast<uintptr_t>(m_end_address) - align_start;
        max_free_size = vp::util::Max(end_free_size, max_free_size);

        return max_free_size;
    }
}

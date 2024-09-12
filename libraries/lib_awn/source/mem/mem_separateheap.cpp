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

    MemoryRange SeparateHeap::AdjustHeap() { return MemoryRange{m_start_address, reinterpret_cast<size_t>(m_end_address) - reinterpret_cast<size_t>(m_start_address)}; }
    size_t SeparateHeap::AdjustAllocation([[maybe_unused]] void *byte_offset_of_allocation, [[maybe_unused]] size_t new_size) { return 0; }

    void *SeparateHeap::TryAllocate(size_t size, s32 alignment) {

        /* Integrity checks */
        VP_ASSERT(m_management_free_list_start != nullptr);

        /* Conditional heap lock */
        ScopedHeapLock l(this);

        /* Find new offset */
        SeparateMemoryBlock *prev_block = std::addressof(m_used_block_list.Back());
        size_t new_offset = vp::util::AlignUp(reinterpret_cast<size_t>(m_start_address), alignment);
        for (SeparateMemoryBlock &separate_block : m_used_block_list) {
            if (size <= (separate_block.block_offset - new_offset)) {
                prev_block = std::addressof(separate_block);
                break;
            }
            new_offset = vp::util::AlignUp(separate_block.block_size + separate_block.block_offset, alignment);
        }
        if (m_end_address < reinterpret_cast<void*>(new_offset)) { return nullptr; }

        /* Create new separate heap block with offset */
        SeparateMemoryBlock *new_block = reinterpret_cast<SeparateMemoryBlock*>(m_management_free_list_start);
        std::construct_at(new_block);
        m_management_free_list_start = m_management_free_list_start->next;

        /* Ordered insert to used list */
        prev_block->used_list_node.LinkNext(std::addressof(new_block->used_list_node));

        /* Set block state */
        new_block->block_offset = new_offset;
        new_block->block_size   = size;

        return reinterpret_cast<void*>(new_offset);
    }

    void SeparateHeap::Free(void *byte_offset_of_allocation) {

        /* Conditional heap lock */
        ScopedHeapLock l(this);

        /* Find block */
        for (SeparateMemoryBlock &used_block : m_used_block_list) {
            if (used_block.block_offset == reinterpret_cast<size_t>(byte_offset_of_allocation)) {

                /* Free used block to free list */
                used_block.used_list_node.Unlink();
                reinterpret_cast<SeparateFreeListHelper&>(used_block).next = m_management_free_list_start;
                m_management_free_list_start = reinterpret_cast<SeparateFreeListHelper*>(std::addressof(used_block));
                break;
           }
        }

        return;
    }

    size_t SeparateHeap::GetTotalFreeSize() {

        /* Add up used block sizes */
        size_t used_size = reinterpret_cast<size_t>(m_start_address);
        for (SeparateMemoryBlock &used_block : m_used_block_list) {
            used_size = used_size + used_block.block_size;
        }

        /* Return difference of total memory */
        return reinterpret_cast<size_t>(m_end_address) - used_size;
    }

    size_t SeparateHeap::GetMaximumAllocatableSize([[maybe_unused]] s32 alignment) {
        return 0;
    }
}

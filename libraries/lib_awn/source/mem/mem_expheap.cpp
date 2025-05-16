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

    bool ExpHeap::AddFreeBlock(AddressRange range) {

        /* Coalesce free blocks betweens */
        void *new_start = range.start;
        void *new_end = range.end;
        vp::util::IntrusiveListNode *link_node = nullptr;
        if (!m_free_list.IsEmpty()) {
            
            /* Search blocks */
            ExpHeapMemoryBlock *prev_block = nullptr;
            ExpHeapMemoryBlock *next_block = nullptr;
            for (ExpHeapMemoryBlock &free_block : m_free_list) {
                next_block = std::addressof(free_block);
                if (new_start <= next_block) { break; }
                prev_block = next_block;
            }

            /* Coalesce back */
            if (next_block != nullptr && new_end == reinterpret_cast<void*>(next_block)) {
                next_block->exp_list_node.Unlink();
                new_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(next_block) + next_block->block_size + sizeof(ExpHeapMemoryBlock));
            }

            /* Coalesce front */
            if (prev_block != nullptr) {
                link_node = std::addressof(prev_block->exp_list_node);
                if (new_start == reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(prev_block) + prev_block->block_size + sizeof(ExpHeapMemoryBlock))) {
                    link_node = prev_block->exp_list_node.prev();
                    prev_block->exp_list_node.Unlink();
                    new_start = reinterpret_cast<void*>(prev_block);
                }
            }
        }

        /* Ensure block size is valid */
        const size_t new_size = reinterpret_cast<uintptr_t>(new_end) - reinterpret_cast<uintptr_t>(new_start);
        if (new_size < sizeof(ExpHeapMemoryBlock)) {
            return false;
        }

        /* Add new free block */
        ExpHeapMemoryBlock *new_block = reinterpret_cast<ExpHeapMemoryBlock*>(new_start);
        std::construct_at(new_block);
        new_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
        new_block->alignment = 0;
        new_block->block_size = new_size - sizeof(ExpHeapMemoryBlock);

        /* Link new free block between range */
        if (link_node != nullptr) {            
            link_node->LinkNext(std::addressof(new_block->exp_list_node));
        } else {
            m_free_list.PushFront(*new_block);
        }

        return true;
    }

    void ExpHeap::AddUsedBlock(ExpHeapMemoryBlock *free_block, uintptr_t allocation_address, size_t size) {

        /* Unlink old free block */
        vp::util::IntrusiveListNode *next_node = free_block->exp_list_node.next();
        vp::util::IntrusiveListNode *prev_node = free_block->exp_list_node.prev();
        free_block->exp_list_node.Unlink();

        /* Calculate new free block addresses and sizes */
        uintptr_t new_free_address     = allocation_address + size;
        uintptr_t new_free_end_address = reinterpret_cast<uintptr_t>(free_block) + free_block->block_size + sizeof(ExpHeapMemoryBlock);
        uintptr_t new_free_total_size  = new_free_end_address - new_free_address;
        uintptr_t orig_start_address   = reinterpret_cast<uintptr_t>(free_block);
        uintptr_t used_start_address   = allocation_address - sizeof(ExpHeapMemoryBlock);
        uintptr_t used_alignment       = used_start_address - orig_start_address;

        /* Create a new free block at front if alignment is great enough */
        if (sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity <= used_alignment) {
            ExpHeapMemoryBlock *front_free_block = reinterpret_cast<ExpHeapMemoryBlock*>(orig_start_address);
            std::construct_at(front_free_block);
            front_free_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
            front_free_block->alignment   = 0;
            front_free_block->block_size  = used_alignment - sizeof(ExpHeapMemoryBlock);
            prev_node->LinkNext(std::addressof(front_free_block->exp_list_node));
            used_alignment = 0;
        }

        /* Create new free block at back if leftover space is great enough */
        if (sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity <= new_free_total_size) {
            ExpHeapMemoryBlock *back_free_block = reinterpret_cast<ExpHeapMemoryBlock*>(new_free_address);
            std::construct_at(back_free_block);
            back_free_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
            back_free_block->alignment   = 0;
            back_free_block->block_size  = new_free_total_size - sizeof(ExpHeapMemoryBlock);
            next_node->LinkPrev(std::addressof(back_free_block->exp_list_node));
            new_free_total_size = 0;
        }

        /* Add used block */
        ExpHeapMemoryBlock *used_block = reinterpret_cast<ExpHeapMemoryBlock*>(used_start_address);

        std::construct_at(used_block);
        used_block->alloc_magic = ExpHeapMemoryBlock::cAllocMagic;
        used_block->alignment   = used_alignment;
        used_block->block_size  = size + new_free_total_size;
        m_used_list.PushBack(*used_block);

        return;
    }

    ExpHeap *ExpHeap::TryCreate(const char *name, void *address, size_t size, bool is_thread_safe) {

        /* Integrity checks */
        VP_ASSERT(address != nullptr && (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity) <= size);

        /* Contruct exp heap object */
        ExpHeap *new_heap = reinterpret_cast<ExpHeap*>(address);
        std::construct_at(new_heap, name, nullptr, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + sizeof(ExpHeap)), size - sizeof(ExpHeap), is_thread_safe);

        /* Create and push free node spanning block */
        ExpHeapMemoryBlock *first_block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(new_heap) + sizeof(ExpHeap));
        std::construct_at(first_block);

        first_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
        first_block->alignment   = 0;
        first_block->block_size  = size - (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock));

        new_heap->m_free_list.PushBack(*first_block);

        return new_heap;
    }

    ExpHeap *ExpHeap::TryCreate(const char *name, Heap *parent_heap, size_t size, s32 alignment, bool is_thread_safe) {

        /* Use current heap if one is not provided */
        if (parent_heap == nullptr) {
            parent_heap = mem::GetCurrentThreadHeap();
        }

        /* Respect whole size */
        if (size == mem::Heap::cWholeSize) {
            size = parent_heap->GetMaximumAllocatableSize(alignment);
        }

        /* Enforce minimum size */
        if (size < (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity)) { return nullptr; }

        /* Allocate heap memory from parent heap */
        void *new_heap_memory = parent_heap->TryAllocate(size, alignment);
        ExpHeap *new_heap = reinterpret_cast<ExpHeap*>(new_heap_memory);
        
        if (new_heap_memory == nullptr) { return nullptr; }

        /* Construct new heap */
        std::construct_at(new_heap, name, parent_heap, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(new_heap_memory) + sizeof(ExpHeap)), size - sizeof(ExpHeap), is_thread_safe);

        /* Construct and add free node spanning new heap */
        ExpHeapMemoryBlock *first_block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(new_heap) + sizeof(ExpHeap));
        std::construct_at(first_block);

        first_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
        first_block->block_size  = size - (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock));
        first_block->alignment   = 0;

        new_heap->m_free_list.PushBack(*first_block);

        /* Add to parent heap child list */
        parent_heap->PushBackChild(new_heap);

        return new_heap;
    }

    void ExpHeap::Finalize() {/*...*/}

    MemoryRange ExpHeap::AdjustHeap() {

        /* Lock heap */
        ScopedHeapLock lock(this);

        /* Ensure we have a free block spanning to the end address */
        if (m_free_list.IsEmpty()) {
            return { m_end_address, 0, };
        }
        ExpHeapMemoryBlock *last_block = std::addressof(m_free_list.Back());
        const size_t trim_size = sizeof(ExpHeapMemoryBlock) + last_block->block_size;
        if (reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(last_block) + trim_size) != m_end_address) {
            return { m_end_address, 0, };
        }

        /* Free free block */
        m_free_list.Remove(*last_block);
        void *new_end_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_end_address) - trim_size);
        m_end_address = new_end_address;

        /* Resize parent heap memory block */
        if (m_parent_heap != nullptr) {
            const size_t heap_size = this->GetTotalSize() + sizeof(ExpHeap);
            m_parent_heap->AdjustAllocation(this, heap_size);
        }

        return { new_end_address, trim_size, };
    }

    size_t ExpHeap::ResizeHeapBack(size_t new_size) {

        /* Align new size */
        new_size = vp::util::AlignUp(new_size, cMinimumAllocationGranularity);

        /* Lock the heap */
        ScopedHeapLock lock(this);

        /* Nothing to do if size doesn't change */
        const size_t heap_size = this->GetTotalSize();
        if (new_size == heap_size) { return heap_size; }

        /* Get last free block */
        ExpHeapMemoryBlock *last_free_block = std::addressof(m_free_list.Back());
        void *last_free_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(last_free_block) + last_free_block->block_size);

        /* Shrink free block */
        if (new_size < heap_size) {

            /* Ensure last free block spans to the heap end, and has enough space to shrink */
            const size_t free_size   = last_free_block->block_size + sizeof(ExpHeapMemoryBlock);
            const size_t delta_space = (heap_size - new_size);
            if (last_free_end != m_end_address || free_size < delta_space) { return heap_size; }

            /* Set end address and unlink free block if it matches the shrinkage (effectively AdjustHeap) */
            if (delta_space == free_size) {
                last_free_block->exp_list_node.Unlink();
                m_end_address = reinterpret_cast<void*>(last_free_block);
                return new_size;
            }

            /* Set end address and add resized free block */
            last_free_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(last_free_end) - delta_space);
            this->AddFreeBlock( { reinterpret_cast<void*>(last_free_block), last_free_end, } );
            m_end_address = last_free_end;

            return new_size;
        }

        /* Find start of new free block */
        void *last_free_start = m_end_address;
        if (last_free_end == m_end_address) {
            last_free_start = reinterpret_cast<void*>(last_free_block);
        }

        /* Calculate new end */
        void *new_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_start_address) + new_size);

        /* If we are not the root heap the memory must come from a parent heap (not implicit) */
        if (this != mem::GetRootHeap(0)) {

            /* Check we have a parent heap */
            if (m_parent_heap == nullptr) { return heap_size; }

            /* Check whether the start region is a dedicated allocation */
            if (!m_parent_heap->IsAddressAllocation(this)) { return heap_size; }

            /* Try to adjust the parent allocation */
            const size_t adjust_size = m_parent_heap->AdjustAllocation(this, sizeof(ExpHeap) + new_size);
            if (adjust_size != new_size) { return heap_size; }
        }

        /* Add new free block */
        this->AddFreeBlock( { last_free_start, new_end } );

        /* Set end address */
        m_end_address = new_end;

        return new_size;
    }

    bool ExpHeap::IsAddressAllocation(void *address) {
        ScopedHeapLock lock(this);
        return this->IsAddressAllocationUnsafe(address);
    }

    bool ExpHeap::IsAddressAllocationUnsafe(void *address) {

        /* Search if address is used block */
        ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));
        for (const ExpHeapMemoryBlock &used_block : m_used_list) {
            if (std::addressof(used_block) == block) { return true; }
        }

        return false;
    }

    size_t ExpHeap::AdjustAllocation(void *address, size_t adjust_size) {

        /* Align new size */
        adjust_size = vp::util::AlignUp(adjust_size, cMinimumAllocationGranularity);

        /* Lock the heap */
        ScopedHeapLock lock(this);

        /* Find ExpHeapMemory block */
        ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));

        /* Nothing to do if the size doesn't change */
        size_t block_size = block->block_size;
        if (block_size == adjust_size) { return adjust_size; }

        /* Reduce range if new size is lesser */
        if (adjust_size < block_size) {
            this->AddFreeBlock(AddressRange{reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + adjust_size), reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + block->block_size)});
            block->block_size = adjust_size;
            return adjust_size;
        }

        /* Find free block at allocation end address */
        const uintptr_t end_address = reinterpret_cast<uintptr_t>(address) + block_size;
        BlockList::iterator free_iter = m_free_list.begin();
        while (free_iter != m_free_list.end()) {
            if (reinterpret_cast<uintptr_t>(std::addressof(*free_iter)) == end_address) { break; }
            ++free_iter;
        }
        if (free_iter == m_free_list.end()) { return block_size; }

        /* Ensure free block adds enough memory */
        ExpHeapMemoryBlock *free_block = std::addressof(*free_iter);
        const size_t free_total_size = free_block->block_size + sizeof(ExpHeapMemoryBlock);
        if (free_total_size + block_size < adjust_size) { return block_size; }

        /* Adjust allocation */
        vp::util::IntrusiveListNode *prev_free_block = free_block->exp_list_node.prev();
        free_block->exp_list_node.Unlink();
        const uintptr_t new_end_address = reinterpret_cast<uintptr_t>(free_block) + free_total_size;
        uintptr_t new_free_base = reinterpret_cast<uintptr_t>(address) + adjust_size + sizeof(ExpHeapMemoryBlock);
        const size_t new_free_size = new_end_address - reinterpret_cast<uintptr_t>(new_free_base);
        if (sizeof(ExpHeapMemoryBlock) <= new_free_size) {
            new_free_base -= new_free_size;
        }
        block->block_size = new_free_base - reinterpret_cast<uintptr_t>(address);

        if (sizeof(ExpHeapMemoryBlock) <= new_free_size) {
            ExpHeapMemoryBlock *new_free_block = reinterpret_cast<ExpHeapMemoryBlock*>(new_free_base);
            std::construct_at(new_free_block);
            new_free_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
            new_free_block->alignment = 0;
            new_free_block->block_size = new_free_size;
            prev_free_block->LinkNext(std::addressof(new_free_block->exp_list_node));
        }

        return adjust_size;
    }

    void *ExpHeap::TryAllocate(size_t size, s32 alignment) {

        /* Lock heap */
        ScopedHeapLock lock(this);

        /* Label for out of memory restart */
        _ExpHeap_OutOfMemoryRestart:

        /* Integrity check existing free block */
        alignment = vp::util::Max(alignment, cMinimumAlignment);
        if (m_free_list.IsEmpty()) {

            /* Attempt out of memory callback */
            OutOfMemoryInfo out_of_memory = {
                .out_of_memory_heap      = this,
                .allocation_size         = size,
                .aligned_allocation_size = vp::util::AlignUp(size, alignment),
                .alignment               = alignment,
            };
            const bool result = mem::OutOfMemoryImpl(std::addressof(out_of_memory));
            if (result) { goto _ExpHeap_OutOfMemoryRestart; }

            return nullptr;
        }

        /* Calculate adjusted size */
        const size_t aligned_size = (size != Heap::cWholeSize) ? vp::util::AlignUp(size, alignment) : this->GetMaximumAllocatableSizeUnsafe(alignment);

        uintptr_t base_address = 0;
        BlockList::iterator free_block = m_free_list.begin();
        if (m_allocation_mode == AllocationMode::FirstFit) {

            /* Find first fitting block */
            while (free_block != m_free_list.end()) {
                ExpHeapMemoryBlock *block = std::addressof(*free_block);
                base_address = vp::util::AlignUp(reinterpret_cast<uintptr_t>(block) + sizeof(ExpHeapMemoryBlock), alignment);
                const uintptr_t alloc_size = (aligned_size - sizeof(ExpHeapMemoryBlock)) - reinterpret_cast<uintptr_t>(block) + base_address;
                if (alloc_size <= block->block_size) { break; }
                ++free_block;
            }

        } else {

            /* Best fit mode */
            size_t smaller_size = 0xffff'ffff'ffff'ffff;
            uintptr_t smaller_address = 0;
            ExpHeapMemoryBlock *smaller_block = std::addressof(*m_free_list.end());
            while (free_block != m_free_list.end()) {

                /* Get block */
                ExpHeapMemoryBlock *block = std::addressof(*free_block);

                /* Find the starting address for the allocation */
                base_address = vp::util::AlignUp(reinterpret_cast<uintptr_t>(block) + sizeof(ExpHeapMemoryBlock), alignment);

                /* Check if we've found a smaller block that fits */
                const uintptr_t alloc_size = (aligned_size - sizeof(ExpHeapMemoryBlock)) - reinterpret_cast<uintptr_t>(block) + base_address;
                const size_t block_size = block->block_size;
                if (block_size < smaller_size && alloc_size <= block_size) {
                    smaller_size    = block_size;
                    smaller_block   = block;
                    smaller_address = base_address;

                    /* Complete if we found an exact fit */
                    if (block_size == aligned_size) { break; }
                }

                ++free_block;
                base_address = smaller_address;
            }

            free_block = m_free_list.IteratorTo(*smaller_block);
        }

        /* Ensure we found a block */
        if (free_block == m_free_list.end()) {

            /* Attempt out of memory callback */
            OutOfMemoryInfo out_of_memory = {
                .out_of_memory_heap      = this,
                .allocation_size         = size,
                .aligned_allocation_size = aligned_size,
                .alignment               = alignment,
            };
            const bool result = mem::OutOfMemoryImpl(std::addressof(out_of_memory));
            if (result) { goto _ExpHeap_OutOfMemoryRestart; }

            return nullptr;
        }

        /* Convert our new allocation to a used block */
        this->AddUsedBlock(std::addressof(*free_block), base_address, aligned_size);

        return reinterpret_cast<void*>(base_address);
    }

    void ExpHeap::Free(void *address) {

        /* Lock heap */
        ScopedHeapLock lock(this);

        /* Unlink used block */
        ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));
        VP_ASSERT(block->alloc_magic == ExpHeapMemoryBlock::cAllocMagic);
        block->exp_list_node.Unlink();

        /* Add back to free list */
        void *start = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(block) - block->alignment);
        void *end   = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + block->block_size);
        this->AddFreeBlock(AddressRange{start, end});

        return;
    }

    size_t ExpHeap::GetTotalFreeSize() {

        /* Sum free block sizes */
        ScopedHeapLock lock(this);
        size_t free_size = 0;
        for (const ExpHeapMemoryBlock &block : m_free_list) {
            free_size += block.block_size;
        }

        return free_size;
    }

    size_t ExpHeap::GetMaximumAllocatableSize(s32 alignment) {
        ScopedHeapLock lock(this);
        return this->GetMaximumAllocatableSizeUnsafe(alignment);
    }

    size_t ExpHeap::GetMaximumAllocatableSizeUnsafe(s32 alignment) {

        /* Find largest contiguous free block while respecting alignment */
        alignment = vp::util::Max(alignment, cMinimumAlignment);
        size_t max_free_size = 0;
        for (const ExpHeapMemoryBlock &block : m_free_list) {

            const uintptr_t block_start = reinterpret_cast<uintptr_t>(std::addressof(block)) + sizeof(ExpHeapMemoryBlock);
            const uintptr_t block_end   = block_start + block.block_size;
            const uintptr_t align_start = vp::util::AlignUp(block_start, alignment);
            const size_t    size        = block_end - align_start;

            /* Select greater size if valid address range */
            if (align_start <= block_end && max_free_size < size) {
                max_free_size = size;
            }
        }

        return max_free_size;
    }

    size_t ExpHeap::GetAllocationSize(void *address) {
        const ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));
        return block->block_size;
    }
}

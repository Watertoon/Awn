#include <awn.hpp>

namespace awn::mem {

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
        if (size < (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock) + MinimumAllocationGranularity)) { return nullptr; }

        /* Allocate heap memory from parent heap */
        void *new_heap_memory = parent_heap->TryAllocate(size, alignment);
        ExpHeap *new_heap = reinterpret_cast<ExpHeap*>(new_heap_memory);
        
        if (new_heap_memory == nullptr) { return nullptr; }

        /* Construct new heap */
        std::construct_at(new_heap, name, parent_heap, new_heap_memory, size, is_thread_safe);

        /* Construct and add free node spanning new heap */
        ExpHeapMemoryBlock *first_block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(new_heap) + sizeof(ExpHeap));
        std::construct_at(first_block);

        first_block->alloc_magic = ExpHeapMemoryBlock::FreeMagic;
        first_block->block_size = size - (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock));

        new_heap->m_free_block_list.PushBack(*first_block);
        
        /* Add to parent heap child list */
        parent_heap->PushBackChild(new_heap);
        
        return new_heap;
    }

    void ExpHeap::Finalize() {/*...*/}

    MemoryRange ExpHeap::AdjustHeap() {

        /* Lock heap */
        ScopedHeapLock lock(this);

        /* Get last free block */
        ExpHeapMemoryBlock *last_block = std::addressof(m_free_block_list.Back());

        /* Ensure free list is not empty */
        if (m_free_block_list.IsEmpty() == true) {
            return { m_end_address, 0 };
        }

        /* Ensure the last block encompasses our end address */
        const size_t trimed_size = sizeof(ExpHeapMemoryBlock) + last_block->block_size;
        if (reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(last_block) + trimed_size) != m_end_address) {
            return { m_end_address, 0 };
        }

        /* Remove block from free list */
        m_free_block_list.Remove(*last_block);

        /* Adjust end address */
        void *new_end_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_end_address) - trimed_size);
        m_end_address = new_end_address;

        /* Resize parent heap memory block */
        if (m_parent_heap != nullptr) {
            m_parent_heap->AdjustAllocation(m_start_address, this->GetTotalSize() - trimed_size);
        }

        return { new_end_address, trimed_size };
    }

    size_t ExpHeap::ResizeHeapBack(size_t new_size) {

        /* Align new size */
        new_size = vp::util::AlignUp(new_size, MinimumAllocationGranularity);

        /* Lock the heap */
        ScopedHeapLock lock(this);
        
        /* Get heap size */
        const size_t heap_size = this->GetTotalSize();

        /* Nothing to do if size doesn't change */
        if (new_size == heap_size) { return heap_size; }

        /* Get last free block */
        ExpHeapMemoryBlock &last_free_block = m_allocated_block_list.Back();
        void               *last_free_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(last_free_block)) + last_free_block.block_size);

        /* Shrink free block */
        if (new_size < heap_size) {

            /* Ensure last free block spans to the heap end, and has enough space to shrink */
            const size_t free_size   = last_free_block.block_size + sizeof(ExpHeapMemoryBlock);
            const size_t delta_space = (heap_size - new_size);
            if (last_free_end != m_end_address || free_size < delta_space) { return heap_size; }

            /* Set end address and unlink free block if it matches the shrinkage (effectively AdjustHeap) */
            if (delta_space == free_size) {
                last_free_block.exp_list_node.Unlink();
                m_end_address = reinterpret_cast<void*>(std::addressof(last_free_block));
                return new_size;
            }

            /* Set end address and add resized free block */
            last_free_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(last_free_end) - delta_space);
            this->AddFreeBlock({ reinterpret_cast<void*>(std::addressof(last_free_block)), last_free_end });
            m_end_address = last_free_end;

            return new_size;
        }

        /* Find start of new free block */
        void *last_free_start = m_end_address;
        if (last_free_end == m_end_address) {
            last_free_start = reinterpret_cast<void*>(std::addressof(last_free_block));
        }

        /* Calculate new end */
        void *new_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_start_address) + new_size);

        /* If we are not the root heap the memory must come from a parent heap (not implicit) */
        if (this != mem::GetRootHeap(0)) {

            /* Check we have a parent heap */
            if (m_parent_heap == nullptr) { return heap_size; }

            /* Check whether the start region is a dedicated allocation */
            if (m_parent_heap->IsAddressAllocation(m_start_address) == false) { return heap_size; }

            /* Try to adjust the parent allocation */
            const size_t adjust_size = m_parent_heap->AdjustAllocation(m_start_address, new_size);
            if (adjust_size != new_size) { return heap_size; }
        }

        /* Add new free block */
        this->AddFreeBlock( { last_free_start, new_end } );

        /* Set end address */
        m_end_address = new_end;

        return new_size;
    }

    bool ExpHeap::IsAddressAllocation(void *address) {

        /* Lock heap */
        ScopedHeapLock lock(this);

        /* Use unsafe method safely */
        return this->IsAddressAllocationUnsafe(address);
    }
    bool ExpHeap::IsAddressAllocationUnsafe(void *address) {

        /* Get block */
        ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));

        /* Search for the block */
        for (const ExpHeapMemoryBlock &used_block : m_allocated_block_list) {
            if (std::addressof(used_block) == block) { return true; }
        }

        return false;
    }

    size_t ExpHeap::AdjustAllocation(void *address, size_t new_size) {

        /* Align new size */
        new_size = vp::util::AlignUp(new_size, MinimumAllocationGranularity);

        /* Lock the heap */
        ScopedHeapLock lock(this);

        /* Find ExpHeapMemory block */
        ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));

        /* Nothing to do if the size doesn't change */
        if (block->block_size == new_size) {
            return new_size;
        }

        /* Reduce range if new size is lesser */
        if (new_size < block->block_size) {
            this->AddFreeBlock(AddressRange{reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + new_size), reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + block->block_size)});
            block->block_size = new_size;
            return new_size;
        }

        /* Find end of block */
        const uintptr_t end_address = reinterpret_cast<uintptr_t>(address) + block->block_size;

        /* Walk free list for the free block directly after this allocation */
        FreeList::iterator free_block = m_free_block_list.begin();
        while (free_block != m_free_block_list.end()) {

            /* Complete if the block is directly after our current free block */
            if (reinterpret_cast<uintptr_t>(std::addressof((*free_block))) == end_address) {
                break;
            }

            free_block = ++free_block;
        }

        /* Ensure we found the block after and it is large enough */
        ExpHeapMemoryBlock *block_after = std::addressof((*free_block));
        const size_t after_size = block_after->block_size + sizeof(ExpHeapMemoryBlock);
        if (free_block != m_free_block_list.end() && after_size + block->block_size > new_size) { return block->block_size; }

        /* Remove after block from free list */
        block_after->exp_list_node.Unlink();

        /* Find new free node address and size */
        const uintptr_t     new_end_address = reinterpret_cast<uintptr_t>(block_after) + after_size;
        ExpHeapMemoryBlock *new_free        = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) + new_size + sizeof(ExpHeapMemoryBlock));
        const size_t        new_free_size   = new_end_address - reinterpret_cast<uintptr_t>(new_free);

        /* Adjust allocation */
        if (sizeof(ExpHeapMemoryBlock) <= new_free_size) {
            new_free = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(new_free) - new_free_size);
        }
        block->block_size = reinterpret_cast<uintptr_t>(new_free) - reinterpret_cast<uintptr_t>(address);

        if (sizeof(ExpHeapMemoryBlock) <= new_free_size) {
            std::construct_at(new_free);

            new_free->alloc_magic = ExpHeapMemoryBlock::FreeMagic;
            new_free->alignment   = 0;
            new_free->block_size  = new_free_size;
            m_free_block_list.PushBack(*new_free);
        }

        return new_size;
    }

    void *ExpHeap::TryAllocate(size_t size, s32 alignment) {

        /* Enforce allocation limits */
        if (alignment < MinimumAlignment) {
            alignment = MinimumAlignment;
        }

        /* Lock heap */
        ScopedHeapLock lock(this);

        /* Label for out of memory restart */
        _ExpHeap_OutOfMemoryRestart:

        /* Existing free block check */
        if (m_free_block_list.IsEmpty() == true) {
            
            /* Attempt out of memory callback */
            OutOfMemoryInfo out_of_memory = {
                .out_of_memory_heap      = this,
                .allocation_size         = size,
                .aligned_allocation_size = vp::util::AlignUp(size, alignment),
                .alignment               = alignment,
            };
            const bool result = mem::OutOfMemoryImpl(std::addressof(out_of_memory));
            if (result == true) { goto _ExpHeap_OutOfMemoryRestart; }

            return nullptr;
        }

        /* Calculate adjusted size */
        const size_t aligned_size = (size != Heap::cWholeSize) ? vp::util::AlignUp(size, alignment) : this->GetMaximumAllocatableSize(alignment);

        /* Find a free memory block that corresponds to our desired size */
        uintptr_t allocation_address = 0;
        FreeList::iterator free_block = m_free_block_list.begin();

        /* First fit mode */
        if (m_allocation_mode == AllocationMode::FirstFit) {

            /* Walk the free list */
            while (free_block != m_free_block_list.end()) {

                /* Get block */
                ExpHeapMemoryBlock *block = std::addressof(*free_block);

                /* Find the starting address for the allocation */
                allocation_address = vp::util::AlignUp(reinterpret_cast<uintptr_t>(block) + sizeof(ExpHeapMemoryBlock), alignment);

                /* Find the total size of the allocation with alignment */
                const uintptr_t aligned_allocation_size = (allocation_address + aligned_size) - reinterpret_cast<uintptr_t>(block);

                /* Complete if the allocated size is within our free block's range */
                if (aligned_allocation_size <= block->block_size) { break; }

                free_block = ++free_block;
            }
        } else {

            /* Best fit mode */
            size_t    smaller_size    = 0xffff'ffff'ffff'ffff;
            uintptr_t smaller_address = 0;
            ExpHeapMemoryBlock *smaller_block = std::addressof(*m_free_block_list.end());

            /* Walk the free list */
            while (free_block != m_free_block_list.end()) {

                /* Get block */
                ExpHeapMemoryBlock *block = std::addressof(*free_block);

                /* Find the starting address for the allocation */
                allocation_address = vp::util::AlignUp(reinterpret_cast<uintptr_t>(block) + sizeof(ExpHeapMemoryBlock), alignment);

                /* Find the total size of the allocation with alignment */
                const uintptr_t aligned_allocation_size = (aligned_size - sizeof(ExpHeapMemoryBlock)) - reinterpret_cast<uintptr_t>(block) + allocation_address;

                /* Check if we've found a smaller block that fits */
                if (block->block_size < smaller_size && aligned_allocation_size <= block->block_size) {
                    smaller_size    = block->block_size;
                    smaller_block   = block;
                    smaller_address = allocation_address;

                    /* Break if our size requirements are exact */
                    if (block->block_size == aligned_size) { break; }
                }

                free_block = ++free_block;
                allocation_address = smaller_address;
            }

            free_block = m_free_block_list.IteratorTo(*smaller_block);
        }

        /* Ensure we found a block */
        if ((free_block != m_free_block_list.end()) == false) {

            /* Attempt out of memory callback */
            OutOfMemoryInfo out_of_memory = {
                .out_of_memory_heap      = this,
                .allocation_size         = size,
                .aligned_allocation_size = aligned_size,
                .alignment               = alignment,
            };
            const bool result = mem::OutOfMemoryImpl(std::addressof(out_of_memory));
            if (result == true) { goto _ExpHeap_OutOfMemoryRestart; }

            return nullptr;
        }

        /* Convert our new allocation to a used block */
        this->AddUsedBlock(std::addressof(*free_block), allocation_address, aligned_size);

        return reinterpret_cast<void*>(allocation_address);
    }

    void ExpHeap::Free(void *address) {
        ScopedHeapLock lock(this);

        /* Unlink used block */
        ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));
        block->exp_list_node.Unlink();

        /* Add back to free list */
        void *start = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(block) - block->alignment);
        void *end   = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + block->block_size);
        this->AddFreeBlock(AddressRange{start, end});
    }

    size_t ExpHeap::GetTotalFreeSize() {
        ScopedHeapLock lock(this);

        /* Sum free block sizes */
        size_t free_size = 0;
        for (const ExpHeapMemoryBlock &block : m_free_block_list) {
            free_size += block.block_size;
        }

        return free_size;
    }

    size_t ExpHeap::GetMaximumAllocatableSize(s32 alignment) {

        /* Lock the heap */
        ScopedHeapLock lock(this);

        /* Find largest contiguous free block while respecting alignment */
        size_t max_free_size = 0;
        for (const ExpHeapMemoryBlock &block : m_free_block_list) {

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

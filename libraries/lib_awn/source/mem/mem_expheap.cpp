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

     ExpHeapMemoryBlock *ExpHeap::GetBlock(void *address) {
        const uintptr_t *walkback = reinterpret_cast<const uintptr_t*>(reinterpret_cast<uintptr_t>(address) - sizeof(size_t));
        ExpHeapMemoryBlock *ret = (*walkback & 1) ? reinterpret_cast<ExpHeapMemoryBlock*>(*walkback & (~1)) : reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));
        return ret;
    }
    const ExpHeapMemoryBlock *ExpHeap::GetBlock(const void *address) {
        const uintptr_t *walkback = reinterpret_cast<const uintptr_t*>(reinterpret_cast<uintptr_t>(address) - sizeof(size_t));
        const ExpHeapMemoryBlock *ret = (*walkback & 1) ? reinterpret_cast<const ExpHeapMemoryBlock*>(*walkback & (~1)) : reinterpret_cast<const ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));
        return ret;
    }

    void ExpHeap::AddUsedBlockNoAlign(ExpHeapMemoryBlock *block, size_t size) {

        /* Convert to used */
        const size_t prev_size = block->size;
        block->size = size;
        block->magic = ExpHeapMemoryBlock::cUsedMagic;
        vp::util::IntrusiveListNode *free_node = block->list_node.next();
        block->list_node.Unlink();
        --m_free_count;
        m_used_list.PushBack(*block);
        ++m_used_count;

        /* Insert new free block */
        const size_t free_left = prev_size - size;
        if (sizeof(ExpHeapMemoryBlock) < free_left) {
            ExpHeapMemoryBlock *new_block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(block->GetBaseAddress()) + size);
            std::construct_at(new_block);
            new_block->magic = ExpHeapMemoryBlock::cFreeMagic;
            new_block->size = free_left - sizeof(ExpHeapMemoryBlock);
            free_node->LinkPrev(std::addressof(new_block->list_node));
            ++m_free_count;
        } else if (free_left != 0) {
            block->size = prev_size;
        }

        return;
    }

    ExpHeapMemoryBlock *ExpHeap::AddUsedBlockAlign(ExpHeapMemoryBlock *block, size_t size, s32 align) {

        /* Convert to used */
        const size_t prev_size = block->size;
        block->size = size;
        vp::util::IntrusiveListNode *free_node = block->list_node.next();
        const uintptr_t base = reinterpret_cast<uintptr_t>(block->GetBaseAddress());
        const uintptr_t align_left = vp::util::AlignUp(base, align) - base;
        if (align_left < vp::util::c64KB) {
            block->magic = ExpHeapMemoryBlock::cUsedMagic;
            block->alignment = static_cast<u16>(align_left);
            block->list_node.Unlink();
            --m_free_count;
            block->size = size;
            if (align_left != 0) {
                uintptr_t *walkback = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(block->GetBaseAddress()) - alignof(ExpHeapMemoryBlock*));
                *walkback = reinterpret_cast<uintptr_t>(block) | 1;
            }
        } else {
            /* Split free block for large align */
            block->size = align_left - sizeof(ExpHeapMemoryBlock);
            block->alignment = 0;

            block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(block) + align_left);
            std::construct_at(block);
            block->magic = ExpHeapMemoryBlock::cUsedMagic;
            block->alignment = 0;
            block->size = size;
        }

        m_used_list.PushBack(*block);
        ++m_used_count;

        /* Insert new free block */
        const size_t free_left = prev_size - (size + align_left);
        if (sizeof(ExpHeapMemoryBlock) < free_left) {
            ExpHeapMemoryBlock *new_block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(block->GetBaseAddress()) + size);
            std::construct_at(new_block);
            new_block->magic = ExpHeapMemoryBlock::cFreeMagic;
            new_block->size = free_left - sizeof(ExpHeapMemoryBlock);
            free_node->LinkPrev(std::addressof(new_block->list_node));
            ++m_free_count;
        } else if (free_left != 0) {
            block->size = free_left + size;
        }

        return block;
    }

    bool ExpHeap::FindFreeBlockRangeFreeList(ExpHeapMemoryBlock **out_before, ExpHeapMemoryBlock **out_after, ExpHeapMemoryBlock *block) {

        /* Walk free list */
        BlockList::iterator iter = m_free_list.begin();
        BlockList::iterator end = m_free_list.end();
        if (iter == end) { return false; }
        ExpHeapMemoryBlock *before = nullptr;
        ExpHeapMemoryBlock *after = std::addressof(*iter);
        while (after <= block) {
            before = after;
            ++iter;
            after = std::addressof(*iter);
            if (iter == end) { return false; }
        }

        *out_before = before;
        *out_after = after;
        return true;
    }

    bool ExpHeap::FindFreeBlockRangeIterate(ExpHeapMemoryBlock **out_before, ExpHeapMemoryBlock **out_after, ExpHeapMemoryBlock *block) {
        /* Walk contiguous blocks until free block */
        ExpHeapMemoryBlock *after = reinterpret_cast<ExpHeapMemoryBlock*>(block->GetEndAddress());
        for (; after < m_end_address; after = reinterpret_cast<ExpHeapMemoryBlock*>(after->GetEndAddress())) {
            if (after->magic != ExpHeapMemoryBlock::cFreeMagic) { continue; }
            ExpHeapMemoryBlock *before = BlockListTraits::GetParent(after->list_node.prev());
            *out_before = (m_free_list.IteratorTo(*before) == m_free_list.end()) ? nullptr : before;
            *out_after = after;
            return true;
        }
        return false;
    }

    ExpHeapMemoryBlock *ExpHeap::AddFreeBlock(ExpHeapMemoryBlock *block) {

        /* Find free blocks between block */
        ExpHeapMemoryBlock *before;
        ExpHeapMemoryBlock *after;
        const bool mode = (m_used_count + 3 < m_free_count * 3);
        const bool is_range = (mode) ? this->FindFreeBlockRangeIterate(std::addressof(before), std::addressof(after), block) : this->FindFreeBlockRangeFreeList(std::addressof(before), std::addressof(after), block);

        /* Failed to find a range */
        if (!is_range) {
            /* Coalesce end block */
            if (!m_free_list.IsEmpty()) {
                ExpHeapMemoryBlock *back = std::addressof(m_free_list.Back());
                if (back->GetEndAddress() == block) {
                    back->size += sizeof(ExpHeapMemoryBlock) + block->alignment + block->size;
                    return back;
                }
            }
            block->magic = ExpHeapMemoryBlock::cFreeMagic;
            m_free_list.PushBack(*block);
            ++m_free_count;
            return block;
        }

        /* Coalesce before */
        if (before != nullptr && before->GetEndAddress() == block) {
            before->size += sizeof(ExpHeapMemoryBlock) + block->alignment + block->size;
            block = before;
        } else {
            block->magic = ExpHeapMemoryBlock::cFreeMagic;
            after->list_node.LinkPrev(std::addressof(block->list_node));
            ++m_free_count;
        }
        /* Coalesce after */
        if (after == block->GetEndAddress()) {
            block->size += sizeof(ExpHeapMemoryBlock) + after->alignment + after->size;
            after->list_node.Unlink();
            --m_free_count;
        }

        return block;
    }

    ExpHeapMemoryBlock *ExpHeap::FindFreeBlockFirstFit(size_t size, s32 align) {

        /* Find first fitting block */
        BlockList::iterator iter = m_free_list.begin();
        BlockList::iterator end = m_free_list.end();
        while (iter != end) {
            ExpHeapMemoryBlock *block = std::addressof(*iter);
            const size_t free_size = block->size;
            if (size <= free_size) {
                const uintptr_t addr_base = reinterpret_cast<uintptr_t>(block) + sizeof(ExpHeapMemoryBlock) + block->alignment;
                const uintptr_t align_size = (size - addr_base) + vp::util::AlignUp(addr_base, align);
                if (align_size <= free_size) { return block; }
            }
            ++iter;
        }

        return nullptr;
    }

    ExpHeapMemoryBlock *ExpHeap::FindFreeBlockBestFit(size_t size, s32 align) {

        /* Find best fitting block */
        BlockList::iterator iter = m_free_list.begin();
        BlockList::iterator end = m_free_list.end();
        ExpHeapMemoryBlock *max_block = nullptr;
        size_t max_size = 0;
        while (iter != end) {
            ExpHeapMemoryBlock *block = std::addressof(*iter);
            ++iter;
            const size_t free_size = block->size;
            if (size <= free_size) {
                const uintptr_t addr_base = reinterpret_cast<uintptr_t>(block) + sizeof(ExpHeapMemoryBlock) + block->alignment;
                const uintptr_t align_size = (size - addr_base) + vp::util::AlignUp(addr_base, align);
                if (align_size <= free_size) {
                    max_block = (max_block != nullptr && max_size <= free_size) ? max_block : block;
                    max_size = max_block->size;
                }
            }
        }

        return max_block;
    }

    ExpHeapMemoryBlock *ExpHeap::FindFreeBlockWorstFit(size_t size, s32 align) {

        /* Find worst fitting block */
        BlockList::iterator iter = m_free_list.begin();
        BlockList::iterator end = m_free_list.end();
        ExpHeapMemoryBlock *max_block = nullptr;
        size_t max_size = 0;
        while (iter != end) {
            ExpHeapMemoryBlock *block = std::addressof(*iter);
            ++iter;
            const size_t free_size = block->size;
            if (size <= free_size) {
                const uintptr_t addr_base = reinterpret_cast<uintptr_t>(block) + sizeof(ExpHeapMemoryBlock) + block->alignment;
                const uintptr_t align_size = (size - addr_base) + vp::util::AlignUp(addr_base, align);
                if (align_size <= free_size) {
                    max_block = (max_block != nullptr && free_size <= max_size) ? max_block : block;
                    max_size = max_block->size;
                }
            }
        }

        return max_block;
    }

    ExpHeapMemoryBlock *ExpHeap::FindFreeBlock(size_t size, s32 align, AllocationMode mode) {
        switch (mode) {
            case AllocationMode::FirstFit:
                return this->FindFreeBlockFirstFit(size, align);
            default:
            case AllocationMode::BestFit:
                return this->FindFreeBlockBestFit(size, align);
            case AllocationMode::WorstFit:
                return this->FindFreeBlockWorstFit(size, align);
        };
    }

    ExpHeapMemoryBlock *ExpHeap::FindFreeBlockFirstFit(size_t size) {

        /* Find first fitting block */
        BlockList::iterator iter = m_free_list.begin();
        BlockList::iterator end = m_free_list.end();
        while (iter != end) {
            ExpHeapMemoryBlock *block = std::addressof(*iter);
            const size_t free_size = block->size;
            if (size <= free_size) { return block; }
            ++iter;
        }

        return nullptr;
    }

    ExpHeapMemoryBlock *ExpHeap::FindFreeBlockBestFit(size_t size) {

        /* Find best fitting block */
        BlockList::iterator iter = m_free_list.begin();
        BlockList::iterator end = m_free_list.end();
        ExpHeapMemoryBlock *max_block = nullptr;
        size_t max_size = 0;
        while (iter != end) {
            ExpHeapMemoryBlock *block = std::addressof(*iter);
            ++iter;
            const size_t free_size = block->size;
            if (size <= free_size) {
                max_block = (max_block != nullptr && max_size <= free_size) ? max_block : block;
                max_size = max_block->size;
            }
        }

        return max_block;
    }

    ExpHeapMemoryBlock *ExpHeap::FindFreeBlockWorstFit(size_t size) {

        /* Find worst fitting block */
        BlockList::iterator iter = m_free_list.begin();
        BlockList::iterator end = m_free_list.end();
        ExpHeapMemoryBlock *max_block = nullptr;
        size_t max_size = 0;
        while (iter != end) {
            ExpHeapMemoryBlock *block = std::addressof(*iter);
            ++iter;
            const size_t free_size = block->size;
            if (size <= free_size) {
                max_block = (max_block != nullptr && free_size <= max_size) ? max_block : block;
                max_size = max_block->size;
            }
        }

        return max_block;
    }

    ExpHeapMemoryBlock *ExpHeap::FindFreeBlock(size_t size, AllocationMode mode) {
        switch (mode) {
            case AllocationMode::FirstFit:
                return this->FindFreeBlockFirstFit(size);
            default:
            case AllocationMode::BestFit:
                return this->FindFreeBlockBestFit(size);
            case AllocationMode::WorstFit:
                return this->FindFreeBlockWorstFit(size);
        };
    }

    ExpHeapMemoryBlock *ExpHeap::GetEndFreeBlock() {
        if (m_free_count < 1) { return nullptr; }
        m_used_list.SortByAddress();
        ExpHeapMemoryBlock *free = std::addressof(m_free_list.Back());
        ExpHeapMemoryBlock *used = std::addressof(m_used_list.Back());
        ExpHeapMemoryBlock *ret = (used < free) ? free : nullptr;
        return ret;
    }

    ExpHeap *ExpHeap::TryCreate(const char *name, void *address, size_t size, bool is_thread_safe) {

        /* Integrity checks */
        VP_ASSERT(address != nullptr && (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock)) <= size);

        /* Contruct exp heap object */
        ExpHeap *new_heap = reinterpret_cast<ExpHeap*>(address);
        std::construct_at(new_heap, name, nullptr, address, vp::util::AlignDown(size, alignof(size_t)), is_thread_safe);

        /* Create and push free node spanning block */
        ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) + sizeof(ExpHeap));
        std::construct_at(block);
        block->magic = ExpHeapMemoryBlock::cFreeMagic;
        block->alignment   = 0;
        block->size  = size - (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock));

        new_heap->m_free_list.PushBack(*block);
        new_heap->m_free_count = 1;

        return new_heap;
    }

    ExpHeap *ExpHeap::TryCreate(const char *name, Heap *parent_heap, size_t size, s32 alignment, bool is_thread_safe) {

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

        /* Enforce minimum size */
        if (size < (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock))) { return nullptr; }
        if (!vp::util::IsAligned(alignment, alignment)) { return nullptr; }

        /* Allocate heap memory from parent heap */
        void *address = parent_heap->TryAllocate(size, alignment);
        if (address == nullptr) { return nullptr; }

        /* Construct new heap */
        ExpHeap *new_heap = reinterpret_cast<ExpHeap*>(address);
        std::construct_at(new_heap, name, parent_heap, address, size, is_thread_safe);

        /* Add free block spanning memory */
        ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(new_heap) + sizeof(ExpHeap));
        std::construct_at(block);
        block->magic = ExpHeapMemoryBlock::cFreeMagic;
        block->size = size - (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock));
        block->alignment = 0;

        new_heap->m_free_list.PushBack(*block);
        new_heap->m_free_count = 1;

        /* Add to parent heap child list */
        parent_heap->PushBackChild(new_heap);

        return new_heap;
    }

    void ExpHeap::Finalize() {/*...*/}

    MemoryRange ExpHeap::AdjustHeap() {

        /* Ensure parent heap */
        if (m_parent_heap == nullptr) {
            return { m_end_address, 0, };
        }

        /* Lock heap */
        ScopedHeapLock l0(this);
        ScopedHeapLock l1(reinterpret_cast<mem::Heap*>(m_parent_heap));

        /* Ensure we have a free block spanning to the end address */
        ExpHeapMemoryBlock *block = this->GetEndFreeBlock();
        if (block == nullptr) {
            return { m_end_address, 0, };
        }

        /* Adjust allocation */
        block->list_node.Unlink();
        const size_t b_size = block->size + + block->alignment + sizeof(ExpHeapMemoryBlock);
        const size_t trim_size = reinterpret_cast<uintptr_t>(block) - reinterpret_cast<uintptr_t>(m_start_address);
        const size_t new_size = m_parent_heap->AdjustAllocation(m_start_address, trim_size);
        /* Ensure we shrunk */
        if (new_size == 0) {
            m_free_list.PushBack(*block);
            return { m_end_address, 0, };
        }
        --m_free_count;

        /* Calculate new end address */
        m_end_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_start_address) + new_size);
        return { m_end_address, b_size, };
    }

    bool ExpHeap::IsAddressAllocation(const void *address) {
        ScopedHeapLock lock(this);
        return this->IsAddressAllocationUnsafe(address);
    }

    bool ExpHeap::IsAddressAllocationUnsafe(const void *address) const {

        /* Search if address is used block */
        const ExpHeapMemoryBlock *block = this->GetBlock(address);
        for (const ExpHeapMemoryBlock &used_block : m_used_list) {
            if (std::addressof(used_block) == block) { return true; }
        }

        return false;
    }

    size_t ExpHeap::AdjustAllocation(void *address, size_t adjust_size) {

        /* Align new size */
        VP_ASSERT(this->IsAddressInHeap(address));

        /* Lock the heap */
        ScopedHeapLock lock(this);

        /* Calc size diff */
        ExpHeapMemoryBlock *block = this->GetBlock(address);
        size_t block_size = block->size;
        size_t new_size = vp::util::AlignUp(adjust_size, cMinimumAllocationGranularity);
        size_t diff_size = block_size - new_size;
        /* Fail if the size grew */
        if (block_size < new_size) { return 0; }
        /* No adjust for too small diff */
        if (diff_size <= sizeof(ExpHeapMemoryBlock)) { return 0; }
        /* Adjust block and add new free */
        block->size = new_size;
        ExpHeapMemoryBlock *new_block = reinterpret_cast<ExpHeapMemoryBlock*>(block->GetEndAddress());
        std::construct_at(new_block);
        new_block->size = diff_size - sizeof(ExpHeapMemoryBlock);
        this->AddFreeBlock(new_block);

        return new_size;
    }

    void *ExpHeap::TryAllocate(size_t size, s32 alignment) {

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

        /* Size granularity */
        const size_t gran_size = vp::util::AlignUp(vp::util::Max(size, cMinimumAllocationGranularity), cMinimumAllocationGranularity);
        /* Lock heap */
        ScopedHeapLock lock(this);
        /* Find free memory, convert to used */
        ExpHeapMemoryBlock *block = nullptr;
        if (alignment <= static_cast<s32>(alignof(size_t))) {
            /* Strip alignment management for pointer aligned (or less) */
            auto alloc_no_align_lambda = [&]() -> ExpHeapMemoryBlock* {
                ExpHeapMemoryBlock *block = this->FindFreeBlock(gran_size, m_allocation_mode);
                if (block == nullptr) { return block; }
                this->AddUsedBlockNoAlign(block, gran_size);
                return block;
            };
            block = alloc_no_align_lambda();
        } else {
            auto alloc_align_lambda = [&]() -> ExpHeapMemoryBlock* {
                ExpHeapMemoryBlock *fblock = this->FindFreeBlock(gran_size, alignment, m_allocation_mode);
                if (fblock == nullptr) { return block; }
                ExpHeapMemoryBlock *ublock = this->AddUsedBlockAlign(fblock, gran_size, alignment);
                return ublock;
            };
            block = alloc_align_lambda();
        }
        if (block == nullptr) { return nullptr; }

        fail_guard.Cancel();
        return block->GetBaseAddress();
    }

    void *ExpHeap::TryReallocate(void *address, size_t size, s32 alignment) {

        /* Allocate for no address */
        if (address == nullptr) {
            return this->TryAllocate(size, alignment);
        }
        /* Free for new size 0 */
        if (size == 0) {
            this->Free(address);
            return nullptr;
        }
        /* Ensure address is in heap */
        if (!this->IsAddressInHeap(address)) { return nullptr; }
        
        ScopedHeapLock l(this);
        
        ExpHeapMemoryBlock *block = this->GetBlock(address);
        const size_t gran_size = vp::util::AlignUp(size, cMinimumAllocationGranularity);
        auto copy_alloc_lambda = [&](size_t new_size, s32 new_align) -> void* {
            void *new_addr = this->TryAllocate(new_size, new_align);
            if (new_addr == nullptr) { return nullptr; }
            ::memcpy(new_addr, block->GetBaseAddress(), block->size);
            this->Free(address);
            return new_addr;
        };
        const size_t diff = (block->size - gran_size);
        if (block->size < gran_size) {
            /* Reallocate larger block */
            const s32 align = vp::util::Max(alignment, static_cast<s32>(alignof(size_t)));
            return copy_alloc_lambda(gran_size, align);
        } else if (diff == 0) {
            /* Avoid realloc for aligned */
            if (alignment == 0 || vp::util::IsAligned(address, alignment)) { return block->GetBaseAddress(); }
        } else if (alignment == 0 || vp::util::IsAligned(address, alignment)) {
            /* Shrink block if aligned and space */
            if (sizeof(ExpHeapMemoryBlock) < diff) {
                block->size = gran_size;
                ExpHeapMemoryBlock *new_block = reinterpret_cast<ExpHeapMemoryBlock*>(block->GetEndAddress());
                std::construct_at(new_block);
                new_block->size = diff - sizeof(ExpHeapMemoryBlock);
                this->AddFreeBlock(new_block);
            }
            return address;
        }
        /* Realloc for unaligned smaller or same */
        return copy_alloc_lambda(gran_size, alignment);
    }

    void ExpHeap::Free(void *address) {

        /* Ensure valid address */
        VP_ASSERT(address != nullptr);
        VP_ASSERT(this->IsAddressInHeap(address));

        /* Lock heap */
        ScopedHeapLock lock(this);

        /* Unlink used block */
        ExpHeapMemoryBlock *block = this->GetBlock(address);
        VP_ASSERT(block->magic == ExpHeapMemoryBlock::cUsedMagic);
        block->list_node.Unlink();
        --m_used_count;
        block->size += block->alignment;
        block->alignment = 0;
        this->AddFreeBlock(block);

        return;
    }

    size_t ExpHeap::GetTotalFreeSize() {

        /* Sum free block sizes */
        ScopedHeapLock lock(this);
        size_t free_size = 0;
        for (const ExpHeapMemoryBlock &block : m_free_list) {
            free_size += block.size;
        }

        return free_size;
    }

    size_t ExpHeap::GetMaximumAllocatableSize(s32 alignment) {
        ScopedHeapLock lock(this);
        return this->GetMaximumAllocatableSizeUnsafe(alignment);
    }

    size_t ExpHeap::GetMaximumAllocatableSizeUnsafe(s32 alignment) {

        /* Find largest contiguous free block while respecting alignment */
        if (!vp::util::IsAligned(alignment, alignment)) { return 0; }
        size_t max_free_size = 0;
        const ExpHeapMemoryBlock *block;
        if (alignment <= static_cast<s32>(alignof(size_t))) {
            block = this->FindFreeBlock(cMinimumAllocationGranularity, AllocationMode::WorstFit);
        } else {
            block = this->FindFreeBlock(cMinimumAllocationGranularity, alignment, AllocationMode::WorstFit);
        }
        max_free_size = (block != nullptr) ? block->size : 0;

        return max_free_size;
    }

    size_t ExpHeap::GetAllocationSize(void *address) {
        const ExpHeapMemoryBlock *block = ExpHeap::GetBlock(address);
        return block->size;
    }
}

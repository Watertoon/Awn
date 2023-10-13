#pragma once

namespace awn::mem {

	class VirtualAddressHeap : public Heap {
        public:
            static constexpr size_t cSmallMemoryRegionSize        = vp::util::c64KB;
            static constexpr size_t cSmallMemoryPageMaskBitCount  = cSmallMemoryRegionSize >> 0xc;
            static constexpr size_t cMaxPageMask                  = 0xffff;
            static constexpr size_t cMaxAllocationsPerSmallMemory = 8;
            static constexpr size_t cMinimumSize                  = 8;
            static constexpr size_t cMinimumAlignment             = 8;
        public:
            using PageMask = u16;
            static_assert((sizeof(PageMask) * 8) >= cSmallMemoryPageMaskBitCount);
		private:
            struct VirtualHeapSmallMemoryBlock {
                vp::util::IntrusiveListNode list_node;
                PageMask                    page_mask;
                u8                          allocation_size_array[cMaxAllocationsPerSmallMemory];
            };
            struct VirtualHeapLargeMemoryBlock {
                vp::util::IntrusiveRedBlackTreeNode<uintptr_t> tree_node;
                size_t                                         memory_size;
            };
        public:
            using SmallMemoryList = vp::util::IntrusiveListTraits<VirtualHeapSmallMemoryBlock, &VirtualHeapSmallMemoryBlock::list_node>::List;
            using LargeMemoryMap  = vp::util::IntrusiveRedBlackTreeTraits<VirtualHeapLargeMemoryBlock, &VirtualHeapLargeMemoryBlock::tree_node>::Tree;
        private:
            SmallMemoryList m_free_small_memory_list;
            SmallMemoryList m_filled_small_memory_list;
            LargeMemoryMap  m_large_memory_map;
        public:
            constexpr  VirtualAddressHeap(const char *name, void *start, size_t size) : Heap(name, nullptr, start, size, true) {

                /* Create a small memory block */
                VirtualHeapSmallMemoryBlock *first_block = reinterpret_cast<VirtualHeapSmallMemoryBlock*>(start);
                std::construct_at(first_block);
                first_block->page_mask                = 1;
                
                m_free_small_memory_list.PushBack(*first_block);

                return;
            }
            virtual constexpr ~VirtualAddressHeap() {/*...*/}

            static VirtualAddressHeap *Create(const char *name) {
                
                /* Query memory size */
                MEMORYSTATUSEX memory_status = { .dwLength = sizeof(MEMORYSTATUSEX) };
                const bool result = ::GlobalMemoryStatusEx(std::addressof(memory_status));
                VP_ASSERT(result == true);
                VP_ASSERT(cSmallMemoryRegionSize <= memory_status.ullAvailPhys);

                /* Allocate virtual memory */
                void *reserve = ::VirtualAlloc(nullptr, cSmallMemoryRegionSize, MEM_RESERVE, PAGE_READWRITE);
                VP_ASSERT(reserve != nullptr);
                void *commit = ::VirtualAlloc(reserve, vp::util::c4KB, MEM_COMMIT, PAGE_READWRITE);
                VP_ASSERT(commit != nullptr);
    
                /* Construct VirtualAddressHeap */
                VirtualAddressHeap *heap = reinterpret_cast<VirtualAddressHeap*>(reinterpret_cast<uintptr_t>(commit) + sizeof(VirtualHeapSmallMemoryBlock));
                std::construct_at(heap, name, commit, memory_status.ullAvailPhys);

                return heap;
            }

            virtual void *TryAllocate(size_t size, s32 alignment) override {

                /* Align memory */
                if (size < cMinimumSize) { size = cMinimumSize; }
                if (alignment < cMinimumAlignment) { alignment = cMinimumAlignment; }

                /* Find aligned size TODO; fix alignment for all cases */
                const size_t address_base_offset    = vp::util::AlignUp(sizeof(VirtualHeapSmallMemoryBlock), alignment);
                const size_t small_page_count       = size >> 0xc;
                const size_t small_page_count_first = (size + (vp::util::c4KB - 1) + address_base_offset) >> 0xc;

                /* Handle as Large memory if over 64 KB or over a page alignment */
                if (cSmallMemoryPageMaskBitCount <= small_page_count || vp::util::c4KB < alignment) {

                    /* Allocate address space and memory pages for Large memory */
                    const size_t final_size = vp::util::AlignUp(size + sizeof(VirtualHeapLargeMemoryBlock), vp::util::c4KB);
                    void *new_address = ::VirtualAlloc(nullptr, final_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                    VP_ASSERT(new_address != nullptr);

                    /* Contruct large memory node at back of allocation */
                    VirtualHeapLargeMemoryBlock *new_block = reinterpret_cast<VirtualHeapLargeMemoryBlock*>(reinterpret_cast<uintptr_t>(new_address) + final_size - sizeof(VirtualHeapLargeMemoryBlock));
                    std::construct_at(new_block);
                    new_block->tree_node.SetKey(reinterpret_cast<uintptr_t>(new_address));
                    new_block->memory_size = final_size;

                    /* Insert large memory end into tree map */
                    {
                        ScopedHeapLock l(this);
                        m_large_memory_map.Insert(new_block);
                    }

                    /* Adjust start and end addresses lockless */
                    void *start = m_start_address;
                    while (new_address < start) {
                        vp::util::InterlockedCompareExchange(std::addressof(m_start_address), new_address, start);
                        start = m_start_address;
                    }
                    void *new_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(new_address) + final_size);
                    void *end     = m_end_address;
                    while (end < new_end) {
                        vp::util::InterlockedCompareExchange(std::addressof(m_end_address), new_end, end);
                        end = m_end_address;
                    }

                    return new_address;
                }

                /* Try to find a free small memory block */
                ScopedHeapLock l(this);
                for (VirtualHeapSmallMemoryBlock &small_memory_block : m_free_small_memory_list) {

                    /* Check first range */
                    PageMask page_mask  = small_memory_block.page_mask;
                    u32 free_page_count = vp::util::CountRightZeroBits32(page_mask) & 0xf;

                    /* Handle a first time allocation */
                    if (page_mask == 0 || small_page_count_first <= free_page_count) {

                        /* Commit uncomitted regions */
                        if (1 < small_page_count_first) {
                            void *commit = ::VirtualAlloc(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(small_memory_block)) | vp::util::c4KB), (small_page_count - 1) << 0xc, MEM_COMMIT, PAGE_READWRITE);
                            VP_ASSERT(commit != nullptr);
                        }

                        /* Update page mask for odd or even*/
                        small_memory_block.page_mask                = (address_base_offset < vp::util::c4KB) ? ~(-1 << small_page_count_first) : (~(-1 << (small_page_count_first - 1)) << 1);
                        const u8 last_alloc_size                    = small_memory_block.allocation_size_array[0];
                        small_memory_block.allocation_size_array[0] = (address_base_offset < vp::util::c4KB) ? (last_alloc_size & 0xf0) | small_page_count_first : (last_alloc_size & 0xf) | ((small_page_count_first - 1) << 0x4);

                        /* Swap to filled list if necessary */
                        if (small_memory_block.page_mask == cMaxPageMask) {
                            m_free_small_memory_list.Remove(small_memory_block);
                            m_filled_small_memory_list.PushBack(small_memory_block);
                        }

                        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(small_memory_block)) + address_base_offset);
                    }

                    /* Find free range */
                    u32 i = free_page_count + 1;
                    while (i < (cSmallMemoryPageMaskBitCount - 1)) {

                        /* Iterate bits until next free range */
                        const u32 mask_offset = (page_mask >> i);
                        if ((mask_offset & 1) != 0) {
                            i = i + vp::util::CountRightOneBits32(mask_offset);
                            continue;
                        }

                        /* Calculate free memory size */
                        free_page_count = vp::util::CountRightZeroBits32(mask_offset);

                        /* Adjust free page count for max blocks */
                        const u32 max_blocks = (cSmallMemoryPageMaskBitCount - 1) - i;
                        free_page_count = (max_blocks < free_page_count) ? max_blocks : free_page_count;
                        if (small_page_count <= free_page_count) { break; }

                        i = i + free_page_count;
                    }
                    if (i == (cSmallMemoryPageMaskBitCount - 1)) { continue; }

                    /* Commit region */
                    void *allocation_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(small_memory_block)) + (i << 0xc));
                    void *commit             = ::VirtualAlloc(allocation_address, small_page_count << 0xc, MEM_COMMIT, PAGE_READWRITE);
                    VP_ASSERT(commit != nullptr);

                    /* Update page mask for odd or even*/
                    small_memory_block.page_mask                = (~(-1 << small_page_count)) <<  i;
                    const u8 last_alloc_size                    = small_memory_block.allocation_size_array[i >> 1];
                    small_memory_block.allocation_size_array[i >> 1] = ((i & 1) == 0) ? (last_alloc_size & 0xf0) | small_page_count : (last_alloc_size & 0xf) | ((small_page_count - 1) << 0x4);

                    /* Swap lists if page block is fully reserved */
                    if (small_memory_block.page_mask == cMaxPageMask) {
                       m_free_small_memory_list.Remove(small_memory_block);
                       m_filled_small_memory_list.PushBack(small_memory_block);
                    }

                    return allocation_address;
                }

                /* Allocate a new SmallMemoryRegion */
                void *reserve = ::VirtualAlloc(nullptr, cSmallMemoryRegionSize, MEM_RESERVE, PAGE_READWRITE);
                VP_ASSERT(reserve != nullptr);

                /* Commit pages */
                void *commit = ::VirtualAlloc(reserve, small_page_count_first << 0xc, MEM_COMMIT, PAGE_READWRITE);
                VP_ASSERT(commit != nullptr);

                /* Create Small memory block */
                VirtualHeapSmallMemoryBlock *new_small_block = reinterpret_cast<VirtualHeapSmallMemoryBlock*>(reserve);
                std::construct_at(new_small_block);
                new_small_block->page_mask                = (address_base_offset < vp::util::c4KB) ? ~(-1 << small_page_count_first) : (~(-1 << (small_page_count_first - 1)) << 1);
                new_small_block->allocation_size_array[0] = (address_base_offset < vp::util::c4KB) ? small_page_count_first : ((small_page_count_first - 1) << 0x4);

                /* Add to memory list */
                if (new_small_block->page_mask == 0xffff) {
                    m_filled_small_memory_list.PushBack(*new_small_block);
                } else {
                    m_free_small_memory_list.PushBack(*new_small_block);
                }

                /* Adjust start and end addresses lockless */
                void *start = m_start_address;
                while (commit < start) {
                    vp::util::InterlockedCompareExchange(std::addressof(m_start_address), commit, start);
                    start = m_start_address;
                }
                void *new_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(commit) + (small_page_count_first << 0xc));
                void *end     = m_end_address;
                while (end < new_end) {
                    vp::util::InterlockedCompareExchange(std::addressof(m_end_address), new_end, end);
                    end = m_end_address;
                }

                return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(new_small_block) + address_base_offset);
            }

            virtual void Free(void *address) override {

                /* If size aligned by 64 KB */
                uintptr_t address_t = reinterpret_cast<uintptr_t>(address);
                if ((address_t & (cSmallMemoryRegionSize - 1)) == 0) {

                    /* Remove from map */
                    m_large_memory_map.Remove(address_t);

                    /* Free virtual address range */
                    const bool result = ::VirtualFree(address, 0, MEM_RELEASE);
                    VP_ASSERT(result ==  true);

                    return;
                }

                /* Handle small memory */
                void                        *base_address = reinterpret_cast<void*>(vp::util::AlignDown(address_t, cSmallMemoryRegionSize));
                VirtualHeapSmallMemoryBlock *block        = reinterpret_cast<VirtualHeapSmallMemoryBlock*>(base_address);

                /* Get 4-bit allocation size */
                const u32 base_offset = (address_t >> 0xc) & 0xf;
                u32 alloc_size        = (block->allocation_size_array[((address_t >> 0xc) >> 1) & 7] >> ((address_t >> 0xa) & 0x4)) & 0xf;
                VP_ASSERT(alloc_size != 0);
                VP_ASSERT(base_offset + alloc_size <= 0x10);

                /* Clear page mask */
                const PageMask last_page_mask  = block->page_mask;
                block->page_mask               = last_page_mask & (~((~(-1 << alloc_size)) << base_offset));

                /* Free block if no longer in use */
                if (block->page_mask == 0) {

                    /* Remove block from lists and free virtual address region */
                    if (last_page_mask == 0xffff) { m_filled_small_memory_list.Remove(*block); }
                    else { m_free_small_memory_list.Remove(*block); }
                    const bool result = ::VirtualFree(base_address, 0, MEM_RELEASE);
                    VP_ASSERT(result == true);

                    return;
                }

                /* Swap block to free list if necessary */
                if (last_page_mask == 0xffff) { 
                    m_filled_small_memory_list.Remove(*block); 
                    m_free_small_memory_list.PushBack(*block); 
                }

                /* Adjust address and size to avoid decomitting the block meta data region */
                if (base_offset == 0) {
                    if (alloc_size < 2) { return; }
                    base_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(base_address) | vp::util::c4KB);
                    --alloc_size;
                } else {
                    base_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(base_address) | (base_offset << 0xc));
                }

                /* Decommit memory */
                const bool result = ::VirtualFree(base_address, (alloc_size << 0xc), MEM_DECOMMIT);
                VP_ASSERT(result == true);

                return;
            }

            virtual size_t GetTotalFreeSize() override {

                /* Query memory size */
                MEMORYSTATUSEX memory_status = { .dwLength = sizeof(MEMORYSTATUSEX) };
                const bool result = ::GlobalMemoryStatusEx(std::addressof(memory_status));
                VP_ASSERT(result == true);

                return memory_status.ullAvailPhys;
            }

            virtual size_t GetMaximumAllocatableSize(s32 alignment) override {

                /* Check if there is enough system memory for a large page */
                const size_t total_free_size = this->GetTotalFreeSize();
                if (cSmallMemoryRegionSize <= total_free_size) { return total_free_size - sizeof(VirtualHeapLargeMemoryBlock); }

                /* Desperate check for a commited small memory block head that is free */
                ScopedHeapLock l(this);

                const size_t adjusted_alignment = vp::util::AlignUp(sizeof(VirtualHeapSmallMemoryBlock), alignment);
                size_t max_size = 0;
                for (VirtualHeapSmallMemoryBlock &small_memory_block : m_free_small_memory_list) {
                    if ((small_memory_block.page_mask & 1) != 0) { continue; }
                    return vp::util::c4KB - adjusted_alignment;
                }

                return 0;
            }
	};
}
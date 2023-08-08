#pragma once

namespace awn::mem {

    struct ExpHeapMemoryBlock {
        u16                         alloc_magic;
        u8                          alignment;
        u8                          reserve0;
        u32                         reserve1;
        size_t                      block_size;
        vp::util::IntrusiveListNode exp_list_node;

        static constexpr u16 cFreeMagic  = vp::util::TCharCode16("FR");
        static constexpr u16 cAllocMagic = vp::util::TCharCode16("UD");

        constexpr  ExpHeapMemoryBlock() : alloc_magic(0), alignment(0), reserve0(0), block_size(0), exp_list_node() {/*...*/}
        constexpr ~ExpHeapMemoryBlock() {/*...*/}
    };
    static_assert(sizeof(ExpHeapMemoryBlock) == 0x20);

    class ExpHeap : public Heap {
        public:
            using FreeList      = vp::util::IntrusiveListTraits<ExpHeapMemoryBlock, &ExpHeapMemoryBlock::exp_list_node>::List;
            using AllocatedList = vp::util::IntrusiveListTraits<ExpHeapMemoryBlock, &ExpHeapMemoryBlock::exp_list_node>::List;
        public:
            static constexpr s32    cMinimumAlignment             = 4;
            static constexpr size_t cMinimumAllocationGranularity = 4;
        protected:
            FreeList       m_free_block_list;
            AllocatedList  m_allocated_block_list;
            AllocationMode m_allocation_mode;
        public:
            VP_RTTI_DERIVED(ExpHeap, Heap);
        protected:
            bool AddFreeBlock(AddressRange range) {

                void *new_start = range.start;
                void *new_end   = range.end;

                /* Skip if empty free list */
                vp::util::IntrusiveListNode *link_node = std::addressof((*m_free_block_list.end()).exp_list_node);

                /* Find free block before and after */
                if (m_free_block_list.IsEmpty() == false) {
                    ExpHeapMemoryBlock *prev_block = nullptr;
                    ExpHeapMemoryBlock *next_block = nullptr;
                    for (ExpHeapMemoryBlock &free_block : m_free_block_list) {
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
                    if (prev_block != nullptr && new_start == reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(prev_block) + prev_block->block_size + sizeof(ExpHeapMemoryBlock))) {
                        link_node = prev_block->exp_list_node.prev();
                        prev_block->exp_list_node.Unlink();
                        new_start = reinterpret_cast<void*>(prev_block);
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
                new_block->alignment   = 0;
                new_block->block_size  = new_size - sizeof(ExpHeapMemoryBlock);

                /* Link new free block between range */
                link_node->LinkNext(std::addressof(new_block->exp_list_node));

                return true;
            }
            
            void AddUsedBlock(ExpHeapMemoryBlock *free_block, uintptr_t allocation_address, size_t size) {

                /* Unlink old free block */
                free_block->exp_list_node.Unlink();

                /* Calculate new free block addresses and sizes */
                uintptr_t new_free_address     = allocation_address + size;
                uintptr_t new_free_end_address = reinterpret_cast<uintptr_t>(free_block) + free_block->block_size + sizeof(ExpHeapMemoryBlock);
                uintptr_t new_free_total_size  = new_free_end_address - new_free_address;

                uintptr_t orig_start_address   = reinterpret_cast<uintptr_t>(free_block) - free_block->alignment;
                uintptr_t used_start_address   = allocation_address - sizeof(ExpHeapMemoryBlock);
                uintptr_t used_alignment       = used_start_address - orig_start_address;

                /* Create a new free block at front if alignment is great enough */
                if (sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity <= used_alignment) {

                    ExpHeapMemoryBlock *front_free_block = reinterpret_cast<ExpHeapMemoryBlock*>(orig_start_address);

                    std::construct_at(front_free_block);
                    front_free_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
                    front_free_block->alignment   = 0;
                    front_free_block->block_size  = used_alignment - sizeof(ExpHeapMemoryBlock);

                    m_free_block_list.PushBack(*front_free_block);

                    used_alignment = 0;
                }
                
                /* Create new free block at back if leftover space is great enough */
                if (sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity <= new_free_total_size) {

                    ExpHeapMemoryBlock *back_free_block = reinterpret_cast<ExpHeapMemoryBlock*>(new_free_address);

                    std::construct_at(back_free_block);
                    back_free_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
                    back_free_block->alignment   = 0;
                    back_free_block->block_size  = new_free_total_size - sizeof(ExpHeapMemoryBlock);

                    m_free_block_list.PushBack(*back_free_block);

                    new_free_total_size = 0;
                }
                
                /* Add used block */
                ExpHeapMemoryBlock *used_block = reinterpret_cast<ExpHeapMemoryBlock*>(used_start_address);

                std::construct_at(used_block);
                used_block->alloc_magic = ExpHeapMemoryBlock::cAllocMagic;
                used_block->alignment   = used_alignment;
                used_block->block_size  = size + new_free_total_size;

                m_allocated_block_list.PushBack(*used_block);

                return;
            }

            bool IsAddressAllocationUnsafe(void *address);
        public:
            static ExpHeap *TryCreate(const char *name, void *address, size_t size, bool is_thread_safe) {

                /* Integrity checks */
                VP_ASSERT(address != nullptr && (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity) <= size);

                /* Contruct exp heap object */
                ExpHeap *new_heap = reinterpret_cast<ExpHeap*>(address);
                std::construct_at(new_heap, name, nullptr, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + sizeof(ExpHeap)), size - sizeof(ExpHeap), is_thread_safe);

                /* Create and push free node spanning block */
                ExpHeapMemoryBlock *first_block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(new_heap) + sizeof(ExpHeap));
                std::construct_at(first_block);

                first_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
                first_block->block_size  = size - (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock));

                new_heap->m_free_block_list.PushBack(*first_block);

                return new_heap;
            }
        public:
            explicit ExpHeap(const char *name, Heap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : Heap(name, parent_heap, start_address, size, is_thread_safe), m_free_block_list(), m_allocated_block_list(), m_allocation_mode(AllocationMode::BestFit) {/*...*/}
            virtual ~ExpHeap() override {/*...*/}

            static ExpHeap *TryCreate(const char *name, Heap *parent_heap, size_t size, s32 alignment, bool is_thread_safe);

            virtual void Finalize() override;

            virtual MemoryRange AdjustHeap() override;

            virtual size_t AdjustAllocation(void *address, size_t new_size) override;

            virtual void *TryAllocate(size_t size, s32 alignment) override;

            virtual void Free(void *address) override;

            virtual size_t GetTotalFreeSize() override;

            virtual size_t GetMaximumAllocatableSize(s32 alignment) override;

            virtual bool IsAddressAllocation(void *address) override;

            virtual size_t ResizeHeapBack(size_t new_size) override;

            static size_t GetAllocationSize(void *address);

            constexpr void SetAllocationMode(AllocationMode allocation_mode) { m_allocation_mode = allocation_mode; }
    };
    static_assert(sizeof(ExpHeap) == 0x98);
}

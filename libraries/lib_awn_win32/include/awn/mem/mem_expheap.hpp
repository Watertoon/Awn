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
            bool AddFreeBlock(AddressRange range);
            void AddUsedBlock(ExpHeapMemoryBlock *free_block, uintptr_t allocation_address, size_t size);

            bool IsAddressAllocationUnsafe(void *address);
        public:
            static ExpHeap *TryCreate(const char *name, void *address, size_t size, bool is_thread_safe);
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
            size_t GetMaximumAllocatableSizeUnsafe(s32 alignment);

            virtual bool IsAddressAllocation(void *address) override;

            virtual size_t ResizeHeapBack(size_t new_size) override;

            static size_t GetAllocationSize(void *address);

            constexpr void SetAllocationMode(AllocationMode allocation_mode) { m_allocation_mode = allocation_mode; }
    };
    static_assert(sizeof(ExpHeap) == 0xa0);
}

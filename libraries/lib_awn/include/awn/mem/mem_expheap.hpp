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
        vp::util::IntrusiveListNode list_node;
        u16 magic;
        u16 alignment;
        u32 reserve0;
        size_t size;

        static constexpr u16 cFreeMagic  = vp::util::TCharCode16("FR");
        static constexpr u16 cUsedMagic = vp::util::TCharCode16("UD");

        ALWAYS_INLINE void *GetBaseAddress() {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + sizeof(ExpHeapMemoryBlock) + this->alignment);
        }
        ALWAYS_INLINE void *GetEndAddress() {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this->GetBaseAddress()) + this->size);
        }
    };
    static_assert(sizeof(ExpHeapMemoryBlock) == 0x20);

    class ExpHeap : public Heap {
        public:
            using BlockList = vp::util::IntrusiveListTraits<ExpHeapMemoryBlock, &ExpHeapMemoryBlock::list_node>::List;
            using BlockListTraits = vp::util::IntrusiveListTraits<ExpHeapMemoryBlock, &ExpHeapMemoryBlock::list_node>::Traits;
        public:
            static constexpr size_t cMinimumAllocationGranularity = alignof(size_t);
        protected:
            BlockList m_free_list;
            BlockList m_used_list;
            u32 m_free_count;
            u32 m_used_count;
            AllocationMode m_allocation_mode;
        public:
            VP_RTTI_DERIVED(ExpHeap, Heap);
        protected:
            bool IsAddressAllocationUnsafe(void *address);

            static ExpHeapMemoryBlock *GetBlock(void *address);
        protected:
            void AddUsedBlockNoAlign(ExpHeapMemoryBlock *block, size_t size);
            ExpHeapMemoryBlock *AddUsedBlockAlign(ExpHeapMemoryBlock *block, size_t size, s32 align);
        protected:
            bool FindFreeBlockRangeFreeList(ExpHeapMemoryBlock **out_before, ExpHeapMemoryBlock **out_after, ExpHeapMemoryBlock *block);
            bool FindFreeBlockRangeIterate(ExpHeapMemoryBlock **out_before, ExpHeapMemoryBlock **out_after, ExpHeapMemoryBlock *block);
            ExpHeapMemoryBlock *AddFreeBlock(ExpHeapMemoryBlock *block);
        protected:
            ExpHeapMemoryBlock *FindFreeBlockFirstFit(size_t size, s32 align);
            ExpHeapMemoryBlock *FindFreeBlockBestFit(size_t size, s32 align);
            ExpHeapMemoryBlock *FindFreeBlockWorstFit(size_t size, s32 align);
            ExpHeapMemoryBlock *FindFreeBlock(size_t size, s32 align, AllocationMode mode);

            ExpHeapMemoryBlock *FindFreeBlockFirstFit(size_t size);
            ExpHeapMemoryBlock *FindFreeBlockBestFit(size_t size);
            ExpHeapMemoryBlock *FindFreeBlockWorstFit(size_t size);
            ExpHeapMemoryBlock *FindFreeBlock(size_t size, AllocationMode mode);
        public:
            ExpHeapMemoryBlock *GetEndFreeBlock();
        public:
            static ExpHeap *TryCreate(const char *name, void *address, size_t size, bool is_thread_safe);
        public:
            explicit ExpHeap(const char *name, Heap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : Heap(name, parent_heap, start_address, size, is_thread_safe), m_free_list(), m_used_list(), m_free_count(), m_used_count(), m_allocation_mode(AllocationMode::BestFit) {/*...*/}
            virtual ~ExpHeap() override {/*...*/}

            static ExpHeap *TryCreate(const char *name, Heap *parent_heap, size_t size, s32 alignment, bool is_thread_safe);

            virtual void Finalize() override;

            virtual MemoryRange AdjustHeap() override;
            virtual size_t AdjustAllocation(void *address, size_t new_size) override;

            virtual void *TryAllocate(size_t size, s32 alignment) override;
            virtual void *TryReallocate(void *address, size_t size, s32 alignment) override;
            virtual void Free(void *address) override;

            virtual size_t GetTotalFreeSize() override;

            virtual size_t GetMaximumAllocatableSize(s32 alignment) override;
            size_t GetMaximumAllocatableSizeUnsafe(s32 alignment);

            virtual bool IsEmpty() const override { return m_used_list.IsEmpty(); }

            virtual bool IsAddressAllocation(void *address) override;

            static size_t GetAllocationSize(void *address);

            constexpr void SetAllocationMode(AllocationMode allocation_mode) { m_allocation_mode = allocation_mode; }
    };
}

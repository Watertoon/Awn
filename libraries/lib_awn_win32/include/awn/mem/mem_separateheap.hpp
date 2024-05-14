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
    
    struct SeparateMemoryBlock {
        size_t                      block_offset;
        size_t                      block_size;
        vp::util::IntrusiveListNode used_list_node;
    };
    
    class SeparateHeap final : public Heap {
        public:
            using SeparateMemoryBlockList = vp::util::IntrusiveListTraits<SeparateMemoryBlock, &SeparateMemoryBlock::used_list_node>::List;
        public:
            static constexpr size_t cOffsetBase = 0x10000;
        private:
            struct SeparateFreeListHelper {
                SeparateFreeListHelper *next;
                char _dummystorage[sizeof(SeparateMemoryBlock) - sizeof(SeparateFreeListHelper*)];
            };
            static_assert(sizeof(SeparateMemoryBlock) == sizeof(SeparateFreeListHelper));
        private:
            SeparateMemoryBlockList  m_used_block_list;
            SeparateFreeListHelper  *m_management_free_list_start;
            u32                      m_used_management_block_count;
            u32                      m_total_management_block_count;
            u64                      m_management_area_size;
            void                    *m_management_area_start;
        public:
            VP_RTTI_DERIVED(SeparateHeap, Heap);
        public:
            SeparateHeap(const char *name, void *heap_start, u64 heap_size, void *management_start, u64 management_size, bool is_thread_safe);
            virtual ~SeparateHeap() override;

            static constexpr ALWAYS_INLINE size_t CalculateManagementAreaSize(size_t block_count) {
                return block_count * sizeof(SeparateMemoryBlock);
            }

            static SeparateHeap *Create(const char *name, void *heap_and_management_area, u64 heap_and_management_size, u64 heap_size, bool is_thread_safe) {

                /* Construct new seperate heap */
                SeparateHeap *heap = reinterpret_cast<SeparateHeap*>(heap_and_management_area);
                std::construct_at(heap, name, reinterpret_cast<void*>(cOffsetBase), heap_size, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(heap_and_management_area) + sizeof(SeparateHeap)), heap_and_management_size - sizeof(SeparateHeap), is_thread_safe);

                return heap;
            }
            
            virtual void Finalize() override final;
    
            virtual MemoryRange AdjustHeap() override final;
            virtual size_t AdjustAllocation([[maybe_unused]] void *byte_offset_of_allocation, [[maybe_unused]] size_t new_size) override final;
    
            virtual void *TryAllocate(size_t size, s32 alignment) override final;
            virtual void Free(void *byte_offset_of_allocation) override final;
    
            virtual size_t GetTotalFreeSize() override final;
            virtual size_t GetMaximumAllocatableSize([[maybe_unused]] s32 alignment) override final;
    };
}

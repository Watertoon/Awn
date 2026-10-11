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

    struct UnitHeapBlock {
        vp::util::IntrusiveListNode list_node;
    };

	class UnitHeap : public Heap {
        public:
            using FreeList = vp::util::IntrusiveListTraits<UnitHeapBlock, &UnitHeapBlock::list_node>::List;
		private:
			FreeList m_free_list;
            uintptr_t m_free_start;
            size_t m_free_size;
            u32 m_block_size;
            u32 m_block_count;
        public:
            VP_RTTI_DERIVED(UnitHeap);
        public:
            UnitHeap(const char *name, mem::Heap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : Heap(name, parent_heap, start_address, size, is_thread_safe), m_free_list(), m_free_start(), m_free_size(), m_block_size(), m_block_count() {/*...*/}
            virtual ~UnitHeap() override {/*...*/}

            static constexpr size_t CalculateMemorySize(u32 block_size, u32 block_count, s32 alignment) {
                return sizeof(UnitHeap) + alignment + vp::util::AlignUp(block_size, alignment) * block_count;
            }

            static UnitHeap *TryCreate(const char *name, mem::Heap *parent_heap, size_t size, u32 block_size, s32 alignment, bool is_thread_safe);

            virtual void *TryAllocate(size_t size, s32 alignment) override;

            virtual void Free(void *address) override;
            virtual void FreeAll() override;

            virtual size_t GetTotalFreeSize() override;
            virtual size_t GetMaximumAllocatableSize(s32 alignment) override;
	};
}

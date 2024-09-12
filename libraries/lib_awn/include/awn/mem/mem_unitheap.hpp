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
            u32      m_block_size;
        public:
            VP_RTTI_DERIVED(UnitHeap, Heap);
        public:
            UnitHeap(const char *name, mem::Heap *parent_heap, void *start_address, size_t size, u32 block_size) : Heap(name, parent_heap, start_address, size, false), m_free_list(), m_block_size(block_size) {/*...*/}
            virtual ~UnitHeap() override {/*...*/}

            static UnitHeap *TryCreate(const char *name, mem::Heap *parent_heap, u32 block_size, u32 block_count, s32 base_alignment) {

                /* Integrity checks */
                VP_ASSERT(name != nullptr);
                VP_ASSERT(parent_heap != nullptr);
                VP_ASSERT(sizeof(UnitHeapBlock) <= block_size);
                VP_ASSERT((block_size & (base_alignment - 1)) == 0);
                VP_ASSERT(block_count != 0);

                /* Calculate size */
                const size_t size = block_size * block_count;

                /* Allocate unit heap and memory */
                const size_t meta_data_size = vp::util::AlignUp(sizeof(UnitHeap), base_alignment);
                void *allocation = ::operator new(meta_data_size + size, parent_heap, base_alignment);
                if (allocation == nullptr) { return nullptr; }

                /* Construct frame heap */
                UnitHeap *unit_heap = reinterpret_cast<UnitHeap*>(allocation);
                std::construct_at(unit_heap, name, parent_heap, unit_heap, size, block_size);

                /* Find list start */
                uintptr_t list_start = reinterpret_cast<uintptr_t>(allocation) + meta_data_size;

                /* Add blocks to free list */
                for (u32 i = 0; i < block_count; ++i) {

                    /* Construct unit heap block */
                    UnitHeapBlock *block = reinterpret_cast<UnitHeapBlock*>(list_start);
                    std::construct_at(block);

                    /* Add to free list */
                    unit_heap->m_free_list.PushBack(*block);

                    list_start += block_size;
                }

                return unit_heap;
            }

            virtual void *TryAllocate(size_t size, s32 alignment) override {
                VP_UNUSED(alignment);

                /* Integrity checks */
                VP_ASSERT(size <= m_block_size && !m_free_list.IsEmpty());

                /* Pop a block */
                void *block = reinterpret_cast<void*>(std::addressof(m_free_list.PopBack()));

                return block;
            }

            virtual void Free(void *address) override {

                /* Construct free block */
                UnitHeapBlock *block = reinterpret_cast<UnitHeapBlock*>(address);
                std::construct_at(block);

                /* Add block to free list */
                m_free_list.PushBack(*block);

                return;
            }

            virtual size_t GetTotalFreeSize() override {
                return m_free_list.GetCount() * m_block_size;
            }

            virtual size_t GetMaximumAllocatableSize(s32 alignment) override {
                VP_UNUSED(alignment);
                return (!m_free_list.IsEmpty()) * m_block_size;
            }
	};
}

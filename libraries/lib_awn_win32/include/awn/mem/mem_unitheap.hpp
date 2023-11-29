#pragma once

namespace awn::mem {

    struct UnitHeapBlock {
        vp::util::IntrusiveListNode m_list_node;
    };

	class UnitHeap : public Heap {
		private:
			FreeList m_free_list;
            u32      m_block_size;
        public:
            constexpr UnitHeap(const char *name, mem::Heap *parent_heap, void *start_address, size_t size, u32 block_size) : Heap(name, parent_heap, start_address, size), m_block_size(block_size) {/*...*/}
            virtual constexpr ~UnitHeap() override {/*...*/}

            static UnitHeap *TryCreate(const char *name, mem::Heap *parent_heap, u32 block_size, u32 block_count) {

                /* Integrity checks */
                VP_ASSERT(name        != nullptr);
                VP_ASSERT(parent_heap != nullptr);
                VP_ASSERT(block_size  != 0);
                VP_ASSERT(block_count != 0);

                /* Calculate size */
                const size_t size = block_size * block_count;

                /* Allocate unit heap and memory */
                void *allocation = ::operator new(sizeof(UnitHeap) + size, parent_heap, alignof(UnitHeap));
                if (allocation == nullptr) { return nullptr; }

                /* Construct frame heap */
                UnitHeap *unit_heap = reinterpret_cast<UnitHeap*>(allocation);
                std::construct_at(unit_heap, name, parent_heap, start_address, size, block_size);

                /* Find list start */
                uintptr_t list_start = reinterpret_cast<uintptr_t>(allocation) + sizeof(UnitHeap);

                /* Add blocks to free list */
                for (u32 i = 0; i < block_count; ++i) {
                    
                    /* Construct unit heap block */
                    UnitHeapBlock *block = reinterpret_cast<UnitHeapBlock*>(list_start);
                    std::construct_at(block);
                    
                    /* Add to free list */
                    unit_heap->m_free_list.PushBack(block);

                    list_start += block_size;
                }
                
                return unit_heap;
            }

            virtual void TryAllocate(size_t size, [[maybe_unused]] u32 alignment) override {

                /* Integrity checks */
                VP_ASSERT(m_block_size < size && m_free_list.IsEmpty() == false);

                /* Pop a block */
                void *block = reinterpret_cast<void*>(m_free_list.PopBack());

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

            virtual size_t GetTotalFreeSize() const override {
                return m_free_list.GetCount() * block_size;
            }

            virtual size_t GetMaximumAllocatableSize(u32 alignment) const override {
                return (m_free_list.IsEmpty() == false) * block_size;
            }
	};
}

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
            SeparateHeap(const char *name, void *heap_start, u64 heap_size, void *management_start, u64 management_size, bool is_thread_safe) : Heap(name, nullptr, reinterpret_cast<void*>(heap_start), heap_size, is_thread_safe), m_used_block_list(), m_management_free_list_start(reinterpret_cast<SeparateFreeListHelper*>(management_start)), m_used_management_block_count(0), m_total_management_block_count(management_size/ sizeof(SeparateMemoryBlock)), m_management_area_size(management_size), m_management_area_start(management_start) {

                /* Initialize managment free list */
                SeparateFreeListHelper *block_list = reinterpret_cast<SeparateFreeListHelper*>(management_start);
                for (u32 i = 0; i < m_total_management_block_count - 1; ++i) {
                    block_list[i].next = std::addressof(block_list[i + 1]);
                }
            }
            virtual ~SeparateHeap() override {/*...*/}

            static constexpr ALWAYS_INLINE size_t GetManagementAreaSize(size_t block_count) {
                return block_count * sizeof(SeparateMemoryBlock);
            }

            static SeparateHeap *Create(const char *name, void *heap_and_management_area, u64 heap_and_management_size, u64 heap_size, bool is_thread_safe) {

                /* Construct new seperate heap */
                SeparateHeap *heap = reinterpret_cast<SeparateHeap*>(heap_and_management_area);
                std::construct_at(heap, name, reinterpret_cast<void*>(cOffsetBase), heap_size, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(heap_and_management_area) + sizeof(SeparateHeap)), heap_and_management_size - sizeof(SeparateHeap), is_thread_safe);

                return heap;
            }
            
            virtual void Finalize() override final {/*...*/}
    
            virtual MemoryRange AdjustHeap() override final { return MemoryRange{m_start_address, reinterpret_cast<size_t>(m_end_address) - reinterpret_cast<size_t>(m_start_address)}; }
            virtual size_t AdjustAllocation([[maybe_unused]] void *byte_offset_of_allocation, [[maybe_unused]] size_t new_size) override final { return 0; }
    
            virtual void *TryAllocate(size_t size, s32 alignment) override final {

                /* Integrity checks */
                VP_ASSERT(m_management_free_list_start != nullptr);

                /* Conditional heap lock */
                ScopedHeapLock l(this);

                /* Find new offset */
                SeparateMemoryBlock *prev_block = std::addressof(m_used_block_list.Back());
                size_t new_offset = vp::util::AlignUp(reinterpret_cast<size_t>(m_start_address), alignment);
                for (SeparateMemoryBlock &separate_block : m_used_block_list) {
                    if (size <= (separate_block.block_offset - new_offset)) {
                        prev_block = std::addressof(separate_block);
                        break;
                    }
                    new_offset = vp::util::AlignUp(separate_block.block_size + separate_block.block_offset, alignment);
                }
                if (m_end_address < reinterpret_cast<void*>(new_offset)) { return nullptr; }

                /* Create new separate heap block with offset */
                SeparateMemoryBlock *new_block = reinterpret_cast<SeparateMemoryBlock*>(m_management_free_list_start);
                std::construct_at(new_block);
                m_management_free_list_start = m_management_free_list_start->next;

                /* Ordered insert to used list */
                prev_block->used_list_node.LinkNext(std::addressof(new_block->used_list_node));

                /* Set block state */
                new_block->block_offset = new_offset;
                new_block->block_size   = size;

                return reinterpret_cast<void*>(new_offset);
            }

            virtual void Free(void *byte_offset_of_allocation) override final {

                /* Conditional heap lock */
                ScopedHeapLock l(this);

                /* Find block */
                for (SeparateMemoryBlock &used_block : m_used_block_list) {
                    if (used_block.block_offset == reinterpret_cast<size_t>(byte_offset_of_allocation)) {

                        /* Free used block to free list */
                        used_block.used_list_node.Unlink();
                        reinterpret_cast<SeparateFreeListHelper&>(used_block).next = m_management_free_list_start;
                        m_management_free_list_start = reinterpret_cast<SeparateFreeListHelper*>(std::addressof(used_block));
                        break;
                   }
                }

                return;
            }
    
            virtual size_t GetTotalFreeSize() override final {

                /* Add up used block sizes */
                size_t used_size = reinterpret_cast<size_t>(m_start_address);
                for (SeparateMemoryBlock &used_block : m_used_block_list) {
                    used_size = used_size + used_block.block_size;
                }

                /* Return difference of total memory */
                return reinterpret_cast<size_t>(m_end_address) - used_size;
            }

            virtual size_t GetMaximumAllocatableSize([[maybe_unused]] s32 alignment) override final {
                return 0;
            }
    };
}

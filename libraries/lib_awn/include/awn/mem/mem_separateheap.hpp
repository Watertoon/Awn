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
        vp::util::IntrusiveListNode list_node;
        size_t address;
        size_t size;
    };

    class SeparateHeap final : public Heap {
        public:
            using BlockAllocator = vp::util::ObjectAllocator<SeparateMemoryBlock>;
            using BlockList = vp::util::IntrusiveListTraits<SeparateMemoryBlock, &SeparateMemoryBlock::list_node>::List;
        public:
            static constexpr size_t cMinimumAllocationGranularity = sizeof(size_t);
        private:
            BlockAllocator m_block_allocator;
            BlockList m_block_list;
            u32 m_used_count;
            u32 m_total_count;
        public:
            VP_RTTI_DERIVED(SeparateHeap, Heap);
        public:
            SeparateHeap(const char *name, void *address, size_t size, bool is_thread_safe) : Heap(name, nullptr, address, size, is_thread_safe), m_block_allocator(), m_block_list(), m_used_count(), m_total_count() {/*...*/}
            virtual ~SeparateHeap() override {/*...*/}

            static constexpr ALWAYS_INLINE size_t CalculateManagementSize(u32 block_count) {
                return sizeof(SeparateHeap) + block_count * sizeof(SeparateMemoryBlock);
            }

            static SeparateHeap *Create(const char *name, void *management_area, size_t management_size, void *address, size_t size, bool is_thread_safe);

            virtual void *TryAllocate(size_t size, s32 alignment) override;

            virtual void Free(void *address) override;
            virtual void FreeAll() override;

            virtual size_t GetTotalFreeSize() override;
            virtual size_t GetMaximumAllocatableSize(s32 alignment) override;

            virtual bool IsEmpty() const override { return m_used_count == 0; }
    };
}

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

	class VirtualAddressHeap final : public Heap {
        public:
            static constexpr size_t cSmallMemoryRegionSize        = vp::util::c64KB;
            static constexpr size_t cSmallMemoryPageMaskBitCount  = cSmallMemoryRegionSize >> 0xc;
            static constexpr size_t cMaxPageMask                  = 0xffff;
            static constexpr size_t cMaxAllocationsPerSmallMemory = 8;
            static constexpr size_t cMinimumSize                  = 8;
            static constexpr s32    cMinimumAlignment             = 8;
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
            VirtualAddressHeap(const char *name, void *start, size_t size);
            virtual ~VirtualAddressHeap() override;

            static VirtualAddressHeap *Create(const char *name);

            virtual void *TryAllocate(size_t size, s32 alignment) override;
            virtual void Free(void *address) override;

            void FreeAll();

            virtual size_t AdjustAllocation(void *address, size_t new_size) override;

            virtual size_t GetTotalFreeSize() override;
            virtual size_t GetMaximumAllocatableSize(s32 alignment) override;

            size_t GetSizeOfAllocation(void *address);
	};
}

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

    class GpuExpHeap : public ExpHeap {
        public:
            VP_RTTI_DERIVED(GpuExpHeap, ExpHeap);
        public:
            static GpuExpHeap *TryCreate(const char *name, mem::Heap *cpu_heap, void *gpu_address, size_t size) {

                /* Integrity checks */
                VP_ASSERT(gpu_address != nullptr && (sizeof(GpuExpHeap) + sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity) <= size);

                /* Contruct exp heap object */
                GpuExpHeap *new_heap = new (cpu_heap, alignof(GpuExpHeap)) GpuExpHeap(name, nullptr, gpu_address, size);
                new_heap->SetAllocationMode(AllocationMode::FirstFit);

                /* Create and push free node spanning block */
                ExpHeapMemoryBlock *first_block = reinterpret_cast<ExpHeapMemoryBlock*>(gpu_address);
                std::construct_at(first_block);

                first_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
                first_block->block_size  = size - (sizeof(ExpHeapMemoryBlock));

                new_heap->m_free_block_list.PushBack(*first_block);

                return new_heap;
            }
        public:
            explicit GpuExpHeap(const char *name, Heap *parent_heap, void *start_address, size_t size) : ExpHeap(name, parent_heap, start_address, size, false) {/*...*/}
            virtual ~GpuExpHeap() override {/*...*/}

            static GpuExpHeap *TryCreate(const char *name, mem::Heap *cpu_heap, mem::Heap *parent_gpu_heap, size_t size, s32 alignment) {

                /* Try current heap if one is not provided */
                if (parent_gpu_heap == nullptr) {
                    parent_gpu_heap = mem::GetCurrentThreadHeap();
                }

                /* Ensure parent heap is a gpu heap */
                if (parent_gpu_heap->IsGpuHeap() == false) { return nullptr; }

                /* Respect whole size */
                if (size == mem::Heap::cWholeSize) {
                    size = parent_gpu_heap->GetMaximumAllocatableSize(alignment);
                }

                /* Enforce minimum size */
                if (size < (sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity)) { return nullptr; }

                /* Allocate gpu heap memory from parent heap */
                void *new_heap_memory = parent_gpu_heap->TryAllocate(size, alignment);

                /* Allocate heap head on gpu */
                GpuExpHeap *new_heap  = new (cpu_heap, alignof(GpuExpHeap)) GpuExpHeap(name, parent_gpu_heap, new_heap_memory, size);
                new_heap->SetAllocationMode(AllocationMode::FirstFit);

                if (new_heap_memory == nullptr || new_heap == nullptr) { return nullptr; }

                /* Construct and add free node spanning new heap */
                ExpHeapMemoryBlock *first_block = reinterpret_cast<ExpHeapMemoryBlock*>(new_heap_memory);
                std::construct_at(first_block);

                first_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
                first_block->block_size = size - (sizeof(ExpHeapMemoryBlock));

                new_heap->m_free_block_list.PushBack(*first_block);

                /* Add to parent heap child list */
                parent_gpu_heap->PushBackChild(new_heap);

                return new_heap;
            }

            virtual constexpr bool IsGpuHeap() const override { return true; }
    };
}

#pragma once

namespace awn::mem {

    class GpuExpHeap : public ExpHeap {
        protected:
            GpuRootHeapContext *m_root_heap_context;
        public:
            VP_RTTI_DERIVED(GpuExpHeap, ExpHeap);
        public:
            static GpuExpHeap *TryCreate(const char *name, mem::Heap *cpu_heap, GpuRootHeapContext *root_heap_context, void *address, size_t size) {

                /* Integrity checks */
                VP_ASSERT(address != nullptr && (sizeof(GpuExpHeap) + sizeof(ExpHeapMemoryBlock) + cMinimumAllocationGranularity) <= size);

                /* Contruct exp heap object */
                GpuExpHeap *new_heap = new (cpu_heap, alignof(GpuExpHeap)) GpuExpHeap(name, nullptr, address, size);
                new_heap->SetAllocationMode(AllocationMode::FirstFit);

                /* Create and push free node spanning block */
                ExpHeapMemoryBlock *first_block = reinterpret_cast<ExpHeapMemoryBlock*>(address);
                std::construct_at(first_block);

                first_block->alloc_magic = ExpHeapMemoryBlock::cFreeMagic;
                first_block->block_size  = size - (sizeof(ExpHeapMemoryBlock));

                new_heap->m_free_block_list.PushBack(*first_block);

                /* Set gpu root heap context */
                new_heap->m_root_heap_context = root_heap_context;

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

            virtual GpuMemoryAddress TryAllocateGpuMemoryAddress(size_t size, s32 alignment) override {
                return { m_root_heap_context, this->TryAllocate(size, alignment) };
            }
            virtual constexpr bool IsGpuHeap() const override { return true; }
            virtual constexpr GpuRootHeapContext *GetGpuRootHeapContext() const override { return m_root_heap_context; }
    };
}

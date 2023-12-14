#pragma once

namespace awn::mem {
    
    class FrameHeap : public Heap {
        protected:
            void *m_start_offset;
        public:
            FrameHeap(const char *name, Heap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : Heap(name, parent_heap, start_address, size, is_thread_safe), m_start_offset(start_address) {/*...*/}
            virtual ~FrameHeap() override {/*...*/}

            static FrameHeap *TryCreate(const char *name, mem::Heap *parent_heap, size_t size, s32 alignment, bool is_thread_safe) {

                /* Get size */
                const size_t max_size = (size != 0) ? vp::util::AlignUp(size, alignof(size_t)) : parent_heap->GetMaximumAllocatableSize(alignment);
                VP_ASSERT(sizeof(FrameHeap) < max_size);

                /* Allocate new frameheap */
                void *memory = parent_heap->TryAllocate(max_size, alignment);

                /* Construct frame heap */
                FrameHeap *new_frameheap = reinterpret_cast<FrameHeap*>(memory);
                std::construct_at(new_frameheap, name, parent_heap, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + sizeof(FrameHeap)), max_size - sizeof(FrameHeap), is_thread_safe);

                return new_frameheap;
            }

            virtual MemoryRange AdjustHeap() override {

                /* Lock heap */
                ScopedHeapLock l(this);

                /* Adjust end addresses */
                m_end_address = m_start_offset;

                /* Adjust parent allocation */
                const size_t adjusted_size = reinterpret_cast<uintptr_t>(m_start_offset) - reinterpret_cast<uintptr_t>(m_start_address);
                if (m_parent_heap != nullptr) {
                    m_parent_heap->AdjustAllocation(this, adjusted_size);
                }

                return { m_end_address, adjusted_size };
            }

            virtual void *TryAllocate(size_t size, s32 alignment) override {

                /* Adjust size */
                const size_t adjusted_size = size;

                /* Lock heap */
                ScopedHeapLock l(this);

                /* Out of memory restart label */
                _FrameHeap_OutOfMemoryRestart:

                /* Calculate new start offset */
                void *old_start        = vp::util::AlignUp(m_start_offset, alignment);
                void *new_start_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(old_start) + adjusted_size);

                /* Check out of memory */
                if (m_end_address <= new_start_offset) {

                    /* Attempt out of memory callback */
                    OutOfMemoryInfo out_of_memory = {
                        .out_of_memory_heap      = this,
                        .allocation_size         = size,
                        .aligned_allocation_size = vp::util::AlignUp(size, alignment),
                        .alignment               = alignment,
                    };
                    const bool result = mem::OutOfMemoryImpl(std::addressof(out_of_memory));
                    if (result == true) { goto _FrameHeap_OutOfMemoryRestart; }

                    return nullptr;
                }

                /* Set start */
                m_start_offset  = new_start_offset;

                return old_start;
            }

            void FreeAll() {

                /* Lock heap */
                ScopedHeapLock l(this);

                /* Dispose */
                this->DisposeAll();

                /* Reset start offset */
                m_start_offset = m_start_address;

                return;
            }

            virtual size_t ResizeHeapBack(size_t new_size) override {

                /* Lock heap */
                ScopedHeapLock l(this);

                /* Calculate new end */
                void *new_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_start_address) + new_size);

                /* Check new end is within free region */
                if (new_end < m_start_offset) { return 0; }

                /* Adjust parent_allocation */
                const size_t adjusted_size = reinterpret_cast<uintptr_t>(m_start_offset) - reinterpret_cast<uintptr_t>(m_start_address);
                if (m_parent_heap != nullptr) {
                    m_parent_heap->AdjustAllocation(this, adjusted_size);
                }

                /* Adjust end */
                m_end_address = new_end;

                return new_size;
            }

            virtual size_t GetTotalFreeSize() override { return reinterpret_cast<uintptr_t>(m_end_address) - reinterpret_cast<uintptr_t>(m_start_offset); }
            virtual size_t GetMaximumAllocatableSize(s32 alignment) override { return reinterpret_cast<uintptr_t>(m_end_address) - vp::util::AlignUp(reinterpret_cast<uintptr_t>(m_start_offset), alignment); }
    };

    class GpuFrameHeap : public FrameHeap {
        public:
            GpuFrameHeap(const char *name, Heap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : FrameHeap(name, parent_heap, start_address, size, is_thread_safe) {/*...*/}
            virtual ~GpuFrameHeap() override {/*...*/}

            GpuFrameHeap *TryCreate(const char *name, mem::Heap *cpu_heap, mem::Heap *gpu_heap, size_t size, s32 alignment, bool is_thread_safe) {

                /* Get size */
                const size_t max_size = (size != 0) ? vp::util::AlignUp(size, alignof(size_t)) : gpu_heap->GetMaximumAllocatableSize(alignment);
                VP_ASSERT(sizeof(FrameHeap) < max_size);

                /* Allocate new gpu frameheap */
                void *gpu_memory = ::operator new(size, gpu_heap, alignment);
                GpuFrameHeap *gpu_frame_heap = new (cpu_heap, alignment) GpuFrameHeap(name, gpu_heap, gpu_memory, max_size, is_thread_safe);

                return gpu_frame_heap;
            }

            virtual MemoryRange AdjustHeap() override {

                /* Adjust end addresses */
                m_end_address = m_start_offset;

                /* Adjust parent allocation */
                const size_t adjusted_size = reinterpret_cast<uintptr_t>(m_start_offset) - reinterpret_cast<uintptr_t>(m_start_address);
                if (m_parent_heap != nullptr) {
                    m_parent_heap->AdjustAllocation(m_start_address, adjusted_size);
                }

                return { m_end_address, adjusted_size };
            }

            virtual size_t ResizeHeapBack(size_t new_size) override {

                /* Lock heap */
                ScopedHeapLock l(this);

                /* Calculate new end */
                void *new_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_start_address) + new_size);

                /* Check new end is within free region */
                if (new_end < m_start_offset) { return 0; }

                /* Adjust parent_allocation */
                const size_t adjusted_size = reinterpret_cast<uintptr_t>(m_start_offset) - reinterpret_cast<uintptr_t>(m_start_address);
                if (m_parent_heap != nullptr) {
                    m_parent_heap->AdjustAllocation(m_start_address, adjusted_size);
                }

                /* Adjust end */
                m_end_address = new_end;

                return new_size;
            }

            virtual constexpr bool IsGpuHeap() const override { return true; }
    };
}
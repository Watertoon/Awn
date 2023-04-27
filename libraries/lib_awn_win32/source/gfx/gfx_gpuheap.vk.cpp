#include <awn.hpp>

namespace awn::gfx {

    Result GpuHeap::TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, mem::Heap *heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags, size_t minimum_size, size_t maximum_size) {

        /* Find valid GpuHeapMemory */
        GpuHeapMemory *gpu_heap_memory = nullptr;
        {
            std::scoped_lock lock(m_heap_memory_list_mutex);

            for (GpuHeapMemory &memory_iter : m_gpu_heap_memory_list) {
                if ((memory_iter.m_memory_property_flags & memory_property_flags) == memory_property_flags && size < memory_iter.m_gpu_separate_heap->GetMaxAllocatableSize(alignment)) {
                    gpu_heap_memory = std::addressof(memory_iter);
                    break;
                }
            }
        }

        /* Create new GpuHeapMemory if needed */
        if (gpu_heap_memory == nullptr) {
            gpu_heap_memory = GpuHeapMemory::Create(this, size, alignment, memory_property_flags);
            m_gpu_heap_memory_list.PushBack(*gpu_heap_memory);
        }

        /* Allocate from GpuHeapMemory */
        return gpu_heap_memory->TryAllocateGpuMemory(out_allocation, size, alignment);
    }

    bool GpuHeap::FreeGpuMemoryAllocation(GpuMemoryAllocation *allocation) {

        /* Free allocation from heap memory */
        const u32 result = allocation->m_parent_gpu_heap_memory->FreeGpuMemoryAllocation(this);
        GpuHeapMemory *heap_memory           = allocation->m_parent_gpu_heap_memory;
        allocation->m_parent_gpu_heap_memory = nullptr;

        /* Finished if heap memory is still referenced */
        if (result == false) { return false; }

        /* Delete heap memory if unreferenced */
        {
            std::scoped_lock lock(m_heap_memory_list_mutex);
            heap_memory->m_gpu_heap_list_node.Unlink();
        }
        ::operator delete(heap_memory->m_seperate_heap);
        ::operator delete(heap_memory);

        return true;
    }
}

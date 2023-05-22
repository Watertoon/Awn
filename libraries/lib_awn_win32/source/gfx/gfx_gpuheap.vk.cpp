#include <awn.hpp>

namespace awn::gfx {

    Result GpuHeap::TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, mem::Heap *heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags, size_t minimum_size, [[maybe_unused]] size_t maximum_size) {

        /* Adjust size */
        if (size < minimum_size) {
            size = minimum_size;
        }

        /* Find valid GpuHeapMemory */
        GpuHeapMemory *gpu_heap_memory = nullptr;
        {
            std::scoped_lock lock(m_heap_memory_list_mutex);

            for (GpuHeapMemory &heap_memory_i : m_gpu_heap_memory_list) {
                if ((heap_memory_i.m_memory_property_flags & static_cast<u32>(memory_property_flags)) == static_cast<u32>(memory_property_flags) && size < heap_memory_i.m_gpu_separate_heap->GetMaximumAllocatableSize(alignment)) {
                    gpu_heap_memory = std::addressof(heap_memory_i);
                    break;
                }
            }
        }

        /* Create new GpuHeapMemory if needed */
        if (gpu_heap_memory == nullptr) {
            gpu_heap_memory = GpuHeapMemory::Create(this, heap, size, alignment, memory_property_flags);
            m_gpu_heap_memory_list.PushBack(*gpu_heap_memory);
        }

        /* Allocate from GpuHeapMemory */
        return gpu_heap_memory->TryAllocateGpuMemory(out_allocation, size, alignment, memory_property_flags);
    }

    bool GpuHeap::FreeGpuMemoryAllocation(GpuMemoryAllocation *allocation) {

        /* Free allocation from heap memory */
        GpuHeapMemory *heap_memory = allocation->m_parent_gpu_heap_memory;
        const u32 result = allocation->m_parent_gpu_heap_memory->FreeGpuMemoryAllocation(allocation);
        allocation->m_parent_gpu_heap_memory = nullptr;

        /* Finished if heap memory is still referenced */
        if (result == false) { return false; }

        /* Delete heap memory if unreferenced */
        {
            std::scoped_lock lock(m_heap_memory_list_mutex);
            heap_memory->m_gpu_heap_list_node.Unlink();
        }
        ::operator delete(heap_memory->m_gpu_separate_heap);
        ::operator delete(heap_memory);

        return true;
    }
}

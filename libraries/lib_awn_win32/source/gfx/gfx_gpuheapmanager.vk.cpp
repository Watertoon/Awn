#include <awn.hpp>

namespace awn::gfx {

    AWN_SINGLETON_TRAITS_IMPL(GpuHeapManager);

    Result GpuHeapManager::TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, mem::Heap *heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags) {

        /* Use a previous GpuHeap if our current Heap corresponds to one */
        GpuHeap *gpu_heap = nullptr;
        {
            std::scoped_lock lock(m_gpu_heap_list_mutex);

            for (GpuHeap &heap_iter : m_gpu_heap_list) {
                if (heap_iter.m_parent_heap == heap) {
                    gpu_heap = std::addressof(heap_iter);
                    break;
                }
            }

            /* Allocate new gpu heap if we can't find one */
            if (gpu_heap == nullptr) {

                gpu_heap = m_gpu_heap_allocator.Allocate();
                RESULT_RETURN_UNLESS(gpu_heap == nullptr, ResultMaximumGpuHeaps);

                m_gpu_heap_list.PushBack(*gpu_heap);
            }
        }

        /* Allocate from gpu heap */
        return gpu_heap->TryAllocateGpuMemory(out_allocation, heap, size, alignment, memory_property_flags, m_minimum_block_size, 0xffff'ffff);
    }

    void GpuHeapManager::FreeGpuMemoryAllocation(GpuMemoryAllocation *allocation) {

        /* Free allocation from GpuHeapMemory */
        GpuHeap *gpu_heap = allocation->m_parent_gpu_heap_memory->m_parent_gpu_heap;
        const bool result = allocation->m_parent_gpu_heap_memory->m_parent_gpu_heap->FreeGpuMemoryAllocation(allocation);

        /* Finished if gpu heap is still referenced */
        if (result == false) { return; }

        /* Delete gpu heap if unreferenced */
        {
            std::scoped_lock lock(m_gpu_heap_list_mutex);
            gpu_heap->m_gpu_heap_manager_list_node.Unlink();
            m_gpu_heap_allocator.Free(gpu_heap);
        }

        return;
    }
}

#include <awn.hpp>

namespace awn::gfx {

    GpuHeapMemory *GpuHeapMemory::Create(GpuHeap *parent_heap, mem::Heap *heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags) {

        /* Find block count */
        const u32 block_count = 32;

        /* Allocate new GpuHeapMemory and Separate Heap */
        GpuHeapMemory *gpu_heap_memory = reinterpret_cast<GpuHeapMemory*>(::operator new(sizeof(GpuHeapMemory) + sizeof(mem::SeparateHeap) + mem::SeparateHeap::GetManagementAreaSize(block_count), heap, 8));
        std::construct_at(gpu_heap_memory);

        /* Create separate heap */
        const size_t aligned_size = vp::util::AlignUp(size, alignment);
        void *management_area     = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(gpu_heap_memory) + sizeof(GpuHeapMemory));
        gpu_heap_memory->m_gpu_separate_heap = mem::SeparateHeap::Create("AwnGpuHeap", management_area, aligned_size, mem::SeparateHeap::GetManagementAreaSize(block_count), false);

        /* Allocate device memory */
        const VkMemoryAllocateInfo allocate_info = {
            .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize  = aligned_size,
            .memoryTypeIndex = Context::GetInstance()->GetVkMemoryTypeIndex(memory_property_flags)
        };
        const u32 result = ::pfn_vkAllocateMemory(Context::GetInstance()->GetVkDevice(), std::addressof(allocate_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(gpu_heap_memory->m_vk_device_memory));
        VP_ASSERT(result == VK_SUCCESS);

        /* Set state */
        gpu_heap_memory->m_parent_gpu_heap = parent_heap;

        return gpu_heap_memory;
    }
    
    Result GpuHeapMemory::TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags) {

        /* Allocate from separate heap */
        void *alloc = m_gpu_separate_heap->TryAllocate(size, alignment);
        VP_ASSERT(alloc != nullptr);

        out_allocation->m_offset                 = reinterpret_cast<uintptr_t>(alloc) - mem::SeparateHeap::cOffsetBase;
        out_allocation->m_size                   = size;
        out_allocation->m_memory_property_flags  = static_cast<u32>(memory_property_flags);
        out_allocation->m_parent_gpu_heap_memory = this;
        m_gpu_memory_allocation_list.PushBack(*out_allocation);

        return ResultSuccess;
    }
    
    bool GpuHeapMemory::FreeGpuMemoryAllocation(GpuMemoryAllocation *allocation) {

        /* Free address */
        m_gpu_separate_heap->Free(reinterpret_cast<void*>(allocation->m_offset + mem::SeparateHeap::cOffsetBase));

        /* Unlink allocation */
        allocation->m_gpu_heap_memory_list_node.Unlink();

        /* Free complete if memory is still referenced */
        if (m_gpu_memory_allocation_list.IsEmpty() == false) { return false; }

        /* Free VkDeviceMemory if unreferenced */
        ::pfn_vkFreeMemory(Context::GetInstance()->GetVkDevice(), m_vk_device_memory, Context::GetInstance()->GetVkAllocationCallbacks());
        m_vk_device_memory = VK_NULL_HANDLE;

        return true;
    }
}

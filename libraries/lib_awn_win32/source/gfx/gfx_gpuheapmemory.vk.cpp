#include <awn.hpp>

namespace awn::gfx {

    GpuHeapMemory *GpuHeapMemory::Create(GpuHeap *parent_heap, mem::Heap *heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags) {

        /* Calculate block count */
        const u32 page_count  = size >> 0x6;
        const u32 block_count = (page_count <= 0) ? ((cMaxBlockCount < page_count) ? page_count : cMaxBlockCount) : 1;

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
        const u32 result0 = ::pfn_vkAllocateMemory(Context::GetInstance()->GetVkDevice(), std::addressof(allocate_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(gpu_heap_memory->m_vk_device_memory));
        VP_ASSERT(result0 == VK_SUCCESS);

        /* Map memory if it's cpu visible */
        if ((static_cast<u32>(memory_property_flags) & (static_cast<u32>(MemoryPropertyFlags::CpuUncached) | static_cast<u32>(MemoryPropertyFlags::CpuCached))) != 0) {

            const VkMemoryMapInfoKHR map_info = {
                .sType  = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO_KHR,
                .memory = gpu_heap_memory->m_vk_device_memory,
                .offset = 0,
                .size   = VK_WHOLE_SIZE
            };
            const u32 result1 = ::pfn_vkMapMemory2KHR(Context::GetInstance()->GetVkDevice(), std::addressof(map_info), std::addressof(gpu_heap_memory->m_mapped_memory));
            VP_ASSERT(result1 == VK_SUCCESS);
        }

        /* Set state */
        gpu_heap_memory->m_parent_gpu_heap = parent_heap;

        return gpu_heap_memory;
    }
    
    Result GpuHeapMemory::TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags) {

        /* Allocate from separate heap */
        void *alloc = m_gpu_separate_heap->TryAllocate(size, alignment);
        VP_ASSERT(alloc != nullptr);

        /* Remove seperate heap address offset */
        const size_t offset = reinterpret_cast<uintptr_t>(alloc) - mem::SeparateHeap::cOffsetBase;;

        /* Set GpuMemoryAllocation state */
        out_allocation->m_mapped_memory          = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_mapped_memory) + offset);
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

    void GpuHeapMemory::FlushCpuCache(size_t size, size_t offset) {

        /* Flush Cpu cache */
        const VkMappedMemoryRange mapped_range = {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = m_vk_device_memory,
            .offset = offset,
            .size   = size
        };
        const u32 result0 = ::pfn_vkFlushMappedMemoryRanges(Context::GetInstance()->GetVkDevice(), 1, std::addressof(mapped_range));
        VP_ASSERT(result0 == VK_SUCCESS);
    }

    void GpuHeapMemory::InvalidateCpuCache(size_t size, size_t offset) {

        /* Invalidate Cpu cache */
        const VkMappedMemoryRange mapped_range = {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = m_vk_device_memory,
            .offset = offset,
            .size   = size
        };
        const u32 result0 = ::pfn_vkInvalidateMappedMemoryRanges(Context::GetInstance()->GetVkDevice(), 1, std::addressof(mapped_range));
        VP_ASSERT(result0 == VK_SUCCESS);
    }
}

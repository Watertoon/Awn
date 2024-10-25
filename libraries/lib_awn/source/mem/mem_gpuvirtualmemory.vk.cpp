#include <awn.hpp>

namespace awn::mem {

    GpuMemoryHandle AllocateGpuMemoryHandle(mem::Heap *meta_data_heap, gfx::MemoryPropertyFlags memory_property_flags, size_t size) {
        const VkMemoryAllocateFlagsInfo flags_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        };
        return AllocateGpuMemoryHandle(meta_data_heap, memory_property_flags, size, std::addressof(flags_info));
    }

    GpuMemoryHandle AllocateGpuMemoryHandle(mem::Heap *meta_data_heap, gfx::MemoryPropertyFlags memory_property_flags, size_t size, const void *pnext) {
        VP_UNUSED(meta_data_heap);

        /* Allocate device memory */
        VkDeviceMemory vk_device_memory = VK_NULL_HANDLE;
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
        const VkMemoryAllocateInfo allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = pnext,
            .allocationSize = size,
            .memoryTypeIndex = context->GetVkMemoryTypeIndex(memory_property_flags),
        };
        const VkResult result0 = ::pfn_vkAllocateMemory(context->GetVkDevice(), std::addressof(allocate_info), context->GetVkAllocationCallbacks(), std::addressof(vk_device_memory));
        if (result0 != VK_SUCCESS) { return {}; }

        /* Map device memory into process address space if cpu mapped */
        if ((static_cast<u32>(memory_property_flags) & static_cast<u32>(gfx::MemoryPropertyFlags::CpuUncached | gfx::MemoryPropertyFlags::CpuCached)) == 0) {
            return { vk_device_memory, nullptr};
        }
        void *memory = nullptr;
        const VkMemoryMapInfoKHR map_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO_KHR,
            .memory = vk_device_memory,
            .size = VK_WHOLE_SIZE,
        };
        const VkResult result1 = ::pfn_vkMapMemory2KHR(context->GetVkDevice(), std::addressof(map_info), std::addressof(memory));
        if (result1 != VK_SUCCESS) { FreeGpuMemoryHandle({ vk_device_memory, nullptr }); return {}; }

        return { vk_device_memory, memory, };
    }

    void FreeGpuMemoryHandle(GpuMemoryHandle handle) {
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
        ::pfn_vkFreeMemory(context->GetVkDevice(), handle.vk_device_memory, context->GetVkAllocationCallbacks());
    }

    void *MapGpuMemoryHandle(GpuMemoryHandle handle) {
        return handle.memory;
    }
}

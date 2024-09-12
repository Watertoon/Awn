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

        VkDeviceMemory vk_device_memory = VK_NULL_HANDLE;
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
        const VkMemoryAllocateInfo allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = pnext,
            .allocationSize = size,
            .memoryTypeIndex = context->GetVkMemoryTypeIndex(memory_property_flags),
        };
        const VkResult result0 = ::pfn_vkAllocateMemory(context->GetVkDevice(), std::addressof(allocate_info), context->GetVkAllocationCallbacks(), std::addressof(vk_device_memory));
        VP_ASSERT(result0 == VK_SUCCESS);

        return { vk_device_memory, };
    }

    void FreeGpuMemoryHandle(GpuMemoryHandle handle) {
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
        ::pfn_vkFreeMemory(context->GetVkDevice(), handle.vk_device_memory, context->GetVkAllocationCallbacks());
    }

    void *MapGpuMemoryHandle(GpuMemoryHandle handle) {
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
        void *memory = nullptr;
        const VkResult result0 = ::pfn_vkMapMemory(context->GetVkDevice(), handle.vk_device_memory, 0, VK_WHOLE_SIZE, 0, std::addressof(memory));
        VP_ASSERT(result0 == VK_SUCCESS);
        return memory;
    }
}

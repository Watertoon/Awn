#include <awn.hpp>

namespace awn::mem {

    AWN_SINGLETON_TRAITS_IMPL(GpuHeapManager);

    void GpuRootHeapContext::FlushCpuCache(void *address, size_t size) {

        /* Flush Cpu cache to memory for gpu visibility */
        const size_t offset = this->GetVkDeviceMemoryOffset(address);
        const VkMappedMemoryRange range = {
            .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = vk_device_memory,
            .offset = offset,
            .size   = size,
        };
        const u32 result = ::pfn_vkFlushMappedMemoryRanges(gfx::Context::GetInstance()->GetVkDevice(), 1, std::addressof(range));
        VP_ASSERT(result == VK_SUCCESS);

        return;
    }

    void GpuRootHeapContext::InvalidateCpuCache(void *address, size_t size) {

        /* Invalidate cpu cache for cpu visibility of memory */
        const size_t offset = this->GetVkDeviceMemoryOffset(address);
        const VkMappedMemoryRange range = {
            .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = vk_device_memory,
            .offset = offset,
            .size   = size,
        };
        const u32 result = ::pfn_vkInvalidateMappedMemoryRanges(gfx::Context::GetInstance()->GetVkDevice(), 1, std::addressof(range));
        VP_ASSERT(result == VK_SUCCESS);

        return;
    }

	bool GpuHeapManager::AllocateContext(mem::Heap *heap, GpuRootHeapContext *out_heap_context, const char *name, size_t size, gfx::MemoryPropertyFlags memory_properties) {

		/* Get context */
		gfx::Context *context = gfx::Context::GetInstance();

		/* Allocate memory */
        const VkMemoryAllocateFlagsInfo flag_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        };
		const VkMemoryAllocateInfo allocate_info = {
			.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext           = std::addressof(flag_info),
			.allocationSize  = size,
			.memoryTypeIndex = context->GetVkMemoryTypeIndex(memory_properties),
		};
		const u32 result0 = ::pfn_vkAllocateMemory(context->GetVkDevice(), std::addressof(allocate_info), context->GetVkAllocationCallbacks(), std::addressof(out_heap_context->vk_device_memory));
		VP_ASSERT(result0 == VK_SUCCESS);

		/* Map memory */
		const u32 result1 = ::pfn_vkMapMemory(context->GetVkDevice(), out_heap_context->vk_device_memory, 0, VK_WHOLE_SIZE, 0, std::addressof(out_heap_context->base_address));
		VP_ASSERT(result1 == VK_SUCCESS);

		/* Construct GpuExpHeap */
		out_heap_context->root_heap = mem::GpuExpHeap::TryCreate(name, heap, out_heap_context, out_heap_context->base_address, size);

        /* Set memory property flags */
        out_heap_context->memory_property_flags = memory_properties;

		return true;
	}
	void GpuHeapManager::FreeContext(GpuRootHeapContext *heap_context) {

		/* Destroy root heap */
		std::destroy_at(heap_context->root_heap);

		/* Get context */
		gfx::Context *context = gfx::Context::GetInstance();

		/* Free memory */
		::pfn_vkFreeMemory(context->GetVkDevice(), heap_context->vk_device_memory, context->GetVkAllocationCallbacks());

		return;
	}

	bool GpuHeapManager::Initialize(mem::Heap *heap, const GpuHeapManagerInfo *gpu_heap_mgr_info) {

		/* Integrity checks */
		VP_ASSERT(gpu_heap_mgr_info != nullptr);

		/* Copy info */
		m_manager_info = *gpu_heap_mgr_info;

		/* Allocate memory */
        vp::util::FixedString<0x100> heap_name;
        const char *cpu_uncached_name_array[] = {
            "CpuUncachedGpuRootHeap0",
            "CpuUncachedGpuRootHeap1",
            "CpuUncachedGpuRootHeap2",
            "CpuUncachedGpuRootHeap3",
            "CpuUncachedGpuRootHeap4",
            "CpuUncachedGpuRootHeap5",
            "CpuUncachedGpuRootHeap6",
            "CpuUncachedGpuRootHeap7",
        };
        const char *cpu_cached_name_array[] = {
            "CpuCachedGpuRootHeap0",
            "CpuCachedGpuRootHeap1",
            "CpuCachedGpuRootHeap2",
            "CpuCachedGpuRootHeap3",
            "CpuCachedGpuRootHeap4",
            "CpuCachedGpuRootHeap5",
            "CpuCachedGpuRootHeap6",
            "CpuCachedGpuRootHeap7",
        };
        const char *cpu_uncached_gpu_cached_name_array[] = {
            "CpuUncachedGpuCachedGpuRootHeap0",
            "CpuUncachedGpuCachedGpuRootHeap1",
            "CpuUncachedGpuCachedGpuRootHeap2",
            "CpuUncachedGpuCachedGpuRootHeap3",
            "CpuUncachedGpuCachedGpuRootHeap4",
            "CpuUncachedGpuCachedGpuRootHeap5",
            "CpuUncachedGpuCachedGpuRootHeap6",
            "CpuUncachedGpuCachedGpuRootHeap7",
        };
		for (u32 i = 0; i < gpu_heap_mgr_info->host_uncached_root_heap_count; ++i) {

			/* Allocate host uncached memory */
			this->AllocateContext(heap, std::addressof(m_host_uncached_heap_context_array[i]), cpu_uncached_name_array[i], gpu_heap_mgr_info->host_uncached_size_array[i], gfx::MemoryPropertyFlags::CpuUncached);
		}
		for (u32 i = 0; i < gpu_heap_mgr_info->host_cached_root_heap_count; ++i) {

			/* Allocate host cached memory */
			this->AllocateContext(heap, std::addressof(m_host_cached_heap_context_array[i]), cpu_cached_name_array[i], gpu_heap_mgr_info->host_cached_size_array[i], gfx::MemoryPropertyFlags::CpuCached);
		}
		for (u32 i = 0; i < gpu_heap_mgr_info->gpu_host_uncached_root_heap_count; ++i) {

			/* Allocate gpu host uncached memory */
			this->AllocateContext(heap, std::addressof(m_gpu_host_uncached_heap_context_array[i]), cpu_uncached_gpu_cached_name_array[i], gpu_heap_mgr_info->gpu_host_uncached_size_array[i], gfx::MemoryPropertyFlags::GpuUncached | gfx::MemoryPropertyFlags::CpuUncached);
		}

		return true;
	}

	void GpuHeapManager::Finalize() {

		/* Free memory */
		for (u32 i = 0; i < m_manager_info.host_uncached_root_heap_count; ++i) {

			/* Free host uncached memory */
			this->FreeContext(std::addressof(m_host_uncached_heap_context_array[i]));
		}
		for (u32 i = 0; i < m_manager_info.host_cached_root_heap_count; ++i) {

			/* Free host cached memory */
			this->FreeContext(std::addressof(m_host_cached_heap_context_array[i]));
		}
		for (u32 i = 0; i < m_manager_info.gpu_host_uncached_root_heap_count; ++i) {

			/* Free gpu host uncached memory */
			this->FreeContext(std::addressof(m_gpu_host_uncached_heap_context_array[i]));
		}

		return;
	}
}

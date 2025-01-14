#include <awn.hpp>

namespace awn::mem {

    namespace {

        struct GpuAddressRange {
            void *start;
            void *end;
        };

        struct HandleUnit {
            vp::util::IntrusiveRedBlackTreeNode<GpuAddressRange> rb_node;
            VkDeviceMemory vk_device_memory;
        };

        constexpr std::strong_ordering HandleUnitCompare(const GpuAddressRange &lhs, const GpuAddressRange &rhs) {
            const std::strong_ordering first  = (rhs.end <= lhs.start) ? std::strong_ordering::greater : std::strong_ordering::equal;
            const std::strong_ordering second = (lhs.end <= rhs.start) ? std::strong_ordering::less : first;
            return second;
        }

        using GpuMemoryHandleTree = vp::util::IntrusiveRedBlackTreeTraits<HandleUnit, &HandleUnit::rb_node, HandleUnitCompare>::Tree;

        constinit GpuMemoryHandleTree sAddressTree;
        constinit sys::ReaderWriterLock sTreeLock;
    }

    Result AllocateGpuMemoryHandle(GpuMemoryHandle *out_memory_handle, mem::Heap *meta_data_heap, gfx::MemoryPropertyFlags memory_property_flags, size_t size) {
        const VkMemoryAllocateFlagsInfo flags_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        };
        return AllocateGpuMemoryHandle(out_memory_handle, meta_data_heap, memory_property_flags, size, std::addressof(flags_info));
    }

    Result AllocateGpuMemoryHandle(GpuMemoryHandle *out_memory_handle, mem::Heap *meta_data_heap, gfx::MemoryPropertyFlags memory_property_flags, size_t size, const void *pnext) {

        /* Allocate device memory */
        VkDeviceMemory vk_device_memory = VK_NULL_HANDLE;
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
        const VkMemoryAllocateInfo allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = pnext,
            .allocationSize = size,
            .memoryTypeIndex = context->GetVkMemoryTypeIndex(memory_property_flags),
        };
        const VkResult vk_result = ::pfn_vkAllocateMemory(context->GetVkDevice(), std::addressof(allocate_info), context->GetVkAllocationCallbacks(), std::addressof(vk_device_memory));
        const Result result = gfx::impl::ConvertVkResultToResult(vk_result);
        RESULT_TRY(result);

        /* Skip mapping and address tree if not cpu mappable */
        if ((static_cast<u32>(memory_property_flags) & static_cast<u32>(gfx::MemoryPropertyFlags::CpuUncached | gfx::MemoryPropertyFlags::CpuCached)) == 0) {
            *out_memory_handle = { vk_device_memory, nullptr, };
            RESULT_RETURN_SUCCESS;
        }

        /* Free memory if handle unit creation or memory mapping fails */
        auto map_fail_guard0 = SCOPE_GUARD {
            ::pfn_vkFreeMemory(context->GetVkDevice(), vk_device_memory, context->GetVkAllocationCallbacks());
        };

        /* Map memory into process */
        void *memory = nullptr;
        const VkMemoryMapInfoKHR map_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO_KHR,
            .memory = vk_device_memory,
            .size = VK_WHOLE_SIZE,
        };
        const VkResult vk_result1 = ::pfn_vkMapMemory2(context->GetVkDevice(), std::addressof(map_info), std::addressof(memory));
        const Result result1 = gfx::impl::ConvertVkResultToResult(vk_result1);
        if (RESULT_FAILED(result1)) {
            RESULT_RETURN(result1);
        }

        /* Create address tree entry for lookup by address */
        HandleUnit *unit = new (meta_data_heap, alignof(HandleUnit)) HandleUnit();
        if (unit == nullptr) {
            delete unit;
            RESULT_RETURN(ResultFailedToAllocateMetaData);
        }
        map_fail_guard0.Cancel();
        unit->rb_node.SetKey({ memory, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + size), });
        unit->vk_device_memory = vk_device_memory;

        {
            sTreeLock.AcquireWriteLock();
            sAddressTree.Insert(unit);
            sTreeLock.ReleaseWriteLock();
        }

        *out_memory_handle = { vk_device_memory, memory, };

        RESULT_RETURN_SUCCESS;
    }

    void FreeGpuMemoryHandle(GpuMemoryHandle handle) {

        if (!handle.IsValid()) { return; }

        /* Remove CPU address from address tree and unmap */
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
        if (handle.memory != nullptr) {
            sTreeLock.AcquireWriteLock();
            sAddressTree.Remove({ handle.memory, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(handle.memory) + 1), });
            sTreeLock.ReleaseWriteLock();

            //const VkMemoryUnmapInfo unmap_info = {
            //    .sType = VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO,
            //    .memory = handle.vk_device_memory,
            //};
            //const VkResult vk_result = ::pfn_vkUnmapMemory2(context->GetVkDevice(), std::addressof(unmap_info));
            //const Result result = gfx::impl::ConvertVkResultToResult(vk_result);
            //RESULT_ABORT_UNLESS(result);
        }

        /* Free memory */
        ::pfn_vkFreeMemory(context->GetVkDevice(), handle.vk_device_memory, context->GetVkAllocationCallbacks());
    }

    GpuMemoryHandle FindGpuMemoryHandle(size_t *out_offset, void *address) {

        /* Ensure non-null */
        if (address == nullptr) { return {}; }

        /* Find handle from gpu root heap */
        GpuRootHeapContext *context = FindGpuRootHeapContextFromAddress(address);
        if (context != nullptr) {
            *out_offset = reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(context->base_address);
            return context->gpu_memory_handle;
        }

        /* Find handle from address tree */
        sTreeLock.AcquireReadLock();
        const HandleUnit *unit = sAddressTree.Find({ address, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + 1), });
        sTreeLock.ReleaseReadLock();
        if (unit == nullptr) { return {}; }

        void *start = unit->rb_node.GetKey().start;
        *out_offset = reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(start);

        return { unit->vk_device_memory, start, };
    }
}

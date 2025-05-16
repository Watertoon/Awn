#include <awn.hpp>

namespace awn::gfx {

    namespace impl {

        constexpr std::strong_ordering GpuAddressRangeCompare(const impl::GpuAddressRange &lhs, const impl::GpuAddressRange &rhs) {
            const std::strong_ordering first  = (rhs.end <= lhs.start) ? std::strong_ordering::greater : std::strong_ordering::equal;
            const std::strong_ordering second = (lhs.end <= rhs.start) ? std::strong_ordering::less : first;
            return second;
        }

        class MemoryPoolTree {
            public:
                using AddressTree = vp::util::IntrusiveRedBlackTreeTraits<MemoryPool, &MemoryPool::m_rb_node, GpuAddressRangeCompare>::Tree;
            private:
                AddressTree m_tree;
                sys::ReaderWriterLock m_lock;
            public:
                constexpr MemoryPoolTree() : m_tree(), m_lock() {/*...*/}
                constexpr ~MemoryPoolTree() {/*...*/}

                void Insert(MemoryPool *pool) {
                    m_lock.AcquireWriteLock();
                    m_tree.Insert(pool);
                    m_lock.ReleaseWriteLock();
                }
                void Remove(GpuAddressRange range) {
                    m_lock.AcquireWriteLock();
                    m_tree.Remove(range);
                    m_lock.ReleaseWriteLock();
                }
                MemoryPool *Find(GpuAddressRange range) {
                    m_lock.AcquireReadLock();
                    MemoryPool *pool = m_tree.Find(range);
                    m_lock.ReleaseReadLock();
                    return pool;
                }
        };

        constinit MemoryPoolTree sMemoryPoolTree;
    }

    MemoryPool *FindMemoryPool(uintptr_t *out_address_offset, void *gpu_address) {

        /* Ensure non-null */
        if (gpu_address == nullptr) { return nullptr; }

        /* Find handle from gpu root heap */
        mem::GpuRootHeapContext *context = mem::FindGpuRootHeapContextFromAddress(gpu_address);
        if (context != nullptr) {
            const uintptr_t offset = context->GetMemoryOffset(gpu_address);
            *out_address_offset = offset;
            return context->memory_pool;
        }

        /* Find handle from address tree */
        const impl::GpuAddressRange range = { gpu_address, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(gpu_address) + 1), };
        MemoryPool *memory_pool = impl::sMemoryPoolTree.Find(range);
        if (memory_pool == nullptr) { return nullptr; }

        void *start = memory_pool->GetAddress();
        const uintptr_t offset = reinterpret_cast<uintptr_t>(gpu_address) - reinterpret_cast<uintptr_t>(start);
        *out_address_offset = offset;

        return memory_pool;
    }

    Result MemoryPool::Initialize(const MemoryPoolInfo *memory_pool_info) {
        /* Handle import memory and buffer device address */
        const VkImportMemoryHostPointerInfoEXT import_info = {
            .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT,
            .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT,
            .pHostPointer = memory_pool_info->gpu_region_base,
        };
        const VkMemoryAllocateFlagsInfo flags_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .pNext = (memory_pool_info->gpu_region_base != nullptr) ? std::addressof(import_info) : nullptr,
            .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        };
        const void *pnext = std::addressof(flags_info);
        return this->InitializeImpl(memory_pool_info, pnext);
    }

    void MemoryPool::Finalize() {
        const impl::GpuAddressRange range = m_rb_node.GetKey();
        if (range.start != nullptr) {
            impl::sMemoryPoolTree.Remove(range);
        }
        if (m_vk_device_memory != VK_NULL_HANDLE) {
            impl::Context *context = impl::Context::GetInstance();
            ::pfn_vkFreeMemory(context->GetVkDevice(), m_vk_device_memory, context->GetVkAllocationCallbacks());
            m_vk_device_memory = VK_NULL_HANDLE;
            m_flags = MemoryPropertyFlags::CpuNoAccess;
            m_is_cached = false;
        }
        return;
    }

    void MemoryPool::FlushMappedRange(size_t offset, size_t size) {

        /* Skip if not cached memory */
        if (!this->IsCached()) { return; }

        const VkMappedMemoryRange range = {
            .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = this->GetVkDeviceMemory(),
            .offset = offset,
            .size   = size,
        };
        const VkResult result = ::pfn_vkFlushMappedMemoryRanges(gfx::impl::Context::GetInstance()->GetVkDevice(), 1, std::addressof(range));
        gfx::impl::ConvertVkResultToResult(result);

        return;
    }

    void MemoryPool::InvalidateMappedRange(size_t offset, size_t size) {

        /* Skip if not cached memory */
        if (!this->IsCached()) { return; }

        const VkMappedMemoryRange range = {
            .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = this->GetVkDeviceMemory(),
            .offset = offset,
            .size   = size,
        };
        const VkResult result = ::pfn_vkInvalidateMappedMemoryRanges(gfx::impl::Context::GetInstance()->GetVkDevice(), 1, std::addressof(range));
        gfx::impl::ConvertVkResultToResult(result);

        return;
    }

    Result MemoryPool::InitializeImpl(const MemoryPoolInfo *memory_pool_info, const void *pnext) {

        /* Allocate device memory */
        const MemoryPropertyFlags memory_property_flags = memory_pool_info->GetMemoryPropertyFlags();
        VkDeviceMemory vk_device_memory = VK_NULL_HANDLE;
        impl::Context *context = impl::Context::GetInstance();
        const VkMemoryAllocateInfo allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = pnext,
            .allocationSize = memory_pool_info->size,
            .memoryTypeIndex = context->GetVkMemoryTypeIndex(memory_property_flags),
        };
        const VkResult vk_result = ::pfn_vkAllocateMemory(context->GetVkDevice(), std::addressof(allocate_info), context->GetVkAllocationCallbacks(), std::addressof(vk_device_memory));
        const Result result = impl::ConvertVkResultToResult(vk_result);
        RESULT_TRY(result);
        m_vk_device_memory = vk_device_memory;
        m_flags = memory_property_flags;
        m_is_cached = (static_cast<u32>(memory_property_flags) & static_cast<u32>(gfx::MemoryPropertyFlags::CpuCached)) != 0;

        /* Skip mapping and address tree if not cpu mappable */
        if ((static_cast<u32>(memory_property_flags) & static_cast<u32>(gfx::MemoryPropertyFlags::CpuUncached | gfx::MemoryPropertyFlags::CpuCached)) == 0) {
            RESULT_RETURN_SUCCESS;
        }

        /* Map memory into process */
        auto map_fail_guard0 = SCOPE_GUARD {
            this->Finalize();
        };
        void *memory = nullptr;
        const VkMemoryMapInfoKHR map_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO_KHR,
            .memory = vk_device_memory,
            .size = VK_WHOLE_SIZE,
        };
        const VkResult vk_result1 = ::pfn_vkMapMemory2(context->GetVkDevice(), std::addressof(map_info), std::addressof(memory));
        const Result result1 = gfx::impl::ConvertVkResultToResult(vk_result1);
        RESULT_TRY(result1);

        /* Create address tree entry for lookup by address */
        m_rb_node.SetKey({ memory, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + memory_pool_info->size), });
        impl::sMemoryPoolTree.Insert(this);

        map_fail_guard0.Cancel();
        RESULT_RETURN_SUCCESS;
    }
}

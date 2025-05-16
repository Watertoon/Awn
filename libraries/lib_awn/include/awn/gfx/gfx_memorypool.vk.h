#pragma once

namespace awn::gfx {

    namespace impl {

        struct GpuAddressRange {
            void *start;
            void *end;
        };

        class MemoryPoolTree;
    }
    class MemoryPool;

    MemoryPool *FindMemoryPool(uintptr_t *out_address_offset, void *gpu_address);

    class MemoryPool {
        public:
            friend class impl::MemoryPoolTree;
        private:
            vp::util::IntrusiveRedBlackTreeNode<impl::GpuAddressRange> m_rb_node;
            VkDeviceMemory m_vk_device_memory;
            gfx::MemoryPropertyFlags m_flags;
            bool m_is_cached;
        public:
            constexpr MemoryPool() : m_rb_node(), m_vk_device_memory(), m_flags(), m_is_cached() {/*...*/}
            constexpr ~MemoryPool() {/*...*/}

            Result Initialize(const MemoryPoolInfo *memory_pool_info);
            void Finalize();

            constexpr void *GetAddress() const {
                return this->m_rb_node.GetKey().start;
            }
            constexpr bool IsValid() const {
                return this->m_vk_device_memory != VK_NULL_HANDLE;
            }
        public:
            constexpr bool IsCached() const {
                return this->m_is_cached;
            }

            void FlushMappedRange(size_t offset, size_t size);
            void InvalidateMappedRange(size_t offset, size_t size);
        public:
            constexpr VkDeviceMemory GetVkDeviceMemory() const {
                return this->m_vk_device_memory;
            }
            Result InitializeImpl(const MemoryPoolInfo *memory_pool_info, const void *pnext);
    };
    static_assert(sizeof(MemoryPool) <= vp::res::cMaxGfxMemoryPoolSize);
}

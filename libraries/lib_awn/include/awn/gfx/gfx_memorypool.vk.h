#pragma once

namespace awn::gfx {

    //namespace impl {
    //
    //    struct GpuAddressRange {
    //        void *start;
    //        void *end;
    //    };
    //
    //    class MemoryPoolTree;
    //}
    //class MemoryPool;
    //
    //MemoryPool *FindMemoryPool(uintptr_t *out_address_offset, void *gpu_address);

    class MemoryPool {
        //public:
        //    friend class impl::MemoryPoolTree;
        //private:
        //    vp::util::IntrusiveRedBlackTreeNode<impl::GpuAddressRange> m_rb_node;
            VkDeviceMemory m_vk_device_memory;
            gfx::MemoryPropertyFlags m_flags;
            void *m_address;
            bool m_is_cached;
        public:
            constexpr MemoryPool() : m_vk_device_memory(), m_flags(), m_is_cached() {/*...*/}
            constexpr ~MemoryPool() {/*...*/}

            Result Initialize(const MemoryPoolInfo *memory_pool_info);
            void Finalize();

            constexpr void *GetAddress() const {
                return m_address;
            }
            void *GetAddress(size_t offset) const { return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this->GetAddress()) + offset); }
            uintptr_t GetOffset(void *address) const { return reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(this->GetAddress()); }
            constexpr bool IsValid() const {
                return this->m_vk_device_memory != VK_NULL_HANDLE;
            }
        public:
            constexpr bool IsCpuCached() const {
                return this->m_is_cached;
            }

            void FlushMappedRange(size_t offset, size_t size);
            void InvalidateMappedRange(size_t offset, size_t size);
        public:
            constexpr VkDeviceMemory GetVkDeviceMemory() const {
                return this->m_vk_device_memory;
            }
            Result InitializeImpl(const MemoryPoolInfo *memory_pool_info, u32 restrict_memory_type_bits, const void *pnext);
    };
    static_assert(sizeof(MemoryPool) <= vp::res::cMaxGfxMemoryPoolSize);
}

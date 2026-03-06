/*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

namespace awn::gfx::impl {

    struct DescriptorMemoryManagerInfo {
    //    u32 max_command_buffer_count;
    //    u32 max_command_list_count;
    #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
        u32 max_seperate_texture_count;
        u32 max_seperate_sampler_count;
    #else
        u32 per_stage_max_allocation_count;
        u32 per_stage_memory_size;
        u32 transient_descriptor_memory_size;
    #endif
    };

    //struct CommandListUnit {
    //    VkCommandBuffer vk_command_buffer;
    //    void *descriptor_memory;
    //#ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
    //    VkDeviceAddress descriptor_memory_address;
    //#endif
    //    size_t descriptor_memory_size;
    //    size_t descriptor_memory_iter;
    //    CommandPoolId pool_id;
    //
    //    constexpr void ResetDescriptorMemory() { this->descriptor_memory_iter = 0; }
    //#ifndef AWN_ENABLE_VK_DESCRIPTOR_HEAP
    //    void *AllocateDescriptorMemory(size_t memory_size) {
    //        const size_t memory_offset = this->descriptor_memory_iter;
    //        const size_t next_offset = vp::util::AlignUp(memory_offset + memory_size, impl::Context::cTargetDescriptorBufferOffsetAlignment);
    //        VP_ASSERT(next_offset <= this->descriptor_memory_size);
    //        this->descriptor_memory_iter = next_offset;
    //        void *memory = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this->descriptor_memory) + memory_offset);
    //        return memory;
    //    }
    //#else
    //    void *AllocateDescriptorMemory(VkDeviceAddress *out_dev_addr, size_t memory_size) {
    //        const size_t memory_offset = this->descriptor_memory_iter;
    //        const size_t next_offset = vp::util::AlignUp(memory_offset + memory_size, impl::Context::cTargetUniformBufferAlignment);
    //        VP_ASSERT(next_offset <= this->descriptor_memory_size);
    //        this->descriptor_memory_iter = next_offset;
    //        *out_dev_addr = this->descriptor_memory_address + next_offset;
    //        void *memory = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this->descriptor_memory) + memory_offset);
    //        return memory;
    //    }
    //#endif
    //};

    class DescriptorMemoryManager {
        public:
            //using UnitAllocator = vp::util::AtomicIndexAllocator<u32>;
            //using UnitArray = vp::util::HeapArray<CommandListUnit>;
        private:
            //UnitAllocator      m_unit_allocator;
            //UnitArray          m_unit_array;
            //mem::UnitHeap     *m_per_stage_staging_heap;
        #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
            //void              *m_memory_per_stage;
            //Buffer             m_per_stage_buffer;
            //VkDeviceAddress    m_per_stage_device_address;
            MemoryPool         m_descriptor_heap_memory_pool;
            VkBuffer           m_descriptor_heap_buffer;
            VkBindHeapInfoEXT  m_bind_heap_info_resource;
            VkBindHeapInfoEXT  m_bind_heap_info_sampler;
            void              *m_memory_texture;
            void              *m_memory_image;
            void              *m_memory_sampler;
            size_t             m_texture_size;
            size_t             m_image_size;
            size_t             m_sampler_size;
        #else
            VkDescriptorBufferBindingInfoEXT m_binding_info;
            MemoryPool         m_memory_pool;
            mem::SeparateHeap *m_per_stage_heap;
            VkBuffer           m_descriptor_vk_buffer;
            GpuAddress         m_descriptor_buffer_address;
            void              *m_memory_bindless;
            void              *m_memory_per_stage;
            size_t             m_bindless_size;
            size_t             m_texture_combined_size;
            size_t             m_buffer_combined_size;
            uintptr_t          m_transient_head;
            uintptr_t          m_transient_iter;
            size_t             m_transient_size;
        #endif
        public:
            AWN_SINGLETON_TRAITS(DescriptorMemoryManager);
        public:
            constexpr DescriptorMemoryManager() :
        #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
            , /*m_memory_per_stage(), m_per_stage_buffer(), m_per_stage_device_address(),*/ m_descriptor_heap_memory_pool(), m_descriptor_heap_buffer(), m_bind_heap_info_resource(), m_bind_heap_info_sampler(), m_memory_texture(), m_memory_image(), m_memory_sampler(), m_texture_size(), m_image_size(), m_sampler_size()
        #else
            m_binding_info(), m_memory_pool(), m_per_stage_heap(), m_descriptor_vk_buffer(), m_descriptor_buffer_address(), m_memory_bindless(), m_memory_per_stage(), m_bindless_size(), m_texture_combined_size(), m_buffer_combined_size(), m_transient_head(), m_transient_iter(), m_transient_size()
        #endif
            {/*...*/}
            constexpr ~DescriptorMemoryManager() = default;

            void Initialize(mem::Heap *heap, const DescriptorMemoryManagerInfo *mgr_info);
            void Finalize();
        public:
            //Result AllocateCommandListUnit(CommandListUnit **out_unit, size_t desc_memory_size, CommandPoolId pool_id, bool is_primary);
            //void FreeCommandListUnit(CommandListUnit *unit);

            //void *AllocateStagingMemory();
            //void FreeStagingMemory(void *address);

        #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
            constexpr const VkBindHeapInfoEXT *GetVkBindHeapInfoResource() const {
                return std::addressof(m_bind_heap_info_resource);
            }
            constexpr const VkBindHeapInfoEXT *GetVkBindHeapInfoSampler() const {
                return std::addressof(m_bind_heap_info_sampler);
            }

            constexpr void *GetTextureDescriptorMemory() const {
                return m_memory_texture;
            }
            constexpr void *GetImageDescriptorMemory() const {
                return m_memory_image;
            }
            constexpr void *GetSamplerDescriptorMemory() const {
                return m_memory_sampler;
            }
            constexpr size_t GetTextureSize() const { return m_texture_size; }
            constexpr size_t GetImageSize() const { return m_image_size; }
            constexpr size_t GetSamplerSize() const { return m_sampler_size; }
        #else
            void *AllocateResourceSetMemory(uintptr_t *out_offset, size_t size) {
                const size_t align_size = vp::util::AlignUp(size, Context::cTargetDescriptorBufferOffsetAlignment);
                void *memory = m_per_stage_heap->TryAllocate(align_size, Context::cTargetDescriptorBufferOffsetAlignment);
                if (memory == nullptr) { return nullptr; }
                *out_offset = reinterpret_cast<uintptr_t>(memory) - reinterpret_cast<uintptr_t>(m_memory_per_stage);
                return memory;
            }
            void FreeResourceSetMemory(void *memory) {
                m_per_stage_heap->Free(memory);
            }

            void *AllocateTransientMemory(uintptr_t *out_offset, size_t size) {
                const size_t align_size = vp::util::AlignUp(size, Context::cTargetDescriptorBufferOffsetAlignment);
                const uintptr_t offset = vp::util::InterlockedFetchAddRelaxed(std::addressof(m_transient_iter), align_size);
                VP_ASSERT(offset + size < m_transient_size);
                *out_offset = offset;
                void *ret = reinterpret_cast<void*>(m_transient_head + offset);
                return ret;
            }
            constexpr void ResetTransientMemory() {
                m_transient_iter = 0;
            }

            constexpr const VkDescriptorBufferBindingInfoEXT *GetBindingInfo() const {
                return std::addressof(m_binding_info);
            }
            constexpr GpuAddress GetDescriptorBufferAddress() const {
                return m_descriptor_buffer_address;
            }

            constexpr void *GetBindlessDescriptorMemory() const {
                return m_memory_bindless;
            }
            constexpr size_t GetCombinedTextureSize() const { return m_texture_combined_size; }
            constexpr size_t GetCombinedBufferSize() const { return m_buffer_combined_size; }
        #endif

            constexpr void *GetPerStageDescriptorMemory() const {
                return m_memory_per_stage;
            }
    };
}

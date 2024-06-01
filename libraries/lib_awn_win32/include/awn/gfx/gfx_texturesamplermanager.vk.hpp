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

namespace awn::gfx {

    using DescriptorSlot = u64;
    constexpr inline DescriptorSlot cInvalidDescriptorSlot = 0;
    
    class CommandBufferBase;

    struct TextureSamplerManagerInfo {
        u32    max_texture_unit_count;
        u32    max_sampler_unit_count;
        size_t texture_memory_size;
    };

    class TextureSamplerManager {
        public:
            friend class CommandBuffer;
        public:
            struct SamplerUnit {
                vp::util::IntrusiveRedBlackTreeNode<u32> rb_node;
                u32                                      reference_count;
                u32                                      handle;
                Sampler                                  sampler;

                constexpr  SamplerUnit() : rb_node(), reference_count(), handle(), sampler() {/*...*/}
                constexpr ~SamplerUnit() {/*...*/}
            };
            struct TextureUnit {
                u32          reference_count;
                size_t       separate_memory;
                Texture      texture;
                TextureView  texture_view;

                static constexpr inline size_t cInvalidMemoryOffset = 0xffff'ffff'ffff'ffff;

                constexpr  TextureUnit() : reference_count(), separate_memory(cInvalidMemoryOffset), texture(), texture_view() {/*...*/}
                constexpr ~TextureUnit() {/*...*/}
            };
        public:
            using TextureUnitArray   = vp::util::HeapArray<TextureUnit>;
            using SamplerUnitArray   = vp::util::HeapArray<SamplerUnit>;
            using TextureHandleTable = vp::util::AtomicIndexAllocator<u16>;
            using SamplerHandleTable = vp::util::AtomicIndexAllocator<u16>;
            using SamplerMap         = vp::util::IntrusiveRedBlackTreeTraits<SamplerUnit, &SamplerUnit::rb_node>::Tree;
        private:
            VkDeviceMemory               m_vk_device_memory_texture;
            mem::SeparateHeap           *m_separate_heap;
            VkBuffer                     m_texture_descriptor_vk_buffer;
            VkBuffer                     m_sampler_descriptor_vk_buffer;
            VkDeviceAddress              m_texture_vk_device_address;
            VkDeviceAddress              m_sampler_vk_device_address;
            void                        *m_texture_descriptor_gpu_address;
            void                        *m_sampler_descriptor_gpu_address;
            u16                          m_texture_descriptor_size;
            u16                          m_sampler_descriptor_size;
            u16                          m_texture_descriptor_stride;
            u16                          m_sampler_descriptor_stride;
            TextureHandleTable           m_texture_handle_table;
            TextureUnitArray             m_texture_unit_array;
            SamplerMap                   m_sampler_map;
            SamplerHandleTable           m_sampler_handle_table;
            SamplerUnitArray             m_sampler_unit_array;
            sys::ServiceCriticalSection  m_sampler_tree_cs;
        public:
            AWN_SINGLETON_TRAITS(TextureSamplerManager);
        public:
            constexpr  TextureSamplerManager() : m_vk_device_memory_texture(VK_NULL_HANDLE), m_separate_heap(nullptr), m_texture_descriptor_vk_buffer(), m_sampler_descriptor_vk_buffer(), m_texture_vk_device_address(), m_sampler_vk_device_address(), m_texture_descriptor_gpu_address{}, m_sampler_descriptor_gpu_address{},
                                                 m_texture_descriptor_size(), m_sampler_descriptor_size(), m_texture_descriptor_stride(), m_sampler_descriptor_stride(), m_texture_handle_table(), m_texture_unit_array{}, m_sampler_map(), m_sampler_handle_table(), m_sampler_unit_array{}, m_sampler_tree_cs() {/*...*/}
            constexpr ~TextureSamplerManager() {/*...*/}

            void Initialize(mem::Heap *heap, mem::Heap *gpu_heap, const TextureSamplerManagerInfo *manager_info);
            void Finalize();

            DescriptorSlot RegisterTextureView(TextureInfo *texture_info, TextureViewInfo *texture_view);
            DescriptorSlot RegisterTextureView(void *gpu_texture_memory, TextureInfo *texture_info, TextureViewInfo *texture_view);
            DescriptorSlot ReferenceTextureView(DescriptorSlot texture_slot);
            void           UnregisterTextureView(DescriptorSlot texture_slot);

            DescriptorSlot RegisterSampler(SamplerInfo *sampler_info);
            DescriptorSlot ReferenceSampler(DescriptorSlot sampler_slot);
            void           UnregisterSampler(DescriptorSlot sampler_slot);

            constexpr VkDeviceAddress GetTextureDescriptorBufferAddress() const { return m_texture_vk_device_address; }
            constexpr VkDeviceAddress GetSamplerDescriptorBufferAddress() const { return m_sampler_vk_device_address; }

            constexpr Texture *GetTexture(DescriptorSlot texture_slot) {
                return std::addressof(m_texture_unit_array[texture_slot].texture);
            }

            constexpr TextureView *GetTextureView(DescriptorSlot texture_slot) {
                return std::addressof(m_texture_unit_array[texture_slot].texture_view);
            }

            constexpr Sampler *GetSampler(DescriptorSlot sampler_slot) {
                return std::addressof(m_sampler_unit_array[sampler_slot].sampler);
            }
    };
}

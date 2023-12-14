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
        u32    max_texture_handles;
        u32    max_sampler_handles;
        size_t texture_memory_size;
    };

    class TextureSamplerManager {
        public:
            friend class CommandBuffer;
        public:
            struct SamplerNode {
                vp::util::IntrusiveRedBlackTreeNode<u32> rb_node;
                u32                                      reference_count;
                u32                                      handle;
                Sampler                                  sampler;

                constexpr  SamplerNode() : rb_node(), reference_count(), handle(), sampler() {/*...*/}
                constexpr ~SamplerNode() {/*...*/}
            };
            struct TextureNode {
                u32          reference_count;
                size_t       separate_memory;
                Texture      texture;
                TextureView  texture_view;

                static constexpr inline size_t cInvalidMemoryOffset = 0xffff'ffff'ffff'ffff;

                constexpr  TextureNode() : reference_count(), separate_memory(cInvalidMemoryOffset), texture(), texture_view() {/*...*/}
                constexpr ~TextureNode() {/*...*/}
            };
        public:
            using TextureHandleTable = vp::util::FixedAtomicIndexAllocator<u32, Context::cTargetMaxTextureDescriptorCount>;
            using SamplerHandleTable = vp::util::FixedAtomicIndexAllocator<u16, Context::cTargetMaxSamplerDescriptorCount>;
            using SamplerMap         = vp::util::IntrusiveRedBlackTreeTraits<SamplerNode, &SamplerNode::rb_node>::Tree;
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
            TextureNode                  m_texture_array[Context::cTargetMaxTextureDescriptorCount];
            SamplerMap                   m_sampler_map;
            SamplerHandleTable           m_sampler_handle_table;
            SamplerNode                  m_sampler_array[Context::cTargetMaxSamplerDescriptorCount];
            sys::ServiceCriticalSection  m_sampler_tree_cs;
        public:
            AWN_SINGLETON_TRAITS(TextureSamplerManager);
        public:
            constexpr  TextureSamplerManager() : m_vk_device_memory_texture(VK_NULL_HANDLE), m_separate_heap(nullptr), m_texture_descriptor_vk_buffer(), m_sampler_descriptor_vk_buffer(), m_texture_vk_device_address(), m_sampler_vk_device_address(), m_texture_descriptor_gpu_address{}, m_sampler_descriptor_gpu_address{},
                                                 m_texture_descriptor_size(), m_sampler_descriptor_size(), m_texture_descriptor_stride(), m_sampler_descriptor_stride(), m_texture_handle_table(), m_texture_array{}, m_sampler_map(), m_sampler_handle_table(), m_sampler_array{}, m_sampler_tree_cs() {/*...*/}
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

            void BindDescriptorBuffers(CommandBufferBase *command_buffer);

            Texture     *TryGetTextureByHandle(DescriptorSlot texture_slot);
            TextureView *TryGetTextureViewByHandle(DescriptorSlot texture_slot);
            Sampler     *TryGetSamplerByHandle(DescriptorSlot sampler_slot);
    };
}

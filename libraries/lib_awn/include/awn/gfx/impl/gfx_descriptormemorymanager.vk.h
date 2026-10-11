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
        u32 max_separate_texture_count;
        u32 max_separate_sampler_count;
    };

    class DescriptorMemoryManager {
        private:
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
        public:
            AWN_SINGLETON_TRAITS(DescriptorMemoryManager);
        public:
            constexpr DescriptorMemoryManager() : m_descriptor_heap_memory_pool(), m_descriptor_heap_buffer(), m_bind_heap_info_resource(), m_bind_heap_info_sampler(), m_memory_texture(), m_memory_image(), m_memory_sampler(), m_texture_size(), m_image_size(), m_sampler_size() {/*...*/}
            constexpr ~DescriptorMemoryManager() = default;

            Result Initialize(mem::Heap *heap, const DescriptorMemoryManagerInfo *mgr_info);
            void Finalize();
        public:
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
    };
}

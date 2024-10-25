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
        u32 max_command_buffer_count;
        size_t per_stage_memory_size;
        bool is_enable_separate_texture_sampler;
    };

    class DescriptorMemoryManager {
        public:
            struct Staging {
                char descriptor_memory_ts_i_st_ss[8][Context::cTargetMaxPerStageMemorySizeTextureSamplerImageSeparate];
                char descriptor_memory_ubo_ssbo[8][Context::cTargetMaxPerStageMemorySizeUniformBufferStorageBuffer];
            };
        private:
            mem::SeparateHeap *m_descriptor_heap;
            mem::UnitHeap     *m_staging_heap;
            void              *m_memory;
            VkBuffer           m_descriptor_vk_buffer;
            GpuBufferAddress   m_descriptor_buffer_address;
            bool               m_is_enable_separate_texture_sampler;
        public:
            AWN_SINGLETON_TRAITS(DescriptorMemoryManager);
        public:
            constexpr DescriptorMemoryManager() : m_descriptor_heap(), m_staging_heap(), m_memory(), m_descriptor_vk_buffer(), m_descriptor_buffer_address(), m_is_enable_separate_texture_sampler() {/*...*/}
            constexpr ~DescriptorMemoryManager() {/*...*/}

            void Initialize(mem::Heap *heap, mem::Heap *gpu_heap, const DescriptorMemoryManagerInfo *mgr_info);
            void Finalize();

            void *AllocateCommandMemory(size_t *out_offset, size_t size);
            void FreeCommandMemory(void *address);

            Staging *AllocateStagingMemory();
            void FreeStagingMemory(Staging *address);

            constexpr GpuBufferAddress GetDescriptorBufferAddress() const {
                return m_descriptor_buffer_address;
            }
    };
}

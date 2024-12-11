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
        u32 per_stage_memory_size;
    };

    class DescriptorMemoryManager {
        private:
            mem::SeparateHeap *m_per_stage_heap;
            mem::UnitHeap     *m_per_stage_staging_heap;
            void              *m_memory_bindless;
            void              *m_memory_per_stage;
            VkBuffer           m_descriptor_vk_buffer;
            GpuBufferAddress   m_descriptor_buffer_address;
            size_t             m_ts_i_size;
            size_t             m_ub_sb_size;
        public:
            AWN_SINGLETON_TRAITS(DescriptorMemoryManager);
        public:
            constexpr DescriptorMemoryManager() : m_per_stage_heap(), m_per_stage_staging_heap(), m_memory_bindless(), m_memory_per_stage(), m_descriptor_vk_buffer(), m_descriptor_buffer_address(), m_ts_i_size(), m_ub_sb_size() {/*...*/}
            constexpr ~DescriptorMemoryManager() {/*...*/}

            void Initialize(mem::Heap *heap, mem::Heap *gpu_heap, const DescriptorMemoryManagerInfo *mgr_info);
            void Finalize();

        public:
            void *AllocateCommandMemory(size_t *out_offset, size_t size);
            void FreeCommandMemory(void *address);

            void *AllocateStagingMemory();
            void FreeStagingMemory(void *address);

            constexpr GpuBufferAddress GetDescriptorBufferAddress() const {
                return m_descriptor_buffer_address;
            }

            constexpr void *GetBindlessDescriptorMemory() const {
                return m_memory_bindless;
            }

            constexpr void *GetPerStageDescriptorMemory() const {
                return m_memory_per_stage;
            }
    };
}

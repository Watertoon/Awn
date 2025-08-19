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
        u32 max_command_list_count;
        u32 per_stage_memory_size;
    };

    struct CommandListUnit {
        VkCommandBuffer vk_command_buffer;
        void *descriptor_memory;
        size_t descriptor_memory_size;
        size_t descriptor_memory_iter;
        CommandPoolId pool_id;

        constexpr void ResetDescriptorMemory() { this->descriptor_memory_iter = 0; }
        void *AllocateDescriptorMemory(size_t memory_size) {
            const size_t memory_offset = this->descriptor_memory_iter;
            const size_t next_offset = vp::util::AlignUp(memory_offset + memory_size, impl::Context::cTargetDescriptorBufferOffsetAlignment);
            VP_ASSERT(next_offset <= this->descriptor_memory_size);
            this->descriptor_memory_iter = next_offset;
            void *memory = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this->descriptor_memory) + memory_offset);
            return memory;
        }
    };

    class DescriptorMemoryManager {
        public:
            using UnitAllocator = vp::util::AtomicIndexAllocator<u32>;
            using UnitArray = vp::util::HeapArray<CommandListUnit>;
        private:
            UnitAllocator      m_unit_allocator;
            UnitArray          m_unit_array;
            mem::SeparateHeap *m_per_stage_heap;
            mem::UnitHeap     *m_per_stage_staging_heap;
            VkBuffer           m_descriptor_vk_buffer;
            GpuAddress         m_descriptor_buffer_address;
            void              *m_memory_bindless;
            void              *m_memory_per_stage;
            size_t             m_bindless_size;
            size_t             m_texture_combined_size;
            size_t             m_buffer_combined_size;
        public:
            AWN_SINGLETON_TRAITS(DescriptorMemoryManager);
        public:
            constexpr DescriptorMemoryManager() : m_unit_allocator(), m_unit_array(), m_per_stage_heap(), m_per_stage_staging_heap(), m_descriptor_vk_buffer(), m_descriptor_buffer_address(), m_memory_bindless(), m_memory_per_stage(), m_bindless_size(), m_texture_combined_size(), m_buffer_combined_size() {/*...*/}
            constexpr ~DescriptorMemoryManager() {/*...*/}

            void Initialize(mem::Heap *heap, mem::Heap *gpu_heap, const DescriptorMemoryManagerInfo *mgr_info);
            void Finalize();
        public:
            Result AllocateCommandListUnit(CommandListUnit **out_unit, size_t desc_memory_size, CommandPoolId pool_id, bool is_primary);
            void FreeCommandListUnit(CommandListUnit *unit);

            void *AllocateStagingMemory();
            void FreeStagingMemory(void *address);

            constexpr GpuAddress GetDescriptorBufferAddress() const {
                return m_descriptor_buffer_address;
            }

            constexpr void *GetBindlessDescriptorMemory() const {
                return m_memory_bindless;
            }

            constexpr void *GetPerStageDescriptorMemory() const {
                return m_memory_per_stage;
            }

            constexpr size_t GetCombinedTextureSize() const { return m_texture_combined_size; }
            constexpr size_t GetCombinedBufferSize() const { return m_buffer_combined_size; }
    };
}

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

    class DescriptorMemoryManager {
        private:
            void            *m_memory_array[3];
            VkBuffer         m_descriptor_vk_buffer_array[3];
            VkDeviceAddress  m_descriptor_vk_device_address_array[3];
            u32              m_memory_iter;
            u32              m_memory_size;
            u32              m_current_index;
        public:
            AWN_SINGLETON_TRAITS(DescriptorMemoryManager);
        public:
            constexpr DescriptorMemoryManager() : m_memory_array{}, m_descriptor_vk_buffer_array{}, m_descriptor_vk_device_address_array{}, m_memory_iter(), m_memory_size(), m_current_index() {/*...*/}
            constexpr ~DescriptorMemoryManager() {/*...*/}

            void Initialize(mem::Heap *heap, mem::Heap *gpu_heap, size_t memory_size);
            void Finalize();

            constexpr VkDeviceAddress GetDescriptorBufferAddress() const {
                return m_descriptor_vk_device_address_array[m_current_index];
            }

            constexpr void AdvanceFrame() {
                m_current_index = (1 < m_current_index) ? 0 : ++m_current_index;
                m_memory_iter   = 0;
            }

            size_t AllocateDescriptorMemory(u32 memory_size);

            void *GetMemoryOffset(size_t offset);
    };
}

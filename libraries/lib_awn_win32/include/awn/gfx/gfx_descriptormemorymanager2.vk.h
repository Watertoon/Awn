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
            mem::SeparateHeap *m_descriptor_heap;
            void              *m_memory;
            VkBuffer           m_descriptor_vk_buffer;
            VkDeviceAddress    m_descriptor_vk_device_address;
        public:
            AWN_SINGLETON_TRAITS(DescriptorMemoryManager);
        public:
            constexpr DescriptorMemoryManager() : m_descriptor_heap(), m_memory(), m_descriptor_vk_buffer(), m_descriptor_vk_device_address() {/*...*/}
            constexpr ~DescriptorMemoryManager() {/*...*/}

            void Initialize(mem::Heap *heap, mem::Heap *gpu_heap, size_t memory_size, u32 max_command_buffer_count);
            void Finalize();

            void *AllocateCommandMemory(size_t *out_offset, size_t size);
            void FreeCommandMemory(void *address);

            constexpr VkDeviceAddress GetDescriptorBufferAddress() const {
                return m_descriptor_vk_device_address;
            }
    };
}

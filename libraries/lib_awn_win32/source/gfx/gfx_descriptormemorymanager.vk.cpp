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
#include <awn.hpp>

namespace awn::gfx {

    AWN_SINGLETON_TRAITS_IMPL(DescriptorMemoryManager);

    void DescriptorMemoryManager::Initialize([[maybe_unused]] mem::Heap *heap, mem::Heap *gpu_heap, size_t memory_size) {

        /* Allocate descriptor memory */
        m_memory_size     = memory_size;
        m_memory_array[0] = ::operator new(memory_size, gpu_heap, Context::cTargetDescriptorBufferAlignment);
        m_memory_array[1] = ::operator new(memory_size, gpu_heap, Context::cTargetDescriptorBufferAlignment);
        m_memory_array[2] = ::operator new(memory_size, gpu_heap, Context::cTargetDescriptorBufferAlignment);

        /* Create descriptor buffers */
        m_descriptor_vk_buffer_array[0] = gfx::CreateVkBuffer(m_memory_array[0], VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, memory_size);
        m_descriptor_vk_buffer_array[1] = gfx::CreateVkBuffer(m_memory_array[1], VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, memory_size);
        m_descriptor_vk_buffer_array[2] = gfx::CreateVkBuffer(m_memory_array[2], VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, memory_size);

        /* Get buffer device addresses */
        m_descriptor_vk_device_address_array[0] = gfx::GetVkDeviceAddress(m_descriptor_vk_buffer_array[0]);
        m_descriptor_vk_device_address_array[1] = gfx::GetVkDeviceAddress(m_descriptor_vk_buffer_array[1]);
        m_descriptor_vk_device_address_array[2] = gfx::GetVkDeviceAddress(m_descriptor_vk_buffer_array[2]);

        /* Reset memory iters */
        m_memory_iter   = 0;
        m_current_index = 0;

        return;
    }

    void DescriptorMemoryManager::Finalize() {
        if (m_memory_array[0] != nullptr) {
            gfx::DestroyVkBuffer(m_descriptor_vk_buffer_array[0]);
            ::operator delete(m_memory_array[0]);
            m_memory_array[0] = nullptr;
        }
        if (m_memory_array[1] != nullptr) {
            gfx::DestroyVkBuffer(m_descriptor_vk_buffer_array[1]);
            ::operator delete(m_memory_array[1]);
            m_memory_array[1] = nullptr;
        }
        if (m_memory_array[2] != nullptr) {
            gfx::DestroyVkBuffer(m_descriptor_vk_buffer_array[2]);
            ::operator delete(m_memory_array[2]);
            m_memory_array[2] = nullptr;
        }
    }

    size_t DescriptorMemoryManager::AllocateDescriptorMemory(u32 memory_size) {
        const u32 memory_offset = vp::util::InterlockedFetchAdd(std::addressof(m_memory_iter), memory_size);
        VP_ASSERT(memory_offset < m_memory_size);
        return memory_offset;
    }

    void *DescriptorMemoryManager::GetMemoryOffset(size_t offset) {
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_memory_array[m_current_index]) + offset);
    }
}

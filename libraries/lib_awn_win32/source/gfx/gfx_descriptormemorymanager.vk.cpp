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

    void DescriptorMemoryManager::Initialize(mem::Heap *heap, mem::Heap *gpu_heap, size_t memory_size, u32 command_buffer_count) {

        /* Allocate descriptor buffer */
        m_memory = ::operator new(memory_size, gpu_heap, Context::cTargetDescriptorBufferAlignment);
        m_descriptor_vk_buffer = gfx::CreateVkBuffer(m_memory, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, memory_size);
        m_descriptor_vk_device_address = gfx::GetVkDeviceAddress(m_descriptor_vk_buffer);

        /* Create separate heap */
        const size_t management_size = sizeof(mem::SeparateHeap) + mem::SeparateHeap::CalculateManagementAreaSize(command_buffer_count);
        void *management_memory = ::operator new(management_size, heap, alignof(mem::SeparateHeap));
        m_descriptor_heap = mem::SeparateHeap::Create("awn::gfx::DescriptorMemoryHeap", management_memory, management_size, memory_size, true);

        return;
    }

    void DescriptorMemoryManager::Finalize() {
        if (m_memory != nullptr) {
            gfx::DestroyVkBuffer(m_descriptor_vk_buffer);
            ::operator delete(m_memory);
            m_memory = nullptr;
            m_descriptor_vk_device_address = 0;
        }
        if (m_descriptor_heap != nullptr) {
            ::operator delete(m_descriptor_heap);
            m_descriptor_heap = nullptr;
        }
    }

    void *DescriptorMemoryManager::AllocateCommandMemory(size_t *out_offset, size_t size) {
        void *base_address = m_descriptor_heap->TryAllocate(size, Context::cTargetDescriptorBufferAlignment);
        const size_t offset = reinterpret_cast<uintptr_t>(base_address) - mem::SeparateHeap::cOffsetBase;
        *out_offset = offset;
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_memory) + offset);
    }

    void DescriptorMemoryManager::FreeCommandMemory(void *address) {
        void *base_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(m_memory) + mem::SeparateHeap::cOffsetBase);
        m_descriptor_heap->Free(base_address);
    }
}

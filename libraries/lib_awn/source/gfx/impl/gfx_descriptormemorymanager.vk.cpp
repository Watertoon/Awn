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

namespace awn::gfx::impl {

    AWN_SINGLETON_TRAITS_IMPL(DescriptorMemoryManager);

    void DescriptorMemoryManager::Initialize(mem::Heap *heap, mem::Heap *gpu_heap, const DescriptorMemoryManagerInfo *mgr_info) {

        /* Allocate descriptor buffer */
        m_memory = ::operator new(mgr_info->per_stage_memory_size, gpu_heap, Context::cTargetDescriptorBufferAlignment);
        m_descriptor_vk_buffer = gfx::impl::CreateVkBuffer(m_memory, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, mgr_info->per_stage_memory_size);
        m_descriptor_buffer_address = gfx::impl::GetGpuBufferAddress(m_descriptor_vk_buffer);

        /* Create descriptor heap */
        const size_t management_size = sizeof(mem::SeparateHeap) + mem::SeparateHeap::CalculateManagementAreaSize(mgr_info->max_command_buffer_count);
        void *management_memory = ::operator new(management_size, heap, alignof(mem::SeparateHeap));
        m_descriptor_heap = mem::SeparateHeap::Create("awn::gfx::DescriptorHeap", management_memory, management_size, mgr_info->per_stage_memory_size, true);

        /* Create staging heap */
        m_is_enable_separate_texture_sampler = mgr_info->is_enable_separate_texture_sampler;
        const size_t block_size = (mgr_info->is_enable_separate_texture_sampler) ? sizeof(StagingExt) : sizeof(Staging);
        m_staging_heap = mem::UnitHeap::TryCreate("awn::gfx::DescriptorStagingHeap", heap, block_size, mgr_info->max_command_buffer_count, alignof(size_t));

        return;
    }

    void DescriptorMemoryManager::Finalize() {
        if (m_memory != nullptr) {
            gfx::impl::DestroyVkBuffer(m_descriptor_vk_buffer);
            ::operator delete(m_memory);
            m_memory = nullptr;
            m_descriptor_buffer_address = cInvalidGpuBufferAddress;
        }
        if (m_descriptor_heap != nullptr) {
            ::operator delete(m_descriptor_heap);
            m_descriptor_heap = nullptr;
        }
        if (m_staging_heap != nullptr) {
            ::operator delete(m_staging_heap);
            m_staging_heap = nullptr;
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

    DescriptorMemoryManager::Staging *DescriptorMemoryManager::AllocateStagingMemory() {
        const size_t block_size = (m_is_enable_separate_texture_sampler) ? sizeof(StagingExt) : sizeof(Staging);
        void *memory = m_staging_heap->TryAllocate(block_size, alignof(size_t));
        return reinterpret_cast<Staging*>(memory);
    }

    void DescriptorMemoryManager::FreeStagingMemory(Staging *address) {
        m_staging_heap->Free(address);
    }
}

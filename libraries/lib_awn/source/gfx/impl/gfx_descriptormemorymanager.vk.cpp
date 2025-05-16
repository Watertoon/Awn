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

        /* Get bindless texture view and sampler count */
        Context *context = Context::GetInstance();
        size_t bindless_descriptor_memory_size = context->GetBindlessDescriptorLayoutSize();   
        m_bindless_descriptor_memory_size = bindless_descriptor_memory_size;

        /* Allocate descriptor buffer */
        const size_t buffer_size = bindless_descriptor_memory_size + mgr_info->per_stage_memory_size;
        m_memory_bindless = ::operator new(buffer_size, gpu_heap, Context::cTargetDescriptorBufferAlignment);
        VP_ASSERT(m_memory_bindless != nullptr);
        m_memory_per_stage = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_memory_bindless) + bindless_descriptor_memory_size);
        RESULT_ABORT_UNLESS(gfx::impl::CreateVkBuffer(std::addressof(m_descriptor_vk_buffer), m_memory_bindless, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, buffer_size));
        m_descriptor_buffer_address = gfx::impl::GetGpuAddress(m_descriptor_vk_buffer);

        /* Create descriptor heap */
        const size_t management_size = mem::SeparateHeap::GetManagementSize(mgr_info->max_command_buffer_count);
        void *management_memory = ::operator new(management_size, heap, alignof(mem::SeparateHeap));
        VP_ASSERT(management_memory != nullptr);
        m_per_stage_heap = mem::SeparateHeap::Create("awn::gfx::PerStageDescriptorHeap", management_memory, management_size, mgr_info->per_stage_memory_size, true);

        /* Calculate staging block sizes */
        m_ts_i_size = context->GetCombinedTextureSamplerDescriptorLayoutSize();
        m_ub_sb_size = context->GetCombinedBufferDescriptorLayoutSize();

        /* Create staging heap */
        const size_t block_size = m_ts_i_size + m_ub_sb_size;
        m_per_stage_staging_heap = mem::UnitHeap::TryCreate("awn::gfx::DescriptorStagingHeap", heap, block_size, mgr_info->max_command_buffer_count, alignof(size_t));

        return;
    }

    void DescriptorMemoryManager::Finalize() {
        if (m_memory_bindless != nullptr) {
            gfx::impl::DestroyVkBuffer(m_descriptor_vk_buffer);
            ::operator delete(m_memory_bindless);
            m_memory_bindless = nullptr;
            m_memory_per_stage = nullptr;
            m_descriptor_buffer_address = cInvalidGpuAddress;
        }
        if (m_per_stage_heap != nullptr) {
            ::operator delete(m_per_stage_heap);
            m_per_stage_heap = nullptr;
        }
        if (m_per_stage_staging_heap != nullptr) {
            ::operator delete(m_per_stage_staging_heap);
            m_per_stage_staging_heap = nullptr;
        }
    }

    void *DescriptorMemoryManager::AllocateCommandMemory(size_t *out_offset, size_t size) {
        void *base_address = m_per_stage_heap->TryAllocate(size, Context::cTargetDescriptorBufferAlignment);
        const size_t offset = reinterpret_cast<uintptr_t>(base_address) - mem::SeparateHeap::cOffsetBase;
        *out_offset = offset + m_bindless_descriptor_memory_size;
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_memory_per_stage) + offset);
    }

    void DescriptorMemoryManager::FreeCommandMemory(void *address) {
        void *base_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(m_memory_per_stage) + mem::SeparateHeap::cOffsetBase);
        m_per_stage_heap->Free(base_address);
    }

    void *DescriptorMemoryManager::AllocateStagingMemory() {
        const size_t block_size = m_ts_i_size + m_ub_sb_size;
        void *memory = m_per_stage_staging_heap->TryAllocate(block_size, alignof(size_t));
        return memory;
    }

    void DescriptorMemoryManager::FreeStagingMemory(void *address) {
        m_per_stage_staging_heap->Free(address);
    }
}

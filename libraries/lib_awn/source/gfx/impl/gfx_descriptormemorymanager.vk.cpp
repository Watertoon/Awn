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

        /* Calculate staging block sizes */
        Context *context = Context::GetInstance();
        m_texture_combined_size = context->GetCombinedTextureSamplerDescriptorLayoutSize();
        m_buffer_combined_size = context->GetCombinedBufferDescriptorLayoutSize();

        const u32 cmd_buf_count = mgr_info->max_command_buffer_count;
        if (0 < cmd_buf_count) {            
            /* Create staging heap */
            const size_t block_size = m_texture_combined_size + m_buffer_combined_size;
            const size_t size = block_size * cmd_buf_count;
            m_per_stage_staging_heap = mem::UnitHeap::TryCreate("awn::gfx::DescriptorStagingHeap", heap, size, block_size, alignof(size_t), true);
        }

        /* Get bindless texture view and sampler count */
        size_t bindless_descriptor_memory_size = context->GetBindlessDescriptorLayoutSize();   
        m_bindless_size = bindless_descriptor_memory_size;

        /* Allocate descriptor buffer */
        const size_t buffer_size = bindless_descriptor_memory_size + mgr_info->per_stage_memory_size;
        m_memory_bindless = ::operator new(buffer_size, gpu_heap, Context::cTargetDescriptorBufferAlignment);
        VP_ASSERT(m_memory_bindless != nullptr);
        RESULT_ABORT_UNLESS(gfx::impl::CreateVkBuffer(std::addressof(m_descriptor_vk_buffer), m_memory_bindless, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, buffer_size));
        m_descriptor_buffer_address = gfx::impl::GetGpuAddress(m_descriptor_vk_buffer);
        m_memory_per_stage = (mgr_info->per_stage_memory_size == 0) ? nullptr : reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_memory_bindless) + bindless_descriptor_memory_size);

        const u32 cmd_list_count = mgr_info->max_command_list_count;
        if (0 < cmd_list_count) {            
            /* Unit array */
            m_unit_allocator.Initialize(heap, cmd_list_count);
            m_unit_array.Initialize(heap, cmd_list_count);

            if (m_memory_per_stage != nullptr) {                
                /* Create cmd list descriptor heap */
                const size_t management_size = mem::SeparateHeap::CalculateManagementSize(cmd_list_count);
                void *management_memory = ::operator new(management_size, heap, alignof(mem::SeparateHeap));
                VP_ASSERT(management_memory != nullptr);
                m_per_stage_heap = mem::SeparateHeap::Create("awn::gfx::PerStageDescriptorHeap", management_memory, management_size, m_memory_per_stage, mgr_info->per_stage_memory_size, true);
            }
        }

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
        m_unit_allocator.Finalize();
        m_unit_array.Finalize();
    }

    Result DescriptorMemoryManager::AllocateCommandListUnit(CommandListUnit **out_unit, size_t size, CommandPoolId pool_id, bool is_primary) {

        /* Allocate unit */
        const u32 id = m_unit_allocator.Allocate();
        RESULT_RETURN_IF(id == UnitAllocator::cInvalidEntryIndex, ResultOutOfCommandUnits);
        auto fail_guard0 = SCOPE_GUARD {
            m_unit_allocator.Free(id);
        };
        
        /* Allocate descriptor memory */
        void *address = m_per_stage_heap->TryAllocate(size, Context::cTargetDescriptorBufferAlignment);
        RESULT_RETURN_IF(address == nullptr, ResultOutOfMemory);
        auto fail_guard1 = SCOPE_GUARD {
            m_per_stage_heap->Free(address);
        };
        CommandListUnit *unit = std::addressof(m_unit_array[id]);
        unit->descriptor_memory = address;
        unit->descriptor_memory_size = size;
        unit->descriptor_memory_iter = 0;

        /* Allocate command buffer */
        RESULT_TRY(CommandPoolManager::GetInstance()->CreateCommandBuffer(std::addressof(unit->vk_command_buffer), pool_id, is_primary));
        unit->pool_id = pool_id;

        *out_unit = unit;

        fail_guard0.Cancel();
        fail_guard1.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    void DescriptorMemoryManager::FreeCommandListUnit(CommandListUnit *unit) {

        /* Free descriptor memory */
        if (unit->descriptor_memory != nullptr) {            
            m_per_stage_heap->Free(unit->descriptor_memory);
            unit->descriptor_memory = nullptr;
        }

        /* Free command buffer */
        CommandPoolManager::GetInstance()->FreeCommandBuffer(unit->vk_command_buffer, unit->pool_id);

        /* Free unit */
        const u32 index = m_unit_array.IndexOf(unit);
        m_unit_allocator.Free(index);

        return;
    }

    void *DescriptorMemoryManager::AllocateStagingMemory() {
        const size_t block_size = m_texture_combined_size + m_buffer_combined_size;
        void *memory = m_per_stage_staging_heap->TryAllocate(block_size, alignof(size_t));
        return memory;
    }

    void DescriptorMemoryManager::FreeStagingMemory(void *address) {
        m_per_stage_staging_heap->Free(address);
    }
}

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
#ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
    void DescriptorMemoryManager::Initialize(mem::Heap *heap, const DescriptorMemoryManagerInfo *mgr_info) {

        /* Calculate descriptor heap sizes */
        Context *context = Context::GetInstance();
        m_image_size = context->GetImageDescriptorSize() + mgr_info->max_seperate_texture_count;
        m_texture_size = context->GetSeparateTextureDescriptorSize() + mgr_info->max_seperate_texture_count;
        m_sampler_size = context->GetSeparateSamplerDescriptorSize() + mgr_info->max_seperate_sampler_count;

        //const u32 cmd_buf_count = mgr_info->max_command_buffer_count;
        //if (0 < cmd_buf_count) {            
        //    /* Create staging heap */
        //    const size_t block_size = 230 * sizeof(u64) * cMaxShaderStageCount;
        //    const size_t size = block_size * cmd_buf_count;
        //    m_per_stage_staging_heap = mem::UnitHeap::TryCreate("awn::gfx::DescriptorStagingHeap", heap, size, block_size, alignof(size_t), true);
        //}

        /* Allocate per stage buffer */
        //BufferInfo per_stage_info;
        //per_stage_info.SetDefaults();
        //per_stage_info.SetSize(mgr_info->per_stage_memory_size);
        //per_stage_info.SetGpuAccessFlags(GpuAccessFlags::UniformBuffer);
        //const GpuMemoryRequirements per_reqs = Buffer::GetGpuMemoryRequirements(std::addressof(per_stage_info));
        //MemoryPoolInfo per_pool_info;
        //per_pool_info.SetDefaults();
        //per_pool_info.SetSize(per_reqs.size);
        //per_pool_info.SetMemoryPropertyFlags(MemoryPropertyFlags::CpuUncached | MemoryPropertyFlags::GpuUncached);
        //RESULT_ABORT_UNLESS(m_memory_pool.Initialize(std::addressof(per_pool_info)));
        //RESULT_ABORT_UNLESS(m_per_stage_buffer.Initialize(std::addressof(m_memory_pool), 0, std::addressof(per_stage_info)));
        //m_memory_per_stage = m_per_stage_buffer.GetAddress();
        //m_per_stage_device_address = m_per_stage_buffer.GetGpuAddress().vk_device_address;

        /* Create cmd list descriptor heap */
        //if (m_memory_per_stage != nullptr) {
        //    const size_t management_size = mem::SeparateHeap::CalculateManagementSize(cmd_list_count);
        //    void *management_memory = ::operator new(management_size, heap, alignof(mem::SeparateHeap));
        //    VP_ASSERT(management_memory != nullptr);
        //    m_per_stage_heap = mem::SeparateHeap::Create("awn::gfx::PerStageDescriptorHeap", management_memory, management_size, m_memory_per_stage, mgr_info->per_stage_memory_size, true);
        //}

        /* Allocate descriptor heap */
        const size_t image_offset = vp::util::AlignUp(m_texture_size, context->GetImageDescriptorAlignment());
        const size_t resource_reserve_offset = vp::util::AlignUp(image_offset + m_image_size, context->GetResourceDescriptorHeapReservedAlignment());
        const size_t sampler_heap_offset = vp::util::AlignUp(resource_reserve_offset + context->GetResourceDescriptorHeapReservedRange(), context->GetSamplerDescriptorHeapAlignment());
        const size_t heap_size = sampler_heap_offset + m_sampler_size + context->GetSamplerDescriptorHeapReservedRange();
        VkMemoryRequirements2 memory_requirements_2 = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        };
        const VkBufferCreateInfo vk_buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = heap_size,
            .usage = VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        };
        const VkDeviceBufferMemoryRequirements buffer_requirements = {
            .sType       = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
            .pCreateInfo = std::addressof(vk_buffer_info),
        };
        ::pfn_vkGetDeviceBufferMemoryRequirements(impl::Context::GetInstance()->GetVkDevice(), std::addressof(buffer_requirements), std::addressof(memory_requirements_2));

        MemoryPoolInfo pool_info;
        pool_info.SetDefaults();
        pool_info.SetSize(memory_requirements_2.memoryRequirements.size);
        pool_info.SetMemoryPropertyFlags(MemoryPropertyFlags::CpuUncached | MemoryPropertyFlags::GpuUncached);
        const VkMemoryAllocateFlagsInfo flags_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        };
        RESULT_ABORT_UNLESS(m_descriptor_heap_memory_pool.InitializeImpl(std::addressof(pool_info), memory_requirements_2.memoryRequirements.memoryTypeBits, std::addressof(flags_info)));
        RESULT_ABORT_UNLESS(gfx::impl::CreateVkBuffer(std::addressof(m_descriptor_heap_buffer), std::addressof(m_descriptor_heap_memory_pool), 0, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, memory_requirements_2.memoryRequirements.size));
        m_memory_texture = m_descriptor_heap_memory_pool.GetAddress();
        m_memory_image = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_memory_texture) + image_offset);
        m_memory_sampler = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_memory_texture) + sampler_heap_offset);

        const GpuAddress res_addr = gfx::impl::GetGpuAddress(m_descriptor_heap_buffer);
        const GpuAddress sampler_addr = res_addr + sampler_heap_offset;
        m_bind_heap_info_resource.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT;
        m_bind_heap_info_resource.pNext = nullptr;
        m_bind_heap_info_resource.heapRange.address = res_addr.vk_device_address;
        m_bind_heap_info_resource.heapRange.size = sampler_heap_offset;
        m_bind_heap_info_resource.reservedRangeOffset = resource_reserve_offset;
        m_bind_heap_info_resource.reservedRangeSize = context->GetResourceDescriptorHeapReservedRange();
        m_bind_heap_info_sampler.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT;
        m_bind_heap_info_sampler.pNext = nullptr;
        m_bind_heap_info_sampler.heapRange.address = sampler_addr.vk_device_address;
        m_bind_heap_info_sampler.heapRange.size = m_sampler_size + context->GetSamplerDescriptorHeapReservedRange();
        m_bind_heap_info_sampler.reservedRangeOffset = m_sampler_size;
        m_bind_heap_info_sampler.reservedRangeSize = context->GetSamplerDescriptorHeapReservedRange();

        //const u32 cmd_list_count = mgr_info->max_command_list_count;
        //if (0 < cmd_list_count) {            
        //    /* Unit array */
        //    RESULT_ABORT_UNLESS(m_unit_allocator.Initialize(heap, cmd_list_count));
        //    RESULT_ABORT_UNLESS(m_unit_array.Initialize(heap, cmd_list_count));
        //}

        return;
    }

    void DescriptorMemoryManager::Finalize() {
        if (m_descriptor_heap_buffer != nullptr) {
            gfx::impl::DestroyVkBuffer(m_descriptor_heap_buffer);
            m_descriptor_heap_buffer = nullptr;
            m_memory_texture = nullptr;
            m_memory_image = nullptr;
            m_memory_sampler = nullptr;
        }
        m_descriptor_heap_memory_pool.Finalize();
        m_per_stage_buffer.Finalize();
        m_memory_pool.Finalize();
        m_memory_per_stage = nullptr;
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
#else
    void DescriptorMemoryManager::Initialize(mem::Heap *heap, const DescriptorMemoryManagerInfo *mgr_info) {

        /* Calculate staging block sizes */
        Context *context = Context::GetInstance();
        m_texture_combined_size = context->GetCombinedTextureSamplerDescriptorLayoutSize();
        m_buffer_combined_size = context->GetCombinedBufferDescriptorLayoutSize();

        //const u32 cmd_buf_count = mgr_info->max_command_buffer_count;
        //if (0 < cmd_buf_count) {            
        //    /* Create staging heap */
        //    const size_t block_size = m_texture_combined_size + m_buffer_combined_size;
        //    const size_t size = block_size * cmd_buf_count;
        //    m_per_stage_staging_heap = mem::UnitHeap::TryCreate("awn::gfx::DescriptorStagingHeap", heap, size, block_size, alignof(size_t), true);
        //}

        /* Get bindless texture view and sampler count */
        size_t bindless_descriptor_memory_size = context->GetBindlessDescriptorLayoutSize();   
        m_bindless_size = bindless_descriptor_memory_size;

        /* Allocate descriptor buffer */
        const size_t buffer_size = bindless_descriptor_memory_size + mgr_info->per_stage_memory_size + mgr_info->transient_descriptor_memory_size;
        VkMemoryRequirements2 memory_requirements_2 = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        };
        const VkBufferCreateInfo vk_buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = buffer_size,
            .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        };
        const VkDeviceBufferMemoryRequirements buffer_requirements = {
            .sType       = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
            .pCreateInfo = std::addressof(vk_buffer_info),
        };
        ::pfn_vkGetDeviceBufferMemoryRequirements(impl::Context::GetInstance()->GetVkDevice(), std::addressof(buffer_requirements), std::addressof(memory_requirements_2));
        MemoryPoolInfo pool_info;
        pool_info.SetDefaults();
        pool_info.SetSize(memory_requirements_2.memoryRequirements.size);
        pool_info.SetMemoryPropertyFlags(MemoryPropertyFlags::CpuUncached | MemoryPropertyFlags::GpuUncached);
        const VkMemoryAllocateFlagsInfo flags_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        };
        RESULT_ABORT_UNLESS(m_memory_pool.InitializeImpl(std::addressof(pool_info), memory_requirements_2.memoryRequirements.memoryTypeBits, std::addressof(flags_info)));
        m_memory_bindless = m_memory_pool.GetAddress();
        VP_ASSERT(m_memory_bindless != nullptr);
        RESULT_ABORT_UNLESS(gfx::impl::CreateVkBuffer(std::addressof(m_descriptor_vk_buffer), m_memory_bindless, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, buffer_size));
        m_descriptor_buffer_address = gfx::impl::GetGpuAddress(m_descriptor_vk_buffer);

        /* Setup resource set memory */
        m_memory_per_stage = (mgr_info->per_stage_memory_size == 0) ? nullptr : reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_memory_bindless) + bindless_descriptor_memory_size);
        if (m_memory_per_stage != nullptr) {
            const size_t management_size = mem::SeparateHeap::CalculateManagementSize(mgr_info->per_stage_max_allocation_count);
            void *management_memory = ::operator new(management_size, heap, alignof(mem::SeparateHeap));
            VP_ASSERT(management_memory != nullptr);
            m_per_stage_heap = mem::SeparateHeap::Create("awn::gfx::PerStageDescriptorHeap", management_memory, management_size, m_memory_per_stage, mgr_info->per_stage_memory_size, true);
        }

        /* Setup transient memory */
        m_transient_head = reinterpret_cast<uintptr_t>(m_memory_bindless) + bindless_descriptor_memory_size + mgr_info->per_stage_memory_size;
        m_transient_iter = 0;
        m_transient_size = mgr_info->transient_descriptor_memory_size;

        /* Setup binding info */
        m_binding_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
        m_binding_info.pNext = nullptr;
        m_binding_info.address = m_descriptor_buffer_address.vk_device_address;
        m_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

        return;
    }

    void DescriptorMemoryManager::Finalize() {
        if (m_memory_bindless != nullptr) {
            gfx::impl::DestroyVkBuffer(m_descriptor_vk_buffer);
            m_memory_bindless = nullptr;
            m_memory_per_stage = nullptr;
            m_descriptor_buffer_address = cInvalidGpuAddress;
        }
        m_memory_pool.Finalize();
        if (m_per_stage_heap != nullptr) {
            ::operator delete(m_per_stage_heap);
            m_per_stage_heap = nullptr;
        }
        //if (m_per_stage_staging_heap != nullptr) {
        //    ::operator delete(m_per_stage_staging_heap);
        //    m_per_stage_staging_heap = nullptr;
        //}
        //m_unit_allocator.Finalize();
        //m_unit_array.Finalize();
    }
#endif

    //Result DescriptorMemoryManager::AllocateCommandListUnit(CommandListUnit **out_unit, size_t size, CommandPoolId pool_id, bool is_primary) {
    //
    //    /* Allocate unit */
    //    const u32 id = m_unit_allocator.Allocate();
    //    RESULT_RETURN_IF(id == UnitAllocator::cInvalidEntryIndex, ResultOutOfCommandUnits);
    //    auto fail_guard0 = SCOPE_GUARD {
    //        m_unit_allocator.Free(id);
    //    };
    //    
    //    /* Allocate descriptor memory */
    //#ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
    //    void *address = m_per_stage_heap->TryAllocate(size, Context::cTargetUniformBufferAlignment);
    //#else
    //    void *address = m_per_stage_heap->TryAllocate(size, Context::cTargetDescriptorBufferAlignment);
    //#endif
    //    RESULT_RETURN_IF(address == nullptr, ResultOutOfMemory);
    //    auto fail_guard1 = SCOPE_GUARD {
    //        m_per_stage_heap->Free(address);
    //    };
    //    CommandListUnit *unit = std::addressof(m_unit_array[id]);
    //    unit->descriptor_memory = address;
    //#ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
    //    unit->descriptor_memory_address = m_per_stage_device_address + (reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(m_memory_per_stage));
    //#endif
    //    unit->descriptor_memory_size = size;
    //    unit->descriptor_memory_iter = 0;
    //
    //    /* Allocate command buffer */
    //    RESULT_TRY(CommandPoolManager::GetInstance()->CreateCommandBuffer(std::addressof(unit->vk_command_buffer), pool_id, is_primary));
    //    unit->pool_id = pool_id;
    //
    //    *out_unit = unit;
    //
    //    fail_guard0.Cancel();
    //    fail_guard1.Cancel();
    //    RESULT_RETURN_SUCCESS;
    //}
    //
    //void DescriptorMemoryManager::FreeCommandListUnit(CommandListUnit *unit) {
    //
    //    /* Free descriptor memory */
    //    if (unit->descriptor_memory != nullptr) {            
    //        m_per_stage_heap->Free(unit->descriptor_memory);
    //        unit->descriptor_memory = nullptr;
    //    }
    //
    //    /* Free command buffer */
    //    CommandPoolManager::GetInstance()->FreeCommandBuffer(unit->vk_command_buffer, unit->pool_id);
    //
    //    /* Free unit */
    //    const u32 index = m_unit_array.IndexOf(unit);
    //    m_unit_allocator.Free(index);
    //
    //    return;
    //}
    //
    //void *DescriptorMemoryManager::AllocateStagingMemory() {
    //#ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
    //    const size_t block_size = 230 * sizeof(u64) * cMaxShaderStageCount;
    //#else
    //    const size_t block_size = m_texture_combined_size + m_buffer_combined_size;
    //#endif
    //    void *memory = m_per_stage_staging_heap->TryAllocate(block_size, alignof(size_t));
    //    return memory;
    //}
    //
    //void DescriptorMemoryManager::FreeStagingMemory(void *address) {
    //    m_per_stage_staging_heap->Free(address);
    //}
}

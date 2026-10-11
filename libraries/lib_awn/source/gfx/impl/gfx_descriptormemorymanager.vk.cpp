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
    Result DescriptorMemoryManager::Initialize(mem::Heap *heap, const DescriptorMemoryManagerInfo *mgr_info) {
        VP_UNUSED(heap);
        auto fail_guard = SCOPE_GUARD { this->Finalize(); };

        /* Calculate descriptor heap sizes */
        Context *context = Context::GetInstance();
        m_image_size = impl::Context::cTargetMaxSeparateTextureDescriptorSize + mgr_info->max_separate_texture_count;
        m_texture_size = impl::Context::cTargetMaxSeparateTextureDescriptorSize + mgr_info->max_separate_texture_count;
        m_sampler_size = impl::Context::cTargetMaxSeparateSamplerDescriptorSize + mgr_info->max_separate_sampler_count;

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
        ::pfn_vkGetDeviceBufferMemoryRequirements(context->GetVkDevice(), std::addressof(buffer_requirements), std::addressof(memory_requirements_2));

        MemoryPoolInfo pool_info;
        pool_info.SetDefaults();
        pool_info.SetSize(memory_requirements_2.memoryRequirements.size);
        pool_info.SetMemoryPropertyFlags(MemoryPropertyFlags::CpuUncached | MemoryPropertyFlags::GpuUncached);
        const VkMemoryAllocateFlagsInfo flags_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        };
        RESULT_TRY(m_descriptor_heap_memory_pool.InitializeImpl(std::addressof(pool_info), memory_requirements_2.memoryRequirements.memoryTypeBits, std::addressof(flags_info)));
        RESULT_TRY(gfx::impl::CreateVkBuffer(std::addressof(m_descriptor_heap_buffer), std::addressof(m_descriptor_heap_memory_pool), 0, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, memory_requirements_2.memoryRequirements.size));
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

        fail_guard.Cancel();
        RESULT_RETURN_SUCCESS;
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
    }
}

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

    Result ResourceSet::Initialize(mem::Heap *heap, const ResourceSetInfo *info) {
        VP_UNUSED(heap);

    #ifndef AWN_ENABLE_VK_DESCRIPTOR_HEAP
        /* In VK_EXT_descriptor_buffer we write descriptors directly. */
        /* Allocate descriptor memory */
        impl::Context *context = impl::Context::GetInstance();
        const size_t combine_tex_size = context->GetCombinedTextureSamplerDescriptorSize();
        const size_t combine_buf_size = context->GetCombinedBufferDescriptorSize();
        const size_t texture_size = vp::util::AlignUp(info->max_location_texture_sampler * combine_tex_size, impl::Context::cTargetDescriptorBufferAlignment);
        const size_t buffer_size = vp::util::AlignUp(info->max_location_buffer * combine_buf_size, impl::Context::cTargetDescriptorBufferAlignment);
        impl::DescriptorMemoryManager *mgr = impl::DescriptorMemoryManager::GetInstance();
        uintptr_t offset = 0;
        void *memory = mgr->AllocateResourceSetMemory(std::addressof(offset), texture_size + buffer_size);
        RESULT_RETURN_IF(memory == nullptr, ResultNoMemory);
        /* Setup offsets */
        if (0 < texture_size) {
            m_desc_memory_array[0] = memory;
            m_size_array[0] = texture_size;
            m_offset_array[0] = offset;
        }
        if (0 < buffer_size) {
            m_desc_memory_array[1] = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + texture_size);
            m_size_array[1] = buffer_size;
            m_offset_array[1] = offset + texture_size;
        }
    #else
        VP_NOT_IMPLEMENTED();
    #endif
        RESULT_RETURN_SUCCESS;
    }

    Result ResourceSet::Initialize(MemoryPool *pool, size_t offset, const ResourceSetInfo *info) {
        /* In VK_EXT_descriptor_heap we remap interface slots to a linear index range and bind an indirect index buffer to persistant descriptors. */
    #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
        /* Ensure valid address */
        const GpuMemoryRequirements reqs = GetGpuMemoryRequirements(info);
        void *address = pool->GetAddress(offset, reqs.size);
        RESULT_RETURN_IF(address == nullptr, ResultNoMemory);
        RESULT_RETURN_IF(vp::util::IsAligned(address, cTargetUniformBufferAlignment), ResultAlignmentNG);
        /* Setup */
        m_gpu_address = pool->GetGpuAddress(offset);
        m_desc_array.SetMemory(reinterpret_cast<u64*>(address), count);
    #else
        VP_UNUSED(pool, offset, info);
        VP_NOT_IMPLEMENTED();
    #endif
        RESULT_RETURN_SUCCESS;
    }
    void ResourceSet::Finalize() {
    #if defined(AWN_ENABLE_VK_DESCRIPTOR_HEAP)
        m_desc_array.SetMemory(nullptr, 0);
        m_gpu_address = cInvalidGpuAddress;
    #else
        if (m_desc_memory_array[0] != nullptr) {
            impl::DescriptorMemoryManager::GetInstance()->FreeResourceSetMemory(m_desc_memory_array[0]);
            m_desc_memory_array[0] = nullptr;
            m_desc_memory_array[1] = nullptr;
        }
    #endif
    }

    void ResourceSet::BindImpl(gfx::CommandBuffer *buffer) const {
    #if defined(AWN_ENABLE_VK_DESCRIPTOR_HEAP)
        /* Push data */
        const VkPushDataInfoEXT info = {
            .sType = VK_STRUCTURE_TYPE_PUSH_DATA_INFO_EXT,
            .offset = 0,
            .data = {
                .address = std::addressof(m_gpu_address.vk_device_address),
                .size = sizeof(VkDeviceAddress),
            },
        };
        ::pfn_vkCmdPushData(buffer->GetVkCommandBuffer(), std::addressof(info));
    #else
        /* Set offsets */
        const QueueType queue_type = buffer->GetQueueType();
        VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;
        if (queue_type == QueueType::Graphics) {
            vk_pipeline_layout = impl::Context::GetInstance()->GetVkPipelineLayoutGraphics();
        } else if (queue_type == QueueType::Compute) {
            vk_pipeline_layout = impl::Context::GetInstance()->GetVkPipelineLayoutCompute();
        }
        const u32 desc_buf_index_array[2] {
            impl::Context::cTargetDescriptorBufferIndexPerStage,
            impl::Context::cTargetDescriptorBufferIndexPerStage,
        };
        const u32 is_no_texture = (m_size_array[0] < 1);
        const u32 count = (0 < m_size_array[0]) + (0 < m_size_array[1]);
        const VkSetDescriptorBufferOffsetsInfoEXT info = {
            .sType = VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT,
            .pNext = nullptr,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .layout = vk_pipeline_layout,
            .firstSet = (!is_no_texture) ? static_cast<u32>(impl::Context::cTargetPerStageTextureSamplerLayoutSet) : static_cast<u32>(impl::Context::cTargetPerStageBufferLayoutSet),
            .setCount = count,
            .pBufferIndices = desc_buf_index_array,
            .pOffsets = m_offset_array + is_no_texture,
        };
        ::pfn_vkCmdSetDescriptorBufferOffsets2EXT(buffer->GetVkCommandBuffer(), std::addressof(info));
    #endif
        return;
    }
}

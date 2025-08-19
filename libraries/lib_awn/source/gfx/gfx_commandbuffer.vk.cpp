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

    size_t EstimateDescriptorMemorySize(u32 max_draw_dispatch_count, u32 max_descriptor_count) {

        /* Find worst case amount of memory to cover max of X descriptors across Y descriptor buffer bindings */
        impl::Context *context = impl::Context::GetInstance();
        const size_t desc_size = context->GetMaxDescriptorSize();
        const size_t bind_align = context->GetDescriptorOffsetAlignment();
        const u32 desc_per_block_count = (0 != desc_size) ? (bind_align / desc_size) + ((bind_align % desc_size) != 0) : 0;
        size_t command_memory_size;
        if (1 < desc_per_block_count) {
            const u32 real_count = max_draw_dispatch_count * impl::Context::cMaxPerStageBindCount;
            const u32 bind_count = vp::util::Min(real_count, max_descriptor_count);
            constexpr const u32 cMaxDescPerBind = vp::util::Max(impl::Context::cTargetMaxPerStageTextureSamplerCombinedCount * impl::Context::cTargetMaxShaderStageCount, impl::Context::cTargetMaxPerStageBufferCombinedCount * impl::Context::cTargetMaxShaderStageCount);
            const u32 avg_desc_per_bind_count = (max_descriptor_count / bind_count) + ((max_descriptor_count % bind_count) != 0);
            const u32 wc_desc_per_bind_count = vp::util::Min(avg_desc_per_bind_count, cMaxDescPerBind);
            const u32 bind_worst_case_block_count = (wc_desc_per_bind_count / desc_per_block_count) + ((wc_desc_per_bind_count % desc_per_block_count) != 0);
            command_memory_size = bind_worst_case_block_count * desc_size * desc_per_block_count * bind_count;
        } else {
            command_memory_size = vp::util::Max(desc_size * max_descriptor_count, bind_align);
        }

        return command_memory_size;
    }

    void *CommandBuffer::GetDescriptorMemoryTextureSamplerImageSeparate() {
        return m_staging_memory;
    }

    void *CommandBuffer::GetDescriptorMemoryUniformBufferStorageBuffer() {
        const size_t ts_i_size = impl::DescriptorMemoryManager::GetInstance()->GetCombinedTextureSize();
        const size_t base_offset = ts_i_size;
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_staging_memory) + base_offset);
    }

    //void CommandBuffer::TransitionRenderTargetsImpl(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target, SyncScopeInfo *sync_scope, VkImageLayout image_layout) {
    //
    //    /* Setup color target barriers */
    //    VkImageMemoryBarrier2 barrier_array[cTargetMaxBoundRenderTargetColorCount + 1];
    //    for (u32 i = 0; i < color_target_count; ++i) {
    //        barrier_array[i].sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    //        barrier_array[i].pNext               = nullptr;
    //        barrier_array[i].srcStageMask        = sync_scope->vk_src_stage_mask;
    //        barrier_array[i].srcAccessMask       = sync_scope->vk_src_access_mask;
    //        barrier_array[i].dstStageMask        = sync_scope->vk_dst_stage_mask;
    //        barrier_array[i].dstAccessMask       = sync_scope->vk_dst_access_mask;
    //        barrier_array[i].oldLayout           = color_target_array[i]->GetVkImageLayout();
    //        barrier_array[i].newLayout           = image_layout;
    //        barrier_array[i].srcQueueFamilyIndex = 0;
    //        barrier_array[i].dstQueueFamilyIndex = 0;
    //        barrier_array[i].image               = color_target_array[i]->GetVkImage();
    //        barrier_array[i].subresourceRange.aspectMask     = color_target_array[i]->GetVkAspectMask();
    //        barrier_array[i].subresourceRange.baseMipLevel   = 0;
    //        barrier_array[i].subresourceRange.levelCount     = 1;
    //        barrier_array[i].subresourceRange.baseArrayLayer = 0;
    //        barrier_array[i].subresourceRange.layerCount     = color_target_array[i]->GetViewCount();
    //
    //        color_target_array[i]->SetVkImageLayout(image_layout);
    //    }
    //
    //    if (depth_stencil_target != nullptr && image_layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    //        barrier_array[color_target_count].sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    //        barrier_array[color_target_count].pNext               = nullptr;
    //        barrier_array[color_target_count].srcStageMask        = (sync_scope->vk_src_stage_mask == VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT) ? VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT : sync_scope->vk_src_stage_mask;
    //        barrier_array[color_target_count].srcAccessMask       = (sync_scope->vk_src_access_mask == VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT) ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : sync_scope->vk_src_access_mask;
    //        barrier_array[color_target_count].dstStageMask        = sync_scope->vk_dst_stage_mask;
    //        barrier_array[color_target_count].dstAccessMask       = sync_scope->vk_dst_access_mask;
    //        barrier_array[color_target_count].oldLayout           = depth_stencil_target->GetVkImageLayout();
    //        barrier_array[color_target_count].newLayout           = (image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : image_layout;
    //        barrier_array[color_target_count].srcQueueFamilyIndex = 0;
    //        barrier_array[color_target_count].dstQueueFamilyIndex = 0;
    //        barrier_array[color_target_count].image               = depth_stencil_target->GetVkImage();
    //        barrier_array[color_target_count].subresourceRange.aspectMask     = depth_stencil_target->GetVkAspectMask();
    //        barrier_array[color_target_count].subresourceRange.baseMipLevel   = 0;
    //        barrier_array[color_target_count].subresourceRange.levelCount     = 1;
    //        barrier_array[color_target_count].subresourceRange.baseArrayLayer = 0;
    //        barrier_array[color_target_count].subresourceRange.layerCount     = depth_stencil_target->GetViewCount();
    //
    //        depth_stencil_target->SetVkImageLayout(image_layout);
    //        color_target_count = color_target_count + 1;
    //    }
    //
    //    const VkDependencyInfo dep_info = {
    //        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //        .imageMemoryBarrierCount = color_target_count,
    //        .pImageMemoryBarriers    = barrier_array
    //    }; 
    //    ::pfn_vkCmdPipelineBarrier2(m_command_list_unit->vk_command_buffer, std::addressof(dep_info));
    //
    //    return;
    //}

    size_t CommandBuffer::CommitDescriptorMemory(void *desc_memory, u32 memory_size) {

        /* Copy descriptors to gpu */
        void *memory = m_command_list_unit->AllocateDescriptorMemory(memory_size);
        ::memcpy(memory, desc_memory, memory_size);

        /* Set per stage offset */
        const size_t gpu_offset = reinterpret_cast<uintptr_t>(memory) - reinterpret_cast<uintptr_t>(impl::DescriptorMemoryManager::GetInstance()->GetBindlessDescriptorMemory());
        return gpu_offset;
    }

    void CommandBuffer::CommitDescriptors() {

        const auto commit_desc_lambda = [this](bool is_texture) {

            /* Allocate descriptor buffer space, copy descriptor memory to buffer, and update binding offsets for each changed stage */
            impl::Context *context = impl::Context::GetInstance();
            const s32 size = (is_texture) ? m_texture_commit_size : m_buffer_commit_size;
            const auto memory_desc_lambda = [this, context, is_texture](s32 size) -> size_t {                
                if (-1 < size) {
                    void *desc_memory = (is_texture) ? this->GetDescriptorMemoryTextureSamplerImageSeparate() : this->GetDescriptorMemoryUniformBufferStorageBuffer();
                    return this->CommitDescriptorMemory(desc_memory, size);
                }
                return 0ull;
            };
            const size_t offset = memory_desc_lambda(size);

            /* Reset descriptor memory commit sizes */
            if (!m_is_preserve_descriptors_between_commit) {                
                if (is_texture) {
                    m_texture_commit_size = -1;
                } else {
                    m_buffer_commit_size = -1;
                }
            }

            return offset;
        };
        /* Commit texture sampler descriptors */
        u32 base_set = 0xffff'ffff;
        u32 set_count = 0;
        VkDeviceSize offset_array[2];
        u32 indices[2];
        if (m_is_dirty_texture) {
            const size_t offset = commit_desc_lambda(true);
            m_is_dirty_texture = false;
            base_set = impl::Context::cTargetPerStageTextureSamplerLayoutSet;
            offset_array[set_count] = offset;
            indices[set_count] = impl::Context::cTargetDescriptorBufferIndexPerStage;
            ++set_count;
        }
        /* Commit buffer descriptors */
        if (m_is_dirty_buffer) {
            const size_t offset = commit_desc_lambda(false);
            m_is_dirty_buffer = false;
            base_set = (base_set == 0xffff'ffff) ? impl::Context::cTargetPerStageBufferLayoutSet : base_set;
            offset_array[set_count] = offset;
            indices[set_count] = impl::Context::cTargetDescriptorBufferIndexPerStage;
            ++set_count;
        }

        /* Set desc buffer offsets */
        if (set_count == 0) { return; }
        VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;
        if (m_queue_type == QueueType::Graphics) {
            vk_pipeline_layout = impl::Context::GetInstance()->GetVkPipelineLayoutGraphics();
        } else if (m_queue_type == QueueType::Compute) {
            vk_pipeline_layout = impl::Context::GetInstance()->GetVkPipelineLayoutCompute();
        }
        const VkSetDescriptorBufferOffsetsInfoEXT offsets_info = {
            .sType = VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT,
            .pNext = nullptr,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .layout = vk_pipeline_layout,
            .firstSet = base_set,
            .setCount = set_count,
            .pBufferIndices = indices,
            .pOffsets = offset_array,
        };
        ::pfn_vkCmdSetDescriptorBufferOffsets2EXT(m_command_list_unit->vk_command_buffer, std::addressof(offsets_info));

        return;
    }

    void CommandBuffer::BeginRenderingIfNotRendering() {

        /* Ignore if still rendering */
        if (m_is_rendering) { 
            if (!m_is_render_target_dirty) { return; }
            this->EndRenderingIfRendering();
        }
        m_is_rendering = true;
        m_is_render_target_dirty = false;

        /* Get values that should be synced across render targets */
        TextureViewBinder **color_target_array = m_color_target_array;
        TextureViewBinder *depth_stencil_target = m_depth_stencil_target;
        const impl::TextureViewKey *key = nullptr;
        if (m_color_target_count != 0) {
            key = color_target_array[0]->GetTextureViewKey();
        } else if (depth_stencil_target != nullptr) {
            key = depth_stencil_target->GetTextureViewKey();
        }

        /* Setup color target attachments */
        const VkAttachmentLoadOp load_op = (m_trigger_clear_type == RenderTargetClearType::None) ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : (m_trigger_clear_type == RenderTargetClearType::Clear) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD ;
        m_trigger_clear_type = RenderTargetClearType::None;
        //u32 color_attachment_index_array[cTargetMaxBoundRenderTargetColorCount];
        VkRenderingAttachmentInfo color_attachment_info_array[cTargetMaxBoundRenderTargetColorCount];
        for (u32 i = 0; i < m_color_target_count; ++i) {
            color_attachment_info_array[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            color_attachment_info_array[i].pNext = nullptr;
            color_attachment_info_array[i].imageView = color_target_array[i]->GetVkImageView();
            color_attachment_info_array[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            color_attachment_info_array[i].resolveMode = VK_RESOLVE_MODE_NONE;
            color_attachment_info_array[i].resolveImageView = VK_NULL_HANDLE;
            color_attachment_info_array[i].resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
            color_attachment_info_array[i].loadOp = load_op;
            color_attachment_info_array[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment_info_array[i].clearValue.color = VkClearColorValue {
                vp::util::cBlack4f.r,
                vp::util::cBlack4f.g,
                vp::util::cBlack4f.b,
                vp::util::cBlack4f.a,
            };
            //color_attachment_index_array[i] = i;
        }

        /* Setup depth stencil attachments */
        VkRenderingAttachmentInfo  depth_attachment_info;
        VkRenderingAttachmentInfo  stencil_attachment_info;
        VkRenderingAttachmentInfo *selected_depth_attachment_info   = nullptr;
        VkRenderingAttachmentInfo *selected_stencil_attachment_info = nullptr;
        if (depth_stencil_target != nullptr) {
            const impl::TextureViewKey *d_key = depth_stencil_target->GetTextureViewKey();
            const u32 depth_stencil_mode = d_key->depth_stencil_mode;
            if (depth_stencil_mode != 1) {
                depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                depth_attachment_info.pNext = nullptr;
                depth_attachment_info.imageView = depth_stencil_target->GetVkImageView();
                depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                depth_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
                depth_attachment_info.resolveImageView = VK_NULL_HANDLE;
                depth_attachment_info.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
                depth_attachment_info.loadOp = load_op;
                depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                depth_attachment_info.clearValue.depthStencil = VkClearDepthStencilValue{
                    .depth = 1.0f,
                };
                selected_depth_attachment_info = std::addressof(depth_attachment_info);
            }
            if (depth_stencil_mode != 0) {
                stencil_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                stencil_attachment_info.pNext = nullptr;
                stencil_attachment_info.imageView = depth_stencil_target->GetVkImageView();
                stencil_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                stencil_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
                stencil_attachment_info.resolveImageView = VK_NULL_HANDLE;
                stencil_attachment_info.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
                stencil_attachment_info.loadOp = load_op;
                stencil_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                stencil_attachment_info.clearValue.depthStencil = VkClearDepthStencilValue{
                    .stencil = 0,
                };
                selected_stencil_attachment_info = std::addressof(stencil_attachment_info);
            }
        }

        /* Set render targets */
        const vp::util::Int2 size = key->texture->GetSize2d();
        const u32 layer_count = key->array_layer_count;
        const u32 view_mask = 0;
        const VkRenderingInfo rendering_info = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT | VK_RENDERING_CONTENTS_INLINE_BIT_KHR,
            .renderArea = {
                .offset = {
                    .x = 0,
                    .y = 0,
                },
                .extent = {
                    .width  = static_cast<u32>(size[0]),
                    .height = static_cast<u32>(size[1]),
                },
            },
            .layerCount           = layer_count,
            .viewMask             = view_mask,
            .colorAttachmentCount = m_color_target_count,
            .pColorAttachments    = color_attachment_info_array,
            .pDepthAttachment     = selected_depth_attachment_info,
            .pStencilAttachment   = selected_stencil_attachment_info,
        };
        ::pfn_vkCmdBeginRendering(m_command_list_unit->vk_command_buffer, std::addressof(rendering_info));

        ///* Map image indices */
        //VkRenderingAttachmentLocationInfoKHR attachment_location_info = {
        //    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO_KHR,
        //    .colorAttachmentCount = m_color_target_count,
        //    .pColorAttachmentLocations = color_attachment_index_array,
        // };
        //::pfn_vkCmdSetRenderingAttachmentLocations(m_command_list_unit->vk_command_buffer, std::addressof(attachment_location_info));

        return;
    }

    void CommandBuffer::EndRenderingIfRendering() {

        /* Force fast clear if no rendering took place */
        if (m_trigger_clear_type == RenderTargetClearType::Clear && m_is_render_target_dirty) {
            this->BeginRenderingIfNotRendering();
        }

        /* Trigger end rendering */
        if (!m_is_rendering) { return; }
        m_is_rendering = false;

        ::pfn_vkCmdEndRendering(m_command_list_unit->vk_command_buffer);

        return;
    }

    void CommandBuffer::SetSystemDescriptorBuffers() {

        /* Set bindless and per stage descriptor buffer */
        impl::DescriptorMemoryManager *desc_memory_mgr = impl::DescriptorMemoryManager::GetInstance();
        const VkDescriptorBufferBindingInfoEXT binding_info = {
            .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
            .address = desc_memory_mgr->GetDescriptorBufferAddress().vk_device_address,
            .usage   = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
        };
        ::pfn_vkCmdBindDescriptorBuffersEXT(m_command_list_unit->vk_command_buffer, 1, std::addressof(binding_info));

        /* Map bindless */
        const size_t gpu_offset = 0;
        VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;
        if (m_queue_type == QueueType::Graphics) {
            vk_pipeline_layout = impl::Context::GetInstance()->GetVkPipelineLayoutGraphics();
        } else if (m_queue_type == QueueType::Compute) {
            vk_pipeline_layout = impl::Context::GetInstance()->GetVkPipelineLayoutCompute();
        }
        const VkSetDescriptorBufferOffsetsInfoEXT offsets_info = {
            .sType = VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT,
            .pNext = nullptr,
            .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT,
            .layout = vk_pipeline_layout,
            .firstSet = impl::Context::GetBindlessSet(),
            .setCount = 1,
            .pBufferIndices = std::addressof(impl::Context::cTargetDescriptorBufferIndexBindless),
            .pOffsets = std::addressof(gpu_offset),
        };
        ::pfn_vkCmdSetDescriptorBufferOffsets2EXT(m_command_list_unit->vk_command_buffer, std::addressof(offsets_info));

        return;
    }

    Result CommandBuffer::Initialize(const CommandBufferInfo *info) {
        this->Finalize();

        /* Allocate descriptor staging memory */
        impl::DescriptorMemoryManager *desc_mgr = impl::DescriptorMemoryManager::GetInstance();
        void *staging_memory = desc_mgr->AllocateStagingMemory();
        RESULT_RETURN_IF(staging_memory == nullptr, ResultOutOfStagingMemory);
        auto fail_guard = SCOPE_GUARD {
            desc_mgr->FreeStagingMemory(staging_memory);
        };

        /* Allocate command pool */
        RESULT_TRY(impl::CommandPoolManager::GetInstance()->CreateCommandPool(std::addressof(m_pool_id), info->queue_type));

        m_queue_type = info->queue_type;
        m_staging_memory = staging_memory;
        fail_guard.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    void CommandBuffer::Finalize() {
        impl::DescriptorMemoryManager *desc_mgr = impl::DescriptorMemoryManager::GetInstance();
        if (m_command_list_unit != nullptr) {
            desc_mgr->FreeCommandListUnit(m_command_list_unit);
            m_command_list_unit = nullptr;
        }
        if (m_staging_memory != nullptr) {
            impl::DescriptorMemoryManager::GetInstance()->FreeStagingMemory(m_staging_memory);
            m_staging_memory = nullptr;
        }
        if (m_pool_id != impl::CommandPoolManager::cInvalidCommandPoolId) {
            impl::CommandPoolManager::GetInstance()->FreeCommandPool(m_pool_id);
            m_pool_id = impl::CommandPoolManager::cInvalidCommandPoolId;
        }
    }

    Result CommandBuffer::Begin(const CommandBufferBeginInfo *info) {
        //this->ResetDescriptorMemory();

        /* Allocate new command list */
        impl::DescriptorMemoryManager *desc_mgr = impl::DescriptorMemoryManager::GetInstance();
        if (m_command_list_unit != nullptr) {
            desc_mgr->FreeCommandListUnit(m_command_list_unit);
            m_command_list_unit = nullptr;
        }
        RESULT_TRY(desc_mgr->AllocateCommandListUnit(std::addressof(m_command_list_unit), info->descriptor_memory_size, m_pool_id, info->is_primary));
        auto fail_guard = SCOPE_GUARD {
            if (m_command_list_unit != nullptr) {
                desc_mgr->FreeCommandListUnit(m_command_list_unit);
                m_command_list_unit = nullptr;
            }
        };

        /* Reset descriptor memory commit sizes */
        m_texture_commit_size = -1;
        m_buffer_commit_size = -1;

        /* Begin command buffer */
        const VkCommandBufferInheritanceInfo inheritance_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        };
        const VkCommandBufferBeginInfo begin_info = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = (info->is_primary) ? nullptr : std::addressof(inheritance_info),
        };
        const VkResult vk_result = ::pfn_vkBeginCommandBuffer(m_command_list_unit->vk_command_buffer, std::addressof(begin_info));
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));

        fail_guard.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    Result CommandBuffer::End(CommandList *out_cmd_list) {

        /* End rendering if necessary */
        this->EndRenderingIfRendering();

        /* End command buffer */
        const VkResult vk_result = ::pfn_vkEndCommandBuffer(m_command_list_unit->vk_command_buffer);
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));

        /* Set command list */
        out_cmd_list->unit = m_command_list_unit;
        m_command_list_unit = nullptr;

        RESULT_RETURN_SUCCESS;
    }

    void CommandBuffer::CallCommands(CommandList secondary_command_list) {
        ::pfn_vkCmdExecuteCommands(m_command_list_unit->vk_command_buffer, 1, std::addressof(secondary_command_list.unit->vk_command_buffer));
    }

    //void CommandBuffer::TransitionRenderTargetsToAttachment(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target) {
    //
    //    this->EndRenderingIfRendering();
    //
    //    SyncScopeInfo scope_info = {
    //        .vk_src_stage_mask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    //        .vk_src_access_mask = VK_ACCESS_2_NONE,
    //        .vk_dst_stage_mask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    //        .vk_dst_access_mask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT,
    //    };
    //    TransitionRenderTargetsImpl(color_target_array, color_target_count, depth_stencil_target, std::addressof(scope_info), VK_IMAGE_LAYOUT_GENERAL);
    //    return;
    //}
    //
    //void CommandBuffer::TransitionRenderTargetsToPresent(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target) {
    //
    //    this->EndRenderingIfRendering();
    //
    //    SyncScopeInfo scope_info = {
    //        .vk_src_stage_mask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    //        .vk_src_access_mask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT,
    //        .vk_dst_stage_mask  = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
    //        .vk_dst_access_mask = VK_ACCESS_2_NONE,
    //    };
    //    TransitionRenderTargetsImpl(color_target_array, color_target_count, depth_stencil_target, std::addressof(scope_info), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    //    return;
    //}

    void CommandBuffer::FlushMemory(GpuAccessFlags access_flags) {
        //this->EndRenderingIfRendering();
        VP_UNUSED(access_flags);
        const VkMemoryBarrier2 memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
        };
        const VkDependencyInfo dep_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .memoryBarrierCount = 1,
            .pMemoryBarriers = std::addressof(memory_barrier),
        };
        ::pfn_vkCmdPipelineBarrier2(m_command_list_unit->vk_command_buffer, std::addressof(dep_info));
    }

    void CommandBuffer::InvalidateMemory(GpuAccessFlags access_flags) {
        //this->EndRenderingIfRendering();
        VP_UNUSED(access_flags);
        const VkMemoryBarrier2 memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
        };
        const VkDependencyInfo dep_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .memoryBarrierCount = 1,
            .pMemoryBarriers = std::addressof(memory_barrier),
        };
        ::pfn_vkCmdPipelineBarrier2(m_command_list_unit->vk_command_buffer, std::addressof(dep_info));
    }

    void CommandBuffer::BlitImage(Texture *source_texture, const TextureCopyRegion *source_copy_region, Texture *dest_texture, const TextureCopyRegion *dest_copy_region, CopyFilter copy_filter) {

        this->EndRenderingIfRendering();

        const VkImageBlit2 image_blit = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .srcSubresource = {
                .aspectMask     = source_texture->GetVkAspectMask(),
                .mipLevel       = source_copy_region->base_mip_level,
                .baseArrayLayer = source_copy_region->base_array_layer,
                .layerCount     = source_copy_region->layer_count,
            },
            .srcOffsets = {
                {
                    .x = static_cast<s32>(source_copy_region->base_x),
                    .y = static_cast<s32>(source_copy_region->base_y),
                    .z = static_cast<s32>(source_copy_region->base_z),
                },
                {
                    .x = static_cast<s32>(source_copy_region->base_x + dest_copy_region->width),
                    .y = static_cast<s32>(source_copy_region->base_y + dest_copy_region->height),
                    .z = static_cast<s32>(source_copy_region->base_z + dest_copy_region->depth),
                },
            },
            .dstSubresource = {
                .aspectMask     = dest_texture->GetVkAspectMask(),
                .mipLevel       = dest_copy_region->base_mip_level,
                .baseArrayLayer = dest_copy_region->base_array_layer,
                .layerCount     = dest_copy_region->layer_count,
            },
            .dstOffsets = {
                {
                    .x = static_cast<s32>(dest_copy_region->base_x),
                    .y = static_cast<s32>(dest_copy_region->base_y),
                    .z = static_cast<s32>(dest_copy_region->base_z),
                },
                {
                    .x = static_cast<s32>(dest_copy_region->base_x + dest_copy_region->width),
                    .y = static_cast<s32>(dest_copy_region->base_y + dest_copy_region->height),
                    .z = static_cast<s32>(dest_copy_region->base_z + dest_copy_region->depth),
                },
            },
        };
        const VkBlitImageInfo2 blit_image_2 = {
            .sType          = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            .srcImage       = source_texture->GetVkImage(),
            .srcImageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .dstImage       = dest_texture->GetVkImage(),
            .dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .regionCount    = 1,
            .pRegions       = std::addressof(image_blit),
            .filter         = (copy_filter == CopyFilter::Linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST,
        };
        ::pfn_vkCmdBlitImage2(m_command_list_unit->vk_command_buffer, std::addressof(blit_image_2));

        return;
    }

    void CommandBuffer::ResolveImage(Texture *source_texture, Texture *dest_texture) {
        this->EndRenderingIfRendering();

        const VkImageResolve2 image_resolve_2 = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2,
            .srcSubresource = {
                .aspectMask = source_texture->GetVkAspectMask(),
                .layerCount = source_texture->GetArrayLayerCount(),
            },
            .dstSubresource = {
                .aspectMask = dest_texture->GetVkAspectMask(),
                .layerCount = dest_texture->GetArrayLayerCount(),
            },
            .extent = {
                .width = source_texture->GetWidth(),
                .height = source_texture->GetHeight(),
                .depth = source_texture->GetDepth(),
            },
        };
        const VkResolveImageInfo2 resolve_image_2_info = {
            .sType = VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2,
            .srcImage = source_texture->GetVkImage(),
            .srcImageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .dstImage = dest_texture->GetVkImage(),
            .dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .regionCount = 1,
            .pRegions = std::addressof(image_resolve_2),
        };
        ::pfn_vkCmdResolveImage2(m_command_list_unit->vk_command_buffer, std::addressof(resolve_image_2_info));
    }

    void CommandBuffer::ClearColor(TextureViewBinder *target_color, const TextureArrayRegion *array_region, vp::util::Color4f color) {

        const auto find_target = [&]() {
            for (u32 i = 0; i < 8; ++i) {
                if (m_color_target_array[i] == target_color) { return i; }
            }
            return 0xffff'ffff;
        };
        u32 target_index = find_target();
        if (target_index == 0xffff'ffff) {

            this->EndRenderingIfRendering();

            const VkClearColorValue color_value = { color.r, color.g, color.b, color.a, };
            const VkImageSubresourceRange subresource_range = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = array_region->base_array_layer,
                .layerCount = array_region->layer_count,
            };
            ::pfn_vkCmdClearColorImage(m_command_list_unit->vk_command_buffer, target_color->GetTextureViewKey()->texture->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, std::addressof(color_value), 1, std::addressof(subresource_range));
            return;
        }

        this->BeginRenderingIfNotRendering();

        const impl::TextureViewKey *key = target_color->GetTextureViewKey();
        const vp::util::Int2 size = key->texture->GetSize2d();
        const VkClearAttachment attachment = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .colorAttachment = target_index,
            .clearValue = { 
                .color {
                    color.r,
                    color.g,
                    color.b,
                    color.a,
                },
            },
        };
        const VkClearRect rect = {
            .rect = {
                .offset = {},
                .extent = {
                    .width = static_cast<u32>(size[0]),
                    .height = static_cast<u32>(size[1]),
                },
            },
            .baseArrayLayer = array_region->base_array_layer,
            .layerCount = array_region->layer_count,
        };
        ::pfn_vkCmdClearAttachments(m_command_list_unit->vk_command_buffer, 1, std::addressof(attachment), 1, std::addressof(rect));

        return;
    }

    void CommandBuffer::ClearDepthStencil(TextureViewBinder *target_depth_stencil, const TextureArrayRegion *array_region, float depth, u32 stencil) {

        const impl::TextureViewKey *key = target_depth_stencil->GetTextureViewKey();
        if (target_depth_stencil != m_depth_stencil_target) {

            this->EndRenderingIfRendering();

            const VkClearDepthStencilValue depth_stencil_value = { depth, stencil, };
            const VkImageSubresourceRange subresource_range = {
                .aspectMask = vp::res::GfxTextureDepthStencilModeToVkImageAspectFlags(static_cast<TextureDepthStencilMode>(key->depth_stencil_mode)),
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = array_region->base_array_layer,
                .layerCount = array_region->layer_count,
            };
            ::pfn_vkCmdClearDepthStencilImage(m_command_list_unit->vk_command_buffer, target_depth_stencil->GetTextureViewKey()->texture->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, std::addressof(depth_stencil_value), 1, std::addressof(subresource_range));
            return;
        }

        this->BeginRenderingIfNotRendering();

        const vp::util::Int2 size = key->texture->GetSize2d();
        const VkClearAttachment attachment = {
            .aspectMask = vp::res::GfxTextureDepthStencilModeToVkImageAspectFlags(static_cast<TextureDepthStencilMode>(key->depth_stencil_mode)),
            .clearValue = {
                .depthStencil = {
                    .depth = depth,
                    .stencil = stencil,
                },
            },
        };
        const VkClearRect rect = {
            .rect = {
                .offset = {},
                .extent = {
                    .width = static_cast<u32>(size[0]),
                    .height = static_cast<u32>(size[1]),
                },
            },
            .baseArrayLayer = array_region->base_array_layer,
            .layerCount = array_region->layer_count,
        };
        ::pfn_vkCmdClearAttachments(m_command_list_unit->vk_command_buffer, 1, std::addressof(attachment), 1, std::addressof(rect));

        return;
    }

    void CommandBuffer::Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdSetPrimitiveTopology(m_command_list_unit->vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
        ::pfn_vkCmdDraw(m_command_list_unit->vk_command_buffer, vertex_count, 1, base_vertex, 0);
    }

    void CommandBuffer::DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdSetPrimitiveTopology(m_command_list_unit->vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
        ::pfn_vkCmdDraw(m_command_list_unit->vk_command_buffer, vertex_count, instance_count, base_vertex, base_instance);
    }

    void CommandBuffer::DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, BufferAddress index_buffer_address, u32 index_count, u32 base_vertex) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdSetPrimitiveTopology(m_command_list_unit->vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
        const VkIndexType index_type = vp::res::GfxIndexFormatToVkIndexType(index_format);
        const u32 index_size0 = (index_format == IndexFormat::U32) ? sizeof(u32) : sizeof(u16);
        const u32 index_size1 = (index_format == IndexFormat::U16) ? index_size0 : sizeof(u8);
        ::pfn_vkCmdBindIndexBuffer2(m_command_list_unit->vk_command_buffer, index_buffer_address.GetVkBuffer(), index_buffer_address.offset, index_count * index_size1, index_type);
        ::pfn_vkCmdDrawIndexed(m_command_list_unit->vk_command_buffer, index_count, 1, 0, base_vertex, 0);
    }

    void CommandBuffer::DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, BufferAddress index_buffer_address, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdSetPrimitiveTopology(m_command_list_unit->vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
        const VkIndexType index_type = vp::res::GfxIndexFormatToVkIndexType(index_format);
        const u32 index_size0 = (index_format == IndexFormat::U32) ? sizeof(u32) : sizeof(u16);
        const u32 index_size1 = (index_format == IndexFormat::U16) ? index_size0 : sizeof(u8);
        ::pfn_vkCmdBindIndexBuffer2(m_command_list_unit->vk_command_buffer, index_buffer_address.GetVkBuffer(), index_buffer_address.offset, index_count * index_size1, index_type);
        ::pfn_vkCmdDrawIndexed(m_command_list_unit->vk_command_buffer, index_count, instance_count, 0, base_vertex, base_instance);
    }

    void CommandBuffer::DrawMeshTasks(u32 x, u32 y, u32 z) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdDrawMeshTasksEXT(m_command_list_unit->vk_command_buffer, x, y, z);
    }

    void CommandBuffer::Dispatch(u32 x, u32 y, u32 z) {
        this->CommitDescriptors();
        ::pfn_vkCmdDispatch(m_command_list_unit->vk_command_buffer, x, y, z);
    }

    void CommandBuffer::SetVertexBuffer(BufferAddress vertex_buffer_address, u32 binding, u32 stride, u32 size) {
        const VkDeviceSize device_offset = vertex_buffer_address.offset;
        const VkDeviceSize device_size = size;
        const VkDeviceSize device_stride = stride;
        VkBuffer vk_buffer = vertex_buffer_address.GetVkBuffer();
        ::pfn_vkCmdBindVertexBuffers2(m_command_list_unit->vk_command_buffer, binding, 1, std::addressof(vk_buffer), std::addressof(device_offset), std::addressof(device_size), std::addressof(device_stride));
    }

    void CommandBuffer::SetRenderTargets(TextureViewBinder **color_target_array, u32 color_target_count, TextureViewBinder *depth_stencil_target, RenderTargetClearType clear_type) {

        /* Set deferred render targets */
        for (u32 i = 0; i < color_target_count; ++i) {
            m_color_target_array[i] = color_target_array[i];
        }
        m_color_target_count = color_target_count;
        m_depth_stencil_target = depth_stencil_target;
        m_trigger_clear_type = clear_type;
        m_is_render_target_dirty = true;

        return;
    }

    void CommandBuffer::SetShader(const Shader *shader) {
        static constexpr VkShaderStageFlagBits cShaderStageFlagBitsArray[impl::Context::cTargetMaxShaderStageCount] = {
            VK_SHADER_STAGE_VERTEX_BIT,
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_SHADER_STAGE_COMPUTE_BIT,
            VK_SHADER_STAGE_TASK_BIT_EXT,
            VK_SHADER_STAGE_MESH_BIT_EXT,
        };
        const bool is_compute = (m_queue_type == QueueType::Compute);
        const u32 stage_count = (is_compute) ? 1 : impl::Context::cTargetMaxShaderStageCount - ((!gfx::IsOptionalFeature(OptionalFeature::SupportsMeshShaders)) * 2);
        const VkShaderStageFlagBits *stage_array = (is_compute) ? cShaderStageFlagBitsArray + 5 : cShaderStageFlagBitsArray;
        const VkShaderEXT *shader_array = (is_compute) ? shader->GetVkShaderArray() + 5 : shader->GetVkShaderArray();
        ::pfn_vkCmdBindShadersEXT(m_command_list_unit->vk_command_buffer, stage_count, stage_array, shader_array);
    }

    void CommandBuffer::SetTextureSampler(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot, DescriptorSlot sampler_slot) {
        VP_UNUSED(shader_stage);

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutCombinedTextureSampler();
        const u32              desc_size     = context->GetTextureSamplerDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseTextureSampler();
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));
        
        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemoryTextureSamplerImageSeparate();
        void *memory_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_memory) + desc_offset);
        
        /* Cache descriptor */
        impl::TextureViewSamplerManager *tex_samp_manager = impl::TextureViewSamplerManager::GetInstance();
        const VkImageView image_view = tex_samp_manager->GetVkImageView(texture_slot);
        const VkSampler sampler = tex_samp_manager->GetVkSampler(sampler_slot);
        const VkDescriptorImageInfo image_info = {
            .sampler     = sampler,
            .imageView   = image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
        const VkDescriptorGetInfoEXT get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .data = {
                .pCombinedImageSampler = std::addressof(image_info),
            },
        };
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(get_info), desc_size, memory_offset);
        
        /* Set max binding */
        const s32 prev = m_texture_commit_size;
        m_texture_commit_size = vp::util::Max(prev, static_cast<s32>(desc_offset + desc_size));
        m_is_dirty_texture = true;

        return;
    }

    void CommandBuffer::SetImage(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot) {
        VP_UNUSED(shader_stage);

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutCombinedTextureSampler();
        const u32              desc_size     = context->GetImageDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseImage();
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));

        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemoryTextureSamplerImageSeparate();
        void *memory_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_memory) + desc_offset);

        /* Cache descriptor */
        impl::TextureViewSamplerManager *tex_samp_manager = impl::TextureViewSamplerManager::GetInstance();
        const VkImageView image_view = tex_samp_manager->GetVkImageView(texture_slot);
        const VkDescriptorImageInfo image_info = {
            .imageView   = image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
        const VkDescriptorGetInfoEXT get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .data = {
                .pStorageImage  = std::addressof(image_info),
            },
        };
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(get_info), desc_size, memory_offset);

        /* Set max binding */
        const s32 prev = m_texture_commit_size;
        m_texture_commit_size = vp::util::Max(prev, static_cast<s32>(desc_offset + desc_size));
        m_is_dirty_texture = true;

        return;
    }

    void CommandBuffer::SetUniformBuffer(u32 location, gfx::ShaderStage shader_stage, GpuAddress gpu_address, size_t size) {
        VP_UNUSED(shader_stage);

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutCombinedBuffer();
        const u32              desc_size     = context->GetUniformBufferDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseUniformBuffer();
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));

        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemoryUniformBufferStorageBuffer();
        void *memory_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_memory) + desc_offset);

        /* Cache descriptor */
        const VkDescriptorAddressInfoEXT address_info = {
            .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
            .address = gpu_address.vk_device_address,
            .range   = size,
        };
        const VkDescriptorGetInfoEXT get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .data  = {
                .pUniformBuffer = std::addressof(address_info),
            },
        };
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(get_info), desc_size, memory_offset);

        /* Set max binding */
        const s32 prev = m_buffer_commit_size;
        m_buffer_commit_size = vp::util::Max(prev, static_cast<s32>(desc_offset + desc_size));
        m_is_dirty_buffer = true;

        return;
    }

    void CommandBuffer::SetStorageBuffer(u32 location, gfx::ShaderStage shader_stage, GpuAddress gpu_address, size_t size) {
        VP_UNUSED(shader_stage);

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutCombinedBuffer();
        const u32              desc_size     = context->GetStorageBufferDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseStorageBuffer();
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));
        
        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemoryUniformBufferStorageBuffer();
        void *memory_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_memory) + desc_offset);
        
        /* Cache descriptor */
        const VkDescriptorAddressInfoEXT address_info = {
            .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
            .address = gpu_address.vk_device_address,
            .range   = size,
        };
        const VkDescriptorGetInfoEXT get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .data  = {
                .pStorageBuffer = std::addressof(address_info),
            },
        };
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(get_info), desc_size, memory_offset);
        
        /* Set max binding */
        const s32 prev = m_buffer_commit_size;
        m_buffer_commit_size = vp::util::Max(prev, static_cast<s32>(desc_offset + desc_size));
        m_is_dirty_buffer = true;

        return;
    }

    void CommandBuffer::SetSeparateTexture(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot) {
        VP_UNUSED(shader_stage);

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutCombinedTextureSampler();
        const u32              desc_size     = context->GetSeparateTextureDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseSeparateTexture();
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));

        /* Get staging memory offset */
        void *desc_memory   = this->GetDescriptorMemoryTextureSamplerImageSeparate();
        void *memory_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_memory) + desc_offset);

        /* Cache descriptor to staging */
        impl::TextureViewSamplerManager *tex_samp_manager = impl::TextureViewSamplerManager::GetInstance();
        VP_ASSERT(tex_samp_manager->IsTextureViewSlotValid(texture_slot));
        const impl::TextureViewSamplerManager::TextureViewUnit *unit = tex_samp_manager->GetTextureViewUnit(texture_slot.index);
        ::memcpy(memory_offset, unit->descriptor_memory, desc_size);

        /* Set max binding */
        const s32 prev = m_texture_commit_size;
        m_texture_commit_size = vp::util::Max(prev, static_cast<s32>(desc_offset + desc_size));
        m_is_dirty_texture = true;

        return;
    }

    void CommandBuffer::SetSeparateSampler(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot sampler_slot) {
        VP_UNUSED(shader_stage);

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutCombinedTextureSampler();
        const u32              desc_size     = context->GetSeparateSamplerDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseSeparateSampler();
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));

        /* Get staging memory offset */
        void *desc_memory   = this->GetDescriptorMemoryTextureSamplerImageSeparate();
        void *memory_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_memory) + desc_offset);

        /* Cache descriptor to staging */
        impl::TextureViewSamplerManager *tex_samp_manager = impl::TextureViewSamplerManager::GetInstance();
        VP_ASSERT(tex_samp_manager->IsSamplerSlotValid(sampler_slot));
        const impl::TextureViewSamplerManager::SamplerUnit *unit = tex_samp_manager->GetSamplerUnit(sampler_slot.index);
        ::memcpy(memory_offset, unit->descriptor_memory, desc_size);

        /* Set max binding */
        const s32 prev = m_texture_commit_size;
        m_texture_commit_size = vp::util::Max(prev, static_cast<s32>(desc_offset + desc_size));
        m_is_dirty_texture = true;

        return;
    }

    void CommandBuffer::SetBlendState(const BlendState *blend_state) {

        ::pfn_vkCmdSetLogicOpEnableEXT(m_command_list_unit->vk_command_buffer, blend_state->is_enable_logic_op);
        const VkLogicOp logic_op = vp::res::GfxLogicOperationToVkLogicOp(static_cast<LogicOperation>(blend_state->logic_op));
        ::pfn_vkCmdSetLogicOpEXT(m_command_list_unit->vk_command_buffer, logic_op);

        VkColorBlendEquationEXT blend_equation_array[cTargetMaxBlendTargetStateCount];
        VkBool32                blend_enable_array[cTargetMaxBlendTargetStateCount];
        VkColorComponentFlags   channel_mask_array[cTargetMaxBlendTargetStateCount];
        for (u32 i = 0; i < blend_state->blend_target_state_count; ++i) {
            blend_enable_array[i]                       = blend_state->blend_target_state_array[i].is_enable;
            blend_equation_array[i].srcColorBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_array[i].blend_factor_source_color));
            blend_equation_array[i].dstColorBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_array[i].blend_factor_destination_color));
            blend_equation_array[i].colorBlendOp        = vp::res::GfxBlendEquationToVkBlendOp(static_cast<BlendEquation>(blend_state->blend_target_state_array[i].blend_equation_color));
            blend_equation_array[i].srcAlphaBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_array[i].blend_factor_source_alpha));
            blend_equation_array[i].dstAlphaBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_array[i].blend_factor_destination_alpha));
            blend_equation_array[i].alphaBlendOp        = vp::res::GfxBlendEquationToVkBlendOp(static_cast<BlendEquation>(blend_state->blend_target_state_array[i].blend_equation_alpha));
            channel_mask_array[i]                       = blend_state->blend_target_state_array[i].channel_mask;
        }
        ::pfn_vkCmdSetColorBlendEnableEXT(m_command_list_unit->vk_command_buffer, 0, blend_state->blend_target_state_count, blend_enable_array);
        ::pfn_vkCmdSetColorBlendEquationEXT(m_command_list_unit->vk_command_buffer, 0, blend_state->blend_target_state_count, blend_equation_array);
        ::pfn_vkCmdSetColorWriteMaskEXT(m_command_list_unit->vk_command_buffer, 0, blend_state->blend_target_state_count, channel_mask_array);

        ::pfn_vkCmdSetBlendConstants(m_command_list_unit->vk_command_buffer, std::addressof(blend_state->blend_constant_color.r));

        return;
    }

    void CommandBuffer::SetDepthStencilState(const DepthStencilState *depth_stencil_state) {

        ::pfn_vkCmdSetDepthTestEnable(m_command_list_unit->vk_command_buffer, depth_stencil_state->is_enable_depth_test);
        ::pfn_vkCmdSetDepthWriteEnable(m_command_list_unit->vk_command_buffer, depth_stencil_state->is_enable_depth_write);

        const VkCompareOp depth_op = static_cast<VkCompareOp>(depth_stencil_state->depth_comparison_function);
        ::pfn_vkCmdSetDepthCompareOp(m_command_list_unit->vk_command_buffer, depth_op);

        ::pfn_vkCmdSetDepthBoundsTestEnable(m_command_list_unit->vk_command_buffer, false);

        ::pfn_vkCmdSetStencilTestEnable(m_command_list_unit->vk_command_buffer, depth_stencil_state->is_enable_stencil_test);
        ::pfn_vkCmdSetStencilCompareMask(m_command_list_unit->vk_command_buffer, VK_STENCIL_FACE_FRONT_AND_BACK, depth_stencil_state->stencil_value_mask);
        ::pfn_vkCmdSetStencilWriteMask(m_command_list_unit->vk_command_buffer, VK_STENCIL_FACE_FRONT_AND_BACK, depth_stencil_state->stencil_write_mask);

        const VkStencilOp fail_op_front = static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_fail_op);
        const VkStencilOp depth_pass_op_front = static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_depth_pass_op);
        const VkStencilOp depth_fail_op_front = static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_depth_fail_op);
        const VkCompareOp compare_op_front = static_cast<VkCompareOp>(depth_stencil_state->front_face_stencil_comparison_function);
        ::pfn_vkCmdSetStencilOp(m_command_list_unit->vk_command_buffer,VK_STENCIL_FACE_FRONT_BIT, fail_op_front, depth_pass_op_front, depth_fail_op_front, compare_op_front);
        ::pfn_vkCmdSetStencilReference(m_command_list_unit->vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->front_face_stencil_reference);
        const VkStencilOp fail_op_back = static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_fail_op);
        const VkStencilOp depth_pass_op_back = static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_depth_pass_op);
        const VkStencilOp depth_fail_op_back = static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_depth_fail_op);
        const VkCompareOp compare_op_back = static_cast<VkCompareOp>(depth_stencil_state->back_face_stencil_comparison_function);
        ::pfn_vkCmdSetStencilOp(m_command_list_unit->vk_command_buffer,VK_STENCIL_FACE_BACK_BIT, fail_op_back, depth_pass_op_back, depth_fail_op_back, compare_op_back);
        ::pfn_vkCmdSetStencilReference(m_command_list_unit->vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->back_face_stencil_reference);

        return;
    }

    void CommandBuffer::SetRasterizerState(const RasterizerState *rasterizer_state) {

        ::pfn_vkCmdSetRasterizerDiscardEnable(m_command_list_unit->vk_command_buffer, rasterizer_state->is_enable_rasterizer_discard);

        const VkConservativeRasterizationModeEXT rasterization_mode = static_cast<VkConservativeRasterizationModeEXT>(rasterizer_state->is_enable_conservative_rasterizer);
        ::pfn_vkCmdSetConservativeRasterizationModeEXT(m_command_list_unit->vk_command_buffer, rasterization_mode);
        ::pfn_vkCmdSetExtraPrimitiveOverestimationSizeEXT(m_command_list_unit->vk_command_buffer, 0.0f);

        const VkSampleCountFlagBits sample_count_flags = static_cast<VkSampleCountFlagBits>(rasterizer_state->sample_count);
        ::pfn_vkCmdSetRasterizationSamplesEXT(m_command_list_unit->vk_command_buffer, sample_count_flags);
        ::pfn_vkCmdSetSampleMaskEXT(m_command_list_unit->vk_command_buffer, sample_count_flags, std::addressof(rasterizer_state->sample_mask));

        ::pfn_vkCmdSetAlphaToCoverageEnableEXT(m_command_list_unit->vk_command_buffer, rasterizer_state->is_enable_alpha_to_coverage);
        ::pfn_vkCmdSetDepthClampEnableEXT(m_command_list_unit->vk_command_buffer, rasterizer_state->is_enable_depth_clamp);

        const VkPolygonMode polygon_mode = vp::res::GfxFillModeToVkPolygonMode(static_cast<FillMode>(rasterizer_state->fill_mode));
        const VkCullModeFlags cull_mode = vp::res::GfxCullModeToVkCullModeFlags(static_cast<CullMode>(rasterizer_state->cull_mode));
        const VkFrontFace front_face = vp::res::GfxFrontFaceToVkFrontFace(static_cast<FrontFace>(rasterizer_state->front_face));
        ::pfn_vkCmdSetPolygonModeEXT(m_command_list_unit->vk_command_buffer, polygon_mode);
        ::pfn_vkCmdSetCullMode(m_command_list_unit->vk_command_buffer, cull_mode);
        ::pfn_vkCmdSetFrontFace(m_command_list_unit->vk_command_buffer, front_face);

        ::pfn_vkCmdSetDepthBiasEnable(m_command_list_unit->vk_command_buffer, true);
        ::pfn_vkCmdSetDepthBias(m_command_list_unit->vk_command_buffer, rasterizer_state->polygon_offset_constant_factor, rasterizer_state->polygon_offset_clamp, rasterizer_state->polygon_offset_slope_factor);

        return;
    }

    void CommandBuffer::SetVertexState(const VertexState *vertex_state) {

        VkVertexInputBindingDescription2EXT   vertex_binding_description_array[cTargetMaxVertexBufferCount];
        VkVertexInputAttributeDescription2EXT vertex_attribute_description_array[cTargetMaxVertexBufferCount];
        for (u32 i = 0; i < vertex_state->vertex_buffer_state_count; ++i) {
            vertex_binding_description_array[i].sType     = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
            vertex_binding_description_array[i].pNext     = nullptr;
            vertex_binding_description_array[i].binding   = vertex_state->vertex_attribute_state_array[i].binding;
            vertex_binding_description_array[i].stride    = vertex_state->vertex_buffer_state_array[i].stride;
            vertex_binding_description_array[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            vertex_binding_description_array[i].divisor   = vertex_state->vertex_buffer_state_array[i].divisor;
        }
        for (u32 i = 0; i < vertex_state->vertex_attribute_state_count; ++i) {
            vertex_attribute_description_array[i].sType    = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
            vertex_attribute_description_array[i].pNext     = nullptr;
            vertex_attribute_description_array[i].location = vertex_state->vertex_attribute_state_array[i].interface_slot;
            vertex_attribute_description_array[i].binding  = vertex_state->vertex_attribute_state_array[i].binding;
            vertex_attribute_description_array[i].format   = vp::res::GfxAttributeFormatToVkFormat(static_cast<AttributeFormat>(vertex_state->vertex_attribute_state_array[i].attribute_format));
            vertex_attribute_description_array[i].offset   = vertex_state->vertex_attribute_state_array[i].offset;
        }
        ::pfn_vkCmdSetVertexInputEXT(m_command_list_unit->vk_command_buffer, vertex_state->vertex_buffer_state_count, vertex_binding_description_array, vertex_state->vertex_attribute_state_count, vertex_attribute_description_array);
        ::pfn_vkCmdSetPrimitiveRestartEnable(m_command_list_unit->vk_command_buffer, false);
    }

    void CommandBuffer::SetViewportScissorState(const ViewportScissorState *viewport_scissor_state) {

        /* Transform viewport state to represent a lower left window origin */
        VkViewport viewport_array[cTargetMaxViewportScissorCount];
        VkRect2D scissor_array[cTargetMaxViewportScissorCount];
        if (viewport_scissor_state->is_enable_scissor) {
            VP_ASSERT(viewport_scissor_state->scissor_state_array != nullptr)
            for (u32 i = 0; i < viewport_scissor_state->state_count; ++i) {
                viewport_array[i].x = viewport_scissor_state->viewport_state_array[i].position[0];
                viewport_array[i].y = viewport_scissor_state->viewport_state_array[i].extents[1] - viewport_scissor_state->viewport_state_array[i].position[1];
                viewport_array[i].width = viewport_scissor_state->viewport_state_array[i].extents[0];
                viewport_array[i].height = -viewport_scissor_state->viewport_state_array[i].extents[1];
                viewport_array[i].maxDepth = viewport_scissor_state->viewport_state_array[i].depth_range[0];
                viewport_array[i].minDepth = viewport_scissor_state->viewport_state_array[i].depth_range[1];
                scissor_array[i].offset.x      = viewport_scissor_state->scissor_state_array[i].position[0];
                scissor_array[i].offset.y      = viewport_scissor_state->scissor_state_array[i].position[1];
                scissor_array[i].extent.width  = viewport_scissor_state->scissor_state_array[i].extents[0];
                scissor_array[i].extent.height = viewport_scissor_state->scissor_state_array[i].extents[1];
            }
        } else {
            for (u32 i = 0; i < viewport_scissor_state->state_count; ++i) {
                viewport_array[i].x = viewport_scissor_state->viewport_state_array[i].position[0];
                viewport_array[i].y = viewport_scissor_state->viewport_state_array[i].extents[1] - viewport_scissor_state->viewport_state_array[i].position[1];
                viewport_array[i].width = viewport_scissor_state->viewport_state_array[i].extents[0];
                viewport_array[i].height = -viewport_scissor_state->viewport_state_array[i].extents[1];
                viewport_array[i].maxDepth = viewport_scissor_state->viewport_state_array[i].depth_range[0];
                viewport_array[i].minDepth = viewport_scissor_state->viewport_state_array[i].depth_range[1];
                scissor_array[i].offset.x      = 0;
                scissor_array[i].offset.y      = 0;
                scissor_array[i].extent.width  = 0x7fff'ffff;
                scissor_array[i].extent.height = 0x7fff'ffff;
            }
        }
        ::pfn_vkCmdSetViewportWithCount(m_command_list_unit->vk_command_buffer, viewport_scissor_state->state_count, viewport_array);
        ::pfn_vkCmdSetScissorWithCount(m_command_list_unit->vk_command_buffer, viewport_scissor_state->state_count, scissor_array);

        return;
    }

    void CommandBuffer::SetLineWidth(float line_width) {
        ::pfn_vkCmdSetLineWidth(m_command_list_unit->vk_command_buffer, line_width);
    }
}

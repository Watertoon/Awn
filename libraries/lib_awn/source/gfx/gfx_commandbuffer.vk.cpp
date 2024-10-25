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

    void CommandBuffer::TransitionRenderTargetsImpl(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target, SyncScopeInfo *sync_scope, VkImageLayout image_layout) {

        VkImageMemoryBarrier2 barrier_array[cTargetMaxBoundRenderTargetColorCount + 1] = {};

        /* Setup color target barriers */
        for (u32 i = 0; i < color_target_count; ++i) {
            barrier_array[i].sType                        = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier_array[i].srcStageMask                 = sync_scope->vk_src_stage_mask;
            barrier_array[i].srcAccessMask                = sync_scope->vk_src_access_mask;
            barrier_array[i].dstStageMask                 = sync_scope->vk_dst_stage_mask;
            barrier_array[i].dstAccessMask                = sync_scope->vk_dst_access_mask;
            barrier_array[i].oldLayout                    = color_target_array[i]->GetVkImageLayout();
            barrier_array[i].newLayout                    = image_layout;
            barrier_array[i].image                        = color_target_array[i]->GetVkImage();
            barrier_array[i].subresourceRange.aspectMask  = color_target_array[i]->GetVkAspectMask();
            barrier_array[i].subresourceRange.levelCount  = 1;
            barrier_array[i].subresourceRange.layerCount  = color_target_array[i]->GetViewCount();

            color_target_array[i]->SetVkImageLayout(image_layout);
        }

        if (depth_stencil_target != nullptr && image_layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier_array[color_target_count].sType                        = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier_array[color_target_count].srcStageMask                 = (sync_scope->vk_src_stage_mask == VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT) ? VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT : sync_scope->vk_src_stage_mask;
            barrier_array[color_target_count].srcAccessMask                = (sync_scope->vk_src_access_mask == VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT) ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : sync_scope->vk_src_access_mask;
            barrier_array[color_target_count].dstStageMask                 = sync_scope->vk_dst_stage_mask;
            barrier_array[color_target_count].dstAccessMask                = sync_scope->vk_dst_access_mask;
            barrier_array[color_target_count].oldLayout                    = depth_stencil_target->GetVkImageLayout();
            barrier_array[color_target_count].newLayout                    = (image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : image_layout;
            barrier_array[color_target_count].image                        = depth_stencil_target->GetVkImage();
            barrier_array[color_target_count].subresourceRange.aspectMask  = depth_stencil_target->GetVkAspectMask();
            barrier_array[color_target_count].subresourceRange.levelCount  = 1;
            barrier_array[color_target_count].subresourceRange.layerCount  = depth_stencil_target->GetViewCount();

            depth_stencil_target->SetVkImageLayout(image_layout);
            color_target_count = color_target_count + 1;
        }

        const VkDependencyInfo dep_info = {
            .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = color_target_count,
            .pImageMemoryBarriers    = barrier_array
        }; 
        ::pfn_vkCmdPipelineBarrier2(m_command_list.vk_command_buffer, std::addressof(dep_info));

        return;
    }

    void CommandBuffer::CommitDescriptorMemory(ShaderStage stage, u32 base_set, void *desc_memory, u32 memory_size) {

        /* Allocate descriptor memory */
        const u32 memory_offset = m_command_memory_iter;
        const u32 next_offset = vp::util::AlignUp(memory_offset + memory_size, impl::Context::cTargetDescriptorBufferAlignment);
        VP_ASSERT(next_offset < m_command_memory_size);
        m_command_memory_iter = next_offset;
        void *memory = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_command_memory_base) + memory_offset);

        /* Copy descriptors to gpu */
        ::memcpy(memory, desc_memory, memory_size);

        /* Set descriptor set offset */
        const size_t gpu_offset = m_command_memory_base_offset + memory_offset;
        const VkSetDescriptorBufferOffsetsInfoEXT offsets_info = {
            .sType = VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT,\
            .pNext = std::addressof(impl::Context::GetInstance()->GetPerStageDescriptorLayout(stage)->vk_pipeline_layout_create_info),
            .stageFlags = vp::res::GfxShaderStageToVkShaderStageFlags(stage),
            .firstSet = base_set,
            .setCount = 1,
            .pBufferIndices = std::addressof(impl::Context::cTargetDescriptorBufferIndexPerStage),
            .pOffsets = std::addressof(gpu_offset),
        };
        ::pfn_vkCmdSetDescriptorBufferOffsets2EXT(m_command_list.vk_command_buffer, std::addressof(offsets_info));

        return;
    }

    void CommandBuffer::CommitDescriptors() {

        const auto commit_desc_lambda = [this]<size_t BaseSetIndex>() {

            /* Allocate descriptor buffer space, copy descriptor memory to buffer, and update binding offsets for each changed stage */
            const s16 *size_array = (BaseSetIndex == impl::Context::cTargetTextureSamplerImageSeparateLayoutSet) ? m_ts_i_st_ss_commit_size_array : m_ubo_ssbo_commit_size_array;
            const auto memory_desc_lambda = [this](ShaderStage stage, s16 size) {                
                if (-1 < size) {
                    void *desc_memory = (BaseSetIndex == impl::Context::cTargetTextureSamplerImageSeparateLayoutSet) ? this->GetDescriptorMemoryTextureSamplerImageSeparate(stage) : this->GetDescriptorMemoryUniformBufferStorageBuffer(stage);
                    this->CommitDescriptorMemory(stage, BaseSetIndex, desc_memory, size);
                }
            };
            memory_desc_lambda(ShaderStage::Vertex, size_array[static_cast<u32>(ShaderStage::Vertex)]);
            memory_desc_lambda(ShaderStage::TessellationControl, size_array[static_cast<u32>(ShaderStage::TessellationControl)]);
            memory_desc_lambda(ShaderStage::TessellationEvaluation, size_array[static_cast<u32>(ShaderStage::TessellationEvaluation)]);
            memory_desc_lambda(ShaderStage::Geometry, size_array[static_cast<u32>(ShaderStage::Geometry)]);
            memory_desc_lambda(ShaderStage::Fragment, size_array[static_cast<u32>(ShaderStage::Fragment)]);
            memory_desc_lambda(ShaderStage::Compute, size_array[static_cast<u32>(ShaderStage::Compute)]);
            memory_desc_lambda(ShaderStage::Task, size_array[static_cast<u32>(ShaderStage::Task)]);
            memory_desc_lambda(ShaderStage::Mesh, size_array[static_cast<u32>(ShaderStage::Mesh)]);

            return;
        };
        /* Commit uniform buffer and storage image descriptors */
        if (m_is_dirty_ts_i_st_ss) {
            commit_desc_lambda.template operator()<impl::Context::cTargetTextureSamplerImageSeparateLayoutSet>();
            m_is_dirty_ts_i_st_ss = false;
        }
        /* Commit separate texture and separate sampler descriptors */
        if (m_is_dirty_ubo_ssbo) {
            commit_desc_lambda.template operator()<impl::Context::cTargetUniformBufferStorageBufferLayoutSet>();
            m_is_dirty_ubo_ssbo = false;
        }

        return;
    }

    void CommandBuffer::BeginRenderingIfNotRendering() {

        /* Ignore if still rendering */
        if (!m_is_rendering && !m_is_render_target_dirty) { return; }
        m_is_rendering           = true;
        m_is_render_target_dirty = false;

        /* Get values that should be synced across render targets */
        RenderTargetColor        **color_target_array   = m_color_target_array;
        RenderTargetDepthStencil  *depth_stencil_target = m_depth_stencil_target;
        u32 render_width  = 0;
        u32 render_height = 0;
        u32 view_count    = 0;
        u32 view_mask     = 0;
        if (m_color_target_count != 0) {
            render_width  = color_target_array[0]->GetRenderWidth();
            render_height = color_target_array[0]->GetRenderHeight();
            view_count    = color_target_array[0]->GetViewCount();
            view_mask     = color_target_array[0]->GetViewMask();
        } else if (depth_stencil_target != nullptr) {
            render_width  = depth_stencil_target->GetRenderWidth();
            render_height = depth_stencil_target->GetRenderHeight();
            view_count    = depth_stencil_target->GetViewCount();
            view_mask     = depth_stencil_target->GetViewMask();
        }

        /* Setup color target attachments */
        u32 color_attachment_index_array[cTargetMaxBoundRenderTargetColorCount] = {};
        VkRenderingAttachmentInfo color_attachment_info_array[cTargetMaxBoundRenderTargetColorCount] = {};
        for (u32 i = 0; i < m_color_target_count; ++i) {
            color_attachment_info_array[i].sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            color_attachment_info_array[i].imageView   = color_target_array[i]->GetVkImageView();
            color_attachment_info_array[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            color_attachment_info_array[i].loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_attachment_info_array[i].storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment_index_array[i] = i;
        }

        /* Setup depth stencil attachments */
        VkRenderingAttachmentInfo  depth_attachment_info             = {};
        VkRenderingAttachmentInfo  stencil_attachment_info           = {};
        VkRenderingAttachmentInfo *selected_depth_attachment_info   = nullptr;
        VkRenderingAttachmentInfo *selected_stencil_attachment_info = nullptr;
        if (depth_stencil_target != nullptr) {
            if (depth_stencil_target->IsDepthFormat()) {
                depth_attachment_info.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                depth_attachment_info.imageView   = depth_stencil_target->GetVkImageView();
                depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                depth_attachment_info.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                depth_attachment_info.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                selected_depth_attachment_info    = std::addressof(depth_attachment_info);
            }
            if (depth_stencil_target->IsStencilFormat()) {
                stencil_attachment_info.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                stencil_attachment_info.imageView   = depth_stencil_target->GetVkImageView();
                stencil_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                stencil_attachment_info.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                stencil_attachment_info.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                selected_stencil_attachment_info = std::addressof(stencil_attachment_info);
            }
        }

        /* Set render targets */
        const VkRenderingInfo rendering_info = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT,
            .renderArea = {
                .offset = {
                    .x = 0,
                    .y = 0,
                },
                .extent = {
                    .width  = render_width,
                    .height = render_height,
                },
            },
            .layerCount           = view_count,
            .viewMask             = view_mask,
            .colorAttachmentCount = m_color_target_count,
            .pColorAttachments    = color_attachment_info_array,
            .pDepthAttachment     = selected_depth_attachment_info,
            .pStencilAttachment   = selected_stencil_attachment_info,
        };
        ::pfn_vkCmdBeginRendering(m_command_list.vk_command_buffer, std::addressof(rendering_info));

        /* Map image indices */
        VkRenderingAttachmentLocationInfoKHR attachment_location_info = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO_KHR,
            .colorAttachmentCount = m_color_target_count,
            .pColorAttachmentLocations = color_attachment_index_array,
        };
        ::pfn_vkCmdSetRenderingAttachmentLocationsKHR(m_command_list.vk_command_buffer, std::addressof(attachment_location_info));

        return;
    }

    void CommandBuffer::EndRenderingIfRendering() {

        if (!m_is_rendering) { return; }
        m_is_rendering = false;

        ::pfn_vkCmdEndRendering(m_command_list.vk_command_buffer);

        return;
    }

    void CommandBuffer::BlitImage(Texture *dest_texture, const TextureCopyRegion *dest_copy_region, Texture *source_texture, const TextureCopyRegion *source_copy_region, CopyFilter copy_filter) {

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
        ::pfn_vkCmdBlitImage2(m_command_list.vk_command_buffer, std::addressof(blit_image_2));

        return;
    }

    void CommandBuffer::ClearColor(RenderTargetColor *target_color, const TextureArrayRegion *array_region, vp::util::Color4f color) {

        if (!m_is_rendering) {

            const VkClearColorValue color_value = { color.r, color.g, color.b, color.a, };
            const VkImageSubresourceRange subresource_range = {
                .aspectMask = target_color->GetVkAspectMask(),
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = array_region->base_array_layer,
                .layerCount = array_region->layer_count,
            };
            ::pfn_vkCmdClearColorImage(m_command_list.vk_command_buffer, target_color->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, std::addressof(color_value), 1, std::addressof(subresource_range));
            return;
        }

        this->BeginRenderingIfNotRendering();

        const VkClearAttachment attachment = {
            .aspectMask = target_color->GetVkAspectMask(),
            .clearValue = { color.r, color.g, color.b, color.a, },
        };
        const VkClearRect rect = {
            .rect = {
                .offset = {},
                .extent = {
                    .width = target_color->GetRenderWidth(),
                    .height = target_color->GetRenderHeight(),
                },
            },
            .baseArrayLayer = array_region->base_array_layer,
            .layerCount = array_region->layer_count,
        };
        ::pfn_vkCmdClearAttachments(m_command_list.vk_command_buffer, 1, std::addressof(attachment), 1, std::addressof(rect));
    }

    void CommandBuffer::SetSystemDescriptorBuffers() {

        /* Set bindless and per stage descriptor buffer */
        VkDeviceAddress descriptor_buffer_address_array[2] = {};
        impl::TextureViewSamplerManager *tex_samp_manager = impl::TextureViewSamplerManager::GetInstance();
        if (tex_samp_manager != nullptr) {
            descriptor_buffer_address_array[0] = tex_samp_manager->GetBindlessDescriptorBufferAddress().vk_device_address;
        }
        impl::DescriptorMemoryManager *desc_memory_mgr = impl::DescriptorMemoryManager::GetInstance();
        if (desc_memory_mgr != nullptr) {
            descriptor_buffer_address_array[1] = desc_memory_mgr->GetDescriptorBufferAddress().vk_device_address;
        }
        const VkDescriptorBufferBindingInfoEXT binding_info[2] = {
            {
                .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .address = descriptor_buffer_address_array[0],
                .usage   = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
            },
            {
                .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .address = descriptor_buffer_address_array[1],
                .usage   = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
            },
        };
        ::pfn_vkCmdBindDescriptorBuffersEXT(m_command_list.vk_command_buffer, sizeof(binding_info) / sizeof(VkDescriptorBufferBindingInfoEXT),binding_info);

        return;
    }

    void CommandBuffer::Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdSetPrimitiveTopology(m_command_list.vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
        ::pfn_vkCmdDraw(m_command_list.vk_command_buffer, vertex_count, 1, base_vertex, 0);
    }

    void CommandBuffer::DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdSetPrimitiveTopology(m_command_list.vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
        ::pfn_vkCmdDraw(m_command_list.vk_command_buffer, vertex_count, instance_count, base_vertex, base_instance);
    }

    void CommandBuffer::DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuBufferAddress index_buffer_address, u32 index_count, u32 base_vertex) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdSetPrimitiveTopology(m_command_list.vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
        ::pfn_vkCmdBindIndexBuffer(m_command_list.vk_command_buffer, index_buffer_address.vk_buffer, 0, vp::res::GfxIndexFormatToVkIndexType(index_format));
        ::pfn_vkCmdDrawIndexed(m_command_list.vk_command_buffer, index_count, 1, 0, base_vertex, 0);
    }

    void CommandBuffer::DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuBufferAddress index_buffer_address, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdSetPrimitiveTopology(m_command_list.vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
        ::pfn_vkCmdBindIndexBuffer(m_command_list.vk_command_buffer, index_buffer_address.vk_buffer, 0, vp::res::GfxIndexFormatToVkIndexType(index_format));
        ::pfn_vkCmdDrawIndexed(m_command_list.vk_command_buffer, index_count, instance_count, 0, base_vertex, base_instance);
    }

    void CommandBuffer::DrawMeshTasks(u32 x, u32 y, u32 z) {
        this->CommitDescriptors();
        this->BeginRenderingIfNotRendering();
        ::pfn_vkCmdDrawMeshTasksEXT(m_command_list.vk_command_buffer, x, y, z);
    }

    void CommandBuffer::Dispatch(u32 x, u32 y, u32 z) {
        this->CommitDescriptors();
        ::pfn_vkCmdDispatch(m_command_list.vk_command_buffer, x, y, z);
    }

    void CommandBuffer::SetVertexBuffer(GpuBufferAddress vertex_buffer_address, u32 binding, u32 stride, u32 size) {
        const VkDeviceSize device_offset = 0;
        const VkDeviceSize device_size   = size;
        const VkDeviceSize device_stride = stride;
        VkBuffer           vk_buffer     = vertex_buffer_address.vk_buffer;
        ::pfn_vkCmdBindVertexBuffers2(m_command_list.vk_command_buffer, binding, 1, std::addressof(vk_buffer), std::addressof(device_offset), std::addressof(device_size), std::addressof(device_stride));
    }

    void CommandBuffer::TransitionRenderTargetsToAttachment(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target) {

        SyncScopeInfo scope_info = {
            .vk_src_stage_mask  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
            .vk_src_access_mask = VK_ACCESS_2_NONE,
            .vk_dst_stage_mask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .vk_dst_access_mask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT,
        };
        TransitionRenderTargetsImpl(color_target_array, color_target_count, depth_stencil_target, std::addressof(scope_info), VK_IMAGE_LAYOUT_GENERAL);
        return;
    }

    void CommandBuffer::TransitionRenderTargetsToPresent(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target) {

        this->EndRenderingIfRendering();

        SyncScopeInfo scope_info = {
            .vk_src_stage_mask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .vk_src_access_mask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .vk_dst_stage_mask  = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            .vk_dst_access_mask = VK_ACCESS_2_NONE,
        };
        TransitionRenderTargetsImpl(color_target_array, color_target_count, depth_stencil_target, std::addressof(scope_info), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        return;
    }

    void CommandBuffer::SetRenderTargets(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target) {

        /* Set deferred render targets */
        for (u32 i = 0; i < color_target_count; ++i) {
            m_color_target_array[i] = color_target_array[i];
        }
        m_color_target_count = color_target_count;
        m_depth_stencil_target = depth_stencil_target;
        m_is_render_target_dirty = true;

        return;
    }

    void CommandBuffer::SetTextureAndSampler(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot, DescriptorSlot sampler_slot) {

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutTextureSamplerImageSeparate(shader_stage);
        const u32              desc_size     = context->GetTextureSamplerDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseTextureSampler(shader_stage);
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));
        
        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemoryTextureSampler(shader_stage);
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
        const s16 prev = m_ts_i_st_ss_commit_size_array[static_cast<u32>(shader_stage)];
        m_ts_i_st_ss_commit_size_array[static_cast<u32>(shader_stage)] = vp::util::Max(prev, static_cast<s16>(desc_offset + desc_size));
        m_is_dirty_ts_i_st_ss = true;

        return;
    }

    void CommandBuffer::SetImage(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot) {

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutTextureSamplerImageSeparate(shader_stage);
        const u32              desc_size     = context->GetImageDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseImage(shader_stage);
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));

        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemoryImage(shader_stage);
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
        const s16 prev = m_ts_i_st_ss_commit_size_array[static_cast<u32>(shader_stage)];
        m_ts_i_st_ss_commit_size_array[static_cast<u32>(shader_stage)] = vp::util::Max(prev, static_cast<s16>(desc_offset + desc_size));
        m_is_dirty_ts_i_st_ss = true;

        return;
    }

    void CommandBuffer::SetUniformBuffer(u32 location, gfx::ShaderStage shader_stage, GpuBufferAddress constant_buffer_address, size_t size) {

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutUniformBufferStorageBuffer(shader_stage);
        const u32              desc_size     = context->GetUniformBufferDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseUniformBuffer(shader_stage);
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));

        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemoryUniformBuffer(shader_stage);
        void *memory_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_memory) + desc_offset);

        /* Cache descriptor */
        const VkDescriptorAddressInfoEXT address_info = {
            .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
            .address = constant_buffer_address.vk_device_address,
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
        const s16 prev = m_ubo_ssbo_commit_size_array[static_cast<u32>(shader_stage)];
        m_ubo_ssbo_commit_size_array[static_cast<u32>(shader_stage)] = vp::util::Max(prev, static_cast<s16>(desc_offset + desc_size));
        m_is_dirty_ubo_ssbo = true;

        return;
    }

    void CommandBuffer::SetStorageBuffer(u32 location, gfx::ShaderStage shader_stage, GpuBufferAddress storage_buffer_address, size_t size) {

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutUniformBufferStorageBuffer(shader_stage);
        const u32              desc_size     = context->GetStorageBufferDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseStorageBuffer(shader_stage);
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));
        
        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemoryStorageBuffer(shader_stage);
        void *memory_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_memory) + desc_offset);
        
        /* Cache descriptor */
        const VkDescriptorAddressInfoEXT address_info = {
            .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
            .address = storage_buffer_address.vk_device_address,
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
        const s16 prev = m_ubo_ssbo_commit_size_array[static_cast<u32>(shader_stage)];
        m_ubo_ssbo_commit_size_array[static_cast<u32>(shader_stage)] = vp::util::Max(prev, static_cast<s16>(desc_offset + desc_size));
        m_is_dirty_ubo_ssbo = true;

        return;
    }

    void CommandBuffer::SetSeparateTexture(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot) {

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutTextureSamplerImageSeparate(shader_stage);
        const u32              desc_size     = context->GetSeparateTextureDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseSeparateTexture(shader_stage);
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));

        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemorySeparateTexture(shader_stage);
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
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .data = {
                .pSampledImage  = std::addressof(image_info),
            },
        };
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(get_info), desc_size, memory_offset);

        /* Set max binding */
        const s16 prev = m_ts_i_st_ss_commit_size_array[static_cast<u32>(shader_stage)];
        m_ts_i_st_ss_commit_size_array[static_cast<u32>(shader_stage)] = vp::util::Max(prev, static_cast<s16>(desc_offset + desc_size));
        m_is_dirty_ts_i_st_ss = true;

        return;
    }

    void CommandBuffer::SetSeparateSampler(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot sampler_slot) {

        /* Get descriptor memory offset */
        impl::Context         *context       = impl::Context::GetInstance();
        VkDevice               vk_device     = context->GetVkDevice();
        VkDescriptorSetLayout  vk_set_layout = context->GetPerStageVkDescriptorSetLayoutTextureSamplerImageSeparate(shader_stage);
        const u32              desc_size     = context->GetSeparateSamplerDescriptorSize();
        const u32              base_binding  = impl::Context::GetPerStageBindingBaseSeparateSampler(shader_stage);
        const u32              binding       = base_binding + location;
        VkDeviceSize           desc_offset   = 0;
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(vk_device, vk_set_layout, binding, std::addressof(desc_offset));

        /* Get memory offset */
        void *desc_memory   = this->GetDescriptorMemorySeparateSampler(shader_stage);
        void *memory_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_memory) + desc_offset);

        /* Cache descriptor */
        impl::TextureViewSamplerManager *tex_samp_manager = impl::TextureViewSamplerManager::GetInstance();
        const VkSampler sampler = tex_samp_manager->GetVkSampler(sampler_slot);
        const VkDescriptorGetInfoEXT get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .data = {
                .pSampler  = std::addressof(sampler),
            },
        };
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(get_info), desc_size, memory_offset);

        /* Set max binding */
        const s16 prev = m_ts_i_st_ss_commit_size_array[static_cast<u32>(shader_stage)];
        m_ts_i_st_ss_commit_size_array[static_cast<u32>(shader_stage)] = vp::util::Max(prev, static_cast<s16>(desc_offset + desc_size));
        m_is_dirty_ts_i_st_ss = true;

        return;
    }

    void CommandBuffer::SetShader(Shader *shader) {
        VkShaderStageFlagBits **flag_bit_array = nullptr;
        VkShaderEXT           **shader_array   = nullptr;
        const u32               stage_count    = shader->SetupForCommandList(shader_array, flag_bit_array);
        ::pfn_vkCmdBindShadersEXT(m_command_list.vk_command_buffer, stage_count, *flag_bit_array, *shader_array);
    }

    void CommandBuffer::SetBlendState(BlendState *blend_state) {

        ::pfn_vkCmdSetLogicOpEnableEXT(m_command_list.vk_command_buffer, true);
        ::pfn_vkCmdSetLogicOpEXT(m_command_list.vk_command_buffer, vp::res::GfxLogicOperationToVkLogicOp(static_cast<LogicOperation>(blend_state->logic_op)));

        VkColorBlendEquationEXT blend_equation_array[cTargetMaxBlendTargetStateCount] = {};
        VkBool32                blend_enable_array[cTargetMaxBlendTargetStateCount] = {};
        VkColorComponentFlags   channel_mask_array[cTargetMaxBlendTargetStateCount] = {};
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
        ::pfn_vkCmdSetColorBlendEnableEXT(m_command_list.vk_command_buffer, 0, blend_state->blend_target_state_count, blend_enable_array);
        ::pfn_vkCmdSetColorBlendEquationEXT(m_command_list.vk_command_buffer, 0, blend_state->blend_target_state_count, blend_equation_array);
        ::pfn_vkCmdSetColorWriteMaskEXT(m_command_list.vk_command_buffer, 0, blend_state->blend_target_state_count, channel_mask_array);

        ::pfn_vkCmdSetBlendConstants(m_command_list.vk_command_buffer, std::addressof(blend_state->blend_constant_color.r));

        return;
    }

    void CommandBuffer::SetDepthStencilState(DepthStencilState *depth_stencil_state) {

        ::pfn_vkCmdSetDepthTestEnable(m_command_list.vk_command_buffer, depth_stencil_state->is_enable_depth_test);
        ::pfn_vkCmdSetDepthWriteEnable(m_command_list.vk_command_buffer, depth_stencil_state->is_enable_depth_write);

        ::pfn_vkCmdSetDepthCompareOp(m_command_list.vk_command_buffer, static_cast<VkCompareOp>(depth_stencil_state->depth_comparison_function));

        if (!depth_stencil_state->is_enable_depth_bounds) {
            ::pfn_vkCmdSetDepthBoundsTestEnable(m_command_list.vk_command_buffer, false);
        } else {
            ::pfn_vkCmdSetDepthBoundsTestEnable(m_command_list.vk_command_buffer, true);
            ::pfn_vkCmdSetDepthBounds(m_command_list.vk_command_buffer, 0.0f, 1.0f);
        }

        ::pfn_vkCmdSetStencilTestEnable(m_command_list.vk_command_buffer, depth_stencil_state->is_enable_stencil_test);
        ::pfn_vkCmdSetStencilCompareMask(m_command_list.vk_command_buffer, VK_STENCIL_FACE_FRONT_AND_BACK, depth_stencil_state->stencil_value_mask);
        ::pfn_vkCmdSetStencilWriteMask(m_command_list.vk_command_buffer, VK_STENCIL_FACE_FRONT_AND_BACK, depth_stencil_state->stencil_write_mask);

        ::pfn_vkCmdSetStencilOp(m_command_list.vk_command_buffer,VK_STENCIL_FACE_FRONT_BIT, static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_fail_op), static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_depth_pass_op), static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_depth_fail_op), static_cast<VkCompareOp>(depth_stencil_state->front_face_stencil_comparison_function));
        ::pfn_vkCmdSetStencilReference(m_command_list.vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->front_face_stencil_reference);
        ::pfn_vkCmdSetStencilOp(m_command_list.vk_command_buffer,VK_STENCIL_FACE_BACK_BIT, static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_fail_op), static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_depth_pass_op), static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_depth_fail_op), static_cast<VkCompareOp>(depth_stencil_state->back_face_stencil_comparison_function));
        ::pfn_vkCmdSetStencilReference(m_command_list.vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->back_face_stencil_reference);
    }

    void CommandBuffer::SetRasterizerState(RasterizerState *rasterizer_state) {

        ::pfn_vkCmdSetRasterizerDiscardEnable(m_command_list.vk_command_buffer, rasterizer_state->is_enable_rasterizer_discard);

        ::pfn_vkCmdSetConservativeRasterizationModeEXT(m_command_list.vk_command_buffer, static_cast<VkConservativeRasterizationModeEXT>(rasterizer_state->is_enable_conservative_rasterizer));
        ::pfn_vkCmdSetExtraPrimitiveOverestimationSizeEXT(m_command_list.vk_command_buffer, 0.0f);

        ::pfn_vkCmdSetSampleMaskEXT(m_command_list.vk_command_buffer, static_cast<VkSampleCountFlagBits>(rasterizer_state->sample_count), std::addressof(rasterizer_state->sample_mask));

        ::pfn_vkCmdSetAlphaToCoverageEnableEXT(m_command_list.vk_command_buffer, rasterizer_state->is_enable_alpha_to_coverage);
        ::pfn_vkCmdSetDepthClampEnableEXT(m_command_list.vk_command_buffer, rasterizer_state->is_enable_depth_clamp);

        ::pfn_vkCmdSetPolygonModeEXT(m_command_list.vk_command_buffer, vp::res::GfxFillModeToVkPolygonMode(static_cast<FillMode>(rasterizer_state->fill_mode)));
        ::pfn_vkCmdSetCullMode(m_command_list.vk_command_buffer, vp::res::GfxCullModeToVkCullModeFlags(static_cast<CullMode>(rasterizer_state->cull_mode)));
        ::pfn_vkCmdSetFrontFace(m_command_list.vk_command_buffer, vp::res::GfxFrontFaceToVkFrontFace(static_cast<FrontFace>(rasterizer_state->front_face)));

        ::pfn_vkCmdSetDepthBiasEnable(m_command_list.vk_command_buffer, true);
        ::pfn_vkCmdSetDepthBias(m_command_list.vk_command_buffer, rasterizer_state->polygon_offset_constant_factor, rasterizer_state->polygon_offset_clamp, rasterizer_state->polygon_offset_slope_factor);
    }

    void CommandBuffer::SetVertexState(VertexState *vertex_state) {

        VkVertexInputBindingDescription2EXT   vertex_binding_description_array[cTargetMaxVertexBufferCount] = {};
        VkVertexInputAttributeDescription2EXT vertex_attribute_description_array[cTargetMaxVertexBufferCount] = {};
        for (u32 i = 0; i < vertex_state->vertex_buffer_state_count; ++i) {
            vertex_binding_description_array[i].sType     = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
            vertex_binding_description_array[i].binding   = vertex_state->vertex_attribute_state_array[i].binding;
            vertex_binding_description_array[i].stride    = vertex_state->vertex_buffer_state_array[i].stride;
            vertex_binding_description_array[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            vertex_binding_description_array[i].divisor   = vertex_state->vertex_buffer_state_array[i].divisor;
        }
        for (u32 i = 0; i < vertex_state->vertex_attribute_state_count; ++i) {
            vertex_attribute_description_array[i].sType    = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
            vertex_attribute_description_array[i].location = vertex_state->vertex_attribute_state_array[i].interface_slot;
            vertex_attribute_description_array[i].binding  = vertex_state->vertex_attribute_state_array[i].binding;
            vertex_attribute_description_array[i].location = vp::res::GfxAttributeFormatToVkFormat(static_cast<AttributeFormat>(vertex_state->vertex_attribute_state_array[i].attribute_format));
            vertex_attribute_description_array[i].location = vertex_state->vertex_attribute_state_array[i].offset;
        }
        ::pfn_vkCmdSetVertexInputEXT(m_command_list.vk_command_buffer, vertex_state->vertex_buffer_state_count, vertex_binding_description_array, vertex_state->vertex_attribute_state_count, vertex_attribute_description_array);
    }

    void CommandBuffer::SetViewportScissorState(ViewportScissorState *viewport_scissor_state) {

        ::pfn_vkCmdSetViewportWithCount(m_command_list.vk_command_buffer, viewport_scissor_state->viewport_scissor_count, reinterpret_cast<VkViewport*>(viewport_scissor_state->viewport_state_array));
        if (viewport_scissor_state->is_enable_scissor) {
            ::pfn_vkCmdSetScissorWithCount(m_command_list.vk_command_buffer, viewport_scissor_state->viewport_scissor_count, reinterpret_cast<VkRect2D*>(viewport_scissor_state->scissor_state_array));
            return;
        }

        VkRect2D scissor_array[cTargetMaxViewportScissorCount] = {};
        for (u32 i = 0; i < viewport_scissor_state->viewport_scissor_count; ++i) {
            scissor_array[i].offset.x      = 0;
            scissor_array[i].offset.y      = 0;
            scissor_array[i].extent.width  = 0x7fff'ffff;
            scissor_array[i].extent.height = 0x7fff'ffff;
        }
        ::pfn_vkCmdSetScissorWithCount(m_command_list.vk_command_buffer, viewport_scissor_state->viewport_scissor_count, scissor_array);

        return;
    }

    void CommandBuffer::SetLineWidth(float line_width) {
        ::pfn_vkCmdSetLineWidth(m_command_list.vk_command_buffer, line_width);
    }

    void CommandBuffer::Begin() {
        //this->ResetDescriptorMemory();

        /* Reset descriptor memory commit sizes */
        for (u32 i = 0; i < vp::res::cMaxBnshShaderStageCount; ++i) {
            m_ts_i_st_ss_commit_size_array[i] = -1;
            m_ubo_ssbo_commit_size_array[i] = -1;
        }

        /* Allocate a command buffer */
        m_command_list = impl::CommandPoolManager::GetInstance()->CreateCommandList(m_queue_type, m_command_list.pool_id, m_is_primary);

        /* Begin command buffer */
        const VkCommandBufferInheritanceInfo inheritance_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        };
        const VkCommandBufferBeginInfo begin_info = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = (m_is_primary) ? nullptr : std::addressof(inheritance_info),
        };
        const u32 result = ::pfn_vkBeginCommandBuffer(m_command_list.vk_command_buffer, std::addressof(begin_info));
        VP_ASSERT(result == VK_SUCCESS);

        return;
    }

    CommandList CommandBuffer::End() {

        /* End rendering if necessary */
        this->EndRenderingIfRendering();

        /* End command buffer */
        const u32 result = ::pfn_vkEndCommandBuffer(m_command_list.vk_command_buffer);
        VP_ASSERT(result == VK_SUCCESS);
        const CommandList out_list = m_command_list;
        m_command_list = gfx::cInvalidCommandList;

        return out_list;
    }
}

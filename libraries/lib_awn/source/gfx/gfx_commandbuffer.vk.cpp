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

    Result CommandBuffer::Initialize(const CommandBufferInfo *info) {
        this->Finalize();

        /* Allocate command pool */
        RESULT_TRY(impl::CommandPoolManager::GetInstance()->CreateCommandPool(std::addressof(m_pool_id), info->queue_type));
        m_queue_type = info->queue_type;

        //m_staging_memory = staging_memory;
        //fail_guard.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    void CommandBuffer::Finalize() {
        if (m_command_list.IsValid()) {
            gfx::FinalizeCommandList(m_command_list);
            m_command_list = cInvalidCommandList;
        }
        if (m_pool_id != impl::cInvalidCommandPoolId) {
            impl::CommandPoolManager::GetInstance()->FreeCommandPool(m_pool_id);
            m_pool_id = impl::cInvalidCommandPoolId;
        }
    }

    /* Recording command api */
    Result CommandBuffer::Begin(const CommandBufferBeginInfo *info) {

        /* Create command buffer */
        if (m_command_list.IsValid()) {
            gfx::FinalizeCommandList(m_command_list);
            m_command_list = cInvalidCommandList;
        }
        RESULT_TRY(impl::CommandPoolManager::GetInstance()->CreateCommandBuffer(std::addressof(m_command_list.vk_command_buffer), m_pool_id, info->is_primary));
        m_command_list.pool_id = m_pool_id;
        auto fail_guard = SCOPE_GUARD {
            if (m_command_list.IsValid()) {
                gfx::FinalizeCommandList(m_command_list);
                m_command_list = cInvalidCommandList;
            }
        };

        /* Begin command buffer */
        const void *prev_begin = nullptr;
        impl::DescriptorMemoryManager *desc_memory_mgr = impl::DescriptorMemoryManager::GetInstance();
        const VkCommandBufferInheritanceConditionalRenderingInfoEXT  cond_render_inheritance = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT,
            .pNext = prev_begin,
            .conditionalRenderingEnable = VK_TRUE,
        };
        prev_begin = std::addressof(cond_render_inheritance);
        const VkCommandBufferInheritanceDescriptorHeapInfoEXT desc_heap_inheritance = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_DESCRIPTOR_HEAP_INFO_EXT,
            .pNext = prev_begin,
            .pSamplerHeapBindInfo = desc_memory_mgr->GetVkBindHeapInfoSampler(),
            .pResourceHeapBindInfo = desc_memory_mgr->GetVkBindHeapInfoResource(),
        };
        prev_begin = std::addressof(desc_heap_inheritance);
        const VkCommandBufferInheritanceInfo inheritance_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
            .pNext = prev_begin,
        };
        const VkCommandBufferBeginInfo begin_info = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = (info->is_primary) ? nullptr : std::addressof(inheritance_info),
        };
        const VkResult vk_result = ::pfn_vkBeginCommandBuffer(this->GetVkCommandBuffer(), std::addressof(begin_info));
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));

        if (info->is_primary) {            
            this->SetSystemDescriptorBuffers();
        }

        fail_guard.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    Result CommandBuffer::End(CommandList *out_cmd_list) {

        /* End command buffer */
        const VkResult vk_result = ::pfn_vkEndCommandBuffer(this->GetVkCommandBuffer());
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));

        /* Set command list */
        *out_cmd_list = m_command_list;
        m_command_list = cInvalidCommandList;

        RESULT_RETURN_SUCCESS;
    }

    void CommandBuffer::CallCommands(CommandList secondary_command_list) {
        VkCommandBuffer cmd_buf = secondary_command_list.GetVkCommandBuffer();
        ::pfn_vkCmdExecuteCommands(this->GetVkCommandBuffer(), 1, std::addressof(cmd_buf));
    }

    /* Memory command api */
    void CommandBuffer::FlushMemory(GpuAccessFlags access_flags) {
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
        ::pfn_vkCmdPipelineBarrier2(this->GetVkCommandBuffer(), std::addressof(dep_info));
    }

    void CommandBuffer::InvalidateMemory(GpuAccessFlags access_flags) {
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
        ::pfn_vkCmdPipelineBarrier2(this->GetVkCommandBuffer(), std::addressof(dep_info));
    }

    /* Transfer command api */
    void CommandBuffer::BlitImage(Texture *source_texture, const TextureCopyRegion *source_copy_region, Texture *dest_texture, const TextureCopyRegion *dest_copy_region, CopyFilter copy_filter) {

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
        ::pfn_vkCmdBlitImage2(this->GetVkCommandBuffer(), std::addressof(blit_image_2));
    }

    void CommandBuffer::CopyImage(Texture *source_texture, const TextureCopyRegion *source_copy_region, Texture *dest_texture, const TextureSubresource *dest_subresource, const Int3 &dest_base_offset) {

        const VkImageCopy2 image_copy = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
            .srcSubresource = {
                .aspectMask     = source_texture->GetVkAspectMask(),
                .mipLevel       = source_copy_region->base_mip_level,
                .baseArrayLayer = source_copy_region->base_array_layer,
                .layerCount     = source_copy_region->layer_count,
            },
            .srcOffset = {
                .x = static_cast<s32>(source_copy_region->base_x),
                .y = static_cast<s32>(source_copy_region->base_y),
                .z = static_cast<s32>(source_copy_region->base_z),
            },
            .dstSubresource = {
                .aspectMask     = dest_texture->GetVkAspectMask(),
                .mipLevel       = dest_subresource->base_mip_level,
                .baseArrayLayer = dest_subresource->base_array_layer,
                .layerCount     = source_copy_region->layer_count,
            },
            .dstOffset = {
                .x = static_cast<s32>(dest_base_offset[0]),
                .y = static_cast<s32>(dest_base_offset[1]),
                .z = static_cast<s32>(dest_base_offset[2]),
            },
            .extent = {
                .width = source_copy_region->width,
                .height = source_copy_region->height,
                .depth = source_copy_region->depth,
            },
        };
        const VkCopyImageInfo2 copy_image = {
            .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
            .srcImage = source_texture->GetVkImage(),
            .srcImageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .dstImage = dest_texture->GetVkImage(),
            .dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .regionCount = 1,
            .pRegions = std::addressof(image_copy),
        };
        ::pfn_vkCmdCopyImage2(this->GetVkCommandBuffer(), std::addressof(copy_image));
    }

    void CommandBuffer::CopyMemory(GpuAddress source_address, GpuAddress dest_address, size_t size) {

        const VkDeviceMemoryCopyKHR copy = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_COPY_KHR,
            .srcRange = {
                .address = source_address.GetVkDeviceAddress(),
                .size = size,
            },
            .srcFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .dstRange = {
                .address = dest_address.GetVkDeviceAddress(),
                .size = size,
            },
            .dstFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
        };
        const VkCopyDeviceMemoryInfoKHR copy_device_memory = {
            .sType = VK_STRUCTURE_TYPE_COPY_DEVICE_MEMORY_INFO_KHR,
            .regionCount = 1,
            .pRegions = std::addressof(copy),
        };
        ::pfn_vkCmdCopyMemoryKHR(this->GetVkCommandBuffer(), std::addressof(copy_device_memory));
    }

    void CommandBuffer::CopyMemoryToImage(GpuAddress source_address, size_t src_size, Texture *dest_texture, const BufferTextureCopyRegion *copy_region) {

        const VkDeviceMemoryImageCopyKHR memory_image_copy = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_IMAGE_COPY_KHR,
            .addressRange = {
                .address = source_address.GetVkDeviceAddress(),
                .size = src_size,
            },
            .addressFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .addressRowLength = copy_region->row_length,
            .addressImageHeight = copy_region->image_height,
            .imageSubresource = {
                .aspectMask = dest_texture->GetVkAspectMask(),
                .mipLevel = copy_region->copy_region.base_mip_level,
                .baseArrayLayer = copy_region->copy_region.base_array_layer,
                .layerCount = copy_region->copy_region.layer_count,
            },
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .imageOffset = {
                .x = static_cast<s32>(copy_region->copy_region.base_x),
                .y = static_cast<s32>(copy_region->copy_region.base_y),
                .z = static_cast<s32>(copy_region->copy_region.base_z),
            },
            .imageExtent = {
                .width = copy_region->copy_region.width,
                .height = copy_region->copy_region.height,
                .depth = copy_region->copy_region.depth,
            },
        };
        const VkCopyDeviceMemoryImageInfoKHR copy_device_memory_image = {
            .sType = VK_STRUCTURE_TYPE_COPY_DEVICE_MEMORY_IMAGE_INFO_KHR,
            .image = dest_texture->GetVkImage(),
            .regionCount = 1,
            .pRegions = std::addressof(memory_image_copy),
        };
        ::pfn_vkCmdCopyMemoryToImageKHR(this->GetVkCommandBuffer(), std::addressof(copy_device_memory_image));
    }

    void CommandBuffer::CopyImageToMemory(Texture *source_texture, GpuAddress dest_address,  size_t dst_size, const BufferTextureCopyRegion *copy_region) {

        //this->EndRenderingIfRendering();

        const VkDeviceMemoryImageCopyKHR memory_image_copy = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_IMAGE_COPY_KHR,
            .addressRange = {
                .address = dest_address.GetVkDeviceAddress(),
                .size = dst_size,
            },
            .addressFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .addressRowLength = copy_region->row_length,
            .addressImageHeight = copy_region->image_height,
            .imageSubresource = {
                .aspectMask = source_texture->GetVkAspectMask(),
                .mipLevel = copy_region->copy_region.base_mip_level,
                .baseArrayLayer = copy_region->copy_region.base_array_layer,
                .layerCount = copy_region->copy_region.layer_count,
            },
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .imageOffset = {
                .x = static_cast<s32>(copy_region->copy_region.base_x),
                .y = static_cast<s32>(copy_region->copy_region.base_y),
                .z = static_cast<s32>(copy_region->copy_region.base_z),
            },
            .imageExtent = {
                .width = copy_region->copy_region.width,
                .height = copy_region->copy_region.height,
                .depth = copy_region->copy_region.depth,
            },
        };
        const VkCopyDeviceMemoryImageInfoKHR copy_device_memory_image = {
            .sType = VK_STRUCTURE_TYPE_COPY_DEVICE_MEMORY_IMAGE_INFO_KHR,
            .image = source_texture->GetVkImage(),
            .regionCount = 1,
            .pRegions = std::addressof(memory_image_copy),
        };
        ::pfn_vkCmdCopyImageToMemoryKHR(this->GetVkCommandBuffer(), std::addressof(copy_device_memory_image));
    }

    void CommandBuffer::ResolveImage(Texture *source_texture, Texture *dest_texture) {
        //this->EndRenderingIfRendering();

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
        ::pfn_vkCmdResolveImage2(this->GetVkCommandBuffer(), std::addressof(resolve_image_2_info));
    }

    /* Pipeline state command api private */
    void CommandBuffer::EndRenderingImpl(const BeginRenderingInfo *info, bool is_unbind) {
        VP_UNUSED(info, is_unbind);
        ::pfn_vkCmdEndRendering(this->GetVkCommandBuffer());
        return;
    }
    void CommandBuffer::BeginRenderingImpl(const BeginRenderingInfo *info, bool is_rebind) {
        /* Get main render targets */
        [[indeterminate]] VkImageView color_view[cMaxRenderTargetColorCount];
        [[indeterminate]] VkImageView ds_view;
        const u32 color_count = info->render_buffer->GetVkImageViewArray(color_view, std::addressof(ds_view));
        /* Get MSAA render targets */
        [[indeterminate]] VkImageView r_color_view[cMaxRenderTargetColorCount];
        [[indeterminate]] VkImageView r_ds_view;
        const bool is_msaa = false; //(gfx::IsOptionalFeature(OptionalFeature::MSAA) && info->resolve_render_buffer != nullptr);
        //if (is_msaa) {
        //    info->resolve_render_buffer->GetVkImageViewArray(r_color_view, std::addressof(r_ds_view));
        //}
        /* Get op masks */
        const u32 load_discard_mask = info->op_mask.GetBits(cLoadDiscardBase, cMaxRenderTargetColorCount);
        const u32 load_clear_mask = (!is_rebind) ? info->op_mask.GetBits(cLoadClearBase, cMaxRenderTargetColorCount) : 0;
        const u32 store_discard_mask = info->op_mask.GetBits(cStoreDiscardBase, cMaxRenderTargetColorCount);
        /* Build color attachment info */
        [[indeterminate]] VkRenderingAttachmentInfo color_attachment_info[cMaxRenderTargetColorCount];
        for (u32 i = 0; i < color_count; ++i) {
            void *pnext = nullptr;
            color_attachment_info[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            color_attachment_info[i].pNext = pnext;
            color_attachment_info[i].imageView = color_view[i];
            color_attachment_info[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            color_attachment_info[i].resolveMode = (is_msaa && r_color_view[i] != VK_NULL_HANDLE) ? VK_RESOLVE_MODE_SAMPLE_ZERO_BIT : VK_RESOLVE_MODE_NONE;
            color_attachment_info[i].resolveImageView = (is_msaa && r_color_view[i] != VK_NULL_HANDLE) ? r_color_view[i] : VK_NULL_HANDLE;
            color_attachment_info[i].resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
            if ((load_discard_mask >> i) & 1) {                
                color_attachment_info[i].loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
            } else if ((load_clear_mask >> i) & 1) {
                color_attachment_info[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            } else {
                color_attachment_info[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            }
            if ((store_discard_mask >> i) & 1) {                
                color_attachment_info[i].storeOp = VK_ATTACHMENT_STORE_OP_NONE;
            } else {
                color_attachment_info[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            }
            color_attachment_info[i].clearValue = VkClearValue{};
        }
        /* Build depth stencil attachment info */
        bool is_depth = false;
        bool is_stencil = false;
        [[indeterminate]] VkRenderingAttachmentInfo depth_stencil_attachment_info;
        if (ds_view != VK_NULL_HANDLE) {
            void *pnext = nullptr;
            depth_stencil_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depth_stencil_attachment_info.pNext = pnext;
            depth_stencil_attachment_info.imageView = ds_view;
            depth_stencil_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            depth_stencil_attachment_info.resolveMode = (is_msaa && r_ds_view != VK_NULL_HANDLE) ? VK_RESOLVE_MODE_SAMPLE_ZERO_BIT : VK_RESOLVE_MODE_NONE;
            depth_stencil_attachment_info.resolveImageView = (is_msaa && r_ds_view != VK_NULL_HANDLE) ? r_ds_view : VK_NULL_HANDLE;
            depth_stencil_attachment_info.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
            depth_stencil_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_stencil_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            if ((load_discard_mask >> 8u) & 1) {                
                depth_stencil_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
            } else if ((load_clear_mask >> 8u) & 1) {
                depth_stencil_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            } else {
                depth_stencil_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            }
            if ((store_discard_mask >> 8u) & 1) {                
                depth_stencil_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
            } else {
                depth_stencil_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            }
            depth_stencil_attachment_info.clearValue = VkClearValue{ .depthStencil = { .depth = 1.0f, .stencil = 0xff, }, };
            const impl::TextureViewKey *key = info->resolve_render_buffer->GetDepthStencilTextureViewKey();
            is_depth = vp::res::IsDepthFormat(key->GetImageFormat());
            is_stencil = vp::res::IsStencilFormat(key->GetImageFormat());
        }
        /* Add fragment density map if enabled and exists */
        void *pnext = nullptr;
        //[[indeterminate]] VkRenderingFragmentDensityMapAttachmentInfoEXT frag_density_attachment_info;
        //if (gfx::IsOptionalFeature(OptionalFeature::FragmentDensityMap) && info->fragment_density_map != nullptr) {
        //    frag_density_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT;
        //    frag_density_attachment_info.pNext = pnext;
        //    frag_density_attachment_info.imageView = info->fragment_density_map->GetVkImageView();
        //    frag_density_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        //    pnext = std::addressof(frag_density_attachment_info);
        //}
        /* Begin rendering */
        const VkRenderingInfo rendering_info = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = pnext,
            .flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT | VK_RENDERING_CONTENTS_INLINE_BIT_KHR,
            .renderArea = VkRect2D{
                .offset = {
                    .x = info->render_area.position[0],
                    .y = info->render_area.position[1],
                },
                .extent = {
                    .width = static_cast<u32>(info->render_area.extents[0]),
                    .height = static_cast<u32>(info->render_area.extents[1]),
                },
            },
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = color_count,
            .pColorAttachments = (0 < color_count) ? color_attachment_info : nullptr,
            .pDepthAttachment = (is_depth) ? std::addressof(depth_stencil_attachment_info) : nullptr,
            .pStencilAttachment = (is_stencil) ? std::addressof(depth_stencil_attachment_info) : nullptr,
        };
        ::pfn_vkCmdBeginRendering(this->GetVkCommandBuffer(), std::addressof(rendering_info));
        return;
    }
    
    /* Pipeline state command api */
    bool CommandBuffer::BeginRendering(const BeginRenderingInfo *info) {
        /* Fail if too many nested passes */
        if (m_render_info_stack.IsFull()) { return false; }
        /* End current pass */
        if (!m_render_info_stack.IsEmpty()) {
            const BeginRenderingInfo *c_info = this->GetLastBeginRenderingInfo();
            this->EndRenderingImpl(c_info, true);
        }
        /* Push new pass */
        BeginRenderingInfo *a_info = m_render_info_stack.Allocate(*info);
        /* Inherit default target size if a render area was not specified */
        if (a_info->render_area.extents == VectorConstant<Int2>::cZero) {
            const RenderBuffer *buf = a_info->render_buffer;
            a_info->render_area.extents = (0 < buf->color_count) ? buf->GetColorTextureViewKey(0)->texture->GetSize2d() : buf->GetDepthStencilTextureViewKey()->texture->GetSize2d();
        }
        this->BeginRenderingImpl(a_info, false);
        return true;
    }

    void CommandBuffer::EndRendering() {
        /* Do nothing for not rendering */
        if (m_render_info_stack.IsEmpty()) { return; }
        /* Pop render pass */
        const BeginRenderingInfo *c_info = this->GetLastBeginRenderingInfo();
        this->EndRenderingImpl(c_info, false);
        m_render_info_stack.RemoveBack();
        /* Restart rendering with previous targets */
        if (!m_render_info_stack.IsEmpty()) {
            const BeginRenderingInfo *p_info = this->GetLastBeginRenderingInfo();
            this->BeginRenderingImpl(p_info, true);
        }
        return;
    }

    void CommandBuffer::ClearColor(u32 target_id, vp::util::Color4f color) {
        const BeginRenderingInfo *info = this->GetLastBeginRenderingInfo();
        const impl::TextureViewKey *key = info->render_buffer->GetColorTextureViewKey(target_id);
        const VkClearAttachment attachment = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .colorAttachment = target_id,
            .clearValue = {
                .color = vp::res::Color4fToVkClearColor(color, key->GetImageFormat()),
            },
        };
        const VkClearRect rect = {
            .rect = {
                .offset = {
                    .x = info->render_area.position[0],
                    .y = info->render_area.position[1],
                },
                .extent = {
                    .width = static_cast<u32>(info->render_area.extents[0]),
                    .height = static_cast<u32>(info->render_area.extents[1]),
                },
            },
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        ::pfn_vkCmdClearAttachments(this->GetVkCommandBuffer(), 1, std::addressof(attachment), 1, std::addressof(rect));
    }

    void CommandBuffer::ClearColors(u32 target_count, const u32 *target_id_array, const vp::util::Color4f *color_array) {
        const BeginRenderingInfo *info = this->GetLastBeginRenderingInfo();
        [[indeterminate]] VkClearAttachment attachments[cMaxRenderTargetColorCount];
        for (u32 i = 0; i < target_count; ++i) {
            const impl::TextureViewKey *key = info->render_buffer->GetColorTextureViewKey(target_id_array[i]);
            attachments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            attachments[i].colorAttachment = target_id_array[i];
            attachments[i].clearValue.color = vp::res::Color4fToVkClearColor(color_array[i], key->GetImageFormat());
        }
        const VkClearRect rect = {
            .rect = {
                .offset = {
                    .x = info->render_area.position[0],
                    .y = info->render_area.position[1],
                },
                .extent = {
                    .width = static_cast<u32>(info->render_area.extents[0]),
                    .height = static_cast<u32>(info->render_area.extents[1]),
                },
            },
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        ::pfn_vkCmdClearAttachments(this->GetVkCommandBuffer(), target_count, attachments, 1, std::addressof(rect));
    }

    void CommandBuffer::ClearDepthStencil(float depth, u32 stencil) {
        const BeginRenderingInfo *info = this->GetLastBeginRenderingInfo();
        const impl::TextureViewKey *key = info->render_buffer->GetDepthStencilTextureViewKey();
        const VkClearAttachment attachment = {
            .aspectMask = vp::res::GfxTextureDepthStencilModeToVkImageAspectFlags(key->GetTextureDepthStencilMode()),
            .colorAttachment = 0,
            .clearValue = {
                .depthStencil = {
                    .depth = depth,
                    .stencil = stencil,
                },
            },
        };
        const VkClearRect rect = {
            .rect = {
                .offset = {
                    .x = info->render_area.position[0],
                    .y = info->render_area.position[1],
                },
                .extent = {
                    .width = static_cast<u32>(info->render_area.extents[0]),
                    .height = static_cast<u32>(info->render_area.extents[1]),
                },
            },
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        ::pfn_vkCmdClearAttachments(this->GetVkCommandBuffer(), 1, std::addressof(attachment), 1, std::addressof(rect));
    }

    void CommandBuffer::SetShader(const Shader *shader) {
        if (m_queue_type != QueueType::Compute) {
            static constexpr VkShaderStageFlagBits cShaderStageFlagBitsArray[8] = {
                VK_SHADER_STAGE_VERTEX_BIT,
                VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                VK_SHADER_STAGE_GEOMETRY_BIT,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                VK_SHADER_STAGE_COMPUTE_BIT,
                VK_SHADER_STAGE_TASK_BIT_EXT,
                VK_SHADER_STAGE_MESH_BIT_EXT,
            };
            u32 stage_count = 6;
            [[indeterminate]] VkShaderEXT shader_array[8];
            for (u32 i = 0; i < 6; ++i) {
                shader_array[i] = shader->GetVkShaderArray()[i];
            }
            if (gfx::IsOptionalFeature(OptionalFeature::SupportsMeshShaders)) {
                for (u32 i = 7; i < 9; ++i) {
                    shader_array[i-1] = shader->GetVkShaderArray()[i];
                }
                stage_count += 2;
                static_assert(9 <= cMaxShaderStageCount);
            }
            ::pfn_vkCmdBindShadersEXT(this->GetVkCommandBuffer(), stage_count, cShaderStageFlagBitsArray, shader_array);
        } else {
            const VkShaderEXT shader_array[1] = {
                shader->GetVkShaderArray()[5],
            };
            static constexpr VkShaderStageFlagBits cShaderStageFlagBitsArray[1] = {
                VK_SHADER_STAGE_COMPUTE_BIT,
            };
            const u32 stage_count = 1;
            ::pfn_vkCmdBindShadersEXT(this->GetVkCommandBuffer(), stage_count, cShaderStageFlagBitsArray, shader_array);
        }
        return;
    }

    void CommandBuffer::SetResourceSet(const ResourceSet *resource_set) {
        resource_set->BindImpl(this);
    }

    void CommandBuffer::SetBlendState(const BlendState *blend_state) {
        VP_ASSERT(0 < blend_state->blend_target_state_count);

        ::pfn_vkCmdSetLogicOpEnableEXT(this->GetVkCommandBuffer(), blend_state->is_enable_logic_op);
        const VkLogicOp logic_op = vp::res::GfxLogicOperationToVkLogicOp(static_cast<LogicOperation>(blend_state->logic_op));
        ::pfn_vkCmdSetLogicOpEXT(this->GetVkCommandBuffer(), logic_op);

        [[indeterminate]] VkColorBlendEquationEXT blend_equation_array[cMaxBlendTargetStateCount];
        [[indeterminate]] VkBool32                blend_enable_array[cMaxBlendTargetStateCount];
        [[indeterminate]] VkColorComponentFlags   channel_mask_array[cMaxBlendTargetStateCount];
        for (u32 i = 0; i < blend_state->blend_target_state_count; ++i) {
            blend_enable_array[i]                       = blend_state->blend_target_state_array[i].is_enable;
            blend_equation_array[i].srcColorBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_array[i].blend_factor_source_color));
            blend_equation_array[i].dstColorBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_array[i].blend_factor_destination_color));
            blend_equation_array[i].dstColorBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_array[i].blend_factor_destination_color));
            blend_equation_array[i].colorBlendOp        = vp::res::GfxBlendEquationToVkBlendOp(static_cast<BlendEquation>(blend_state->blend_target_state_array[i].blend_equation_color));
            blend_equation_array[i].srcAlphaBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_array[i].blend_factor_source_alpha));
            blend_equation_array[i].dstAlphaBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_array[i].blend_factor_destination_alpha));
            blend_equation_array[i].alphaBlendOp        = vp::res::GfxBlendEquationToVkBlendOp(static_cast<BlendEquation>(blend_state->blend_target_state_array[i].blend_equation_alpha));
            channel_mask_array[i]                       = blend_state->blend_target_state_array[i].channel_mask;
        }
        ::pfn_vkCmdSetColorBlendEnableEXT(this->GetVkCommandBuffer(), 0, blend_state->blend_target_state_count, blend_enable_array);
        ::pfn_vkCmdSetColorBlendEquationEXT(this->GetVkCommandBuffer(), 0, blend_state->blend_target_state_count, blend_equation_array);
        ::pfn_vkCmdSetColorWriteMaskEXT(this->GetVkCommandBuffer(), 0, blend_state->blend_target_state_count, channel_mask_array);

        ::pfn_vkCmdSetBlendConstants(this->GetVkCommandBuffer(), std::addressof(blend_state->blend_constant_color[0]));

        return;
    }

    void CommandBuffer::SetDepthStencilState(const DepthStencilState *depth_stencil_state) {

        ::pfn_vkCmdSetDepthTestEnable(this->GetVkCommandBuffer(), depth_stencil_state->is_enable_depth_test);
        ::pfn_vkCmdSetDepthWriteEnable(this->GetVkCommandBuffer(), depth_stencil_state->is_enable_depth_write);

        const VkCompareOp depth_op = static_cast<VkCompareOp>(depth_stencil_state->depth_comparison_function);
        ::pfn_vkCmdSetDepthCompareOp(this->GetVkCommandBuffer(), depth_op);

        ::pfn_vkCmdSetDepthBoundsTestEnable(this->GetVkCommandBuffer(), false);

        ::pfn_vkCmdSetStencilTestEnable(this->GetVkCommandBuffer(), depth_stencil_state->is_enable_stencil_test);
        ::pfn_vkCmdSetStencilCompareMask(this->GetVkCommandBuffer(), VK_STENCIL_FACE_FRONT_AND_BACK, depth_stencil_state->stencil_value_mask);
        ::pfn_vkCmdSetStencilWriteMask(this->GetVkCommandBuffer(), VK_STENCIL_FACE_FRONT_AND_BACK, depth_stencil_state->stencil_write_mask);

        const VkStencilOp fail_op_front = static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_fail_op);
        const VkStencilOp depth_pass_op_front = static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_depth_pass_op);
        const VkStencilOp depth_fail_op_front = static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_depth_fail_op);
        const VkCompareOp compare_op_front = static_cast<VkCompareOp>(depth_stencil_state->front_face_stencil_comparison_function);
        ::pfn_vkCmdSetStencilOp(this->GetVkCommandBuffer(),VK_STENCIL_FACE_FRONT_BIT, fail_op_front, depth_pass_op_front, depth_fail_op_front, compare_op_front);
        ::pfn_vkCmdSetStencilReference(this->GetVkCommandBuffer(), VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->front_face_stencil_reference);
        const VkStencilOp fail_op_back = static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_fail_op);
        const VkStencilOp depth_pass_op_back = static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_depth_pass_op);
        const VkStencilOp depth_fail_op_back = static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_depth_fail_op);
        const VkCompareOp compare_op_back = static_cast<VkCompareOp>(depth_stencil_state->back_face_stencil_comparison_function);
        ::pfn_vkCmdSetStencilOp(this->GetVkCommandBuffer(),VK_STENCIL_FACE_BACK_BIT, fail_op_back, depth_pass_op_back, depth_fail_op_back, compare_op_back);
        ::pfn_vkCmdSetStencilReference(this->GetVkCommandBuffer(), VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->back_face_stencil_reference);

        return;
    }

    void CommandBuffer::SetRasterizerState(const RasterizerState *rasterizer_state) {

        ::pfn_vkCmdSetRasterizerDiscardEnable(this->GetVkCommandBuffer(), rasterizer_state->is_enable_rasterizer_discard);

        const VkConservativeRasterizationModeEXT rasterization_mode = static_cast<VkConservativeRasterizationModeEXT>(rasterizer_state->is_enable_conservative_rasterizer);
        ::pfn_vkCmdSetConservativeRasterizationModeEXT(this->GetVkCommandBuffer(), rasterization_mode);
        ::pfn_vkCmdSetExtraPrimitiveOverestimationSizeEXT(this->GetVkCommandBuffer(), 0.0f);

        const VkSampleCountFlagBits sample_count_flags = static_cast<VkSampleCountFlagBits>(rasterizer_state->sample_count);
        ::pfn_vkCmdSetRasterizationSamplesEXT(this->GetVkCommandBuffer(), sample_count_flags);
        ::pfn_vkCmdSetSampleMaskEXT(this->GetVkCommandBuffer(), sample_count_flags, std::addressof(rasterizer_state->sample_mask));

        ::pfn_vkCmdSetAlphaToCoverageEnableEXT(this->GetVkCommandBuffer(), rasterizer_state->is_enable_alpha_to_coverage);
        ::pfn_vkCmdSetDepthClampEnableEXT(this->GetVkCommandBuffer(), rasterizer_state->is_enable_depth_clamp);

        const VkPolygonMode polygon_mode = vp::res::GfxFillModeToVkPolygonMode(static_cast<FillMode>(rasterizer_state->fill_mode));
        const VkCullModeFlags cull_mode = vp::res::GfxCullModeToVkCullModeFlags(static_cast<CullMode>(rasterizer_state->cull_mode));
        const VkFrontFace front_face = vp::res::GfxFrontFaceToVkFrontFace(static_cast<FrontFace>(rasterizer_state->front_face));
        ::pfn_vkCmdSetPolygonModeEXT(this->GetVkCommandBuffer(), polygon_mode);
        ::pfn_vkCmdSetCullMode(this->GetVkCommandBuffer(), cull_mode);
        ::pfn_vkCmdSetFrontFace(this->GetVkCommandBuffer(), front_face);

        ::pfn_vkCmdSetDepthBiasEnable(this->GetVkCommandBuffer(), true);
        ::pfn_vkCmdSetDepthBias(this->GetVkCommandBuffer(), rasterizer_state->polygon_offset_constant_factor, rasterizer_state->polygon_offset_clamp, rasterizer_state->polygon_offset_slope_factor);

        return;
    }

    void CommandBuffer::SetViewportState(const ViewportState *viewport_state) {
        [[indeterminate]] VkViewport viewport;
        [[indeterminate]] VkRect2D scissor;
        viewport.x = viewport_state->position[0];
        viewport.y = viewport_state->extents[1] - viewport_state->position[1];
        viewport.width = viewport_state->extents[0];
        viewport.height = -viewport_state->extents[1];
        viewport.maxDepth = viewport_state->depth_range[0];
        viewport.minDepth = viewport_state->depth_range[1];
        scissor.offset.x      = 0;
        scissor.offset.y      = 0;
        scissor.extent.width  = 0x7fff'ffff;
        scissor.extent.height = 0x7fff'ffff;
        ::pfn_vkCmdSetViewportWithCount(this->GetVkCommandBuffer(), 1, std::addressof(viewport));
        ::pfn_vkCmdSetScissorWithCount(this->GetVkCommandBuffer(), 1, std::addressof(scissor));
        return;
    }
    void CommandBuffer::SetViewportScissorState(const ViewportScissorState *viewport_scissor_state) {
        VP_ASSERT(0 < viewport_scissor_state->state_count);

        /* Transform viewport state to represent a lower left window origin */
        [[indeterminate]] VkViewport viewport_array[cMaxViewportScissorCount];
        [[indeterminate]] VkRect2D scissor_array[cMaxViewportScissorCount];
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
        ::pfn_vkCmdSetViewportWithCount(this->GetVkCommandBuffer(), viewport_scissor_state->state_count, viewport_array);
        ::pfn_vkCmdSetScissorWithCount(this->GetVkCommandBuffer(), viewport_scissor_state->state_count, scissor_array);

        return;
    }

    void CommandBuffer::SetVertexState(const VertexState *vertex_state) {
        [[indeterminate]] VkVertexInputBindingDescription2EXT vertex_binding_description_array[cMaxVertexBufferCount];
        [[indeterminate]] VkVertexInputAttributeDescription2EXT vertex_attribute_description_array[cMaxVertexBufferCount];
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
            vertex_attribute_description_array[i].pNext    = nullptr;
            vertex_attribute_description_array[i].location = vertex_state->vertex_attribute_state_array[i].interface_slot;
            vertex_attribute_description_array[i].binding  = vertex_state->vertex_attribute_state_array[i].binding;
            vertex_attribute_description_array[i].format   = vp::res::GfxAttributeFormatToVkFormat(static_cast<AttributeFormat>(vertex_state->vertex_attribute_state_array[i].attribute_format));
            vertex_attribute_description_array[i].offset   = vertex_state->vertex_attribute_state_array[i].offset;
        }
        ::pfn_vkCmdSetVertexInputEXT(this->GetVkCommandBuffer(), vertex_state->vertex_buffer_state_count, vertex_binding_description_array, vertex_state->vertex_attribute_state_count, vertex_attribute_description_array);
        ::pfn_vkCmdSetPrimitiveRestartEnable(this->GetVkCommandBuffer(), false);
    }

    
    void CommandBuffer::SetVertexBuffer(u32 binding, GpuAddress vertex_buffer_address, size_t size) {
        [[indeterminate]] VkBindVertexBuffer3InfoKHR bind_info;
        bind_info.sType = VK_STRUCTURE_TYPE_BIND_VERTEX_BUFFER_3_INFO_KHR;
        bind_info.pNext = nullptr;
        bind_info.setStride = VK_FALSE;
        bind_info.addressRange.address = vertex_buffer_address.GetVkDeviceAddress();
        bind_info.addressRange.size = size;
        bind_info.addressRange.stride = 0;
        bind_info.addressFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;
        ::pfn_vkCmdBindVertexBuffers3KHR(this->GetVkCommandBuffer(), binding, 1, std::addressof(bind_info));
    }

    void CommandBuffer::SetVertexBuffers(u32 base_binding, const GpuAddressRange *gpu_address_range_array, u32 count) {
        VP_ASSERT(count <= 0x20);
        [[indeterminate]] VkBindVertexBuffer3InfoKHR bind_info_array[0x20];
        for (u32 i = 0; i < count; ++i) {
            bind_info_array[i].sType = VK_STRUCTURE_TYPE_BIND_VERTEX_BUFFER_3_INFO_KHR;
            bind_info_array[i].pNext = nullptr;
            bind_info_array[i].setStride = VK_FALSE;
            bind_info_array[i].addressRange.address = gpu_address_range_array[i].gpu_address.GetVkDeviceAddress();
            bind_info_array[i].addressRange.size = gpu_address_range_array[i].size;
            bind_info_array[i].addressRange.stride = 0;
            bind_info_array[i].addressFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;
        }
        ::pfn_vkCmdBindVertexBuffers3KHR(this->GetVkCommandBuffer(), base_binding, count, bind_info_array);
    }

    void CommandBuffer::BeginRenderEnableCondition(ConditionalRenderMode mode, GpuAddress value_address) {
        const VkConditionalRenderingBeginInfo2EXT info = {
            .sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_2_EXT,
            .addressRange = {
                .address = value_address.GetVkDeviceAddress(),
                .size = sizeof(ConditionalRenderData),
            },
            .addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .flags = (mode == ConditionalRenderMode::IsZero) ? static_cast<VkConditionalRenderingFlagsEXT>(VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT) : 0,
        };
        ::pfn_vkCmdBeginConditionalRendering2EXT(this->GetVkCommandBuffer(), std::addressof(info));
    }

    void CommandBuffer::EndRenderEnableCondition() {
        ::pfn_vkCmdEndConditionalRenderingEXT(this->GetVkCommandBuffer());
    }

    void CommandBuffer::SetLineWidth(float line_width) {
        ::pfn_vkCmdSetLineWidth(this->GetVkCommandBuffer(), line_width);
    }

    /* Draw command api private */
    void CommandBuffer::SetPrimitiveTopologyImpl(PrimitiveTopology topology) {
        const VkPrimitiveTopology primitive_topology = vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(topology);
        ::pfn_vkCmdSetPrimitiveTopology(this->GetVkCommandBuffer(), primitive_topology);
    }

    void CommandBuffer::SetIndexBufferImpl(IndexFormat index_format, GpuAddress index_buffer_address, u32 max_index_count) {
        /* Calc range by index count */
        const VkIndexType index_type = vp::res::GfxIndexFormatToVkIndexType(index_format);
        const u32 index_size0 = (index_format == IndexFormat::U32) ? sizeof(u32) : sizeof(u16);
        const u32 index_size1 = (index_format == IndexFormat::U16) ? index_size0 : sizeof(u8);
        VP_ASSERT(vp::util::IsAligned(index_buffer_address.GetVkDeviceAddress(), index_size1));
        /* Bind index buffer */
        const VkBindIndexBuffer3InfoKHR bind_info = {
            .sType = VK_STRUCTURE_TYPE_BIND_INDEX_BUFFER_3_INFO_KHR,
            .pNext = nullptr,
            .addressRange = {
                .address = index_buffer_address.GetVkDeviceAddress(),
                .size = max_index_count * index_size1,
            },
            .addressFlags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .indexType = index_type,
        };
        ::pfn_vkCmdBindIndexBuffer3KHR(this->GetVkCommandBuffer(), std::addressof(bind_info));
        return;
    }

    /* Draw command api */
    void CommandBuffer::Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count) {
        this->SetPrimitiveTopologyImpl(primitive_topology);
        ::pfn_vkCmdDraw(this->GetVkCommandBuffer(), vertex_count, 1, base_vertex, 0);
    }

    void CommandBuffer::DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count) {
        this->SetPrimitiveTopologyImpl(primitive_topology);
        ::pfn_vkCmdDraw(this->GetVkCommandBuffer(), vertex_count, instance_count, base_vertex, base_instance);
    }

    void CommandBuffer::DrawIndirect(PrimitiveTopology primitive_topology, GpuAddress indirect_address) {
        this->SetPrimitiveTopologyImpl(primitive_topology);
        const VkDrawIndirect2InfoKHR info = {
            .sType = VK_STRUCTURE_TYPE_DRAW_INDIRECT_2_INFO_KHR,
            .addressRange = {
                .address = indirect_address.GetVkDeviceAddress(),
                .size = sizeof(VkDrawIndirectCommand),
                .stride = sizeof(VkDrawIndirectCommand),
            },
            .addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .drawCount = 1,
        };
        ::pfn_vkCmdDrawIndirect2KHR(this->GetVkCommandBuffer(), std::addressof(info));
        return;
    }

    void CommandBuffer::MultiDrawIndexedIndirect(PrimitiveTopology primitive_topology, GpuAddress indirect_address, u32 max_draw_count, GpuAddress counter_address, size_t stride) {
        this->SetPrimitiveTopologyImpl(primitive_topology);
        const VkDrawIndirectCount2InfoKHR info = {
            .sType = VK_STRUCTURE_TYPE_DRAW_INDIRECT_COUNT_2_INFO_KHR,
            .addressRange = {
                .address = indirect_address.GetVkDeviceAddress(),
                .size = stride * max_draw_count,
                .stride = stride,
            },
            .addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .countAddressRange = {
                .address = counter_address.GetVkDeviceAddress(),
                .size = sizeof(u32),
            },
            .countAddressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .maxDrawCount = max_draw_count,
        };
        ::pfn_vkCmdDrawIndirectCount2KHR(this->GetVkCommandBuffer(), std::addressof(info));
        return;
    }

    void CommandBuffer::DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 index_count, u32 base_vertex) {
        this->DrawIndexedInstanced(primitive_topology, index_format, index_buffer_address, index_count, base_vertex, 0, 1);
    }

    void CommandBuffer::DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count) {
        this->SetPrimitiveTopologyImpl(primitive_topology);
        this->SetIndexBufferImpl(index_format, index_buffer_address, index_count);
        ::pfn_vkCmdDrawIndexed(this->GetVkCommandBuffer(), index_count, instance_count, 0, base_vertex, base_instance);
    }

    void CommandBuffer::DrawIndexedIndirect(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 max_index_count, GpuAddress indirect_address) {
        this->SetPrimitiveTopologyImpl(primitive_topology);
        this->SetIndexBufferImpl(index_format, index_buffer_address, max_index_count);
        const VkDrawIndirect2InfoKHR info = {
            .sType = VK_STRUCTURE_TYPE_DRAW_INDIRECT_2_INFO_KHR,
            .addressRange = {
                .address = indirect_address.GetVkDeviceAddress(),
                .size = sizeof(VkDrawIndexedIndirectCommand),
                .stride = sizeof(VkDrawIndexedIndirectCommand),
            },
            .addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .drawCount = 1,
        };
        ::pfn_vkCmdDrawIndexedIndirect2KHR(this->GetVkCommandBuffer(), std::addressof(info));
    }

    void CommandBuffer::MultiDrawIndexedIndirect(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 max_index_count, GpuAddress indirect_address, u32 max_draw_count, GpuAddress counter_address, size_t stride) {
        this->SetPrimitiveTopologyImpl(primitive_topology);
        this->SetIndexBufferImpl(index_format, index_buffer_address, max_index_count);
        const VkDrawIndirectCount2InfoKHR info = {
            .sType = VK_STRUCTURE_TYPE_DRAW_INDIRECT_COUNT_2_INFO_KHR,
            .addressRange = {
                .address = indirect_address.GetVkDeviceAddress(),
                .size = stride * max_draw_count,
                .stride = stride,
            },
            .addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .countAddressRange = {
                .address = counter_address.GetVkDeviceAddress(),
                .size = sizeof(u32),
            },
            .countAddressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .maxDrawCount = max_draw_count,
        };
        ::pfn_vkCmdDrawIndexedIndirectCount2KHR(this->GetVkCommandBuffer(), std::addressof(info));
    }

    void CommandBuffer::DrawMeshTasks(u32 x, u32 y, u32 z) {
        ::pfn_vkCmdDrawMeshTasksEXT(this->GetVkCommandBuffer(), x, y, z);
    }

    void CommandBuffer::DrawMeshTasksIndirect(GpuAddress indirect_address) {
        const VkDrawIndirect2InfoKHR info = {
            .sType = VK_STRUCTURE_TYPE_DISPATCH_INDIRECT_2_INFO_KHR,
            .addressRange = {
                .address = indirect_address.GetVkDeviceAddress(),
                .size = sizeof(VkDrawMeshTasksIndirectCommandEXT),
                .stride = sizeof(VkDrawMeshTasksIndirectCommandEXT),
            },
            .addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .drawCount = 1,
        };
        ::pfn_vkCmdDrawMeshTasksIndirect2EXT(this->GetVkCommandBuffer(), std::addressof(info));
    }

    void CommandBuffer::MultiDrawMeshTasksIndirect(GpuAddress indirect_address, u32 max_draw_count, GpuAddress counter_address, size_t stride) {
        const VkDrawIndirectCount2InfoKHR info = {
            .sType = VK_STRUCTURE_TYPE_DRAW_INDIRECT_COUNT_2_INFO_KHR,
            .addressRange = {
                .address = indirect_address.GetVkDeviceAddress(),
                .size = stride * max_draw_count,
                .stride = stride,
            },
            .addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .countAddressRange = {
                .address = counter_address.GetVkDeviceAddress(),
                .size = sizeof(u32),
            },
            .countAddressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
            .maxDrawCount = max_draw_count,
        };
        ::pfn_vkCmdDrawMeshTasksIndirectCount2EXT(this->GetVkCommandBuffer(), std::addressof(info));
    }

    /* Dispatch api */
    void CommandBuffer::Dispatch(u32 x, u32 y, u32 z) {
        ::pfn_vkCmdDispatch(this->GetVkCommandBuffer(), x, y, z);
    }

    void CommandBuffer::DispatchIndirect(GpuAddress indirect_address) {
        const VkDispatchIndirect2InfoKHR info = {
            .sType = VK_STRUCTURE_TYPE_DISPATCH_INDIRECT_2_INFO_KHR,
            .addressRange = {
                .address = indirect_address.GetVkDeviceAddress(),
                .size = sizeof(VkDispatchIndirectCommand),
            },
            .addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR,
        };
        ::pfn_vkCmdDispatchIndirect2KHR(this->GetVkCommandBuffer(), std::addressof(info));
    }
    
    /* System */
    void CommandBuffer::SetSystemDescriptorBuffers() {
        /* Set descriptor heaps */
        impl::DescriptorMemoryManager *desc_memory_mgr = impl::DescriptorMemoryManager::GetInstance();
        ::pfn_vkCmdBindResourceHeapEXT(this->GetVkCommandBuffer(), desc_memory_mgr->GetVkBindHeapInfoResource());
        ::pfn_vkCmdBindSamplerHeapEXT(this->GetVkCommandBuffer(), desc_memory_mgr->GetVkBindHeapInfoSampler());
        return;
    }
}

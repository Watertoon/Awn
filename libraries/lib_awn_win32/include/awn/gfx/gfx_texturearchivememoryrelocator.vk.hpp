#pragma once

namespace awn::gfx {

    class TextureArchiveMemoryRelocator {
        private:
            vp::res::ResBntx    *m_bntx;
            GpuMemoryAllocation  m_device_memory;
        private:
            CommandList RecordBntxTransferToGpu(GpuMemoryAllocation *staging_memory) {

                /* Begin a command buffer for transfers */
                ThreadLocalCommandBuffer transfer_cmd_buffer;
                transfer_cmd_buffer.Begin();

                /* Linearly find and transfer textures to the Gpu */
                for (u32 i = 0; i < m_bntx->texture_container.texture_count; ++i) {

                    /* Get texture offset */
                    const size_t base_offset = reinterpret_cast<uintptr_t>(m_bntx->texture_container.texture_info_array[i]->mip_offset_table[0]) - m_bntx->texture_container.texture_data->GetGpuMemoryRegionSize();

                    /* Populate texture view info */
                    const TextureViewInfo texture_view_info = {
                        image_dimension    = m_bntx->texture_container.texture_info_array[i]->image_dimension;
                        depth_stencil_mode = 0,
                        image_format       = m_bntx->texture_container.texture_info_array[i]->texture_info.image_format,
                        swizzle_x          = static_cast<vp::res::GfxChannelSource>(m_bntx->texture_container.texture_info_array[i]->channel_sources[0]),
                        swizzle_y          = static_cast<vp::res::GfxChannelSource>(m_bntx->texture_container.texture_info_array[i]->channel_sources[1]),
                        swizzle_z          = static_cast<vp::res::GfxChannelSource>(m_bntx->texture_container.texture_info_array[i]->channel_sources[2]),
                        swizzle_w          = static_cast<vp::res::GfxChannelSource>(m_bntx->texture_container.texture_info_array[i]->channel_sources[3]),
                        base_mip_level     = 0,
                        mip_levels         = m_bntx->texture_container.texture_info_array[i]->texture_info.mip_levels,
                        base_array_layer   = 0,
                        array_layers       = m_bntx->texture_container.texture_info_array[i]->texture_info.array_layers
                    };

                    /* Create a texture object */
                    const DescriptorSlot  texture_slot = TextureSamplerManager::GetInstance()->RegisterTexture(m_device_memory.GetGpuAddress(texture_offset), std::addressof(bntx->texture_container.texture_info_array[i]->texture_info), std::addressof(texture_view_info));
                    Texture              *texture      = TextureSamplerManager::GetInstance()->GetTexture(texture_slot);

                    /* Set bntx descriptor slot */
                    m_bntx->texture_container.texture_info_array[i]->user_descriptor_slot = texture_slot;

                    /* Schedule a layout transition */
                    VkImageMemoryBarrier2 image_barrer = {
                        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                        .srcStageMask     = 0,
                        .srcAccessMask    = 0,
                        .dstStageMask     = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                        .dstAccessMask    = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                        .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
                        .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
                        .image            = texture->GetVkImage(),
                        .subresourceRange = {
                            .aspectMask     = vp::res::ConvertGfxImageFormatToVkImageAspectFlags(static_cast<ImageFormat>(m_bntx->texture_container.texture_info_array[i]->texture_info.image_format)),
                            .baseMipLevel   = 0,
                            .levelCount     = m_bntx->texture_container.texture_info_array[i]->texture_info.mip_levels,
                            .baseArrayLayer = 0,
                            .layerCount     = m_bntx->texture_container.texture_info_array[i]->texture_info.array_layers,
                        }
                    };
                    VkDependencyInfo dependency_info = {
                        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                        .imageMemoryBarrierCount = 1,
                        .pImageMemoryBarriers    = std::addressof(image_barrier)
                    };
                    ::pfn_vkCmdPipelineBarrier2(transfer_cmd_buffer.GetVkCommandBuffer(), std::addressof(dependency_info));

                    /* Populate copy command for all mips */
                    VP_ASSERT(m_bntx->texture_container.texture_info_array[i]->texture_info.mip_levels <= Context::cMaxMipLevels);
                    VkBufferImageCopy image_copy[Context::cMaxMipLevels];
                    for (u32 y = 0; y < m_bntx->texture_container.texture_info_array[i]->texture_info.mip_levels; ++y) {
                        image_copy[y].bufferOffset                    = ;
                        image_copy[y].bufferRowLength                 = 0;
                        image_copy[y].bufferRowHeight                 = 0;
                        image_copy[y].imageSubresource.aspectMask     = ;
                        image_copy[y].imageSubresource.mipLevel       = i;
                        image_copy[y].imageSubresource.baseArrayLayer = 0;
                        image_copy[y].imageSubresource.arrayLayers    = m_bntx->texture_container.texture_info_array[i]->texture_info.array_layers;
                        image_copy[y].imageOffset.x                   = 0;
                        image_copy[y].imageOffset.y                   = 0;
                        image_copy[y].imageOffset.z                   = 0;
                        image_copy[y].imageExtent.width               = m_bntx->texture_container.texture_info_array[i]->texture_info.width  >> i;
                        image_copy[y].imageExtent.height              = m_bntx->texture_container.texture_info_array[i]->texture_info.height >> i;
                        image_copy[y].imageExtent.depth               = m_bntx->texture_container.texture_info_array[i]->texture_info.depth  >> i;
                    }

                    /* Schedule a copy */
                    ::pfn_vkCmdCopyBufferToImage(transfer_cmd_buffer.GetVkCommandBuffer(), vk_staging_buffer, texture->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, m_bntx->texture_container.texture_info_array[i]->texture_info.mip_levels, image_copy)
                }

                /* Return command list */
                return transfer_cmd_buffer.End();
            }
        public:
            constexpr TextureArchiveMemoryRelocator() {/*...*/}

            bool Initialize(vp::res::ResBntx *bntx) {

                /* Allocate staging and device memory */
                GpuMemoryAllocation staging_memory;
                const size_t gpu_memory_size = bntx->texture_container.texture_data->GetGpuMemoryRegionSize();
                staging_memory.Allocate(gpu_memory_size, MemoryPropertyFlags::HostUncached);
                m_device_memory.Allocate(gpu_memory_size, MemoryPropertyFlags::GpuCached);

                /* Copy gpu region to host memory */
                void *memory = staging_memory.Map();
                ::memcpy(memory, bntx->texture_container.texture_data->GetGpuMemoryRegion(), gpu_memory_size);

                /* Records transfer command list */
                CommandList command_list = this->RecordBntxTransferToGpu();

                /* Transfer the memory */
                gfx::SubmitTransferQueue(command_list);
                gfx::WaitForTransferQueue();

                /* Free command list and staging memory */
                CommandPoolManager::GetInstance()->FreeThreadLocalCommandList(command_list);
                staging_memory.FreeGpuMemory();
            }

            void Finalize() {

                /* Zero out bntx descriptor slots */
                for (u32 i = 0; i < m_bntx->texture_container.texture_count; ++i) {
                    TextureSamplerManager::ReleaseTexture(m_bntx->texture_container.texture_info_array[i]->user_descriptor_slot);
                    m_bntx->texture_container.texture_info_array[i]->user_descriptor_slot = 0;
                }

                /* Deallocate device memory */
                m_device_memory.FreeGpuMemory();
            }
    };
}

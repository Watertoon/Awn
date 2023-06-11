#pragma once

namespace awn::gfx {

    struct GpuAddress {
        VkBuffer        vk_buffer;
        VkDeviceAddress vk_device_address;
    };

    struct CommandList {
        VkCommandBuffer vk_command_buffer;
        QueueType       queue_type;
    };

    class CommandPoolManager {
        public:
            struct CommandPoolHolder {
                VkCommandPool command_pool_array[Context::cTargetMaxQueueCount];

                constexpr CommandPoolHolder() : command_pool_array{VK_NULL_HANDLE} {/*...*/}
            };
        public:
            static constexpr size_t cMaxCommandBufferThreadCount = 128;
        public:
            using CommandPoolHolderAllocator = vp::util::FixedObjectAllocator<CommandPoolHolder, cMaxCommandBufferThreadCount>;
        private:
            sys::TlsSlot               m_command_pool_tls_slot;
            CommandPoolHolderAllocator m_command_pool_holder_allocator;
        public:
            AWN_SINGLETON_TRAITS(CommandPoolManager);
        private:
            static void DestructCommandPoolHolderTls(void *arg) {

                /* Check whether the thread allocated a command pool holder */
                if (arg == nullptr) { return; }

                /* Destruct all vk command pools */
                CommandPoolHolder *holder = reinterpret_cast<CommandPoolHolder*>(arg);
                for (u32 i = 0; i < Context::cTargetMaxQueueCount; ++i) {
                    if (holder->command_pool_array[i] != VK_NULL_HANDLE) {
                        ::pfn_vkDestroyCommandPool(Context::GetInstance()->GetVkDevice(), holder->command_pool_array[i], Context::GetInstance()->GetVkAllocationCallbacks());
                        holder->command_pool_array[i] = VK_NULL_HANDLE;
                    }
                }

                /* Free command pool holder to allocator */
                CommandPoolManager::GetInstance()->m_command_pool_holder_allocator.Free(holder);

                return;
            }
        public:
            constexpr ALWAYS_INLINE CommandPoolManager() : m_command_pool_tls_slot(0) {/*...*/}
            constexpr ALWAYS_INLINE ~CommandPoolManager() {/*...*/}

            void Initialize() {
                const bool result = sys::ThreadManager::GetInstance()->AllocateTlsSlot(std::addressof(m_command_pool_tls_slot), DestructCommandPoolHolderTls, true);
                VP_ASSERT(result == true);
            }

            void Finalize() {
                sys::ThreadManager::GetInstance()->FreeTlsSlot(m_command_pool_tls_slot);
                m_command_pool_tls_slot = 0;
            }

            CommandList CreateThreadLocalCommandList(QueueType queue_type, bool is_primary = false) {

                /* Try to acquire existing command pool */
                CommandPoolHolder *command_pool_holder = reinterpret_cast<CommandPoolHolder*>(sys::ThreadManager::GetInstance()->GetCurrentThread()->GetTlsData(m_command_pool_tls_slot));

                /* Allocate new command pool holder if one doesn't exist */
                if (command_pool_holder == nullptr) {
                    command_pool_holder = m_command_pool_holder_allocator.Allocate();
                    VP_ASSERT(command_pool_holder != nullptr);
                }

                /* Get command pool */
                VkCommandPool *sel_vk_command_pool = std::addressof(command_pool_holder->command_pool_array[static_cast<u32>(queue_type)]);

                /* Allocate new commmand pool if one doesn't exist */
                if (*sel_vk_command_pool == VK_NULL_HANDLE) {
                    const VkCommandPoolCreateInfo command_pool_create_info = {
                        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                        .queueFamilyIndex = Context::GetInstance()->GetQueueFamilyIndex(queue_type)
                    };
                    const u32 result = ::pfn_vkCreateCommandPool(Context::GetInstance()->GetVkDevice(), std::addressof(command_pool_create_info), Context::GetInstance()->GetVkAllocationCallbacks(), sel_vk_command_pool);
                    VP_ASSERT(result == VK_SUCCESS);
                }

                /* Allocate new secondary command buffer */
                const VkCommandBufferAllocateInfo allocate_info {
                    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool        = *sel_vk_command_pool,
                    .level              = (is_primary == true) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
                    .commandBufferCount = 1
                };

                VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
                const u32 result = ::pfn_vkAllocateCommandBuffers(Context::GetInstance()->GetVkDevice(), std::addressof(allocate_info), std::addressof(vk_command_buffer));
                VP_ASSERT(result == VK_SUCCESS);

                return {vk_command_buffer, queue_type};
            }

            void FinalizeThreadLocalCommandList(CommandList command_list) {

                /* Acquire existing command pool */
                CommandPoolHolder *command_pool_holder = reinterpret_cast<CommandPoolHolder*>(sys::ThreadManager::GetInstance()->GetCurrentThread()->GetTlsData(m_command_pool_tls_slot));

                /* Free secondary command buffer */
                ::pfn_vkFreeCommandBuffers(Context::GetInstance()->GetVkDevice(), command_pool_holder->command_pool_array[static_cast<u32>(command_list.queue_type)], 1, std::addressof(command_list.vk_command_buffer));

                return;
            }
    };

    class CommandBufferBase {
        protected:
            CommandList m_command_list;
        private:
            struct SyncScopeInfo {
                u32 vk_src_stage_mask;
                u32 vk_src_access_mask;
                u32 vk_dst_stage_mask;
                u32 vk_dst_access_mask;
            };
        private:
            void TransitionRenderTargetsImpl(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target, SyncScopeInfo *sync_scope, VkImageLayout image_layout) {
                
                VkImageMemoryBarrier2 barrier_array[Context::cTargetMaxBoundRenderTargetColorCount + 1] = {};

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
        public:
            constexpr ALWAYS_INLINE CommandBufferBase() : m_command_list() {/*...*/}

            /* Gpu dispatch commands */
            void Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count) {
                ::pfn_vkCmdSetPrimitiveTopology(m_command_list.vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
                ::pfn_vkCmdDraw(m_command_list.vk_command_buffer, vertex_count, 0, base_vertex, 0);
            }
            void DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count) {
                ::pfn_vkCmdSetPrimitiveTopology(m_command_list.vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
                ::pfn_vkCmdDraw(m_command_list.vk_command_buffer, vertex_count, instance_count, base_vertex, base_instance);
            }
            void DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 base_index, u32 index_count, u32 base_vertex) {
                ::pfn_vkCmdSetPrimitiveTopology(m_command_list.vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
                ::pfn_vkCmdBindIndexBuffer(m_command_list.vk_command_buffer, index_buffer_address.vk_buffer, 0, vp::res::GfxIndexFormatToVkIndexType(index_format));
                ::pfn_vkCmdDrawIndexed(m_command_list.vk_command_buffer, index_count, 0, base_index, base_vertex, 0);
            }
            void DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 base_index, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count) {
                ::pfn_vkCmdSetPrimitiveTopology(m_command_list.vk_command_buffer, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
                ::pfn_vkCmdBindIndexBuffer(m_command_list.vk_command_buffer, index_buffer_address.vk_buffer, 0, vp::res::GfxIndexFormatToVkIndexType(index_format));
                ::pfn_vkCmdDrawIndexed(m_command_list.vk_command_buffer, index_count, instance_count, base_index, base_vertex, base_instance);
            }
            void DrawMeshTasks(u32 x, u32 y, u32 z) {
                ::pfn_vkCmdDrawMeshTasksEXT(m_command_list.vk_command_buffer, x, y, z);
            }
            void Dispatch(u32 x, u32 y, u32 z) {
                ::pfn_vkCmdDispatch(m_command_list.vk_command_buffer, x, y, z);
            }

            /* Resource state setter commands */
            void SetVertexBuffer(GpuAddress vertex_buffer_address, u32 binding, u32 stride, u32 size) {
                const VkDeviceSize device_offset = 0;
                const VkDeviceSize device_size   = size;
                const VkDeviceSize device_stride = stride;
                ::pfn_vkCmdBindVertexBuffers2(m_command_list.vk_command_buffer, binding, 1, std::addressof(vertex_buffer_address.vk_buffer), std::addressof(device_offset), std::addressof(device_size), std::addressof(device_stride));
            }

            void TransitionRenderTargetsToAttachment(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target) {

                SyncScopeInfo scope_info = {
                    .vk_dst_stage_mask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .vk_dst_access_mask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                };
                TransitionRenderTargetsImpl(color_target_array, color_target_count, depth_stencil_target, std::addressof(scope_info), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                return;
            }
            void TransitionRenderTargetsToPresent(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target) {

                SyncScopeInfo scope_info = {
                    .vk_src_stage_mask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .vk_src_access_mask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                };
                TransitionRenderTargetsImpl(color_target_array, color_target_count, depth_stencil_target, std::addressof(scope_info), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
                return;
            }

            void BeginRendering(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target) {

                /* Get values that should be synced across render targets */
                u32 render_width  = 0;
                u32 render_height = 0;
                u32 view_count    = 0;
                u32 view_mask     = 0;
                if (color_target_count != 0) {
                    render_width  = color_target_array[0]->GetRenderWidth();
                    render_height = color_target_array[0]->GetRenderHeight();
                    view_count    = color_target_array[0]->GetViewCount();
                    view_mask     = color_target_array[0]->GetViewMask();
                } else {
                    render_width  = depth_stencil_target->GetRenderWidth();
                    render_height = depth_stencil_target->GetRenderHeight();
                    view_count    = depth_stencil_target->GetViewCount();
                    view_mask     = depth_stencil_target->GetViewMask();
                }

                /* Setup color target attacments */
                VkRenderingAttachmentInfo color_attachment_info_array[Context::cTargetMaxBoundRenderTargetColorCount] = {};
                for (u32 i = 0; i < color_target_count; ++i) {
                    color_attachment_info_array[i].sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                    color_attachment_info_array[i].imageView   = color_target_array[i]->GetVkImageView();
                    color_attachment_info_array[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    color_attachment_info_array[i].loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                    color_attachment_info_array[i].storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                }

                /* Setup depth stencil attachments */
                VkRenderingAttachmentInfo  depth_attachment_info             = {};
                VkRenderingAttachmentInfo  stencil_attachment_info           = {};
                VkRenderingAttachmentInfo *selected_depth_attachment_info   = nullptr;
                VkRenderingAttachmentInfo *selected_stencil_attachment_info = nullptr;
                if (depth_stencil_target != nullptr) {
                    if (depth_stencil_target->IsDepthFormat() == true) {
                        depth_attachment_info.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                        depth_attachment_info.imageView   = depth_stencil_target->GetVkImageView();
                        depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        depth_attachment_info.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                        depth_attachment_info.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                        selected_depth_attachment_info    = std::addressof(depth_attachment_info);
                    }
                    if (depth_stencil_target->IsStencilFormat() == true) {
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
                            .height = render_height
                        },
                    },
                    .layerCount           = view_count,
                    .viewMask             = view_mask,
                    .colorAttachmentCount = color_target_count,
                    .pColorAttachments    = color_attachment_info_array,
                    .pDepthAttachment     = selected_depth_attachment_info,
                    .pStencilAttachment   = selected_stencil_attachment_info,
                };
                ::pfn_vkCmdBeginRendering(m_command_list.vk_command_buffer, std::addressof(rendering_info));
            }

            void EndRendering() {
                ::pfn_vkCmdEndRendering(m_command_list.vk_command_buffer);
            }

            void SetStorageBuffer(u32 location, GpuAddress storage_buffer_address) {

                /* Push 8-byte buffer address to location in push constant range 128-256 */
                ::pfn_vkCmdPushConstants(m_command_list.vk_command_buffer, Context::GetInstance()->GetVkPipelineLayout(), VK_SHADER_STAGE_ALL, location * sizeof(VkDeviceAddress), sizeof(VkDeviceAddress), std::addressof(storage_buffer_address.vk_device_address));
            }

            void SetTextureAndSampler(u32 location, DescriptorSlot texture_slot, DescriptorSlot sampler_slot) {

                /* (Not yet viable) Get texture sampler handle */
                /*const VkImageViewHandleInfoNVX image_handle_info = {
                    .sType          = VK_STRUCTURE_TYPE_IMAGE_VIEW_HANDLE_INFO_NVX,
                    .imageView      = texture_slot.texture_handle,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .sampler        = sampler_slot.sampler_handle
                };
                const u32 view_handle = ::pfn_vkGetImageViewHandleNVX(Context::GetInstance()->GetVkDevice(), std::addressof(image_handle_info));*/

                /* Push 4-byte packed 20:12 TextureIndex:SamplerIndex to location in push constant range 0-128 */
                const u32 view_handle = ((sampler_slot & 0xfff) << Context::cTargetTextureDescriptorIndexBits) | (texture_slot & 0xf'ffff);
                ::pfn_vkCmdPushConstants(m_command_list.vk_command_buffer, Context::GetInstance()->GetVkPipelineLayout(), VK_SHADER_STAGE_ALL, location * sizeof(u32), sizeof(u32), std::addressof(view_handle));
            }

            void SetShader(Shader *shader) {
                VkShaderStageFlagBits **flag_bit_array = nullptr;
                VkShaderEXT           **shader_array   = nullptr;
                const u32               stage_count    = shader->SetupForCommandList(shader_array, flag_bit_array);
                ::pfn_vkCmdBindShadersEXT(m_command_list.vk_command_buffer, stage_count, *flag_bit_array, *shader_array);
            }

            /* Fixed function state setters */
            //void SetPipelineState(PipelineState *pipeline_state) {
            //    this->SetBlendState();
            //    this->SetDepthStencilState();
            //    this->SetRasterizerState();
            //    this->SetVertexState();
            //    this->SetViewportScissorState();
            //}
            void SetBlendState(BlendState *blend_state) {

                ::pfn_vkCmdSetLogicOpEnableEXT(m_command_list.vk_command_buffer, true);
                ::pfn_vkCmdSetLogicOpEXT(m_command_list.vk_command_buffer, vp::res::GfxLogicOperationToVkLogicOp(static_cast<LogicOperation>(blend_state->logic_op)));

                /* Set VkColorBlendEquation array */
                VkColorBlendEquationEXT blend_equation_array[Context::cTargetMaxColorBlendEquationCount] = {};
                VkBool32                blend_enable_array[Context::cTargetMaxColorBlendEquationCount] = {};
                VkColorComponentFlags   channel_mask_array[Context::cTargetMaxColorBlendEquationCount] = {};
                for (u32 i = 0; i < blend_state->blend_target_state_count; ++i) {
                    blend_enable_array[i]                       = blend_state->blend_target_state_info_array[i].enable_blend;
                    blend_equation_array[i].srcColorBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_info_array[i].blend_factor_source_color));
                    blend_equation_array[i].dstColorBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_info_array[i].blend_factor_destination_color));
                    blend_equation_array[i].colorBlendOp        = vp::res::GfxBlendEquationToVkBlendOp(static_cast<BlendEquation>(blend_state->blend_target_state_info_array[i].blend_equation_color));
                    blend_equation_array[i].srcAlphaBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_info_array[i].blend_factor_source_alpha));
                    blend_equation_array[i].dstAlphaBlendFactor = vp::res::GfxBlendFactorToVkBlendFactor(static_cast<BlendFactor>(blend_state->blend_target_state_info_array[i].blend_factor_destination_alpha));
                    blend_equation_array[i].alphaBlendOp        = vp::res::GfxBlendEquationToVkBlendOp(static_cast<BlendEquation>(blend_state->blend_target_state_info_array[i].blend_equation_alpha));
                    channel_mask_array[i]                       = blend_state->blend_target_state_info_array[i].channel_mask;
                }
                ::pfn_vkCmdSetColorBlendEnableEXT(m_command_list.vk_command_buffer, 0, blend_state->blend_target_state_count, blend_enable_array);
                ::pfn_vkCmdSetColorBlendEquationEXT(m_command_list.vk_command_buffer, 0, blend_state->blend_target_state_count, blend_equation_array);
                ::pfn_vkCmdSetColorWriteMaskEXT(m_command_list.vk_command_buffer, 0, blend_state->blend_target_state_count, channel_mask_array);

                ::pfn_vkCmdSetBlendConstants(m_command_list.vk_command_buffer, std::addressof(blend_state->blend_constant_color.m_r));
    
                return;
            }
            void SetDepthStencilState(DepthStencilState *depth_stencil_state) {

                ::pfn_vkCmdSetDepthTestEnable(m_command_list.vk_command_buffer, depth_stencil_state->enable_depth_test);
                ::pfn_vkCmdSetDepthWriteEnable(m_command_list.vk_command_buffer, depth_stencil_state->enable_depth_write);

                ::pfn_vkCmdSetDepthCompareOp(m_command_list.vk_command_buffer, static_cast<VkCompareOp>(depth_stencil_state->depth_comparison_function));

                if (depth_stencil_state->enable_depth_bounds != true) {
                    ::pfn_vkCmdSetDepthBoundsTestEnable(m_command_list.vk_command_buffer, false);
                } else {
                    ::pfn_vkCmdSetDepthBoundsTestEnable(m_command_list.vk_command_buffer, true);
                    ::pfn_vkCmdSetDepthBounds(m_command_list.vk_command_buffer, 0.0f, 1.0f);
                }

                ::pfn_vkCmdSetStencilTestEnable(m_command_list.vk_command_buffer, depth_stencil_state->enable_stencil_test);
                ::pfn_vkCmdSetStencilCompareMask(m_command_list.vk_command_buffer, VK_STENCIL_FACE_FRONT_AND_BACK, depth_stencil_state->stencil_value_mask);
                ::pfn_vkCmdSetStencilWriteMask(m_command_list.vk_command_buffer, VK_STENCIL_FACE_FRONT_AND_BACK, depth_stencil_state->stencil_write_mask);
                
                ::pfn_vkCmdSetStencilOp(m_command_list.vk_command_buffer,VK_STENCIL_FACE_FRONT_BIT, static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_fail_op), static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_depth_pass_op), static_cast<VkStencilOp>(depth_stencil_state->front_face_stencil_depth_fail_op), static_cast<VkCompareOp>(depth_stencil_state->front_face_stencil_comparison_function));
                ::pfn_vkCmdSetStencilReference(m_command_list.vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->front_face_stencil_reference);
                ::pfn_vkCmdSetStencilOp(m_command_list.vk_command_buffer,VK_STENCIL_FACE_BACK_BIT, static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_fail_op), static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_depth_pass_op), static_cast<VkStencilOp>(depth_stencil_state->back_face_stencil_depth_fail_op), static_cast<VkCompareOp>(depth_stencil_state->back_face_stencil_comparison_function));
                ::pfn_vkCmdSetStencilReference(m_command_list.vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->back_face_stencil_reference);
            }
            void SetRasterizerState(RasterizerState *rasterizer_state) {

                ::pfn_vkCmdSetRasterizerDiscardEnable(m_command_list.vk_command_buffer, rasterizer_state->enable_rasterizer_discard);

                ::pfn_vkCmdSetConservativeRasterizationModeEXT(m_command_list.vk_command_buffer, static_cast<VkConservativeRasterizationModeEXT>(rasterizer_state->enable_conservative_rasterizer));
                ::pfn_vkCmdSetExtraPrimitiveOverestimationSizeEXT(m_command_list.vk_command_buffer, 0.0f);

                ::pfn_vkCmdSetRasterizationSamplesEXT(m_command_list.vk_command_buffer, static_cast<VkSampleCountFlagBits>(rasterizer_state->rasterization_samples));
                ::pfn_vkCmdSetSampleMaskEXT(m_command_list.vk_command_buffer, static_cast<VkSampleCountFlagBits>(rasterizer_state->rasterization_samples), std::addressof(rasterizer_state->sample_mask));

                ::pfn_vkCmdSetAlphaToCoverageEnableEXT(m_command_list.vk_command_buffer, rasterizer_state->enable_alpha_to_coverage);
                ::pfn_vkCmdSetDepthClampEnableEXT(m_command_list.vk_command_buffer, rasterizer_state->enable_depth_clamp);

                ::pfn_vkCmdSetPolygonModeEXT(m_command_list.vk_command_buffer, vp::res::GfxFillModeToVkPolygonMode(static_cast<FillMode>(rasterizer_state->fill_mode)));
                ::pfn_vkCmdSetCullMode(m_command_list.vk_command_buffer, vp::res::GfxCullModeToVkCullModeFlags(static_cast<CullMode>(rasterizer_state->cull_mode)));
                ::pfn_vkCmdSetFrontFace(m_command_list.vk_command_buffer, vp::res::GfxFrontFaceToVkFrontFace(static_cast<FrontFace>(rasterizer_state->front_face)));

                ::pfn_vkCmdSetDepthBiasEnable(m_command_list.vk_command_buffer, true);
                ::pfn_vkCmdSetDepthBias(m_command_list.vk_command_buffer, rasterizer_state->polygon_offset_constant_factor, rasterizer_state->polygon_offset_clamp, rasterizer_state->polygon_offset_slope_factor);
            }
            void SetVertexState(VertexState *vertex_state) {

                VkVertexInputBindingDescription2EXT vertex_binding_description_array[Context::cTargetMaxVertexBufferCount] = {};
                VkVertexInputAttributeDescription2EXT vertex_attribute_description_array[Context::cTargetMaxVertexBufferCount] = {};
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
            void SetViewportScissorState(ViewportScissorState *viewport_scissor_state) {

                ::pfn_vkCmdSetViewportWithCount(m_command_list.vk_command_buffer, viewport_scissor_state->viewport_scissor_count, reinterpret_cast<VkViewport*>(viewport_scissor_state->viewport_state_array));

                if (viewport_scissor_state->enable_scissors == true) {
                    ::pfn_vkCmdSetScissorWithCount(m_command_list.vk_command_buffer, viewport_scissor_state->viewport_scissor_count, reinterpret_cast<VkRect2D*>(viewport_scissor_state->scissor_state_array));
                    return;
                }

                VkRect2D scissor_array[Context::cTargetMaxViewportScissorCount] = {};
                for (u32 i = 0; i < viewport_scissor_state->viewport_scissor_count; ++i) {
                    scissor_array[i].offset.x      = 0;
                    scissor_array[i].offset.y      = 0;
                    scissor_array[i].extent.width  = 0x7fff'ffff;
                    scissor_array[i].extent.height = 0x7fff'ffff;
                }
                ::pfn_vkCmdSetScissorWithCount(m_command_list.vk_command_buffer, viewport_scissor_state->viewport_scissor_count, scissor_array);

                return;
            }
            void SetLineWidth(float line_width) {
                ::pfn_vkCmdSetLineWidth(m_command_list.vk_command_buffer, line_width);
            }
        public:
            constexpr ALWAYS_INLINE VkCommandBuffer GetVkCommandBuffer() const { return m_command_list.vk_command_buffer; }
    };

    class ThreadLocalCommandBuffer : public CommandBufferBase {
        public:
            constexpr ALWAYS_INLINE ThreadLocalCommandBuffer() : CommandBufferBase() {/*...*/}

            /* Commmand buffer management */
            void Begin(QueueType queue_type = QueueType::Graphics, bool is_primary = false) {

                /* Allocate a command buffer */
                m_command_list = CommandPoolManager::GetInstance()->CreateThreadLocalCommandList(queue_type, is_primary);

                /* Begin command buffer */
                const VkCommandBufferInheritanceInfo inheritance_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                };
                const VkCommandBufferBeginInfo begin_info = {
                    .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                    .pInheritanceInfo = (is_primary == true) ? nullptr : std::addressof(inheritance_info),
                };
                const u32 result = ::pfn_vkBeginCommandBuffer(m_command_list.vk_command_buffer, std::addressof(begin_info));
                VP_ASSERT(result == VK_SUCCESS);
            }

            CommandList End() {

                /* End command buffer */
                const u32 result = ::pfn_vkEndCommandBuffer(m_command_list.vk_command_buffer);
                VP_ASSERT(result == VK_SUCCESS);

                return m_command_list;
            }
    };
}

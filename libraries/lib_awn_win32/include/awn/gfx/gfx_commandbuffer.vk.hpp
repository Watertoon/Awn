#pragma once

namespace awn::gfx {

    struct GpuAddress {
        VkBuffer        vk_buffer;
        VkDeviceAddress vk_device_address;
    };

    using CommandList = VkCommandBuffer;

    class CommandPoolManager {
        private:
            u32 m_command_pool_tls_slot;
        public:
            VP_SINGLETON_TRAITS(CommandPoolManager);
        public:
            constexpr ALWAYS_INLINE CommandPoolManager() : m_command_pool_tls_slot(0) {/*...*/}

            void Initialize() {
                m_command_pool_tls_slot = sys::AllocateTlsSlot(nullptr);
            }

            void Finalize() {
                sys::FreeTlsSlot(m_tls_slot);
                m_command_pool_tls_slot = 0;
            }

            CommandList CreateThreadLocalCommandList(QueueType queue_type) {

                /* Try to acquire existing command pool */
                VkCommandPool vk_command_pool = reinterpret_cast<VkCommandPool>(sys::GetTlsData(m_command_pool_tls_slot));

                /* Allocate new commmand pool if one doesn't exist */
                if (vk_command_pool == VK_NULL_HANDLE) {

                    ::pfn_vkCreateCommandPool();
                }

                /* Allocate new secondary command buffer */
                VkCommandBufferAllocateInfo {
                    .sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = vk_command_pool;
                };

                VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
                const u32 result = ::pfn_vkAllocateCommandBuffers(Context::GetInstance()->GetVkDevice(), std::addressof(), std::addressof(vk_command_buffer));

                return {vk_command_buffer, sys::GetCurrentThread()};
            }

            void FinalizeThreadLocalCommandList(CommandList command_list) {

                /* Try to acquire existing command pool */
                VkCommandPool vk_command_pool = reinterpret_cast<VkCommandPool>(sys::GetTlsData(m_command_pool_tls_slot));

                /* Free secondary command buffer */
                ::pfn_vkFreeCommandBuffer();
            }
    };

    class CommandBufferBase {
        protected:
            VkCommandBuffer m_command_list;
        public:
            constexpr ALWAYS_INLINE CommandBuffer() {/*...*/}

            /* Gpu dispatch commands */
            void Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count) {
                ::pfn_vkCmdSetPrimitiveTopology(m_command_list, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
                ::pfn_vkCmdDraw(m_command_list, vertex_count, 0, base_vertex, 0);
            }
            void DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count) {
                ::pfn_vkCmdSetPrimitiveTopology(m_command_list, vp::res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
                ::pfn_vkCmdDraw(m_command_list, vertex_count, instance_count, base_vertex, base_instance);
            }
            void DrawIndexed(IndexType index_type, GpuAddress index_buffer_address, u32 base_index, u32 index_count, u32 base_vertex) {
                ::pfn_vkCmdSetPrimitiveTopology(m_command_list, res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
                ::pfn_vkCmdBindIndexBuffer(m_command_list, index_buffer_address->vk_buffer, 0, res::GfxIndexTypeToVkIndexType(index_type));
                ::pfn_vkCmdDrawIndexed(m_command_list, index_count, 0, base_index, base_vertex, 0);
            }
            void DrawIndexedInstanced(IndexType index_type, GpuAddress index_buffer_address, u32 base_index, u32 index_count, u32 base_instance, u32 instance_count) {
                ::pfn_vkCmdSetPrimitiveTopology(m_command_list, res::GfxPrimitiveTopologyToVkPrimitiveTopology(primitive_topology));
                ::pfn_vkCmdBindIndexBuffer(m_command_list, index_buffer_address->vk_buffer, 0, res::GfxIndexTypeToVkIndexType(index_type));
                ::pfn_vkCmdDrawIndexed(m_command_list, index_count, instance_count, base_index, base_vertex, base_instance);
            }
            void DrawMeshTasks(u32 x, u32 y, u32 z) {
                ::pfn_vkCmdDrawMeshTasksEXT(m_command_list, x, y, z);
            }
            void Dispatch(u32 x, u32 y, u32 z) {
                ::pfn_vkCmdDispatch(m_command_list, x, y, z);
            }

            /* Resource state setter commands */
            void SetVertexBuffer(GpuAddress vertex_buffer_address, u32 binding, u32 stride, u32 size) {
                const VkDeviceSize offset = 0;
                const VkDeviceSize size   = size;
                const VkDeviceSize stride = stride;
                ::pfn_vkCmdBindVertexBuffers2(m_command_list, binding, 1, std::addressof(vertex_buffer_address->vk_buffer), std::addressof(offset), std::addressof(size), std::addressof(stride));
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
                VkAttachmentInfo color_attachment_info_array[Context::TargetMaxBoundRenderTargetColor] = {};
                for (u32 i = 0; i < color_target_count; ++i) {
                    color_attachment_info_array[i].sType       = VK_STRUCTURE_TYPE_ATTACHMENT_INFO;
                    color_attachment_info_array[i].imageView   = color_target_array[i]->GetVkImageView();
                    color_attachment_info_array[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    color_attachment_info_array[i].loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                    color_attachment_info_array[i].storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                }

                /* Setup depth stencil attachments */
                VkAttachmentInfo  depth_attachment_info             = {};
                VkAttachmentInfo  stencil_attachment_info           = {};
                VkAttachmentInfo *selected_depth_attachment_info   = nullptr;
                VkAttachmentInfo *selected_stencil_attachment_info = nullptr;
                if (depth_stencil_target != nullptr) {
                    if (depth_stencil_target->IsDepthFormat() == true) {
                        depth_attachment_info.sType       = VK_STRUCTURE_TYPE_ATTACHMENT_INFO;
                        depth_attachment_info.imageView   = depth_stencil_target->GetVkImageView();
                        depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        depth_attachment_info.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                        depth_attachment_info.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                        selected_depth_attachment_info    = std::addressof(selected_depth_attachment_info);
                    }
                    if (depth_stencil_target->IsStencilFormat() == true) {
                        stencil_attachment_info.sType       = VK_STRUCTURE_TYPE_ATTACHMENT_INFO;
                        stencil_attachment_info.imageView   = depth_stencil_target->GetVkImageView();
                        stencil_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        stencil_attachment_info.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                        stencil_attachment_info.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                        selected_stencil_attachment_info = std::addressof(selected_stencil_attachment_info);
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
                            .width = render_width,
                            .width = render_height
                        },
                    },
                    .layerCount           = view_count,
                    .viewMask             = view_mask,
                    .colorAttachmentCount = color_target_count,
                    .pColorAttachments    = color_attachment_info_array,
                    .pDepthAttachment     = selected_depth_attachment_info,
                    .pStencilAttachment   = selected_stencil_attachment_info,
                };
                ::pfn_vkCmdBeginRendering(m_command_list, std::addressof(rendering_info));
            }

            void EndRendering() {
                ::pfn_vkCmdEndRendering(m_command_list);
            }

            void SetStorageBuffer(u32 location, GpuAddress storage_buffer_address) {

                /* Push 8-byte buffer address to location in push constant range 128-256 */
                ::pfn_vkCmdPushConstants(m_command_list, Context::GetInstance()->GetVkPipelineLayout(), VK_SHADER_STAGE_ALL, location * sizeof(VkDeviceAddress), sizeof(VkDeviceAddress));
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
                const u32 view_handle = ((sampler_slot & 0xfff) << Context::cTargetTextureDescriptorIndexBits) | (texture_slot & 0xf'ffff)
                ::pfn_vkCmdPushConstants(m_command_list, Context::GetInstance()->GetVkPipelineLayout(), VK_SHADER_STAGE_ALL, location * sizeof(u32), sizeof(u32), std::addressof(view_handle));
            }

            void SetShader(Shader *shader) {
                VkShaderStageFlagBits flag_bit_array[Context::TargetMaxSimultaneousShaderStages] = {};
                VkShaderEXT           shader_array[Context::TargetMaxSimultaneousShaderStages] = {};
                shader->SetupForCommandList(flag_bit_array, shader_array);
                ::pfn_vkCmdBindShadersEXT(m_command_list, flag_bit_array, shader_array);
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

                ::pfn_vkCmdSetLogicOpEnableEXT(m_command_list, true);
                ::pfn_vkCmdSetLogicOpEXT(m_command_list, res::GfxLogicOperationToVkLogicOp(static_cast<res::GfxLogicOperation>(blend_state->logic_op));

                /* Set VkColorBlendEquation array */
                VkColorBlendEquation  blend_equation_array[Context::TargetMaxColorBlendEquations] = {};
                VkBool32              blend_enable_array[Context::TargetMaxColorBlendEquations] = {};
                VkColorComponentFlags channel_mask_array[Context::TargetMaxColorBlendEquations] = {};
                for (u32 i = 0; i < blend_state->blend_target_state_count; ++i) {
                    blend_enable_array[i]                       = blend_state->blend_target_state_array[i].blend_enable;
                    blend_equation_array[i].srcColorBlendFactor = res::GfxBlendFactorToVkBlendFactor(static_cast<res::GfxBlendFactor>(blend_state->blend_target_state_array[i].blend_factor_source_color));
                    blend_equation_array[i].dstColorBlendFactor = res::GfxBlendFactorToVkBlendFactor(static_cast<res::GfxBlendFactor>(blend_state->blend_target_state_array[i].blend_factor_destination_color));
                    blend_equation_array[i].colorBlendOp        = res::GfxBlendEquationToVkBlendOp(static_cast<res::GfxBlendEquation>(blend_state->blend_target_state_array[i].blend_equation_color));
                    blend_equation_array[i].srcAlphaBlendFactor = res::GfxBlendFactorToVkBlendFactor(static_cast<res::GfxBlendFactor>(blend_state->blend_target_state_array[i].blend_factor_source_alpha));
                    blend_equation_array[i].dstAlphaBlendFactor = res::GfxBlendFactorToVkBlendFactor(static_cast<res::GfxBlendFactor>(blend_state->blend_target_state_array[i].blend_factor_destination_alpha));
                    blend_equation_array[i].alphaBlendOp        = res::GfxBlendEquationToVkBlendOp(static_cast<res::GfxBlendEquation>(blend_state->blend_target_state_array[i].blend_equation_alpha));
                    channel_mask_array[i]                       = blend_state->blend_target_state_array[i].channel_mask;
                }
                ::pfn_vkCmdSetColorBlendEnableEXT(m_command_list, 0, blend_state->blend_target_state_count, blend_enable_array);
                ::pfn_vkCmdSetColorBlendEquationEXT(m_command_list, 0, blend_state->blend_target_state_count, blend_equation_array);
                ::pfn_vkCmdSetColorWriteMaskEXT(m_command_list, 0, blend_state->blend_target_state_count, channel_mask_array);

                ::pfn_vkCmdSetBlendConstants(m_command_list, blend_state->blend_constant_color);
    
                return;
            }
            void SetDepthStencilState(DepthStencilState *depth_stencil_state) {

                ::pfn_vkCmdSetDepthTestEnable(m_command_list, depth_stencil_state->enable_depth_test);
                ::pfn_vkCmdSetDepthWriteEnable(m_command_list, depth_stencil_state->enable_depth_write);

                ::pfn_vkCmdSetDepthCompareOp(m_command_list, depth_stencil_state->depth_comparison_function);

                if (depth_stencil_state->disable_depth_bounds == true) {
                    ::pfn_vkCmdSetDepthBoundsTestEnable(m_command_list, false);
                } else {
                    ::pfn_vkCmdSetDepthBoundsTestEnable(m_command_list, true);
                    ::pfn_vkCmdSetDepthBounds(m_command_list, 0.0f, 1.0f);
                }

                ::pfn_vkCmdSetStencilTestEnable(m_command_list, depth_stencil_state->enable_stencil_test);
                ::pfn_vkCmdSetStencilCompareMask(m_command_list, VK_STENCIL_FACE_FRONT_AND_BACK_BIT, depth_stencil_state->stencil_value_mask);
                ::pfn_vkCmdSetStencilWriteMask(m_command_list, VK_STENCIL_FACE_FRONT_AND_BACK_BIT, depth_stencil_state->stencil_write_mask);
                
                ::pfn_vkCmdSetStencilOp(m_command_list,VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->front_face_stencil_fail_op, depth_stencil_state->front_face_stencil_depth_pass_op, depth_stencil_state->front_face_stencil_depth_fail_op, depth_stencil_state->front_face_stencil_comparison_function);
                ::pfn_vkCmdSetStencilReference(m_command_list, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->front_face_stencil_reference);
                ::pfn_vkCmdSetStencilOp(m_command_list,VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->back_face_stencil_fail_op, depth_stencil_state->back_face_stencil_depth_pass_op, depth_stencil_state->back_face_stencil_depth_fail_op, depth_stencil_state->back_face_stencil_comparison_function);
                ::pfn_vkCmdSetStencilReference(m_command_list, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->back_face_stencil_reference);
            }
            void SetRasterizerState(RasterizerState *rasterizer_state) {

                ::pfn_vkCmdSetRasterizerDiscardEnable(m_command_list, rasterizer_state->enable_rasterizer_discard);

                ::pfn_vkCmdSetConservativeRasterizationModeEXT(m_command_list, rasterizer_state->enable_conservative_rasterizer);
                ::pfn_vkCmdSetExtraPrimitiveOverestimationSizeEXT(m_command_list, 0.0f);

                ::pfn_vkCmdSetRasterizationSamplesEXT(m_command_list, rasterizer_state->rasterization_samples);
                ::pfn_vkCmdSetSampleMaskEXT(m_command_list, rasterizer_state->sample_mask);

                ::pfn_vkCmdSetAlphaToCoverageEnableEXT(m_command_list, rasterizer_state->enable_alpha_to_coverage);
                ::pfn_vkCmdSetDepthClampEnableEXT(m_command_list, rasterizer_state->enable_depth_clamp);

                ::pfn_vkCmdSetPolygonModeEXT(m_command_list, rasterizer_state->fill_mode);
                ::pfn_vkCmdSetCullMode(m_command_list, rasterizer_state->cull_mode);
                ::pfn_vkCmdSetFrontFace(m_command_list, rasterizer_state->front_face);

                ::pfn_vkCmdSetDepthBiasEnable(m_command_list, true);
                ::pfn_vkCmdSetDepthBias(m_command_list, rasterizer_state->polygon_offset_constant_factor, rasterizer_state->polygon_offset_clamp, rasterizer_state->polygon_offset_factor);
            }
            void SetVertexState(VertexState *vertex_state) {

                VkVertexInputBindingDescription2EXT vertex_binding_description_array[Context::TargetMaxVertexBuffers] = {};
                VkVertexInputAttributeDescription2EXT vertex_attribute_description_array[Context::TargetMaxVertexBuffers] = {};
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
                    vertex_attribute_description_array[i].location = res::GfxAttributeFormatToVkFormat(static_cast<res::GfxAttributeFormat>(vertex_state->vertex_attribute_state_array[i].attribute_format);
                    vertex_attribute_description_array[i].location = vertex_state->vertex_attribute_state_array[i].offset;
                }
                ::pfn_vkCmdSetVertexInputEXT(m_command_list, vertex_state->vertex_buffer_state_count, vertex_binding_description_array, vertex_state->vertex_attribute_state_count, vertex_attribute_description_array);
            }
            void SetViewportScissorState(ViewportScissorState *viewport_scissor_state) {

                ::pfn_vkCmdSetViewportWithCount(m_command_list, viewport_scissor_state->viewport_scissor_count, reinterpret_cast<VkViewport*>(viewport_scissor_state->viewport_state_array));

                if (viewport_scissor_state->enable_scissors == true) {
                    ::pfn_vkCmdSetScissorWithCount(m_command_list, viewport_scissor_state->viewport_scissor_count, reinterpret_cast<VkRect2D*>(viewport_scissor_state->scissor_state_array));
                    return;
                }

                VkRect2D scissor_array[Context::TargetMaxViewportScissorCount] = {};
                for (u32 i = 0; i < viewport_scissor_state->viewport_scissor_count; ++i) {
                    scissor_array.offset.x      = 0;
                    scissor_array.offset.y      = 0;
                    scissor_array.extent.width  = 0x7fff'ffff;
                    scissor_array.extent.height = 0x7fff'ffff;
                }
                ::pfn_vkCmdSetScissorWithCount(m_command_list, viewport_scissor_state->viewport_scissor_count, scissor_array);

                return;
            }
            void SetLineWidth(float line_width) {
                ::pfn_vkCmdSetLineWidth(m_command_list, line_width);
            }
    };

    class ThreadLocalCommandBuffer {
        public:
            constexpr ALWAYS_INLINE ThreadLocalCommandBuffer() : CommandBuffer() {/*...*/}

            /* Commmand buffer management */
            void Begin() {

                /* Allocate a command buffer */
                m_command_list = CommandPoolManager::GetInstance()->CreateThreadLocalCommandList();

                /* Begin command buffer */
                const VkCommandBufferBeginInfo begin_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
                };
                const u32 result = ::pfn_vkBeginCommandBuffer(m_command_list.vk_secondary_command_buffer, std::addressof(begin_info));
                VP_ASSERT(result == VK_SUCCESS);
            }

            CommandList End() {

                /* End command buffer */
                const u32 result = ::pfn_vkEndCommandBuffer(m_command_list.vk_secondary_command_buffer);
                VP_ASSERT(result == VK_SUCCESS);

                return m_command_list;
            }
    };
}

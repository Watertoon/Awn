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
#pragma once

namespace awn::gfx {

    enum class CopyFilter {
        Nearest = 0,
        Linear  = 1,
    };

    struct TextureArrayRegion {
        u32 base_array_layer;
        u32 layer_count;
    };

    struct TextureCopyRegion {
        u32 base_x;
        u32 base_y;
        u32 base_z;
        u32 width;
        u32 height;
        u32 depth;
        u16 base_mip_level;
        u16 reserve0;
        u32 base_array_layer;
        u32 layer_count;
    };

    struct CommandList {
        VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
        impl::CommandPoolId pool_id = impl::CommandPoolManager::cInvalidHandle;

        constexpr bool IsValid() const {
            return this->vk_command_buffer != VK_NULL_HANDLE && this->pool_id != impl::CommandPoolManager::cInvalidHandle;
        }

        constexpr QueueType GetQueueType() const {
            return impl::CommandPoolManager::GetInstance()->GetQueueType(this->pool_id);
        }
    };
    static constexpr CommandList cInvalidCommandList = {};

    static constexpr u32 cTargetMaxPerStageMemorySizeCombined = impl::Context::cTargetMaxPerStageMemorySizeCombined;
    static constexpr u32 cTargetMaxBoundRenderTargetColorCount = impl::Context::cTargetMaxBoundRenderTargetColorCount;
    static constexpr u32 cTargetMaxViewportScissorCount = impl::Context::cTargetMaxViewportScissorCount;
    static constexpr u32 cTargetMaxVertexBufferCount = impl::Context::cTargetMaxVertexBufferCount;
    static constexpr u32 cTargetMaxBlendTargetStateCount = impl::Context::cTargetMaxColorBlendEquationCount;

    struct CommandBufferInfo {
        QueueType queue_type = QueueType::Graphics;
        u32 per_stage_memory_size = cTargetMaxPerStageMemorySizeCombined * 2;
        bool is_primary = false;
    };

    class CommandBuffer {
        protected:
            CommandList               m_command_list;
            QueueType                 m_queue_type;
            bool                      m_is_primary;
            bool                      m_is_rendering;
            bool                      m_is_render_target_dirty;
            void                     *m_command_memory_base;
            size_t                    m_command_memory_base_offset;
            u32                       m_command_memory_iter;
            u32                       m_command_memory_size;
            u32                       m_color_target_count;
            RenderTargetColor        *m_color_target_array[cTargetMaxBoundRenderTargetColorCount];
            RenderTargetDepthStencil *m_depth_stencil_target;
            struct {
                u8 m_combined_dirty;
                struct {
                    u8 m_combined_dirty_vertex   : 1;
                    u8 m_combined_dirty_tessellation_control    : 1;
                    u8 m_combined_dirty_tessellation_evaluation : 1;
                    u8 m_combined_dirty_geometry : 1;
                    u8 m_combined_dirty_fragment : 1;
                    u8 m_combined_dirty_compute  : 1;
                    u8 m_combined_dirty_task     : 1;
                    u8 m_combined_dirty_mesh     : 1;
                };
            };
            struct {
                u8 m_separate_texture_sampler_dirty;
                struct {
                    u8 m_separate_texture_sampler_dirty_vertex   : 1;
                    u8 m_separate_texture_sampler_dirty_tessellation_control   : 1;
                    u8 m_separate_texture_sampler_dirty_tessellation_evaluation   : 1;
                    u8 m_separate_texture_sampler_dirty_geometry : 1;
                    u8 m_separate_texture_sampler_dirty_fragment : 1;
                    u8 m_separate_texture_sampler_dirty_compute  : 1;
                    u8 m_separate_texture_sampler_dirty_task     : 1;
                    u8 m_separate_texture_sampler_dirty_mesh     : 1;
                };
            };
            impl::DescriptorMemoryManager::Staging *m_staging_memory;
        protected:
            struct SyncScopeInfo {
                u32 vk_src_stage_mask;
                u32 vk_src_access_mask;
                u32 vk_dst_stage_mask;
                u32 vk_dst_access_mask;
            };
        protected:
            constexpr void *GetDescriptorMemoryTextureSampler(ShaderStage shader_stage) {
                return this->GetDescriptorMemoryCombined(shader_stage);
            }
            constexpr void *GetDescriptorMemoryImage(ShaderStage shader_stage) {
                return this->GetDescriptorMemoryCombined(shader_stage);
            }
            constexpr void *GetDescriptorMemoryUniformBuffer(ShaderStage shader_stage) {
                return this->GetDescriptorMemoryCombined(shader_stage);
            }
            constexpr void *GetDescriptorMemoryStorageBuffer(ShaderStage shader_stage) {
                return this->GetDescriptorMemoryCombined(shader_stage);
            }
            constexpr void *GetDescriptorMemoryCombined(ShaderStage shader_stage) {
                const u32 stage_index = static_cast<u32>(shader_stage);
                return m_staging_memory->descriptor_memory_combined[stage_index];
            }
            constexpr void *GetDescriptorMemorySeparateTextureSampler(ShaderStage shader_stage) {
                const u32 stage_index = static_cast<u32>(shader_stage);
                return reinterpret_cast<impl::DescriptorMemoryManager::StagingExt*>(m_staging_memory)->descriptor_memory_separate_texture_sampler[stage_index];
            }

            constexpr void SetTextureSamplerDirty(ShaderStage shader_stage) {
                this->SetCombinedDirty(shader_stage);
            }
            constexpr void SetImageDirty(ShaderStage shader_stage) {
                this->SetCombinedDirty(shader_stage);
            }
            constexpr void SetUniformBufferDirty(ShaderStage shader_stage) {
                this->SetCombinedDirty(shader_stage);
            }
            constexpr void SetStorageBufferDirty(ShaderStage shader_stage) {
                this->SetCombinedDirty(shader_stage);
            }
            constexpr void SetCombinedDirty(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                m_combined_dirty |= (1 << (stage_index));
                return;
            }
            constexpr void SetSeparateTextureSamplerDirty(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                m_separate_texture_sampler_dirty |= (1 << (stage_index));
                return;
            }

            constexpr void ResetDescriptorMemory() {
                m_command_memory_iter = 0;
            }

            void TransitionRenderTargetsImpl(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target, SyncScopeInfo *sync_scope, VkImageLayout image_layout);

            void CommitDescriptorMemory(VkPipelineBindPoint bind_point, VkPipelineLayout pipeline_layout, u32 base_set, void *desc_memory, u32 memory_size);

            void CommitTextureSamplerDescriptors();
            void CommitImageDescriptors();
            void CommitUniformBufferDescriptors();
            void CommitStorageBufferDescriptors();
            void CommitCombinedDescriptors();
            void CommitSeparateTextureSamplerDescriptors();

            void CommitDescriptors();

            void BeginRenderingIfNotRendering();
            void EndRenderingIfRendering();

            void SetSystemDescriptorBuffers();
        public:
            constexpr CommandBuffer() : m_command_list(), m_queue_type(), m_is_primary(), m_is_rendering(), m_is_render_target_dirty(), m_command_memory_base(), m_command_memory_base_offset(), m_command_memory_iter(), m_command_memory_size(), m_color_target_count(), m_color_target_array(), m_depth_stencil_target(), m_combined_dirty(), m_separate_texture_sampler_dirty(), m_staging_memory() {/*...*/}
            constexpr ~CommandBuffer() {/*...*/}

            void Initialize(const CommandBufferInfo *command_buffer_info) {
                this->Finalize();

                m_queue_type = command_buffer_info->queue_type;
                m_is_primary = command_buffer_info->is_primary;
                const size_t command_memory_size = command_buffer_info->per_stage_memory_size;
                if (command_memory_size == 0) { return; }
                m_staging_memory = impl::DescriptorMemoryManager::GetInstance()->AllocateStagingMemory();
                void *memory = impl::DescriptorMemoryManager::GetInstance()->AllocateCommandMemory(std::addressof(m_command_memory_base_offset), command_memory_size);
                VP_ASSERT(memory != nullptr);
                m_command_memory_base = memory;
                m_command_memory_iter = 0;
                m_command_memory_size = command_memory_size;

                return ;
            }

            void Finalize() {
                if (m_command_list.IsValid()) {
                    impl::CommandPoolManager::GetInstance()->FinalizeCommandList(m_command_list);
                    m_command_list = cInvalidCommandList;
                }
                if (m_command_memory_base != nullptr) {                    
                    impl::DescriptorMemoryManager::GetInstance()->FreeCommandMemory(m_command_memory_base);
                    m_command_memory_base = nullptr;
                    m_command_memory_base_offset = 0;
                }
                if (m_staging_memory != nullptr) {                    
                    impl::DescriptorMemoryManager::GetInstance()->FreeStagingMemory(m_staging_memory);
                    m_staging_memory = nullptr;
                }
            }
        public:
            void Begin();
            CommandList End();

            void TransitionRenderTargetsToAttachment(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target);
            void TransitionRenderTargetsToPresent(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target);

            void BlitImage(Texture *dest_texture, const TextureCopyRegion *dest_copy_region, Texture *source_texture, const TextureCopyRegion *source_copy_region, CopyFilter copy_filter);
            void ClearColor(RenderTargetColor *target_color, const TextureArrayRegion *array_region, vp::util::Color4f color);

            void SetRenderTargets(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target);

            void Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count);
            void DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count);
            void DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuBufferAddress index_buffer_address, u32 index_count, u32 base_vertex);
            void DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuBufferAddress index_buffer_address, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count);

            void DrawMeshTasks(u32 x, u32 y, u32 z);

            void Dispatch(u32 x, u32 y, u32 z);

            void SetVertexBuffer(GpuBufferAddress vertex_buffer_address, u32 binding, u32 stride, u32 size);

            void SetShader(Shader *shader);

            void SetTextureAndSampler(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot, DescriptorSlot sampler_slot);
            void SetImage(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot);
            void SetUniformBuffer(u32 location, gfx::ShaderStage shader_stage, GpuBufferAddress constant_buffer_address, size_t size);
            void SetStorageBuffer(u32 location, gfx::ShaderStage shader_stage, GpuBufferAddress storage_buffer_address, size_t size);
            void SetSeparateTexture(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot);
            void SetSeparateSampler(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot sampler_slot);

            void SetBlendState(BlendState *blend_state);
            void SetDepthStencilState(DepthStencilState *depth_stencil_state);
            void SetRasterizerState(RasterizerState *rasterizer_state);
            void SetVertexState(VertexState *vertex_state);
            void SetViewportScissorState(ViewportScissorState *viewport_scissor_state);

            void SetLineWidth(float line_width);
        public:
            constexpr ALWAYS_INLINE VkCommandBuffer GetVkCommandBuffer() const { return m_command_list.vk_command_buffer; }
    };
}

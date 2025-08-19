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
        u32 base_array_layer = 0;
        u32 layer_count = 1;
    };

    struct CommandList {
        impl::CommandListUnit *unit;

        constexpr bool IsValid() const {
            return this->unit != nullptr;
        }

        constexpr QueueType GetQueueType() const {
            return impl::CommandPoolManager::GetInstance()->GetQueueType(this->unit->pool_id);
        }
    };
    static constexpr CommandList cInvalidCommandList = {};

    static constexpr u32 cTargetMaxBoundRenderTargetColorCount = impl::Context::cTargetMaxBoundRenderTargetColorCount;
    static constexpr u32 cTargetMaxViewportScissorCount = impl::Context::cTargetMaxViewportScissorCount;
    static constexpr u32 cTargetMaxVertexBufferCount = impl::Context::cTargetMaxVertexBufferCount;
    static constexpr u32 cTargetMaxBlendTargetStateCount = impl::Context::cTargetMaxColorBlendEquationCount;

    struct CommandBufferInfo {
        QueueType queue_type = QueueType::Graphics;

        constexpr void SetQueueType(QueueType queue_type) {
            this->queue_type = queue_type;
        }
    };

    size_t EstimateDescriptorMemorySize(u32 max_draw_dispatch_count, u32 max_descriptor_count);

    struct CommandBufferBeginInfo {
        size_t descriptor_memory_size;
        bool is_primary;

        constexpr void SetDefaults() {
            this->descriptor_memory_size = vp::util::c2KB;
            this->is_primary = false;
        }
        void EstimateMemorySize(u32 max_draw_dispatch_count, u32 max_descriptor_count) {
            this->descriptor_memory_size = gfx::EstimateDescriptorMemorySize(max_draw_dispatch_count, max_descriptor_count);
        }
        constexpr void SetMemorySize(size_t memory_size) {
            this->descriptor_memory_size = memory_size;
        }
        constexpr void SetIsPrimary(bool is_primary) {
            this->is_primary = is_primary;
        }
    };

    enum RenderTargetClearType : u8 {
        None,
        Clear,
        Keep,
    };

    class CommandBuffer {
        protected:
            impl::CommandListUnit *m_command_list_unit;
            impl::CommandPoolId m_pool_id;
            QueueType m_queue_type;
            bool m_is_rendering;
            RenderTargetClearType m_trigger_clear_type;
            u32 m_color_target_count;
            TextureViewBinder *m_color_target_array[cTargetMaxBoundRenderTargetColorCount];
            TextureViewBinder *m_depth_stencil_target;
            bool m_is_render_target_dirty;
            bool m_is_preserve_descriptors_between_commit;
            bool m_is_dirty_texture;
            bool m_is_dirty_buffer;
            s32 m_texture_commit_size;
            s32 m_buffer_commit_size;
            void *m_staging_memory;
        protected:
            struct SyncScopeInfo {
                u32 vk_src_stage_mask;
                u32 vk_src_access_mask;
                u32 vk_dst_stage_mask;
                u32 vk_dst_access_mask;
            };
        protected:
            void *GetDescriptorMemoryTextureSamplerImageSeparate();
            void *GetDescriptorMemoryUniformBufferStorageBuffer();

            void TransitionRenderTargetsImpl(TextureViewBinder **color_target_array, u32 color_target_count, TextureViewBinder *depth_stencil_target, SyncScopeInfo *sync_scope, VkImageLayout image_layout);

            size_t CommitDescriptorMemory(void *desc_memory, u32 memory_size);
            void CommitDescriptors();

            void BeginRenderingIfNotRendering();
            void EndRenderingIfRendering();
        public:
            constexpr CommandBuffer() : m_command_list_unit(), m_pool_id(impl::CommandPoolManager::cInvalidCommandPoolId), m_queue_type(), m_is_rendering(), m_trigger_clear_type(),  m_color_target_count(), m_color_target_array(), m_depth_stencil_target(), m_is_render_target_dirty(), m_is_preserve_descriptors_between_commit(), m_is_dirty_texture(), m_is_dirty_buffer(), m_texture_commit_size(-1), m_buffer_commit_size(-1), m_staging_memory() {/*...*/}
            constexpr ~CommandBuffer() {/*...*/}

            Result Initialize(const CommandBufferInfo *command_buffer_info);
            void Finalize() ;

            constexpr void SetPreserveDescriptors(bool is_preserve) {
                m_is_preserve_descriptors_between_commit = is_preserve;
            }
        public:
            Result Begin(const CommandBufferBeginInfo *info);
            Result End(CommandList *out_cmd_list);

            void SetSystemDescriptorBuffers();

            void CallCommands(CommandList secondary_command_list);

            void TransitionRenderTargetsToAttachment(TextureViewBinder **color_target_array, u32 color_target_count, TextureViewBinder *depth_stencil_target);
            void TransitionRenderTargetsToPresent(TextureViewBinder **color_target_array, u32 color_target_count, TextureViewBinder *depth_stencil_target);

            void FlushMemory(GpuAccessFlags gpu_access_flags);
            void InvalidateMemory(GpuAccessFlags gpu_access_flags);

            void BlitImage(Texture *source_texture, const TextureCopyRegion *source_copy_region, Texture *dest_texture, const TextureCopyRegion *dest_copy_region, CopyFilter copy_filter);
            void ResolveImage(Texture *source_texture, Texture *dest_texture);
            void ClearColor(TextureViewBinder *target_color, const TextureArrayRegion *array_region, vp::util::Color4f color);
            void ClearDepthStencil(TextureViewBinder *target_depth_stencil, const TextureArrayRegion *array_region, float depth, u32 stencil);

            void SetRenderTargets(TextureViewBinder **color_target_array, u32 color_target_count, TextureViewBinder *depth_stencil_target, RenderTargetClearType clear_type = RenderTargetClearType::None);

            void Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count);
            void DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count);
            void DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, BufferAddress index_buffer_address, u32 index_count, u32 base_vertex);
            void DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, BufferAddress index_buffer_address, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count);

            void DrawMeshTasks(u32 x, u32 y, u32 z);

            void Dispatch(u32 x, u32 y, u32 z);

            void SetVertexBuffer(BufferAddress vertex_buffer_address, u32 binding, u32 stride, u32 size);

            void SetShader(const Shader *shader);

            void SetTextureSampler(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot, DescriptorSlot sampler_slot);
            void SetImage(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot);
            void SetUniformBuffer(u32 location, gfx::ShaderStage shader_stage, GpuAddress gpu_address, size_t size);
            void SetStorageBuffer(u32 location, gfx::ShaderStage shader_stage, GpuAddress gpu_address, size_t size);
            void SetSeparateTexture(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot texture_slot);
            void SetSeparateSampler(u32 location, gfx::ShaderStage shader_stage, DescriptorSlot sampler_slot);

            void SetBlendState(const BlendState *blend_state);
            void SetDepthStencilState(const DepthStencilState *depth_stencil_state);
            void SetRasterizerState(const RasterizerState *rasterizer_state);
            void SetVertexState(const VertexState *vertex_state);
            void SetViewportScissorState(const ViewportScissorState *viewport_scissor_state);

            void SetLineWidth(float line_width);
        public:
            constexpr ALWAYS_INLINE VkCommandBuffer GetVkCommandBuffer() const { return (m_command_list_unit == nullptr) ? VK_NULL_HANDLE : m_command_list_unit->vk_command_buffer; }
    };

    class ScopedCommandBufferPreserveDescriptors {
        private:
            CommandBuffer *m_cmd_buf;
        public:
            constexpr ScopedCommandBufferPreserveDescriptors(CommandBuffer *command_buffer) : m_cmd_buf(command_buffer) {
                command_buffer->SetPreserveDescriptors(true);
            }
            constexpr ~ScopedCommandBufferPreserveDescriptors() {
                m_cmd_buf->SetPreserveDescriptors(false);
            }
    };
}

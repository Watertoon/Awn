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

    struct GpuAddressRange {
        GpuAddress gpu_address;
        size_t size;
    };

    struct BufferTextureCopyRegion {
        u32 row_length = 0;
        u32 image_height = 0;
        TextureCopyRegion copy_region;
    };
    struct TextureSubresource {
        u32 base_mip_level = 0;
        u32 base_array_layer = 0;
    };
    struct TextureArrayRegion {
        u32 base_array_layer = 0;
        u32 layer_count = 1;
    };

    struct CommandList {
        VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
        impl::CommandPoolId pool_id = impl::cInvalidCommandPoolId;

        constexpr bool IsValid() const {
            return this->vk_command_buffer != VK_NULL_HANDLE && this->pool_id != impl::cInvalidCommandPoolId;
        }

        constexpr QueueType GetQueueType() const {
            return impl::CommandPoolManager::GetInstance()->GetQueueType(this->pool_id);
        }

        constexpr VkCommandBuffer GetVkCommandBuffer() const {
            return this->vk_command_buffer;
        }
    };
    static constexpr const CommandList cInvalidCommandList = {};

    static constexpr u32 cMaxRenderTargetColorCount = impl::Context::cTargetMaxBoundRenderTargetColorCount;
    static constexpr u32 cMaxViewportScissorCount = impl::Context::cTargetMaxViewportScissorCount;
    static constexpr u32 cMaxVertexBufferCount = impl::Context::cTargetMaxVertexBufferCount;
    static constexpr u32 cMaxBlendTargetStateCount = impl::Context::cTargetMaxColorBlendEquationCount;

    struct CommandBufferInfo {
        QueueType queue_type = QueueType::Graphics;

        constexpr void SetQueueType(QueueType queue_type) {
            this->queue_type = queue_type;
        }
    };

    struct CommandBufferBeginInfo {
        bool is_primary;

        constexpr void SetDefaults() {
            this->is_primary = false;
        }
        constexpr void SetIsPrimary(bool is_primary) {
            this->is_primary = is_primary;
        }
    };

    struct RenderBuffer {
        gfx::TextureViewBinder *color_target_array[cMaxRenderTargetColorCount];
        gfx::TextureViewBinder *depth_stencil_target;
        u32 color_count;

        constexpr void SetRenderTargets(gfx::TextureViewBinder **color_target_array, u32 color_count, gfx::TextureViewBinder *depth_stencil_target) {
            VP_ASSERT(color_count <= cMaxRenderTargetColorCount);
            for (u32 i = 0; i < color_count; ++i) {
                this->color_target_array[i] = color_target_array[i];
            }
            this->color_count = color_count;
            this->depth_stencil_target = depth_stencil_target;
            return;
        }
        /* private */
        constexpr u32 GetVkImageViewArray(VkImageView *out_color_array, VkImageView *out_ds) const {
            const u32 count = this->color_count;
            for (u32 i = 0; i < count; ++i) {
                out_color_array[i] = this->color_target_array[i]->GetVkImageView();
            }
            *out_ds = (this->depth_stencil_target != nullptr) ? this->depth_stencil_target->GetVkImageView() : VK_NULL_HANDLE;
            return count;
        }
        constexpr const impl::TextureViewKey *GetColorTextureViewKey(u32 target_id) const {
            return this->color_target_array[target_id]->GetTextureViewKey();
        }
        constexpr const impl::TextureViewKey *GetDepthStencilTextureViewKey() const {
            return this->depth_stencil_target->GetTextureViewKey();
        }
    };

    static constexpr u32 cLoadClearBase = cMaxRenderTargetColorCount * 0;
    static constexpr u32 cLoadDiscardBase = cMaxRenderTargetColorCount * 1;
    static constexpr u32 cStoreDiscardBase = cMaxRenderTargetColorCount * 2;
    static_assert(cStoreDiscardBase + cMaxRenderTargetColorCount < BITSIZEOF(u32));

    struct BeginRenderingInfo {
        RenderBuffer *render_buffer;
        RenderBuffer *resolve_render_buffer;
        ScissorState render_area;
        //gfx::TextureViewBinder *fragment_density_map;
        vp::util::FixedConnection<u32,0x20> op_mask;

        constexpr void SetDefaults() {
            this->render_buffer = nullptr;
            this->resolve_render_buffer = nullptr;
            this->render_area.SetDefaults();
            //this->fragment_density_map = nullptr;
            this->op_mask.Clear();
        }
        constexpr void SetRenderArea(const ScissorState *render_area_scissor) {
            this->render_area = *render_area_scissor;
        }
        constexpr void SetRenderBuffer(RenderBuffer *render_buffer) {
            this->render_buffer = render_buffer;
        }
        constexpr void SetResolveRenderBuffer(RenderBuffer *render_buffer) {
            this->resolve_render_buffer = render_buffer;
        }
        //constexpr void SetFragmentDensityMap(gfx::TextureViewBinder *fragment_density_map) {
        //    this->fragment_density_map = fragment_density_map;
        //}
        constexpr void SetLoadDiscard(u32 mask) {
            this->op_mask.ExchangeBits(cLoadDiscardBase, cMaxRenderTargetColorCount, mask);
        }
        constexpr void SetLoadClearMask(u32 mask) {
            this->op_mask.ExchangeBits(cLoadClearBase, cMaxRenderTargetColorCount, mask);
        }
        constexpr void SetStoreDiscardMask(u32 mask) {
            this->op_mask.ExchangeBits(cStoreDiscardBase, cMaxRenderTargetColorCount, mask);
        }
    };

    enum class ConditionalRenderMode {
        IsZero,
        IsNonZero,
    };

    struct ConditionalRenderData {
        u32 value;

        constexpr u32 GetValue() const { return this->value; }
        constexpr void SetValue(u32 value) { this->value = value; }
    };

    class CommandBuffer {
        public:
            static constexpr u32 cMaxStackCount = 4;
            using RenderInfoStack = vp::util::FixedFrameArray<BeginRenderingInfo, cMaxStackCount>;
        protected:
            CommandList m_command_list;
            impl::CommandPoolId m_pool_id;
            QueueType m_queue_type;
            RenderInfoStack m_render_info_stack;
        protected:
            struct SyncScopeInfo {
                u32 vk_src_stage_mask;
                u32 vk_src_access_mask;
                u32 vk_dst_stage_mask;
                u32 vk_dst_access_mask;
            };
        public:
            constexpr CommandBuffer() : m_command_list(cInvalidCommandList), m_pool_id(impl::cInvalidCommandPoolId), m_queue_type(),
            m_render_info_stack()
            {/*...*/}
            constexpr ~CommandBuffer() = default;

            Result Initialize(const CommandBufferInfo *command_buffer_info);
            void Finalize();

            constexpr bool IsRecording() const { return m_command_list.IsValid(); }
            constexpr bool IsRendering() const { return 0 < m_render_info_stack.GetUsedCount(); }
            constexpr QueueType GetQueueType() const { return m_queue_type; }
        public:
            Result Begin(const CommandBufferBeginInfo *info);
            Result End(CommandList *out_cmd_list);

            void CallCommands(CommandList secondary_command_list);
        public:
            void FlushMemory(GpuAccessFlags gpu_access_flags);
            void InvalidateMemory(GpuAccessFlags gpu_access_flags);
        public:
            void BlitImage(Texture *source_texture, const TextureCopyRegion *source_copy_region, Texture *dest_texture, const TextureCopyRegion *dest_copy_region, CopyFilter copy_filter);
            void CopyImage(Texture *source_texture, const TextureCopyRegion *source_copy_region, Texture *dest_texture, const TextureSubresource *dest_subresource, const Int3 &dest_base_offset);
            void CopyMemory(GpuAddress source_address, GpuAddress dest_address, size_t size);
            void CopyMemoryToImage(GpuAddress source_address, size_t src_size, Texture *dest_texture, const BufferTextureCopyRegion *copy_region);
            void CopyImageToMemory(Texture *source_texture, GpuAddress dest_address,  size_t dst_size, const BufferTextureCopyRegion *copy_region);

            void ResolveImage(Texture *source_texture, Texture *dest_texture);
        private:
            void BeginRenderingImpl(const BeginRenderingInfo *info, bool is_rebind);
            void EndRenderingImpl(const BeginRenderingInfo *info, bool is_unbound);
            constexpr const BeginRenderingInfo *GetLastBeginRenderingInfo() const {
                return std::addressof(m_render_info_stack[m_render_info_stack.GetUsedCount()-1]);
            }
        public:
            bool BeginRendering(const BeginRenderingInfo *info);
            void EndRendering();

            void ClearColor(u32 target_id, vp::util::Color4f color);
            void ClearColors(u32 target_count, const u32 *target_array, const vp::util::Color4f *colors);
            void ClearDepthStencil(float depth, u32 stencil);

            void SetShader(const Shader *shader);
            void SetResourceSet(const ResourceSet *resource_set);
            void SetBlendState(const BlendState *blend_state);
            void SetDepthStencilState(const DepthStencilState *depth_stencil_state);
            void SetRasterizerState(const RasterizerState *rasterizer_state);
            void SetViewportState(const ViewportState *viewport_state);
            void SetViewportScissorState(const ViewportScissorState *viewport_scissor_state);

            void SetVertexState(const VertexState *vertex_state);
            void SetVertexBuffer(u32 binding, GpuAddress gpu_address, size_t size);
            void SetVertexBuffers(u32 base_binding, const GpuAddressRange *gpu_address_range_array, u32 count);

            void BeginRenderEnableCondition(ConditionalRenderMode mode, GpuAddress value_address);
            void EndRenderEnableCondition();

            void SetLineWidth(float line_width);
        private:
            void SetPrimitiveTopologyImpl(PrimitiveTopology topology);
            void SetIndexBufferImpl(IndexFormat index_format, GpuAddress index_buffer_address, u32 max_index_count);
        public:
            void Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count);
            void DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count);
            void DrawIndirect(PrimitiveTopology primitive_topology, GpuAddress indirect_address);
            void MultiDrawIndexedIndirect(PrimitiveTopology primitive_topology, GpuAddress indirect_address, u32 max_draw_count, GpuAddress counter_address, size_t stride);

            void DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 index_count, u32 base_vertex);
            void DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count);
            void DrawIndexedIndirect(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 max_index_count, GpuAddress indirect_address);
            void MultiDrawIndexedIndirect(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 max_index_count, GpuAddress indirect_address, u32 max_draw_count, GpuAddress counter_address, size_t stride);

            void DrawMeshTasks(u32 x, u32 y, u32 z);
            void DrawMeshTasksIndirect(GpuAddress indirect_address);
            void MultiDrawMeshTasksIndirect(GpuAddress indirect_address, u32 max_draw_count, GpuAddress counter_address, size_t stride);
        public:
            void Dispatch(u32 x, u32 y, u32 z);
            void DispatchIndirect(GpuAddress indirect_address);
        public:
            void SetSystemDescriptorBuffers();
        public:
            constexpr ALWAYS_INLINE VkCommandBuffer GetVkCommandBuffer() const { return m_command_list.GetVkCommandBuffer(); }
    };
}

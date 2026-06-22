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
        u32 base_x = 0;
        u32 base_y = 0;
        u32 base_z = 0;
        u32 width;
        u32 height = 1;
        u32 depth = 1;
        u32 base_mip_level = 0;
        u32 base_array_layer = 0;
        u32 array_layer_count = 1;
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

    struct CommandBufferBeginInfo {
        bool is_primary;

        constexpr void SetDefaults() {
            this->is_primary = false;
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
            CommandList m_command_list;
            impl::CommandPoolId m_pool_id;
            QueueType m_queue_type;
            bool m_is_rendering;
            bool m_is_render_target_dirty;
            RenderTargetClearType m_trigger_clear_type;
            u32 m_color_target_count;
            TextureViewBinder *m_color_target_array[cTargetMaxBoundRenderTargetColorCount];
            TextureViewBinder *m_depth_stencil_target;
        protected:
            struct SyncScopeInfo {
                u32 vk_src_stage_mask;
                u32 vk_src_access_mask;
                u32 vk_dst_stage_mask;
                u32 vk_dst_access_mask;
            };
        protected:
            void BeginRenderingIfNotRendering();
            void EndRenderingIfRendering();
        public:
            constexpr CommandBuffer() : m_command_list(cInvalidCommandList), m_pool_id(impl::cInvalidCommandPoolId), m_queue_type(), m_is_rendering(), m_is_render_target_dirty(), m_trigger_clear_type(),  m_color_target_count(), m_color_target_array(), m_depth_stencil_target() {/*...*/}
            constexpr ~CommandBuffer() = default;

            Result Initialize(const CommandBufferInfo *command_buffer_info);
            void Finalize();

            constexpr bool IsRecording() const { return m_command_list.IsValid(); }
            constexpr QueueType GetQueueType() const { return m_queue_type; }
        public:
            Result Begin(const CommandBufferBeginInfo *info);
            Result End(CommandList *out_cmd_list);

            void CallCommands(CommandList secondary_command_list);

            void FlushMemory(GpuAccessFlags gpu_access_flags);
            void InvalidateMemory(GpuAccessFlags gpu_access_flags);

            void BlitImage(Texture *source_texture, const TextureCopyRegion *source_copy_region, Texture *dest_texture, const TextureCopyRegion *dest_copy_region, CopyFilter copy_filter);
            void CopyImage(Texture *source_texture, const TextureCopyRegion *source_copy_region, Texture *dest_texture, const TextureSubresource *dest_subresource, const Int3 &dest_base_offset);
#ifdef AWN_ENABLE_VK_DEVICE_ADDRESS_COMMANDS
            void CopyMemory(GpuAddress source_address, GpuAddress dest_address, size_t size);
            void CopyMemoryToImage(GpuAddress source_address, size_t src_size, Texture *dest_texture, const BufferTextureCopyRegion *copy_region);
            void CopyImageToMemory(Texture *source_texture, GpuAddress dest_address,  size_t dst_size, const BufferTextureCopyRegion *copy_region);
#endif
            void ResolveImage(Texture *source_texture, Texture *dest_texture);
            void ClearColor(TextureViewBinder *target_color, const TextureArrayRegion *array_region, vp::util::Color4f color);
            void ClearDepthStencil(TextureViewBinder *target_depth_stencil, const TextureArrayRegion *array_region, float depth, u32 stencil);

            void SetRenderTargets(TextureViewBinder **color_target_array, u32 color_target_count, TextureViewBinder *depth_stencil_target, RenderTargetClearType clear_type = RenderTargetClearType::None);

            void Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count);
            void DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count);
#ifndef AWN_ENABLE_VK_DEVICE_ADDRESS_COMMANDS
            void DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, BufferAddress index_buffer_address, u32 index_count, u32 base_vertex);
            void DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, BufferAddress index_buffer_address, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count);
#else
            void DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 index_count, u32 base_vertex);
            void DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuAddress index_buffer_address, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count);
#endif

            void DrawMeshTasks(u32 x, u32 y, u32 z);

            void Dispatch(u32 x, u32 y, u32 z);

#ifndef AWN_ENABLE_VK_DEVICE_ADDRESS_COMMANDS
            void SetVertexBuffer(BufferAddress vertex_buffer_address, u32 binding, u32 stride, u32 size);
#else
            void SetVertexBuffer(u32 binding, GpuAddress gpu_address, size_t size);
            void SetVertexBuffers(u32 base_binding, const GpuAddressRange *gpu_address_range_array, u32 count);
#endif

            void SetShader(const Shader *shader);

            void SetResourceSet(const ResourceSet *resource_set);

            void SetBlendState(const BlendState *blend_state);
            void SetDepthStencilState(const DepthStencilState *depth_stencil_state);
            void SetRasterizerState(const RasterizerState *rasterizer_state);
            void SetVertexState(const VertexState *vertex_state);
            void SetViewportState(const ViewportState *viewport_state);
            void SetViewportScissorState(const ViewportScissorState *viewport_scissor_state);

            void SetLineWidth(float line_width);
        public:
            void SetSystemDescriptorBuffers();
        public:
            constexpr ALWAYS_INLINE VkCommandBuffer GetVkCommandBuffer() const { return m_command_list.GetVkCommandBuffer(); }
    };
}

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

    struct GpuBufferAddress {
        gfx::Buffer *buffer;
        size_t       offset;
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
            static void DestructCommandPoolHolderTls(void *arg);
        public:
            constexpr ALWAYS_INLINE  CommandPoolManager() : m_command_pool_tls_slot(0) {/*...*/}
            constexpr ALWAYS_INLINE ~CommandPoolManager() {/*...*/}

            void Initialize();
            void Finalize();

            CommandList CreateThreadLocalCommandList(QueueType queue_type, bool is_primary = false);
            void FinalizeThreadLocalCommandList(CommandList command_list);

            void FreeCurrentThreadCommandPools();
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
            void TransitionRenderTargetsImpl(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target, SyncScopeInfo *sync_scope, VkImageLayout image_layout);
        public:
            constexpr ALWAYS_INLINE CommandBufferBase() : m_command_list() {/*...*/}
            constexpr ~CommandBufferBase() {/*...*/}

            void TransitionRenderTargetsToAttachment(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target);
            void TransitionRenderTargetsToPresent(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target);

            void BeginRendering(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target);
            void EndRendering();

            void Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count);
            void DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count);
            void DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuBufferAddress index_buffer_address, u32 base_index, u32 index_count, u32 base_vertex);
            void DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuBufferAddress index_buffer_address, u32 base_index, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count);

            void DrawMeshTasks(u32 x, u32 y, u32 z);
            
            void Dispatch(u32 x, u32 y, u32 z);

            void SetVertexBuffer(GpuBufferAddress vertex_buffer_address, u32 binding, u32 stride, u32 size);

            void SetShader(Shader *shader);

            void SetUniformBuffer(u32 location, gfx::ShaderStage shader_stage_flags, GpuBufferAddress constant_buffer_address, size_t size);
            void SetStorageBuffer(u32 location, gfx::ShaderStage shader_stage_flags, GpuBufferAddress storage_buffer_address, size_t size);
            void SetTextureAndSampler(u32 location, gfx::ShaderStage shader_stage_flags, DescriptorSlot texture_slot, DescriptorSlot sampler_slot);

            void SetBlendState(BlendState *blend_state);
            void SetDepthStencilState(DepthStencilState *depth_stencil_state);
            void SetRasterizerState(RasterizerState *rasterizer_state);
            void SetVertexState(VertexState *vertex_state);
            void SetViewportScissorState(ViewportScissorState *viewport_scissor_state);

            void SetLineWidth(float line_width);
        public:
            constexpr ALWAYS_INLINE VkCommandBuffer GetVkCommandBuffer() const { return m_command_list.vk_command_buffer; }
    };

    class CommandBuffer : public CommandBufferBase {
        public:
            constexpr ALWAYS_INLINE CommandBuffer() : CommandBufferBase() {/*...*/}
            constexpr ~CommandBuffer() {/*...*/}

            void        Begin(QueueType queue_type = QueueType::Graphics, bool is_primary = false);
            CommandList End();
    };
}

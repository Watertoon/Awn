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

    struct GpuBufferAddress {
        gfx::Buffer *buffer;
        size_t       offset;

        constexpr VkDeviceAddress GetVkDeviceAddress() const {
            return buffer->GetVkDeviceAddress() + offset;
        }
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
            CommandList               m_command_list;
            bool                      m_is_rendering;
            bool                      m_is_render_target_dirty;
            void                     *m_command_memory_base;
            size_t                    m_command_memory_base_offset;
            u32                       m_command_memory_iter;
            u32                       m_command_memory_size;
            u32                       m_color_target_count;
            RenderTargetColor        *m_color_target_array[Context::cTargetMaxBoundRenderTargetColorCount];
            RenderTargetDepthStencil *m_depth_stencil_target;
            struct {
                u8 m_texture_sampler_dirty;
                struct {
                    u8 m_texture_sampler_dirty_vertex   : 1;
                    u8 m_texture_sampler_dirty_geometry : 1;
                    u8 m_texture_sampler_dirty_fragment : 1;
                    u8 m_texture_sampler_dirty_compute  : 1;
                    u8 m_texture_sampler_dirty_task     : 1;
                    u8 m_texture_sampler_dirty_mesh     : 1;
                    u8 m_reserve0                       : 2;
                };
            };
            struct {
                u8 m_image_dirty;
                struct {
                    u8 m_image_dirty_vertex   : 1;
                    u8 m_image_dirty_geometry : 1;
                    u8 m_image_dirty_fragment : 1;
                    u8 m_image_dirty_compute  : 1;
                    u8 m_image_dirty_task     : 1;
                    u8 m_image_dirty_mesh     : 1;
                    u8 m_reserve1             : 2;
                };
            };
            struct {
                u8 m_uniform_buffer_dirty;
                struct {
                    u8 m_uniform_buffer_dirty_vertex   : 1;
                    u8 m_uniform_buffer_dirty_geometry : 1;
                    u8 m_uniform_buffer_dirty_fragment : 1;
                    u8 m_uniform_buffer_dirty_compute  : 1;
                    u8 m_uniform_buffer_dirty_task     : 1;
                    u8 m_uniform_buffer_dirty_mesh     : 1;
                    u8 m_reserve2                      : 2;
                };
            };
            struct {
                u8 m_storage_buffer_dirty;
                struct {
                    u8 m_storage_buffer_dirty_vertex   : 1;
                    u8 m_storage_buffer_dirty_geometry : 1;
                    u8 m_storage_buffer_dirty_fragment : 1;
                    u8 m_storage_buffer_dirty_compute  : 1;
                    u8 m_storage_buffer_dirty_task     : 1;
                    u8 m_storage_buffer_dirty_mesh     : 1;
                    u8 m_reserve3                      : 2;
                };
            };
            struct {
                u8 m_combined_dirty;
                struct {
                    u8 m_combined_dirty_tessellation_control    : 1;
                    u8 m_combined_dirty_tessellation_evaluation : 1;
                    u8 m_reserve4                               : 6;
                };
            };
            struct {
                u8 m_separate_texture_sampler_dirty;
                struct {
                    u8 m_separate_texture_sampler_dirty_vertex   : 1;
                    u8 m_separate_texture_sampler_dirty_geometry : 1;
                    u8 m_separate_texture_sampler_dirty_fragment : 1;
                    u8 m_separate_texture_sampler_dirty_compute  : 1;
                    u8 m_separate_texture_sampler_dirty_task     : 1;
                    u8 m_separate_texture_sampler_dirty_mesh     : 1;
                    u8 m_reserve5                                : 6;
                };
            };
            char m_descriptor_memory_texture_sampler[6][Context::cTargetMaxPerStageMemorySizeTextureSampler];
            char m_descriptor_memory_image[6][Context::cTargetMaxPerStageMemorySizeImage];
            char m_descriptor_memory_uniform_buffer[6][Context::cTargetMaxPerStageMemorySizeUniformBuffer];
            char m_descriptor_memory_storage_buffer[6][Context::cTargetMaxPerStageMemorySizeStorageBuffer];
            char m_descriptor_memory_combined[2][Context::cTargetMaxPerStageMemorySizeCombined];
            char m_descriptor_memory_separate_texture_sampler[8][Context::cTargetMaxPerStageMemorySizeSeparateTextureSampler];
        protected:
            struct SyncScopeInfo {
                u32 vk_src_stage_mask;
                u32 vk_src_access_mask;
                u32 vk_dst_stage_mask;
                u32 vk_dst_access_mask;
            };
        protected:
            constexpr void *GetDescriptorMemoryTextureSampler(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                void      *memory;
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    memory = this->GetDescriptorMemoryCombined(shader_stage);
                } else {
                    memory = (0 < stage_index) ? m_descriptor_memory_texture_sampler[stage_index - 2] : m_descriptor_memory_texture_sampler[0];
                }
                return memory;
            }
            constexpr void *GetDescriptorMemoryImage(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                void      *memory;
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    memory = this->GetDescriptorMemoryCombined(shader_stage);
                } else {
                    memory = (0 < stage_index) ? m_descriptor_memory_texture_sampler[stage_index - 2] : m_descriptor_memory_texture_sampler[0];
                }
                return memory;
            }
            constexpr void *GetDescriptorMemoryUniformBuffer(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                void      *memory;
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    memory = this->GetDescriptorMemoryCombined(shader_stage);
                } else {
                    memory = (0 < stage_index) ? m_descriptor_memory_texture_sampler[stage_index - 2] : m_descriptor_memory_texture_sampler[0];
                }
                return memory;
            }
            constexpr void *GetDescriptorMemoryStorageBuffer(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                void      *memory;
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    memory = this->GetDescriptorMemoryCombined(shader_stage);
                } else {
                    memory = (0 < stage_index) ? m_descriptor_memory_texture_sampler[stage_index - 2] : m_descriptor_memory_texture_sampler[0];
                }
                return memory;
            }
            constexpr void *GetDescriptorMemoryCombined(ShaderStage shader_stage) {
                const u32 stage_index = static_cast<u32>(shader_stage);
                return m_descriptor_memory_combined[stage_index - 1];
            }
            constexpr void *GetDescriptorMemorySeparateTextureSampler(ShaderStage shader_stage) {
                const u32 stage_index = static_cast<u32>(shader_stage);
                return m_descriptor_memory_texture_sampler[stage_index];
            }

            constexpr void SetTextureSamplerDirty(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    m_combined_dirty |= (1 << (stage_index - 1));
                } else {
                    const u32 dirty_bit      = (0 < stage_index) ? (1 << (stage_index - 2)) : 1;
                    m_texture_sampler_dirty |= dirty_bit;
                }
                return;
            }
            constexpr void SetImageDirty(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    m_combined_dirty |= (1 << (stage_index - 1));
                } else {
                    const u32 dirty_bit  = (0 < stage_index) ? (1 << (stage_index - 2)) : 1;
                    m_image_dirty       |= dirty_bit;
                }
                return;
            }
            constexpr void SetUniformBufferDirty(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    m_combined_dirty |= (1 << (stage_index - 1));
                } else {
                    const u32 dirty_bit     = (0 < stage_index) ? (1 << (stage_index - 2)) : 1;
                    m_uniform_buffer_dirty |= dirty_bit;
                }
                return;
            }
            constexpr void SetStorageBufferDirty(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    m_combined_dirty |= (1 << (stage_index - 1));
                } else {
                    const u32 dirty_bit     = (0 < stage_index) ? (1 << (stage_index - 2)) : 1;
                    m_storage_buffer_dirty |= dirty_bit;
                }
                return;
            }
            constexpr void SetSeparateTextureSamplerDirty(ShaderStage shader_stage) {
                const u32  stage_index = static_cast<u32>(shader_stage);
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    m_combined_dirty |= (1 << (stage_index - 1));
                } else {
                    const u32 dirty_bit               = (0 < stage_index) ? (1 << (stage_index - 2)) : 1;
                    m_separate_texture_sampler_dirty |= dirty_bit;
                }
                return;
            }

            constexpr void ResetDescriptorMemory() {
                m_command_memory_iter = 0;
            }

            void FreeCommandMemory() {
                if (m_command_memory_base == nullptr) { return; }
                DescriptorMemoryManager::GetInstance()->FreeCommandMemory(m_command_memory_base);
                m_command_memory_base = nullptr;
                m_command_memory_base_offset = 0;
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
            constexpr CommandBufferBase() : m_command_list(), m_is_rendering(), m_is_render_target_dirty(), m_command_memory_base(), m_command_memory_base_offset(), m_command_memory_iter(), m_command_memory_size(), m_color_target_count(), m_color_target_array(), m_depth_stencil_target(), m_texture_sampler_dirty(), m_image_dirty(), m_uniform_buffer_dirty(), m_storage_buffer_dirty(), m_combined_dirty(), m_separate_texture_sampler_dirty() {/*...*/}
            constexpr ~CommandBufferBase() {/*...*/}

            bool SetCommandMemorySize(size_t command_memory_size) {
                this->FreeCommandMemory();
                if (command_memory_size == 0) { return true; }
                void *memory = DescriptorMemoryManager::GetInstance()->AllocateCommandMemory(std::addressof(m_command_memory_base_offset), command_memory_size);
                if (memory == nullptr) { return false; }
                m_command_memory_base        = memory;
                m_command_memory_iter        = 0;
                m_command_memory_size        = command_memory_size;
                return true;
            }
        public:
            void TransitionRenderTargetsToAttachment(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target);
            void TransitionRenderTargetsToPresent(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target);

            void BlitImage(Texture *dest_texture, const TextureCopyRegion *dest_copy_region, Texture *source_texture, const TextureCopyRegion *source_copy_region, CopyFilter copy_filter);

            void SetRenderTargets(RenderTargetColor **color_target_array, u32 color_target_count, RenderTargetDepthStencil *depth_stencil_target);

            void Draw(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count);
            void DrawInstanced(PrimitiveTopology primitive_topology, u32 base_vertex, u32 vertex_count, u32 base_instance, u32 instance_count);
            void DrawIndexed(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuBufferAddress index_buffer_address, u32 base_index, u32 index_count, u32 base_vertex);
            void DrawIndexedInstanced(PrimitiveTopology primitive_topology, IndexFormat index_format, GpuBufferAddress index_buffer_address, u32 base_index, u32 index_count, u32 base_vertex, u32 base_instance, u32 instance_count);

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

    class CommandBuffer : public CommandBufferBase {
        public:
            constexpr ALWAYS_INLINE CommandBuffer() : CommandBufferBase() {/*...*/}
            constexpr ~CommandBuffer() {/*...*/}

            void        Begin(QueueType queue_type = QueueType::Graphics, bool is_primary = false);
            CommandList End();
    };
}

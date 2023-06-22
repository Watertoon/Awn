#pragma once

namespace awn::gfx {

    using DescriptorSlot = u64;
    constexpr inline DescriptorSlot cInvalidDescriptorSlot = 0;
    
    class CommandBufferBase;

    class TextureSamplerManager {
        public:
            friend class CommandBuffer;
        public:
            struct SamplerNode {
                vp::util::IntrusiveRedBlackTreeNode<u32> rb_node;
                u32                                      reference_count;
                u32                                      handle;
                Sampler                                  sampler;

                constexpr SamplerNode() {/*...*/}
                constexpr ~SamplerNode() {/*...*/}
            };
            struct TextureNode {
                u32         reference_count;
                Texture     texture;
                TextureView texture_view;

                constexpr TextureNode() {/*...*/}
                constexpr ~TextureNode() {/*...*/}
            };
        public:
            using TextureHandleTable = vp::util::FixedHandleTable<Context::cTargetMaxTextureDescriptorCount>;
            using SamplerHandleTable = vp::util::FixedHandleTable<Context::cTargetMaxSamplerDescriptorCount>;
            using TextureAllocator   = vp::util::FixedObjectAllocator<TextureNode, Context::cTargetMaxTextureDescriptorCount>;
            using SamplerAllocator   = vp::util::FixedObjectAllocator<SamplerNode, Context::cTargetMaxSamplerDescriptorCount>;
            using SamplerMap         = vp::util::IntrusiveRedBlackTreeTraits<SamplerNode, u64, &SamplerNode::rb_node>::Tree;
        private:
            GpuMemoryAllocation          m_descriptor_gpu_memory_allocation;
            GpuMemoryAddress             m_texture_descriptor_gpu_address;
            GpuMemoryAddress             m_sampler_descriptor_gpu_address;
            VkBuffer                     m_texture_descriptor_vk_buffer;
            VkBuffer                     m_sampler_descriptor_vk_buffer;
            VkDeviceAddress              m_texture_vk_device_address;
            VkDeviceAddress              m_sampler_vk_device_address;
            void                        *m_texture_descriptor_buffer_address;
            void                        *m_sampler_descriptor_buffer_address;
            u16                          m_texture_descriptor_size;
            u16                          m_sampler_descriptor_size;
            u16                          m_texture_descriptor_stride;
            u16                          m_sampler_descriptor_stride;
            TextureHandleTable           m_texture_handle_table;
            SamplerHandleTable           m_sampler_handle_table;
            SamplerMap                   m_sampler_map;
            TextureAllocator             m_texture_allocator;
            SamplerAllocator             m_sampler_allocator;
            sys::ServiceCriticalSection  m_texture_allocator_critical_section;
            sys::ServiceCriticalSection  m_sampler_allocator_critical_section;
        public:
            AWN_SINGLETON_TRAITS(TextureSamplerManager);
        public:
            constexpr TextureSamplerManager() : m_descriptor_gpu_memory_allocation(), m_texture_descriptor_gpu_address(), m_sampler_descriptor_gpu_address(), m_texture_descriptor_vk_buffer(), m_sampler_descriptor_vk_buffer(), m_texture_vk_device_address(), m_sampler_vk_device_address(), m_texture_descriptor_buffer_address(), m_sampler_descriptor_buffer_address(), m_texture_descriptor_size(), m_sampler_descriptor_size(), m_texture_descriptor_stride(), m_sampler_descriptor_stride(), m_texture_handle_table(), m_sampler_handle_table(), m_sampler_map(), m_texture_allocator(), m_sampler_allocator(), m_texture_allocator_critical_section(), m_sampler_allocator_critical_section() {/*...*/}
            constexpr ~TextureSamplerManager() {/*...*/}

            void Initialize(mem::Heap *heap);
            void Finalize();

            DescriptorSlot RegisterTextureView(GpuMemoryAddress gpu_texture_memory, TextureInfo *texture_info, TextureViewInfo *texture_view);
            DescriptorSlot RegisterTextureView(DescriptorSlot texture_slot);
            DescriptorSlot RegisterSampler(SamplerInfo *sampler_info);
            DescriptorSlot RegisterSampler(DescriptorSlot sampler_slot);

            void           UnregisterTextureView(DescriptorSlot texture_slot);
            void           UnregisterSampler(DescriptorSlot sampler_slot);

            void BindDescriptorBuffers(CommandBufferBase *command_buffer);

            Texture     *TryGetTextureByHandle(DescriptorSlot texture_slot);
            TextureView *TryGetTextureViewByHandle(DescriptorSlot texture_slot);
            Sampler     *TryGetSamplerByHandle(DescriptorSlot sampler_slot);
    };
}

#pragma once

namespace awn::gfx {

    namespace {

        static ALWAYS_INLINE size_t CalculateTextureDescriptorSetLayoutGpuSize() {

            /* Get size of each descriptor set layout */
            size_t texture_layout_size = 0;
            ::pfn_vkGetDescriptorSetLayoutSizeEXT(Context::GetInstance()->GetVkDevice(), Context::GetInstance()->GetTextureVkDescriptorSetLayout(), std::addressof(texture_layout_size));

            return texture_layout_size;
        }

        static ALWAYS_INLINE size_t CalculateSamplerDescriptorSetLayoutGpuSize() {

            /* Get size of each descriptor set layout */
            size_t sampler_layout_size = 0;
            ::pfn_vkGetDescriptorSetLayoutSizeEXT(Context::GetInstance()->GetVkDevice(), Context::GetInstance()->GetSamplerVkDescriptorSetLayout(), std::addressof(sampler_layout_size));

            return sampler_layout_size;
        }
    }

    using DescriptorSlot = u64;
    constexpr inline DescriptorSlot cInvalidDescriptorSlot = 0;
    
    class CommandBuffer;

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
            VkBuffer                     m_texture_descriptor_buffer;
            VkBuffer                     m_sampler_descriptor_buffer;
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
            SamplerAllocator             m_sample_allocator;
            sys::ServiceCriticalSection  m_texture_critical_section;
            sys::ServiceCriticalSection  m_sampler_critical_section;
        public:
            AWN_SINGLETON_TRAITS(TextureSamplerManager);
        public:
            constexpr TextureSamplerManager() {/*...*/}

            void Initialize(mem::Heap *heap);
            void Finalize();

            DescriptorSlot RegisterTextureView(GpuMemoryAddress gpu_texture_memory, TextureInfo *texture_info, TextureViewInfo *texture_view);
            DescriptorSlot RegisterTextureView(DescriptorSlot texture_slot);
            DescriptorSlot RegisterSampler(SamplerInfo *sampler_info);
            DescriptorSlot RegisterSampler(DescriptorSlot sampler_slot);

            void           UnregisterTextureView(DescriptorSlot texture_slot);
            void           UnregisterSampler(DescriptorSlot sampler_slot);

            void BindDescriptorBuffers(CommandBuffer *command_buffer);

            Texture     *TryGetTextureByHandle(DescriptorSlot texture_slot);
            TextureView *TryGetTextureViewByHandle(DescriptorSlot texture_slot);
            Sampler     *TryGetSamplerByHandle(DescriptorSlot sampler_slot);
    };
}

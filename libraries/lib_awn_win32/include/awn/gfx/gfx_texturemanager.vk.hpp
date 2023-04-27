#pragma once

namespace awn::gfx {

    namespace {

        static size_t CalculateTextureDescriptorSetLayoutGpuSize() {

            /* Get size of each descriptor set layout */
            size_t texture_layout_size = 0;
            ::pfn_vkGetDescriptorSetLayoutSizeEXT(Context::GetInstance()->GetDevice(), Context::GetInstance()->GetTextureDescriptorSetLayout(), std::addressof(texture_layout_size));

            return texture_layout_size;
        }

        static size_t CalculateSamplerDescriptorSetLayoutGpuSize() {

            /* Get size of each descriptor set layout */
            size_t sampler_layout_size = 0;
            ::pfn_vkGetDescriptorSetLayoutSizeEXT(Context::GetInstance()->GetDevice(), Context::GetInstance()->GetSamplerDescriptorSetLayout(), std::addressof(sampler_layout_size));

            return sampler_layout_size;
        }
    }

    using DescriptorSlot = u64;
    constexpr inline DescriptorSlot cInvalidDescriptorSlot = 0;

    struct TextureSamplerInfo {
        GpuMemoryAddress  texture_gpu_memory;
        TextureInfo      *texture_info;
        TextureViewInfo  *texture_view_info;
        SamplerInfo      *sampler_info;
    };

    class TextureSamplerBinder {
        private:
            TextureBinder m_texture_binder;
            SamplerBinder m_sampler_binder;
        public:
            constexpr ALWAYS_INLINE TextureSampler() : m_texture(), m_texture_binder(), m_sampler_binder() {/*...*/}

            TextureBinder &operator=(const TextureBinder &rhs) {
                this->BindTexture(rhs.m_texture_slot);
                this->BindTexture(rhs.m_texture_slot);
            }

            void BindTextureSampler(TextureSamplerInfo *texture_sampler_info) {
                m_texture_binder.BindTexture(texture_sampler_info->texture_gpu_memory, texture_sampler_info->texture_info, texture_sampler_info->texture_view_info);
                m_sampler_binder.BindSampler(texture_sampler_info->sampler_info);
            }
            
            void ReleaseTextureSampler() {
                m_texture_binder.ReleaseTexture();
                m_sampler_binder.ReleaseSampler();
            }
    };
    class TextureBinder {
        public:
            DescriptorSlot m_texture_slot;
        public:
            constexpr ALWAYS_INLINE TextureBinder() : m_texture_slot(cInvalidDescriptorSlot) {/*...*/}

            TextureBinder &operator=(const TextureBinder &rhs) {
                this->BindTexture(rhs.m_texture_slot);
            }

            void BindTexture(GpuMemoryAddress texture_gpu_memory_address, TextureInfo *texture_info, TextureViewInfo *texture_view_info) {
                this->ReleaseTexture();
                m_texture_slot = TextureSamplerManager::GetInstance()->RegisterTextureView(texture_gpu_memory_address, texture_info, texture_view_info);
            }

            void BindTexture(DescriptorSlot texture_slot) {
                this->ReleaseTexture();
                m_texture_slot = TextureSamplerManager::GetInstance()->RegisterTextureView(texture_slot);
            }

            void ReleaseTexture() {
                if (m_texture_slot != cInvalidDescriptorSlot) { TextureSamplerManager::GetInstance()->UnregisterTextureView(m_texture_slot); m_texture_slot = cInvalidDescriptorSlot; }
            }

            Texture *GetTexture() {
                return TextureSamplerManager::GetInstance()->TryGetTextureByHandle(m_sampler_slot);
            }

            TextureView *GetTextureView() {
                return TextureSamplerManager::GetInstance()->TryGetTextureViewByHandle(m_sampler_slot);
            }
    };
    class SamplerBinder {
        public:
            DescriptorSlot m_sampler_slot;
        public:
            constexpr ALWAYS_INLINE SamplerBinder() : m_sampler_slot(cInvalidDescriptorSlot) {/*...*/}
            
            TextureBinder &operator=(const TextureBinder &rhs) {
                this->BindSampler(rhs.m_sampler_slot);
            }

            void BindSampler(SamplerInfo *sampler_info) {
                this->ReleaseSampler();
                m_sampler_slot = TextureSamplerManager::GetInstance()->RegisterSampler(sampler_info);
            }

            void BindSampler(DescriptorSlot sampler_slot) {
                this->ReleaseSampler();
                m_sampler_slot = TextureSamplerManager::GetInstance()->RegisterTextureView(sampler_slot);
            }

            void ReleaseSampler() {
                if (m_sampler_slot != cInvalidDescriptorSlot) { TextureSamplerManager::GetInstance()->UnregisterTextureView(m_sampler_slot); m_sampler_slot = cInvalidDescriptorSlot; }
            }

            Sampler *GetSampler() {
                return TextureSamplerManager::GetInstance()->TryGetSamplerByHandle(m_sampler_slot);
            }
    };

    class TextureSamplerManager {
        public:
            friend class CommandBuffer;
        public:
            struct SamplerNode {
                util::IntrusiveRedBlackTreeNode<u32> rb_node;
                u32                                  reference_count;
                u32                                  handle;
                Sampler                              sampler;
            };
            struct TextureNode {
                u32         reference_count;
                Texture     texture;
                TextureView texture_view;
            };
        public:
            using TextureHandleTable = vp::util::FixedHandleTable<Context::TargetMaxTextureDescriptors>;
            using SamplerHandleTable = vp::util::FixedHandleTable<Context::TargetMaxSamplerDescriptors>;
            using TextureAllocator   = vp::util::FixedObjectAllocator<TextureNode, Context::TargetMaxTextureDescriptors>;
            using SamplerAllocator   = vp::util::FixedObjectAllocator<SamplerNode, Context::TargetMaxSamplerDescriptors>;
        private:
            GpuMemoryAllocation   m_descriptor_gpu_memory_allocation;
            GpuMemoryAddress      m_texture_descriptor_gpu_address;
            GpuMemoryAddress      m_sampler_descriptor_gpu_address;
            VkBuffer              m_texture_descriptor_buffer;
            VkBuffer              m_sampler_descriptor_buffer;
            void                 *m_texture_descriptor_buffer_address;
            void                 *m_sampler_descriptor_buffer_address;
            u32                   m_texture_descriptor_size;
            u32                   m_sampler_descriptor_size;
            TextureHandleTable    m_texture_handle_table;
            SamplerHandleTable    m_sampler_handle_table;
            SamplerMap            m_sampler_map;
            TextureAllocator      m_texture_allocator;
            SamplerAllocator      m_sample_allocator;
            sys::CriticalSection  m_texture_critical_section;
            sys::CriticalSection  m_sampler_critical_section;
        public:
            VP_SINGLETON_TRAITS(TextureSamplerManager);
        public:
            constexpr TextureManager() {/*...*/}

            void Initialize(TextureSamplerManagerInfo *manager_info) {

                /* Allocate descriptor memory */
                const size_t texture_descriptor_memory_size = vp::util::AlignUp(CalculateTextureDescriptorSetLayoutGpuSize(), Context::TargetDescriptorBufferAlignment);
                const size_t sampler_descriptor_memory_size   = vp:util::AlignUp(CalculateSamplerDescriptorSetLayoutGpuSize(), Context::TargetDescriptorBufferAlignment);
                m_descriptor_gpu_memory_allocation.TryAllocate(texture_descriptor_memory_size + sampler_descriptor_memory_size, Context::TargetDescriptorBufferAlignment, MemoryPropertyFlags::CpuUncached | MemoryPropertyFlags::GpuUncached);

                /* Get memory addresses */
                m_texture_descriptor_gpu_address = m_descriptor_gpu_memory_allocation.GetGpuAddress(0);
                m_sampler_descriptor_gpu_address = m_descriptor_gpu_memory_allocation.GetGpuAddress(texture_descriptor_memory_size);

                /* Create descriptor buffers */
                m_texture_descriptor_buffer = m_texture_descriptor_gpu_address.CreateBuffer(VK_BUFFER_USAGE_TEXTURE_DESCRIPTOR_BUFFER_BIT_EXT, texture_descriptor_memory_size);
                m_sampler_descriptor_buffer = m_sampler_descriptor_gpu_address.CreateBuffer(VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, sampler_descriptor_memory_size);
                
                /* Map descriptor buffers */
                
                /* Get size of descriptors */
                ::pfn_vkGetDescriptorSizeEXT();
                VP_ASSERT(m_texture_descriptor_size <= 0x40 && m_sampler_descriptor_size <= 0x40);
            }

            void Finalize() {

                /* Destroy descriptor buffers */
                ::pfn_vkDestroyBuffer(Context::GetInstance()->GetVkDevice(), m_texture_descriptor_buffer, Context::GetInstance()->GetVkAllocationCallbacks());
                ::pfn_vkDestroyBuffer(Context::GetInstance()->GetVkDevice(), m_sampler_descriptor_buffer, Context::GetInstance()->GetVkAllocationCallbacks());
                m_texture_descriptor_buffer = VK_NULL_HANDLE;
                m_sampler_descriptor_buffer = VK_NULL_HANDLE;

                /* Free gpu memory */
                m_descriptor_gpu_memory_allocation.FreeGpuMemory();
                
                return;
            }

            DescriptorSlot RegisterTexture(GpuMemoryAddress gpu_texture_memory, TextureInfo *texture_info, TextureViewInfo *texture_view) {

                /* Integrity check infos */
                VP_ASSERT(texture_info != nullptr && texture_view != nullptr && texture_view->texture == nullptr);

                /* Texture management Lock */
                std::scoped_lock l(m_texture_handle_table_critical_section);

                /* Allocate texture node */
                TextureNode *new_texture_node = m_texture_allocator.Allocate();
                VP_ASSERT(new_texture_node != nullptr);

                /* Initialize texture objects */
                new_texture_node->texture.Initialize(gpu_texture_memory, texture_info);
                texture_view_info->texture = std::addressof(new_texture_node->texture);
                new_texture_node->texture_view.Initialize(texture_view);

                /* Register texture node to descriptor buffer */
                char descriptor_storage[Context::TargetMaxTextureDescriptorSize] = {};
                const VkDescriptorImageInfo descriptor_image_info = {
                    .imageView   = new_texture_node->texture_view.GetVkTextureView(),
                    .imageLayout = VK_IMAGE_LAYOUT_GENERAL
                };
                const VkDescriptorGetInfoEXT descriptor_get_info = {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
                    .type  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .data  = std::addressof(descriptor_image_info)
                }; 
                ::pfn_vkGetDescriptorDataEXT(Context::GetInstance()->GetVkDevice(), std::addressof(descriptor_get_info), sizeof(descriptor_storage), descriptor_storage);
                ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_texture_descriptor_buffer_address) + vp::util::AlignUp(Context::GetInstance()->GetTextureDescriptorSize(), Context::GetInstance()->GetDescriptorAlignment())), descriptor_storage, m_texture_descriptor_size);

                /* Allocate texture handle */
                u32 texture_handle = 0;
                bool result = m_texture_handle_table.ReserveHandle(std::addressof(texture_handle), new_texture_node);
                VP_ASSERT(result == true);

                return texture_handle;
            }

            DescriptorSlot RegisterTexture(DescriptorSlot texture_slot) {

                /* Texture management Lock */
                std::scoped_lock l(m_texture_handle_table_critical_section);

                /* Get texture node */
                TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
                VP_ASSERT(texture_node != nullptr);

                /* Increment reference count */
                ::InterlockedIncrement(texture_node->reference_count);
                
                return texture_slot;
            }

            DescriptorSlot RegisterSampler(SamplerInfo *sampler_info) {

                /* Hash sampler info */
                const u32 sampler_hash = HashSampler(sampler_info);

                /* Sampler map lock */
                std::scoped_lock l(m_sampler_map_critical_section);

                /* Try to find existing sampler */
                SamplerNode *sampler_node = m_sampler_map.Find(sampler_hash);

                if (sampler_node == nullptr) {

                    /* Allocate new sampler */
                    sampler_node = m_sampler_allocator.Allocate();
                    VP_ASSERT(sampler_node != nullptr);

                    /* Initialize sampler */
                    sampler_node->sampler.Initialize(sampler_info);

                    /* Register sampler node to descriptor buffer */
                    char descriptor_storage[Context::TargetMaxSamplerDescriptorSize] = {};
                    const VkSampler vk_sampler = sampler_node->sampler.GetVkSampler();
                    const VkDescriptorGetInfoEXT descriptor_get_info = {
                        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
                        .type  = VK_DESCRIPTOR_TYPE_SAMPLER,
                        .data  = std::addressof(vk_sampler)
                    }; 
                    ::pfn_vkGetDescriptorDataEXT(Context::GetInstance()->GetVkDevice(), std::addressof(descriptor_get_info), sizeof(descriptor_storage), descriptor_storage);
                    ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_sampler_descriptor_buffer_address) + vp::util::AlignUp(Context::GetInstance()->GetSamplerDescriptorSize(), Context::GetInstance()->GetDescriptorAlignment())), descriptor_storage, m_sampler_descriptor_size);

                    /* Allocate sampler handle */
                    u32 sampler_handle = 0;
                    bool result = m_texture_handle_table.ReserveHandle(std::addressof(sampler_handle), sampler_node);
                    VP_ASSERT(result == true);
                    sampler_node->handle = sampler_handle;

                    /* Insert sampler node */
                    sampler_node->rb_node.SetKey(sampler_hash);
                    m_sampler_map.Insert(sampler_node);
                }

                /* Increment reference count */
                ::InterlockedIncrement(sampler_node->reference_count);

                return sampler_node->handle;
            }

            DescriptorSlot RegisterSampler(DescriptorSlot sampler_slot) {

                /* Get sampler node */
                SamplerNode *sampler_node = m_sampler_table.GetObjectByHandle(sampler_slot);
                VP_ASSERT(sampler_node != nullptr);

                /* Increment reference count */
                ::InterlockedIncrement(sampler_node->reference_count);

                return sampler_slot;
            }

            void UnregisterTexture(DescriptorSlot texture_slot) {

                /* Get texture node */
                TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
                VP_ASSERT(texture_node != nullptr);

                /* Decrement ref count */
                const u32 last_value = ::InterlockedDecrement();

                /* Destruct if value is now 0 */
                if (last_value != 1) { return; }

                /* Free texture handle */
                const bool result = m_texture_handle_table.FreeHandle(texture_slot);
                VP_ASSERT(result == true);

                /* Finalize texture objects */
                texture_node->texture_view.Finalize();
                texture_node->texture.Finalize();

                /* Free texture node to allocator */
                m_texture_allocator.Free(texture_node);

                return;
            }

            void UnregisterSampler(DescriptorSlot sampler_slot) {

                /* Get sampler node */
                SamplerNode *sampler_node = m_sampler_handle_table.GetObjectByHandle(sampler_slot);
                VP_ASSERT(sampler_node != nullptr);

                /* Decrement ref count */
                const u32 last_value = ::InterlockedDecrement(sampler->reference_count);

                /* Destruct on finish */
                if (last_value != 1) { return; }

                /* Remove sampler from tree */
                m_sampler_map.Remove(sampler_node->rb_node);

                /* Free sampler handle */
                const bool result = m_sampler_handle_table.FreeHandle(sampler_slot);
                VP_ASSERT(result == true);

                /* Finalize sampler */
                sampler_node->sampler.Finalize();

                /* Free sampler to allocator */
                m_sampler_allocator.Free(sampler_node);

                return;
            }

            void BindDescriptorBuffers(CommandBuffer *command_buffer) {

                /* Set descriptor buffers */
                command_buffer->SetDescriptorPool(m_texture_descriptor_buffer);
                command_buffer->SetDescriptorPool(m_sampler_descriptor_buffer);
            }

            void WriteToTexture(DescriptorSlot texture_slot) {
                
                
            }
    };
}

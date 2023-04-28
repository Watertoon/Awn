#include <awn.hpp>

namespace awn::gfx {

    void TextureSamplerManager::Initialize(TextureSamplerManagerInfo *manager_info) {

        /* Allocate descriptor memory */
        const size_t texture_descriptor_memory_size = vp::util::AlignUp(CalculateTextureDescriptorSetLayoutGpuSize(), Context::cTargetDescriptorBufferAlignment);
        const size_t sampler_descriptor_memory_size   = vp:util::AlignUp(CalculateSamplerDescriptorSetLayoutGpuSize(), Context::cTargetDescriptorBufferAlignment);
        m_descriptor_gpu_memory_allocation.TryAllocate(texture_descriptor_memory_size + sampler_descriptor_memory_size, Context::cTargetDescriptorBufferAlignment, MemoryPropertyFlags::CpuUncached | MemoryPropertyFlags::GpuUncached);

        /* Get Gpu memory addresses */
        m_texture_descriptor_gpu_address = m_descriptor_gpu_memory_allocation.GetGpuMemoryAddress(0);
        m_sampler_descriptor_gpu_address = m_descriptor_gpu_memory_allocation.GetGpuMemoryAddress(texture_descriptor_memory_size);

        /* Create descriptor buffers */
        m_texture_descriptor_buffer = m_texture_descriptor_gpu_address.CreateBuffer(VK_BUFFER_USAGE_TEXTURE_DESCRIPTOR_BUFFER_BIT_EXT, texture_descriptor_memory_size);
        m_sampler_descriptor_buffer = m_sampler_descriptor_gpu_address.CreateBuffer(VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, sampler_descriptor_memory_size);
        
        /* Map descriptor buffers */
        m_texture_descriptor_buffer_address = m_descriptor_gpu_memory_allocation.Map();
        m_sampler_descriptor_buffer_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_texture_descriptor_buffer_address) + texture_descriptor_memory_size);

        /* Get size and stride for descriptors */
        m_texture_descriptor_size   = Context::GetInstance()->GetTextureDescriptorSize();
        m_sampler_descriptor_size   = Context::GetInstance()->GetSamplerDescriptorSize();
        m_texture_descriptor_stride = Context::GetInstance()->GetTextureDescriptorStride();
        m_sampler_descriptor_stride = Context::GetInstance()->GetSamplerDescriptorStride();
        VP_ASSERT(m_texture_descriptor_size <= 0x40 && m_sampler_descriptor_size <= 0x40);

        return;
    }

    void TextureSamplerManager::Finalize() {

        /* Destroy descriptor buffers */
        ::pfn_vkDestroyBuffer(Context::GetInstance()->GetVkDevice(), m_texture_descriptor_buffer, Context::GetInstance()->GetVkAllocationCallbacks());
        ::pfn_vkDestroyBuffer(Context::GetInstance()->GetVkDevice(), m_sampler_descriptor_buffer, Context::GetInstance()->GetVkAllocationCallbacks());
        m_texture_descriptor_buffer = VK_NULL_HANDLE;
        m_sampler_descriptor_buffer = VK_NULL_HANDLE;

        /* Free gpu memory */
        m_descriptor_gpu_memory_allocation.FreeGpuMemory();
        
        return;
    }

    DescriptorSlot TextureSamplerManager::RegisterTexture(GpuMemoryAddress gpu_texture_memory, TextureInfo *texture_info, TextureViewInfo *texture_view) {

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
        char descriptor_storage[Context::cTargetMaxTextureDescriptorSize] = {};
        const VkDescriptorImageInfo descriptor_image_info = {
            .imageView   = new_texture_node->texture_view.GetVkTextureView(),
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL
        };
        const VkDescriptorGetInfoEXT descriptor_get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .data  = std::addressof(descriptor_image_info)
        }; 
        ::pfn_vkGetDescriptorEXT(Context::GetInstance()->GetVkDevice(), std::addressof(descriptor_get_info), sizeof(descriptor_storage), descriptor_storage);
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_texture_descriptor_buffer_address) + vp::util::AlignUp(Context::GetInstance()->GetTextureDescriptorSize(), Context::GetInstance()->GetDescriptorAlignment())), descriptor_storage, m_texture_descriptor_size);

        /* Allocate texture handle */
        u32 texture_handle = 0;
        bool result = m_texture_handle_table.ReserveHandle(std::addressof(texture_handle), new_texture_node);
        VP_ASSERT(result == true);

        return texture_handle;
    }

    DescriptorSlot TextureSamplerManager::RegisterTexture(DescriptorSlot texture_slot) {

        /* Texture management Lock */
        std::scoped_lock l(m_texture_handle_table_critical_section);

        /* Get texture node */
        TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
        VP_ASSERT(texture_node != nullptr);

        /* Increment reference count */
        ::InterlockedIncrement(std::addressof(texture_node->reference_count));
        
        return texture_slot;
    }

    DescriptorSlot TextureSamplerManager::RegisterSampler(SamplerInfo *sampler_info) {

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
            char descriptor_storage[Context::cTargetMaxSamplerDescriptorSize] = {};
            const VkSampler vk_sampler = sampler_node->sampler.GetVkSampler();
            const VkDescriptorGetInfoEXT descriptor_get_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
                .type  = VK_DESCRIPTOR_TYPE_SAMPLER,
                .data  = std::addressof(vk_sampler)
            }; 
            ::pfn_vkGetDescriptorEXT(Context::GetInstance()->GetVkDevice(), std::addressof(descriptor_get_info), sizeof(descriptor_storage), descriptor_storage);
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
        ::InterlockedIncrement(std::addressof(sampler_node->reference_count));

        return sampler_node->handle;
    }

    DescriptorSlot TextureSamplerManager::RegisterSampler(DescriptorSlot sampler_slot) {

        /* Get sampler node */
        SamplerNode *sampler_node = m_sampler_table.GetObjectByHandle(sampler_slot);
        VP_ASSERT(sampler_node != nullptr);

        /* Increment reference count */
        ::InterlockedIncrement(std::addressof(sampler_node->reference_count));

        return sampler_slot;
    }

    void TextureSamplerManager::UnregisterTexture(DescriptorSlot texture_slot) {

        /* Get texture node */
        TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
        VP_ASSERT(texture_node != nullptr);

        /* Decrement ref count */
        const u32 last_value = ::InterlockedDecrement(std::addressof(texture_node->reference_count));

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

    void TextureSamplerManager::UnregisterSampler(DescriptorSlot sampler_slot) {

        /* Get sampler node */
        SamplerNode *sampler_node = m_sampler_handle_table.GetObjectByHandle(sampler_slot);
        VP_ASSERT(sampler_node != nullptr);

        /* Decrement ref count */
        const u32 last_value = ::InterlockedDecrement(std::addressof(sampler->reference_count));

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

    void TextureSamplerManager::BindDescriptorBuffers(CommandBuffer *command_buffer) {

        /* Set descriptor buffers */
        const VkDescriptorBufferBindingInfo binding_info_array = {
            {
                .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .address = m_texture_descriptor_gpu_address.GetVkDeviceAddress(),
                .usage   = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT 
            },
            {
                .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .address = m_sampler_descriptor_gpu_address.GetVkDeviceAddress(),
                .usage   = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT 
            },
        }
        ::pfn_vkCmdBindDescriptorBuffers(command_buffer->GetVkCommandBuffer(), sizeof(binding_info_array) / sizeof(VkDescriptorBufferBindingInfo), std::addressof(m_texture_descriptor_buffer));

        return;
    }


    Texture *TextureSamplerManager::TryGetTextureByHandle(DescriptorSlot texture_slot) {

        /* Lookup by handle table */
        TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
        return (texture_node != nullptr) ? texture_node->texture : nullptr;
    }

    TextureView *TextureSamplerManager::TryGetTextureViewByHandle(DescriptorSlot texture_slot) {

        /* Lookup by handle table */
        TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
        return (texture_node != nullptr) ? texture_node->texture_view : nullptr;
    }

    Sampler *TextureSamplerManager::TryGetSamplerByHandle(DescriptorSlot sampler_slot) {

        /* Lookup by handle table */
        SamplerNode *sampler_node = reinterpret_cast<SamplerNode*>(m_sampler_handle_table.GetObjectByHandle(sampler_slot));
        return (sampler_node != nullptr) ? sampler_node->sampler : nullptr;
    }
}

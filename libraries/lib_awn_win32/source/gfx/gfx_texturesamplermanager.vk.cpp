#include <awn.hpp>

namespace awn::gfx {

    namespace {

        ALWAYS_INLINE u32 HashSampler(SamplerInfo *sampler_info) {
            return vp::util::HashDataCrc32b(sampler_info, sizeof(SamplerInfo));
        }

        ALWAYS_INLINE size_t CalculateTextureDescriptorSetLayoutGpuSize() {

            /* Get size of each descriptor set layout */
            size_t texture_layout_size = 0;
            ::pfn_vkGetDescriptorSetLayoutSizeEXT(Context::GetInstance()->GetVkDevice(), Context::GetInstance()->GetTextureVkDescriptorSetLayout(), std::addressof(texture_layout_size));

            return texture_layout_size;
        }

        ALWAYS_INLINE size_t CalculateSamplerDescriptorSetLayoutGpuSize() {

            /* Get size of each descriptor set layout */
            size_t sampler_layout_size = 0;
            ::pfn_vkGetDescriptorSetLayoutSizeEXT(Context::GetInstance()->GetVkDevice(), Context::GetInstance()->GetSamplerVkDescriptorSetLayout(), std::addressof(sampler_layout_size));

            return sampler_layout_size;
        }
    }

    AWN_SINGLETON_TRAITS_IMPL(TextureSamplerManager);

    void TextureSamplerManager::Initialize(mem::Heap *heap, mem::Heap *gpu_heap, const TextureSamplerManagerInfo *manager_info) {

        /* Integrity check */
        VP_ASSERT(gpu_heap != nullptr);
        VP_ASSERT(gpu_heap->IsGpuHeap() == true);

        /* Allocate texture memory */
        const size_t gpu_memory_size = manager_info->texture_memory_size;
        const VkMemoryAllocateInfo allocate_info = {
            .sType            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize   = gpu_memory_size,
            .memoryTypeIndex  = Context::GetInstance()->GetVkMemoryTypeIndex(MemoryPropertyFlags::GpuCached | MemoryPropertyFlags::CpuUncached),
        };
        const u32 result = ::pfn_vkAllocateMemory(Context::GetInstance()->GetVkDevice(), std::addressof(allocate_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_device_memory_texture));
        VP_ASSERT(result == VK_SUCCESS);

        /* Create texture memory separate heap */
        const size_t separate_heap_size = sizeof(mem::SeparateHeap) + mem::SeparateHeap::CalculateManagementAreaSize(Context::cTargetMaxTextureDescriptorCount);
        void *separate_heap_start       = ::operator new(separate_heap_size, heap, alignof(mem::SeparateHeap));
        m_separate_heap                 = mem::SeparateHeap::Create("awn::gfx::TextureMemory", separate_heap_start, separate_heap_size, gpu_memory_size, true);

        /* Allocate descriptor memory */
        const size_t texture_descriptor_memory_size = vp::util::AlignUp(CalculateTextureDescriptorSetLayoutGpuSize(), Context::cTargetDescriptorBufferAlignment);
        m_texture_descriptor_gpu_address = ::operator new(texture_descriptor_memory_size, gpu_heap, Context::cTargetDescriptorBufferAlignment);

        const size_t sampler_descriptor_memory_size = vp::util::AlignUp(CalculateSamplerDescriptorSetLayoutGpuSize(), Context::cTargetDescriptorBufferAlignment);
        m_sampler_descriptor_gpu_address = ::operator new(sampler_descriptor_memory_size, gpu_heap, Context::cTargetDescriptorBufferAlignment);

        /* Create descriptor buffers */
        m_texture_descriptor_vk_buffer = gfx::CreateVkBuffer(m_texture_descriptor_gpu_address, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, texture_descriptor_memory_size);
        m_sampler_descriptor_vk_buffer = gfx::CreateVkBuffer(m_sampler_descriptor_gpu_address, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, sampler_descriptor_memory_size);

        /* Get descriptor buffer device addresses */
        const VkBufferDeviceAddressInfo texture_device_address_info = {
            .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = m_texture_descriptor_vk_buffer
        };
        const VkBufferDeviceAddressInfo sampler_device_address_info = {
            .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = m_sampler_descriptor_vk_buffer
        };
        m_texture_vk_device_address = ::pfn_vkGetBufferDeviceAddress(Context::GetInstance()->GetVkDevice(), std::addressof(texture_device_address_info));
        m_sampler_vk_device_address = ::pfn_vkGetBufferDeviceAddress(Context::GetInstance()->GetVkDevice(), std::addressof(sampler_device_address_info));

        return;
    }

    void TextureSamplerManager::Finalize() {

        /* Destroy descriptor buffers */
        if (m_texture_descriptor_vk_buffer != VK_NULL_HANDLE) {
            ::pfn_vkDestroyBuffer(Context::GetInstance()->GetVkDevice(), m_texture_descriptor_vk_buffer, Context::GetInstance()->GetVkAllocationCallbacks());
        }
        if (m_sampler_descriptor_vk_buffer != VK_NULL_HANDLE) {
            ::pfn_vkDestroyBuffer(Context::GetInstance()->GetVkDevice(), m_sampler_descriptor_vk_buffer, Context::GetInstance()->GetVkAllocationCallbacks());
        }
        m_texture_descriptor_vk_buffer = VK_NULL_HANDLE;
        m_sampler_descriptor_vk_buffer = VK_NULL_HANDLE;

        /* Free separate heap */

        /* Free texture memory */
        if (m_vk_device_memory_texture != VK_NULL_HANDLE) {
            ::pfn_vkFreeMemory(Context::GetInstance()->GetVkDevice(), m_vk_device_memory_texture, Context::GetInstance()->GetVkAllocationCallbacks());
        }
        m_vk_device_memory_texture = VK_NULL_HANDLE;
        
        return;
    }

    
    DescriptorSlot TextureSamplerManager::RegisterTextureView(TextureInfo *texture_info, TextureViewInfo *texture_view_info) {

        /* Integrity check infos */
        VP_ASSERT(texture_info != nullptr && texture_view_info != nullptr && texture_view_info->texture == nullptr);

        /* Get texture memory requirements */
        GpuMemoryRequirements texture_requirements = Texture::GetMemoryRequirements(texture_info);

        /* Allocate texture memory */
        void *separate_address = m_separate_heap->TryAllocate(texture_requirements.size, texture_requirements.alignment);
        VP_ASSERT(separate_address != nullptr);
        const size_t offset = reinterpret_cast<uintptr_t>(separate_address) - mem::SeparateHeap::cOffsetBase;

        /* Texture allocator lock */
        std::scoped_lock l(m_texture_allocator_critical_section);

        /* Allocate texture node */
        TextureNode *new_texture_node = m_texture_allocator.Allocate();
        VP_ASSERT(new_texture_node != nullptr);

        /* Initialize texture objects */
        new_texture_node->separate_memory = offset;
        new_texture_node->texture.Initialize(m_vk_device_memory_texture, offset, texture_info);
        texture_view_info->texture = std::addressof(new_texture_node->texture);
        new_texture_node->texture_view.Initialize(texture_view_info);

        /* Allocate texture handle */
        u32 texture_handle = 0;
        bool result = m_texture_handle_table.ReserveHandle(std::addressof(texture_handle), new_texture_node);
        VP_ASSERT(result == true);

        /* Get texture descriptor */
        char descriptor_storage[Context::cTargetMaxTextureDescriptorSize] = {};
        const VkDescriptorImageInfo descriptor_image_info = {
            .imageView   = new_texture_node->texture_view.GetVkImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
        const VkDescriptorGetInfoEXT descriptor_get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .data  = {
                .pSampledImage = std::addressof(descriptor_image_info)
            }
        }; 
        ::pfn_vkGetDescriptorEXT(Context::GetInstance()->GetVkDevice(), std::addressof(descriptor_get_info), sizeof(descriptor_storage), descriptor_storage);

        /* Register texture descriptor to descriptor buffer */
        const size_t descriptor_offset = vp::util::AlignUp(Context::GetInstance()->GetTextureDescriptorSize(), Context::GetInstance()->GetDescriptorAlignment()) * vp::util::GetHandleIndex(texture_handle);
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_texture_descriptor_gpu_address) + descriptor_offset), descriptor_storage, m_texture_descriptor_size);

        return texture_handle;
    }

    DescriptorSlot TextureSamplerManager::RegisterTextureView(void *gpu_texture_memory, TextureInfo *texture_info, TextureViewInfo *texture_view_info) {

        /* Integrity check infos */
        VP_ASSERT(texture_info != nullptr && texture_view_info != nullptr && texture_view_info->texture == nullptr);

        /* Texture allocator lock */
        std::scoped_lock l(m_texture_allocator_critical_section);

        /* Allocate texture node */
        TextureNode *new_texture_node = m_texture_allocator.Allocate();
        VP_ASSERT(new_texture_node != nullptr);

        /* Initialize texture objects */
        new_texture_node->texture.Initialize(gpu_texture_memory, texture_info);
        texture_view_info->texture = std::addressof(new_texture_node->texture);
        new_texture_node->texture_view.Initialize(texture_view_info);

        /* Allocate texture handle */
        u32 texture_handle = 0;
        bool result = m_texture_handle_table.ReserveHandle(std::addressof(texture_handle), new_texture_node);
        VP_ASSERT(result == true);

        /* Get texture descriptor */
        char descriptor_storage[Context::cTargetMaxTextureDescriptorSize] = {};
        const VkDescriptorImageInfo descriptor_image_info = {
            .imageView   = new_texture_node->texture_view.GetVkImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL
        };
        const VkDescriptorGetInfoEXT descriptor_get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .data  = {
                .pSampledImage = std::addressof(descriptor_image_info)
            }
        }; 
        ::pfn_vkGetDescriptorEXT(Context::GetInstance()->GetVkDevice(), std::addressof(descriptor_get_info), sizeof(descriptor_storage), descriptor_storage);

        /* Register texture descriptor to descriptor buffer */
        const size_t descriptor_offset = vp::util::AlignUp(Context::GetInstance()->GetTextureDescriptorSize(), Context::GetInstance()->GetDescriptorAlignment()) * vp::util::GetHandleIndex(texture_handle);
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_texture_descriptor_gpu_address) + descriptor_offset), descriptor_storage, m_texture_descriptor_size);

        return texture_handle;
    }

    DescriptorSlot TextureSamplerManager::ReferenceTextureView(DescriptorSlot texture_slot) {

        /* Get texture node */
        TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
        VP_ASSERT(texture_node != nullptr);

        /* Increment reference count */
        const u32 last_value = ::InterlockedIncrement(std::addressof(texture_node->reference_count));
        VP_ASSERT(last_value != 0);
        
        return texture_slot;
    }

    DescriptorSlot TextureSamplerManager::RegisterSampler(SamplerInfo *sampler_info) {

        /* Integrity check for non-null sampler info */
        VP_ASSERT(sampler_info != nullptr);

        /* Hash sampler info */
        const u32 sampler_hash = HashSampler(sampler_info);

        /* Sampler allocator lock */
        std::scoped_lock l(m_sampler_allocator_critical_section);

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
            ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_sampler_descriptor_gpu_address) + vp::util::AlignUp(Context::GetInstance()->GetSamplerDescriptorSize(), Context::GetInstance()->GetDescriptorAlignment())), descriptor_storage, m_sampler_descriptor_size);

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

    DescriptorSlot TextureSamplerManager::ReferenceSampler(DescriptorSlot sampler_slot) {

        /* Get sampler node */
        SamplerNode *sampler_node = reinterpret_cast<SamplerNode*>(m_sampler_handle_table.GetObjectByHandle(sampler_slot));
        VP_ASSERT(sampler_node != nullptr);

        /* Increment reference count */
        const u32 last_value = ::InterlockedIncrement(std::addressof(sampler_node->reference_count));
        VP_ASSERT(last_value != 0);

        return sampler_slot;
    }

    void TextureSamplerManager::UnregisterTextureView(DescriptorSlot texture_slot) {

        /* Get texture node */
        TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
        VP_ASSERT(texture_node != nullptr);

        /* Decrement ref count */
        const u32 last_value = ::InterlockedDecrement(std::addressof(texture_node->reference_count));

        /* Destruct if value is now 0 */
        if (last_value != 1) { return; }

        {
            /* Texture allocator lock */
            std::scoped_lock l(m_texture_allocator_critical_section);

            /* Finalize texture objects */
            texture_node->texture_view.Finalize();
            texture_node->texture.Finalize();

            /* Free separate memory if necessary */
            if (texture_node->separate_memory != TextureNode::cInvalidMemoryOffset) {
                void *separate_address = reinterpret_cast<void*>(texture_node->separate_memory + mem::SeparateHeap::cOffsetBase);
                m_separate_heap->Free(separate_address);
            }

            /* Free texture node to allocator */
            m_texture_allocator.Free(texture_node);
        }

        /* Free texture handle */
        const bool result = m_texture_handle_table.FreeHandle(texture_slot);
        VP_ASSERT(result == true);

        return;
    }

    void TextureSamplerManager::UnregisterSampler(DescriptorSlot sampler_slot) {

        /* Get sampler node */
        SamplerNode *sampler_node = reinterpret_cast<SamplerNode*>(m_sampler_handle_table.GetObjectByHandle(sampler_slot));
        VP_ASSERT(sampler_node != nullptr);

        /* Decrement ref count */
        const u32 last_value = ::InterlockedDecrement(std::addressof(sampler_node->reference_count));

        /* Destruct on finish */
        if (last_value != 1) { return; }

        {
            /* Sampler allocator lock */
            std::scoped_lock l(m_sampler_allocator_critical_section);

            /* Finalize sampler */
            sampler_node->sampler.Finalize();

            /* Remove sampler from tree */
            m_sampler_map.Remove(std::addressof(sampler_node->rb_node));

            /* Free sampler to allocator */
            m_sampler_allocator.Free(sampler_node);
        }

        /* Free sampler handle */
        const bool result = m_sampler_handle_table.FreeHandle(sampler_slot);
        VP_ASSERT(result == true);

        return;
    }

    void TextureSamplerManager::BindDescriptorBuffers(CommandBufferBase *command_buffer) {

        /* Set descriptor buffers */
        const VkDescriptorBufferBindingInfoEXT binding_info_array[] = {
            {
                .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .address = m_texture_vk_device_address,
                .usage   = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT 
            },
            {
                .sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .address = m_sampler_vk_device_address,
                .usage   = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT 
            },
        };
        ::pfn_vkCmdBindDescriptorBuffersEXT(command_buffer->GetVkCommandBuffer(), sizeof(binding_info_array) / sizeof(VkDescriptorBufferBindingInfoEXT), binding_info_array);

        return;
    }

    Texture *TextureSamplerManager::TryGetTextureByHandle(DescriptorSlot texture_slot) {

        /* Lookup by handle table */
        TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
        return (texture_node != nullptr) ? std::addressof(texture_node->texture) : nullptr;
    }

    TextureView *TextureSamplerManager::TryGetTextureViewByHandle(DescriptorSlot texture_slot) {

        /* Lookup by handle table */
        TextureNode *texture_node = reinterpret_cast<TextureNode*>(m_texture_handle_table.GetObjectByHandle(texture_slot));
        return (texture_node != nullptr) ? std::addressof(texture_node->texture_view) : nullptr;
    }

    Sampler *TextureSamplerManager::TryGetSamplerByHandle(DescriptorSlot sampler_slot) {

        /* Lookup by handle table */
        SamplerNode *sampler_node = reinterpret_cast<SamplerNode*>(m_sampler_handle_table.GetObjectByHandle(sampler_slot));
        return (sampler_node != nullptr) ? std::addressof(sampler_node->sampler) : nullptr;
    }
}

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
        m_separate_heap                 = mem::SeparateHeap::Create("awn::gfx::TextureMemoryHeap", separate_heap_start, separate_heap_size, gpu_memory_size, true);

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

        /* Allocate texture handle */
        const u32 texture_handle = m_texture_handle_table.Allocate();
        VP_ASSERT(texture_handle != TextureHandleTable::cInvalidHandle);

        /* Get texture node */
        TextureNode *new_texture_node = std::addressof(m_texture_array[texture_handle]);

        /* Initialize texture objects */
        new_texture_node->reference_count = 1;
        new_texture_node->separate_memory = offset;
        new_texture_node->texture.Initialize(m_vk_device_memory_texture, offset, texture_info);
        texture_view_info->texture = std::addressof(new_texture_node->texture);
        new_texture_node->texture_view.Initialize(texture_view_info);

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
        const size_t descriptor_offset = vp::util::AlignUp(Context::GetInstance()->GetTextureDescriptorSize(), Context::GetInstance()->GetDescriptorAlignment()) * texture_handle;
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_texture_descriptor_gpu_address) + descriptor_offset), descriptor_storage, m_texture_descriptor_size);

        return texture_handle;
    }

    DescriptorSlot TextureSamplerManager::RegisterTextureView(void *gpu_texture_memory, TextureInfo *texture_info, TextureViewInfo *texture_view_info) {

        /* Integrity check infos */
        VP_ASSERT(texture_info != nullptr && texture_view_info != nullptr && texture_view_info->texture == nullptr);

        /* Allocate texture handle */
        const u32 texture_handle = m_texture_handle_table.Allocate();
        VP_ASSERT(texture_handle != TextureHandleTable::cInvalidHandle);

        /* Get texture node */
        TextureNode *new_texture_node = std::addressof(m_texture_array[texture_handle]);

        /* Initialize texture objects */
        new_texture_node->reference_count = 1;
        new_texture_node->texture.Initialize(gpu_texture_memory, texture_info);
        texture_view_info->texture = std::addressof(new_texture_node->texture);
        new_texture_node->texture_view.Initialize(texture_view_info);

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
        const size_t descriptor_offset = vp::util::AlignUp(Context::GetInstance()->GetTextureDescriptorSize(), Context::GetInstance()->GetDescriptorAlignment()) * texture_handle;
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_texture_descriptor_gpu_address) + descriptor_offset), descriptor_storage, m_texture_descriptor_size);

        return texture_handle;
    }

    DescriptorSlot TextureSamplerManager::ReferenceTextureView(DescriptorSlot texture_slot) {

        /* Get texture node */
        TextureNode *texture_node = std::addressof(m_texture_array[texture_slot]);

        /* Increment reference count */
        const u32 last_value = vp::util::InterlockedIncrement(std::addressof(texture_node->reference_count));
        VP_ASSERT(last_value != 0);
        
        return texture_slot;
    }

    DescriptorSlot TextureSamplerManager::RegisterSampler(SamplerInfo *sampler_info) {

        /* Integrity check for non-null sampler info */
        VP_ASSERT(sampler_info != nullptr);

        /* Hash sampler info */
        const u32 sampler_hash = HashSampler(sampler_info);

        SamplerNode *sampler_node = nullptr;
        {            
            /* Sampler tree lock */
            std::scoped_lock l(m_sampler_tree_cs);

            /* Try to find existing sampler */
            sampler_node = m_sampler_map.Find(sampler_hash);
        }

        if (sampler_node == nullptr) {

            /* Allocate sampler handle */
            u32 sampler_handle = m_sampler_handle_table.Allocate();
            VP_ASSERT(sampler_handle == SamplerHandleTable::cInvalidHandle);
            sampler_node->handle = sampler_handle;

            /* Get new sampler */
            sampler_node = std::addressof(m_sampler_array[sampler_handle]);

            /* Initialize sampler */
            sampler_node->reference_count = 0;
            sampler_node->sampler.Initialize(sampler_info);

            /* Get Sampler descriptor */
            char descriptor_storage[Context::cTargetMaxSamplerDescriptorSize] = {};
            const VkSampler vk_sampler = sampler_node->sampler.GetVkSampler();
            const VkDescriptorGetInfoEXT descriptor_get_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
                .type  = VK_DESCRIPTOR_TYPE_SAMPLER,
                .data  = std::addressof(vk_sampler)
            }; 
            ::pfn_vkGetDescriptorEXT(Context::GetInstance()->GetVkDevice(), std::addressof(descriptor_get_info), sizeof(descriptor_storage), descriptor_storage);

            /* Register sampler descriptor to descritptor buffer */
            const size_t descriptor_offset = vp::util::AlignUp(Context::GetInstance()->GetSamplerDescriptorSize(), Context::GetInstance()->GetDescriptorAlignment()) * sampler_handle;
            ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_sampler_descriptor_gpu_address) + descriptor_offset), descriptor_storage, m_sampler_descriptor_size);

            /* Insert sampler node to tree */
            sampler_node->rb_node.SetKey(sampler_hash);
            {
                std::scoped_lock l(m_sampler_tree_cs);
                m_sampler_map.Insert(sampler_node);
            }
        }

        /* Increment reference count */
        vp::util::InterlockedIncrement(std::addressof(sampler_node->reference_count));

        return sampler_node->handle;
    }

    DescriptorSlot TextureSamplerManager::ReferenceSampler(DescriptorSlot sampler_slot) {

        /* Get sampler node */
        SamplerNode *sampler_node = std::addressof(m_sampler_array[sampler_slot]);
        VP_ASSERT(sampler_node != nullptr);

        /* Increment reference count */
        const u32 last_value = vp::util::InterlockedIncrement(std::addressof(sampler_node->reference_count));
        VP_ASSERT(last_value != 0);

        return sampler_slot;
    }

    void TextureSamplerManager::UnregisterTextureView(DescriptorSlot texture_slot) {

        /* Get texture node */
        TextureNode *texture_node = std::addressof(m_texture_array[texture_slot]);

        /* Decrement ref count */
        const u32 last_value = vp::util::InterlockedDecrement(std::addressof(texture_node->reference_count));

        /* Destruct if value is now 0 */
        if (last_value != 1) { return; }

        /* Finalize texture objects */
        texture_node->texture_view.Finalize();
        texture_node->texture.Finalize();

        /* Free separate memory if necessary */
        if (texture_node->separate_memory != TextureNode::cInvalidMemoryOffset) {
            void *separate_address = reinterpret_cast<void*>(texture_node->separate_memory + mem::SeparateHeap::cOffsetBase);
            m_separate_heap->Free(separate_address);
        }

        /* Free texture handle */
        m_texture_handle_table.Free(texture_slot);

        return;
    }

    void TextureSamplerManager::UnregisterSampler(DescriptorSlot sampler_slot) {

        /* Get sampler node */
        SamplerNode *sampler_node = std::addressof(m_sampler_array[sampler_slot]);

        /* Decrement ref count */
        const u32 last_value = vp::util::InterlockedDecrement(std::addressof(sampler_node->reference_count));

        /* Destruct on finish */
        if (last_value != 1) { return; }

        /* Finalize sampler */
        sampler_node->sampler.Finalize();

        {
            /* Sampler tree lock */
            std::scoped_lock l(m_sampler_tree_cs);

            /* Remove sampler from tree */
            m_sampler_map.Remove(std::addressof(sampler_node->rb_node));
        }

        /* Free sampler handle */
        m_sampler_handle_table.Free(sampler_slot);

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
        return std::addressof(m_texture_array[texture_slot].texture);
    }

    TextureView *TextureSamplerManager::TryGetTextureViewByHandle(DescriptorSlot texture_slot) {
        return std::addressof(m_texture_array[texture_slot].texture_view);
    }

    Sampler *TextureSamplerManager::TryGetSamplerByHandle(DescriptorSlot sampler_slot) {
        return std::addressof(m_sampler_array[sampler_slot].sampler);
    }
}

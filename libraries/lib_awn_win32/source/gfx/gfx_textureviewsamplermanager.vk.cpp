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

        ALWAYS_INLINE u32 HashSampler(const SamplerInfo *sampler_info) {
            return vp::util::HashDataMurmur64(sampler_info, sizeof(SamplerInfo));
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

    AWN_SINGLETON_TRAITS_IMPL(TextureViewSamplerManager);
	
    void TextureViewSamplerManager::Initialize(mem::Heap *heap, mem::Heap *gpu_heap, const TextureViewSamplerManagerInfo *manager_info) {

        /* Integrity check */
        VP_ASSERT(gpu_heap != nullptr);
        VP_ASSERT(gpu_heap->IsGpuHeap() == true);
        VP_ASSERT(manager_info->max_texture_view_unit_count < Context::cTargetMaxTextureDescriptorCount);
        VP_ASSERT(manager_info->max_sampler_unit_count < Context::cTargetMaxSamplerDescriptorCount);

        /* Initialize texture unit array */
        m_texture_view_unit_array.Initialize(heap, manager_info->max_texture_view_unit_count);
        m_texture_slot_allocator.Initialize(heap, manager_info->max_texture_view_unit_count);

        /* Initialize sampler unit array */
        m_sampler_unit_array.Initialize(heap, manager_info->max_sampler_unit_count);
        m_sampler_slot_allocator.Initialize(heap, manager_info->max_sampler_unit_count);

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

    void TextureViewSamplerManager::Finalize() {
        m_texture_view_unit_array.Finalize();
        m_texture_slot_allocator.Finalize();
        m_sampler_unit_array.Finalize();
        m_sampler_slot_allocator.Finalize();
        gfx::DestroyVkBuffer(m_texture_descriptor_vk_buffer);
        gfx::DestroyVkBuffer(m_sampler_descriptor_vk_buffer);
        if (m_texture_descriptor_gpu_address != nullptr) {
            ::operator delete(m_texture_descriptor_gpu_address);
            m_texture_descriptor_gpu_address = nullptr;
        }
        if (m_sampler_descriptor_gpu_address != nullptr) {
            ::operator delete(m_sampler_descriptor_gpu_address);
            m_sampler_descriptor_gpu_address = nullptr;
        }
    }

    Result TextureViewSamplerManager::RegisterTextureView(DescriptorSlot *out_texture_slot, const TextureViewInfo *texture_view_info, TextureBinder *reference_texture_binder) {

        /* Ensure texture is valid */
        RESULT_RETURN_UNLESS(texture_view_info->texture != nullptr, ResultNoTexture);
        if (reference_texture_binder != nullptr) {
            RESULT_RETURN_UNLESS(reference_texture_binder->IsRegistered(), ResultInvalidTextureBinder);
            Texture *binder_texture = reference_texture_binder->GetTexture();                        
            RESULT_RETURN_UNLESS(binder_texture == texture_view_info->texture, ResultTextureBinderAndTextureViewInfoMismatch);
        }

        /* Allocate new texture slot */
        const TextureSlotType texture_slot = m_texture_slot_allocator.Allocate();
        RESULT_RETURN_IF(texture_slot == TextureSlotAllocator::cInvalidEntryIndex, ResultExhaustedTextureSlots);

        /* Initialize texture view unit */
        TextureViewUnit *texture_view_unit = std::addressof(m_texture_view_unit_array[texture_slot]);
        if (reference_texture_binder != nullptr) { texture_view_unit->reference_texture_binder.ReferenceBinderSync(reference_texture_binder); }
        texture_view_unit->vk_image_view   = gfx::CreateVkImageView(texture_view_info);
        texture_view_unit->reference_count = 1;

        /* Cache texture descriptor */
        Context *context = Context::GetInstance();
        const VkDescriptorImageInfo descriptor_image_info = {
            .imageView   = texture_view_unit->vk_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
        const VkDescriptorGetInfoEXT descriptor_get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .data  = {
                .pSampledImage = std::addressof(descriptor_image_info)
            }
        }; 
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(descriptor_get_info), Context::cTargetMaxTextureDescriptorSize, texture_view_unit->descriptor_memory);

        /* Register texture descriptor to descriptor buffer */
        const u32    descriptor_size   = context->GetSeparateTextureDescriptorSize();
        const size_t descriptor_offset = descriptor_size * texture_slot;
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_texture_descriptor_gpu_address) + descriptor_offset), texture_view_unit->descriptor_memory, Context::cTargetMaxTextureDescriptorSize);

        *out_texture_slot = DescriptorSlot{ texture_slot, texture_view_unit->free_count };

        RESULT_RETURN_SUCCESS;
    }

    Result TextureViewSamplerManager::ReferenceTextureView(DescriptorSlot *out_texture_slot, DescriptorSlot reference_texture_slot) {

        /* Ensure reference slot is valid */
        RESULT_RETURN_UNLESS(this->IsTextureViewSlotValid(reference_texture_slot), ResultInvalidDescriptorSlot);

        /* Reference texture view unit */
        TextureViewUnit *texture_view_unit = std::addressof(m_texture_view_unit_array[reference_texture_slot.index]);
        texture_view_unit->IncrementReferenceCount();

        *out_texture_slot = reference_texture_slot;

        RESULT_RETURN_SUCCESS;
    }

    Result TextureViewSamplerManager::RegisterSampler(DescriptorSlot *out_sampler_slot, const SamplerInfo *sampler_info) {

        /* Try find existing sampler */
        const u64 hash = HashSampler(sampler_info);
        std::scoped_lock l(m_sampler_cs);
        SamplerUnit *ref_sampler_unit = m_sampler_map.Find(hash);

        /* Try to acquire a reference if our sampler already exists */
        if (ref_sampler_unit != nullptr && ref_sampler_unit->IncrementReferenceCountIfValid()) {
            const SamplerSlotType slot_index = static_cast<SamplerSlotType>(m_sampler_unit_array.IndexOf(ref_sampler_unit));
            *out_sampler_slot = DescriptorSlot{ slot_index, ref_sampler_unit->free_count };
            RESULT_RETURN_SUCCESS;
        }

        /* Allocate new sampler slot */
        const SamplerSlotType sampler_slot = m_sampler_slot_allocator.Allocate();
        RESULT_RETURN_IF(sampler_slot == SamplerSlotAllocator::cInvalidEntryIndex, ResultExhaustedSamplerSlots);

        /* Initialize sampler unit */
        SamplerUnit *sampler_unit     = std::addressof(m_sampler_unit_array[sampler_slot]);
        sampler_unit->vk_sampler      = gfx::CreateVkSampler(sampler_info);
        sampler_unit->reference_count = 1;

        /* Cache Sampler descriptor */
        Context *context = Context::GetInstance();
        const VkSampler vk_sampler = sampler_unit->vk_sampler;
        const VkDescriptorGetInfoEXT descriptor_get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type  = VK_DESCRIPTOR_TYPE_SAMPLER,
            .data  = std::addressof(vk_sampler)
        }; 
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(descriptor_get_info), Context::cTargetMaxSamplerDescriptorSize, sampler_unit->descriptor_memory);

        /* Register sampler descriptor to descriptor buffer */
        const u32    descriptor_size   = context->GetSeparateSamplerDescriptorSize();
        const size_t descriptor_offset = descriptor_size * sampler_slot;
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_sampler_descriptor_gpu_address) + descriptor_offset), sampler_unit->descriptor_memory, descriptor_size);

        /* Insert into tree */
        sampler_unit->rb_node.SetKey(hash);
        m_sampler_map.Insert(sampler_unit);

        *out_sampler_slot = DescriptorSlot{ sampler_slot, sampler_unit->free_count };

        RESULT_RETURN_SUCCESS;
    }

    Result TextureViewSamplerManager::ReferenceSampler(DescriptorSlot *out_sampler_slot, DescriptorSlot reference_sampler_slot) {

        /* Ensure reference slot is valid */
        RESULT_RETURN_UNLESS(this->IsSamplerSlotValid(reference_sampler_slot), ResultInvalidDescriptorSlot);

        /* Reference sampler unit */
        SamplerUnit *sampler_unit = std::addressof(m_sampler_unit_array[reference_sampler_slot.index]);
        sampler_unit->IncrementReferenceCount();

        *out_sampler_slot = reference_sampler_slot;

        RESULT_RETURN_SUCCESS;
    }

    void TextureViewSamplerManager::ReleaseTextureView(DescriptorSlot texture_view_slot) {

        /* Ensure slot is valid */
        if (!this->IsTextureViewSlotValid(texture_view_slot)) { return; }

        /* Decrement reference count and complete if we are still valid */
        TextureSlotType  texture_index     = texture_view_slot.index;
        TextureViewUnit *texture_view_unit = std::addressof(m_texture_view_unit_array[texture_index]);
        const u32        new_count         = texture_view_unit->DecrementReferenceCount();
        if (0 < new_count) { return; }

        /* Free texture view */
        gfx::DestroyVkImageView(texture_view_unit->vk_image_view);
        texture_view_unit->reference_texture_binder.Release();
        texture_view_unit->IncrementFreeCount();
        m_texture_slot_allocator.Free(texture_index);

        return;
    }

    void TextureViewSamplerManager::ReleaseSampler(DescriptorSlot sampler_slot) {

        /* Ensure slot is valid */
        if (!this->IsSamplerSlotValid(sampler_slot)) { return; }

        /* Decrement reference count and complete if we are still valid */
        SamplerSlotType  sampler_index = sampler_slot.index;
        SamplerUnit     *sampler_unit  = std::addressof(m_sampler_unit_array[sampler_index]);
        const u32        new_count     = sampler_unit->DecrementReferenceCount();
        if (0 < new_count) { return; }

        /* Remove sampler unit from tree */
        {
            std::scoped_lock l(m_sampler_cs);
            m_sampler_map.Remove(sampler_unit);
        }

        /* Free sampler unit */
        gfx::DestroyVkSampler(sampler_unit->vk_sampler);
        sampler_unit->IncrementFreeCount();
        m_sampler_slot_allocator.Free(sampler_index);

        return;
    }
}

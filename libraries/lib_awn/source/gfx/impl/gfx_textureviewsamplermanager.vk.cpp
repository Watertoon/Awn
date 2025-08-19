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

namespace awn::gfx::impl {

    namespace {

        constexpr ALWAYS_INLINE SamplerKey HashSampler(const SamplerInfo *sampler_info) {
            const SamplerKey ret = {
                .wrap_mode_u = sampler_info->wrap_mode_u,
                .wrap_mode_v = sampler_info->wrap_mode_v,
                .wrap_mode_w = sampler_info->wrap_mode_w,
                .compare_op = sampler_info->compare_op,
                .border_color = sampler_info->border_color,
                .mip_map_filter = sampler_info->mip_map_filter,
                .mag_filter = sampler_info->mag_filter,
                .min_filter = sampler_info->min_filter,
                .is_enable_anisotropy = sampler_info->is_enable_anisotropy,
                .is_compare_r_to_texture = sampler_info->is_compare_r_to_texture,
                .reduction_filter = sampler_info->reduction_filter,
                .max_anisotropy = sampler_info->max_anisotropy,
                .lod_clamp_min = sampler_info->lod_clamp_min,
                .lod_clamp_max = sampler_info->lod_clamp_max,
                .lod_bias = sampler_info->lod_bias,
            };
            return ret;
        }
    }

    AWN_SINGLETON_TRAITS_IMPL(TextureViewSamplerManager);

    void TextureViewSamplerManager::Initialize(mem::Heap *heap, mem::Heap *gpu_heap, const TextureViewSamplerManagerInfo *manager_info) {
        VP_UNUSED(gpu_heap);

        /* Integrity check */
        VP_ASSERT(manager_info->max_texture_view_unit_count < Context::cTargetMaxTextureViewCount);
        VP_ASSERT(manager_info->max_sampler_unit_count < Context::cTargetMaxSamplerCount);

        /* Initialize texture unit array */
        m_texture_view_unit_array.Initialize(heap, manager_info->max_texture_view_unit_count);
        m_texture_slot_allocator.Initialize(heap, manager_info->max_texture_view_unit_count);

        /* Initialize sampler unit array */
        m_sampler_unit_array.Initialize(heap, manager_info->max_sampler_unit_count);
        m_sampler_slot_allocator.Initialize(heap, manager_info->max_sampler_unit_count);

        /* Setup descriptor base */
        Context *context = Context::GetInstance();
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(context->GetVkDevice(), context->GetBindlessVkDescriptorSetLayout(), Context::cBindlessBindingBaseTexture, std::addressof(m_texture_base_offset));
        ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(context->GetVkDevice(), context->GetBindlessVkDescriptorSetLayout(), Context::cBindlessBindingBaseSampler, std::addressof(m_sampler_base_offset));

        return;
    }

    void TextureViewSamplerManager::Finalize() {
        m_texture_view_unit_array.Finalize();
        m_texture_slot_allocator.Finalize();
        m_sampler_unit_array.Finalize();
        m_sampler_slot_allocator.Finalize();
    }

    Result TextureViewSamplerManager::RegisterTextureView(DescriptorSlot *out_texture_slot, const TextureViewInfo *texture_view_info) {

        /* Ensure texture is valid */
        RESULT_RETURN_UNLESS(texture_view_info->texture != nullptr, ResultNoTexture);

        /* Allocate new texture slot */
        const TextureSlotType texture_slot = m_texture_slot_allocator.Allocate();
        RESULT_RETURN_IF(texture_slot == TextureSlotAllocator::cInvalidEntryIndex, ResultExhaustedTextureSlots);
        auto fail_slot_guard = SCOPE_GUARD {
            m_texture_slot_allocator.Free(texture_slot);
        };

        /* Initialize texture view unit */
        TextureViewUnit *texture_view_unit = std::addressof(m_texture_view_unit_array[texture_slot]);
        RESULT_TRY(gfx::impl::CreateVkImageView(std::addressof(texture_view_unit->vk_image_view), texture_view_info));
        fail_slot_guard.Cancel();
        texture_view_unit->key.SetFromInfo(texture_view_info);
        texture_view_unit->reference_count = 1;

        /* Cache texture descriptor */
        Context *context = Context::GetInstance();
        const VkDescriptorImageInfo descriptor_image_info = {
            .imageView = texture_view_unit->vk_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
        const VkDescriptorGetInfoEXT descriptor_get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .data = {
                .pSampledImage = std::addressof(descriptor_image_info)
            },
        }; 
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(descriptor_get_info), context->GetSeparateTextureDescriptorSize(), texture_view_unit->descriptor_memory);

        /* Register texture descriptor to descriptor buffer */
        const u32 descriptor_size = context->GetSeparateTextureDescriptorSize();
        const size_t descriptor_offset = descriptor_size * texture_slot;
        void *memory = DescriptorMemoryManager::GetInstance()->GetBindlessDescriptorMemory();
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + m_texture_base_offset + descriptor_offset), texture_view_unit->descriptor_memory, descriptor_size);

        *out_texture_slot = DescriptorSlot{ texture_slot, texture_view_unit->free_count, };

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
        const SamplerKey hash = HashSampler(sampler_info);
        std::scoped_lock l(m_sampler_cs);
        SamplerUnit *ref_sampler_unit = m_sampler_map.Find(hash);

        /* Try to acquire a reference if our sampler already exists */
        if (ref_sampler_unit != nullptr && ref_sampler_unit->IncrementReferenceCountIfValid()) {
            const SamplerSlotType slot_index = static_cast<SamplerSlotType>(m_sampler_unit_array.IndexOf(ref_sampler_unit));
            *out_sampler_slot = DescriptorSlot{ slot_index, ref_sampler_unit->free_count, };
            RESULT_RETURN_SUCCESS;
        }

        /* Allocate new sampler slot */
        const SamplerSlotType sampler_slot = m_sampler_slot_allocator.Allocate();
        RESULT_RETURN_IF(sampler_slot == SamplerSlotAllocator::cInvalidEntryIndex, ResultExhaustedSamplerSlots);
        auto fail_slot_guard = SCOPE_GUARD {
            m_sampler_slot_allocator.Free(sampler_slot);
        };

        /* Initialize sampler unit */
        SamplerUnit *sampler_unit = std::addressof(m_sampler_unit_array[sampler_slot]);
        RESULT_TRY(gfx::impl::CreateVkSampler(std::addressof(sampler_unit->vk_sampler), sampler_info));
        fail_slot_guard.Cancel();
        sampler_unit->reference_count = 1;

        /* Cache Sampler descriptor */
        Context *context = Context::GetInstance();
        const VkSampler vk_sampler = sampler_unit->vk_sampler;
        const VkDescriptorGetInfoEXT descriptor_get_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .data = std::addressof(vk_sampler),
        }; 
        ::pfn_vkGetDescriptorEXT(context->GetVkDevice(), std::addressof(descriptor_get_info), context->GetSeparateSamplerDescriptorSize(), sampler_unit->descriptor_memory);

        /* Register sampler descriptor to descriptor buffer */        
        const u32 descriptor_size = context->GetSeparateSamplerDescriptorSize();
        const size_t descriptor_offset = descriptor_size * sampler_slot;
        void *memory = DescriptorMemoryManager::GetInstance()->GetBindlessDescriptorMemory();
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + m_sampler_base_offset + descriptor_offset), sampler_unit->descriptor_memory, descriptor_size);

        /* Insert into tree */
        sampler_unit->rb_node.SetKey(hash);
        m_sampler_map.Insert(sampler_unit);

        *out_sampler_slot = DescriptorSlot{ sampler_slot, sampler_unit->free_count, };

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
        TextureSlotType texture_index = texture_view_slot.index;
        TextureViewUnit *texture_view_unit = std::addressof(m_texture_view_unit_array[texture_index]);
        const u32 new_count = texture_view_unit->DecrementReferenceCount();
        if (0 < new_count) { return; }

        /* Free texture view */
        gfx::impl::DestroyVkImageView(texture_view_unit->vk_image_view);
        texture_view_unit->IncrementFreeCount();
        m_texture_slot_allocator.Free(texture_index);

        return;
    }

    void TextureViewSamplerManager::ReleaseSampler(DescriptorSlot sampler_slot) {

        /* Ensure slot is valid */
        if (!this->IsSamplerSlotValid(sampler_slot)) { return; }

        /* Decrement reference count and complete if we are still valid */
        SamplerSlotType sampler_index = sampler_slot.index;
        SamplerUnit *sampler_unit = std::addressof(m_sampler_unit_array[sampler_index]);
        const u32 new_count = sampler_unit->DecrementReferenceCount();
        if (0 < new_count) { return; }

        /* Remove sampler unit from tree */
        {
            std::scoped_lock l(m_sampler_cs);
            m_sampler_map.Remove(sampler_unit);
        }

        /* Free sampler unit */
        gfx::impl::DestroyVkSampler(sampler_unit->vk_sampler);
        sampler_unit->IncrementFreeCount();
        m_sampler_slot_allocator.Free(sampler_index);

        return;
    }
}

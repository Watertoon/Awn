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

    void TextureViewSamplerManager::Initialize(mem::Heap *heap, const TextureViewSamplerManagerInfo *manager_info) {

        /* Integrity check */
        VP_ASSERT(manager_info->max_texture_view_unit_count < Context::cTargetMaxTextureViewCount);
        VP_ASSERT(manager_info->max_sampler_unit_count < Context::cTargetMaxSamplerCount);

        /* Initialize texture unit array */
        RESULT_ABORT_UNLESS(m_texture_view_unit_array.Initialize(heap, manager_info->max_texture_view_unit_count));
        RESULT_ABORT_UNLESS(m_texture_slot_allocator.Initialize(heap, manager_info->max_texture_view_unit_count));

        /* Initialize sampler unit array */
        RESULT_ABORT_UNLESS(m_sampler_unit_array.Initialize(heap, manager_info->max_sampler_unit_count));
        RESULT_ABORT_UNLESS(m_sampler_slot_allocator.Initialize(heap, manager_info->max_sampler_unit_count));

        return;
    }

    void TextureViewSamplerManager::Finalize() {
        m_sampler_slot_allocator.Finalize();
        m_sampler_unit_array.Finalize();
        m_texture_slot_allocator.Finalize();
        m_texture_view_unit_array.Finalize();
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

        /* Create vk texture view */
        TextureViewUnit *texture_view_unit = std::addressof(m_texture_view_unit_array[texture_slot]);
        Context *context = Context::GetInstance();
        const ImageFormat image_format = static_cast<ImageFormat>(texture_view_info->image_format);
        const VkImageViewCreateInfo image_view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = texture_view_info->texture->GetVkImage(),
            .viewType = vp::res::GfxImageDimensionToVkImageViewType(static_cast<ImageDimension>(texture_view_info->image_dimension)),
            .format   = vp::res::GfxImageFormatToVkFormat(image_format),
            .components = {
                .r = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_x)),
                .g = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_y)),
                .b = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_z)),
                .a = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_w)),
            },
            .subresourceRange = {
                .aspectMask     = (vp::res::IsDepthFormat(image_format) || vp::res::IsStencilFormat(image_format)) ? vp::res::GfxTextureDepthStencilModeToVkImageAspectFlags(static_cast<TextureDepthStencilMode>(texture_view_info->depth_stencil_mode)) : static_cast<u32>(VK_IMAGE_ASPECT_COLOR_BIT),
                .baseMipLevel   = texture_view_info->base_mip_level,
                .levelCount     = texture_view_info->mip_level_count,
                .baseArrayLayer = texture_view_info->base_array_layer,
                .layerCount     = texture_view_info->array_layer_count,
            }
        };
        const VkResult vk_result = ::pfn_vkCreateImageView(context->GetVkDevice(), std::addressof(image_view_info), context->GetVkAllocationCallbacks(), std::addressof(texture_view_unit->vk_image_view));
        const Result result = ConvertVkResultToResult(vk_result);
        RESULT_TRY(result);
        auto fail_view_guard = SCOPE_GUARD {
            impl::DestroyVkImageView(texture_view_unit->vk_image_view);
        };

        /* Register descriptors */
        if ((texture_view_info->texture->GetGpuAccessFlags() & gfx::GpuAccessFlags::Texture) == gfx::GpuAccessFlags::Texture) {
            DescriptorMemoryManager *desc_mgr = DescriptorMemoryManager::GetInstance();
            const VkImageDescriptorInfoEXT image_desc = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
                .pView = std::addressof(image_view_info),
                .layout = VK_IMAGE_LAYOUT_GENERAL,
            };
            VkResourceDescriptorInfoEXT resource_desc_info_array[2] = {
                {
                    .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
                    .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .data = {
                        .pImage = std::addressof(image_desc),
                    },
                },
                {
                    .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
                    .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                    .data = {
                        .pImage = std::addressof(image_desc),
                    },
                },
            };
            VkHostAddressRangeEXT range_array[2] = {
                {
                    .address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_mgr->GetTextureDescriptorMemory()) + texture_slot * impl::Context::cTargetMaxSeparateTextureDescriptorSize),
                    .size = context->GetSeparateTextureDescriptorSize(),
                },
                {
                    .address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_mgr->GetImageDescriptorMemory()) + texture_slot * impl::Context::cTargetMaxSeparateTextureDescriptorSize),
                    .size = context->GetImageDescriptorSize(),
                },
            };
            const VkResult vk_result1 = ::pfn_vkWriteResourceDescriptorsEXT(context->GetVkDevice(), 2, resource_desc_info_array, range_array);
            const Result result1 = ConvertVkResultToResult(vk_result1);
            RESULT_TRY(result1);
        }

        fail_view_guard.Cancel();
        fail_slot_guard.Cancel();
        texture_view_unit->key.SetFromInfo(texture_view_info);
        texture_view_unit->reference_count = 1;

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
        m_sampler_rwlock.AcquireReadLock();
        SamplerUnit *ref_sampler_unit = m_sampler_map.Find(hash);
        m_sampler_rwlock.ReleaseReadLock();

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

        /* Register sampler descriptor */
        Context *context = Context::GetInstance();
        DescriptorMemoryManager *desc_mgr = DescriptorMemoryManager::GetInstance();
        const VkSamplerReductionModeCreateInfo reduction_mode_info = {
            .sType         = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO,
            .reductionMode = vp::res::GfxReductionFilterToVkSamplerReductionMode(static_cast<ReductionFilter>(sampler_info->reduction_filter))
        };
        const VkSamplerCreateInfo sampler_create_info = {
            .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext            = std::addressof(reduction_mode_info),
            .magFilter        = vp::res::GfxMagFilterToVkFilter(static_cast<MagFilter>(sampler_info->mag_filter)),
            .minFilter        = vp::res::GfxMinFilterToVkFilter(static_cast<MinFilter>(sampler_info->min_filter)),
            .mipmapMode       = vp::res::GfxMipMapFilterToVkSamplerMipMapMode(static_cast<MipMapFilter>(sampler_info->mip_map_filter)),
            .addressModeU     = vp::res::GfxWrapModeToVkSamplerAddressMode(static_cast<WrapMode>(sampler_info->wrap_mode_u)),
            .addressModeV     = vp::res::GfxWrapModeToVkSamplerAddressMode(static_cast<WrapMode>(sampler_info->wrap_mode_v)),
            .addressModeW     = vp::res::GfxWrapModeToVkSamplerAddressMode(static_cast<WrapMode>(sampler_info->wrap_mode_w)),
            .mipLodBias       = sampler_info->lod_bias,
            .anisotropyEnable = sampler_info->is_enable_anisotropy,
            .maxAnisotropy    = static_cast<float>(sampler_info->max_anisotropy),
            .compareEnable    = true,
            .compareOp        = vp::res::GfxCompareOperationToVkCompareOp(static_cast<CompareOperation>(sampler_info->compare_op)),
            .minLod           = sampler_info->lod_clamp_min,
            .maxLod           = sampler_info->lod_clamp_max,
            .borderColor      = vp::res::GfxBorderColorToVkBorderColor(static_cast<BorderColor>(sampler_info->border_color)),
        };
        VkHostAddressRangeEXT range_array[1] = {
            {
                .address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(desc_mgr->GetSamplerDescriptorMemory()) + sampler_slot * impl::Context::cTargetMaxSeparateSamplerDescriptorSize),
                .size = context->GetSeparateSamplerDescriptorSize(),
            },
        };
        const VkResult vk_result = ::pfn_vkWriteSamplerDescriptorsEXT(context->GetVkDevice(), 1, std::addressof(sampler_create_info), range_array);
        const Result result = ConvertVkResultToResult(vk_result);
        RESULT_TRY(result);

        fail_slot_guard.Cancel();
        SamplerUnit *sampler_unit = std::addressof(m_sampler_unit_array[sampler_slot]);
        sampler_unit->reference_count = 1;

        /* Insert into tree */
        sampler_unit->rb_node.SetKey(hash);
        m_sampler_rwlock.AcquireWriteLock();
        m_sampler_map.Insert(sampler_unit);
        m_sampler_rwlock.ReleaseWriteLock();

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
        m_sampler_rwlock.AcquireWriteLock();
        m_sampler_map.Remove(sampler_unit);
        m_sampler_rwlock.ReleaseWriteLock();

        /* Free sampler unit */
        sampler_unit->IncrementFreeCount();
        m_sampler_slot_allocator.Free(sampler_index);

        return;
    }
}

#pragma once

namespace awn::gfx {

    class Sampler {
        private:
            VkSampler m_vk_sampler;
        public:
            constexpr ALWAYS_INLINE Sampler() : m_vk_sampler(VK_NULL_HANDLE) {/*...*/}

            void Initialize(SamplerInfo *sampler_info) {

                /* Create VkSampler */
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
                    .anisotropyEnable = sampler_info->enable_anisotropy,
                    .maxAnisotropy    = static_cast<float>(sampler_info->max_anisotropy),
                    .compareEnable    = sampler_info->enable_compare_op,
                    .compareOp        = vp::res::GfxCompareOperationToVkCompareOp(static_cast<CompareOperation>(sampler_info->compare_op)),
                    .minLod           = sampler_info->lod_clamp_min,
                    .maxLod           = sampler_info->lod_clamp_max,
                    .borderColor      = vp::res::GfxBorderColorToVkBorderColor(static_cast<BorderColor>(sampler_info->border_color)),
                };
                const u32 result = ::pfn_vkCreateSampler(Context::GetInstance()->GetVkDevice(), std::addressof(sampler_create_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_sampler));
                VP_ASSERT(result == VK_SUCCESS);

                return;
            }

            void Finalize() {

                if (m_vk_sampler != VK_NULL_HANDLE) {
                    ::pfn_vkDestroySampler(Context::GetInstance()->GetVkDevice(), m_vk_sampler, Context::GetInstance()->GetVkAllocationCallbacks());
                }
                m_vk_sampler = VK_NULL_HANDLE;
            }

            constexpr ALWAYS_INLINE VkSampler GetVkSampler() const { return m_vk_sampler; }
    };
    static_assert(sizeof(Sampler) <= cMaxGfxSamplerSize);
}

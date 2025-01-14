#pragma once

namespace awn::gfx {

    enum class RequiredFeature : u32 {
        GraphicsQueue,
        TargetApiVersion,
        TargetUniformBufferSize,
        TargetMemoryMapAlignment,
        TargetUniformBufferAlignment,
        TargetStorageBufferAlignment,
        TargetMaxTextureViewCount,
        TargetMaxSamplerCount,

        /* VkPhysicalDevice features */
        SupportsFullDrawU32,
        SupportsIndependentBlend,
        SupportsGeometryShaders,
        SupportsTessellationShaders,
        SupportsLogicOperation,
        SupportsDepthClamp,
        SupportsDepthBiasClamp,
        SupportsFillModeNonSolid,
        SupportsWideLines,
        SupportsLargePoints,
        SupportsMultiViewport,
        SupportsSamplerAnisotropy,
        SupportsBC,
        SupportsFragmentStoresAndAtomics,
        SupportsShaderStorageImageExtendedFormats,
        SupportsShaderUniformBufferArrayDynamicIndexing,
        SupportsShaderSampledImageArrayDynamicIndexing,
        SupportsShaderStorageBufferArrayDynamicIndexing,
        SupportsShaderStorageImageArrayDynamicIndexing,
        SupportsShaderInt64,

        /* VkPhysicalDeviceFeatures11 features */
        SupportsVariablePointersStorageBuffer,
        SupportsVariablePointers,

        /* VkPhysicalDeviceFeatures12 features */
        SupportsDescriptorIndexing,
        SupportsShaderInputAttachmentArrayDynamicIndexing,
        SupportsShaderUniformBufferArrayNonUniformIndexing,
        SupportsShaderSampledImageArrayNonUniformIndexing,
        SupportsShaderStorageBufferArrayNonUniformIndexing,
        SupportsShaderStorageImageArrayNonUniformIndexing,
        SupportsDescriptorBindingUniformBufferArrayUpdateAfterBind,
        SupportsDescriptorBindingSampledImageArrayUpdateAfterBind,
        SupportsDescriptorBindingStorageBufferArrayUpdateAfterBind,
        SupportsDescriptorBindingStorageImageArrayUpdateAfterBind,
        SupportsDescriptorBindingUpdateUnusedWhilePending,
        SupportsDescriptorBindingPartiallyBound,
        SupportsDescriptorBindingVariableDescriptorCount,
        SupportsRuntimeDescriptorArray,
        SupportsScalarBlockLayout,
        SupportsUniformBufferStandardLayout,
        SupportsTimelineSemaphore,
        SupportsBufferDeviceAddress,

        /* VkPhysicalDeviceFeatures13 features */
        SupportsSynchronization2,
        SupportsDynamicRendering,
        SupportsMaintenance4,

        /* VkPhysicalDeviceFeatures14 features */
        SupportsVertexAttributeInstanceRateDivisor,
        SupportsVertexAttributeInstanceRateZeroDivisor,
        SupportsIndexTypeU8,
        SupportsDynamicRenderingLocalRead,
        SupportsMaintenance5,
        SupportsMaintenance6,
        SupportsHostImageCopy,

        /* VK_EXT_external_memory_host, (VK_KHR_external_memory_win32, VK_KHR_external_semaphore_win32) */
        SupportsExternalMemoryHost,

        /* VK_EXT_shader_object */
        SupportsShaderObject,

        /* VK_EXT_descriptor_buffer */
        SupportsDescriptorBuffer,

        /* VK_EXT_mutable_descriptor_type */
        SupportsMutableDescriptors,

        /* VK_EXT_nested_command_buffer */
        SupportsNestedCommandBuffers,
    };
    VP_ENUM_TRAITSV(
        awn::gfx::RequiredFeature, RequiredFeature,
        GraphicsQueue,
        TargetApiVersion,
        TargetUniformBufferSize,
        TargetMemoryMapAlignment,
        TargetUniformBufferAlignment,
        TargetStorageBufferAlignment,
        TargetMaxTextureViewCount,
        TargetMaxSamplerCount,
        SupportsFullDrawU32,
        SupportsIndependentBlend,
        SupportsGeometryShaders,
        SupportsTessellationShaders,
        SupportsLogicOperation,
        SupportsDepthClamp,
        SupportsDepthBiasClamp,
        SupportsFillModeNonSolid,
        SupportsWideLines,
        SupportsLargePoints,
        SupportsMultiViewport,
        SupportsSamplerAnisotropy,
        SupportsBC,
        SupportsFragmentStoresAndAtomics,
        SupportsShaderStorageImageExtendedFormats,
        SupportsShaderUniformBufferArrayDynamicIndexing,
        SupportsShaderSampledImageArrayDynamicIndexing,
        SupportsShaderStorageBufferArrayDynamicIndexing,
        SupportsShaderStorageImageArrayDynamicIndexing,
        SupportsShaderInt64,
        SupportsVariablePointersStorageBuffer,
        SupportsVariablePointers,
        SupportsDescriptorIndexing,
        SupportsShaderInputAttachmentArrayDynamicIndexing,
        SupportsShaderUniformBufferArrayNonUniformIndexing,
        SupportsShaderSampledImageArrayNonUniformIndexing,
        SupportsShaderStorageBufferArrayNonUniformIndexing,
        SupportsShaderStorageImageArrayNonUniformIndexing,
        SupportsDescriptorBindingUniformBufferArrayUpdateAfterBind,
        SupportsDescriptorBindingSampledImageArrayUpdateAfterBind,
        SupportsDescriptorBindingStorageBufferArrayUpdateAfterBind,
        SupportsDescriptorBindingStorageImageArrayUpdateAfterBind,
        SupportsDescriptorBindingUpdateUnusedWhilePending,
        SupportsDescriptorBindingPartiallyBound,
        SupportsDescriptorBindingVariableDescriptorCount,
        SupportsRuntimeDescriptorArray,
        SupportsScalarBlockLayout,
        SupportsUniformBufferStandardLayout,
        SupportsTimelineSemaphore,
        SupportsBufferDeviceAddress,
        SupportsSynchronization2,
        SupportsDynamicRendering,
        SupportsMaintenance4,
        SupportsVertexAttributeInstanceRateDivisor,
        SupportsVertexAttributeInstanceRateZeroDivisor,
        SupportsIndexTypeU8,
        SupportsDynamicRenderingLocalRead,
        SupportsMaintenance5,
        SupportsMaintenance6,
        SupportsHostImageCopy,
        SupportsExternalMemoryHost,
        SupportsShaderObject,
        SupportsDescriptorBuffer,
        SupportsMutableDescriptors,
        SupportsNestedCommandBuffers
    );

    static constexpr u32 cRequiredFeatureCount = RequiredFeatureTraits::cEnumCount;
    using RequiredFeatureMask = vp::util::FixedConnection<u64, cRequiredFeatureCount>;

    enum class OptionalFeature : u32 {
        SupportsSeparateTextureAndSeparateSampler,

        /* VkPhysicalDevice features */
        SupportsAstcLdr,

        /* VK_KHR_shader_maximal_reconvergence */
        SupportsShaderMaximalReconvergence,

        /* VK_EXT_robustness2 */
        SupportsRobustness2,

        /* VK_EXT_vertex_attribute_robustness */
        SupportsVertexAttributeRobustness,

        /* VK_EXT_mesh_shader */
        SupportsMeshShaders,

        /* VK_NV_command_buffer_inheritance */
        SupportsCommandBufferInheritance,

        /* VK_NV_per_stage_descriptor_set */
        SupportsPerStageDescriptorSets,

        /* VK_NVX_image_view_handle */
        SupportsNvBindlessTexture,
    };
    VP_ENUM_TRAITSV(
        awn::gfx::OptionalFeature, OptionalFeature,
        SupportsSeparateTextureAndSeparateSampler,
        SupportsAstcLdr,
        SupportsShaderMaximalReconvergence,
        SupportsRobustness2,
        SupportsVertexAttributeRobustness,
        SupportsMeshShaders,
        SupportsCommandBufferInheritance,
        SupportsPerStageDescriptorSets,
        SupportsNvBindlessTexture
    );

    static constexpr u32 cOptionalFeatureCount = OptionalFeatureTraits::cEnumCount;
    using OptionalFeatureMask = vp::util::FixedConnection<u32, cOptionalFeatureCount>;

    constexpr bool IsOptionalFeature(OptionalFeature optional_feature);

    struct MissingFeatures {
        RequiredFeatureMask missing_required_feature_mask;
        OptionalFeatureMask missing_optional_feature_mask;

        constexpr void SetMissing(RequiredFeature required_feature) {
            this->missing_required_feature_mask.SetBit(static_cast<u32>(required_feature));
        }
        constexpr void SetMissing(OptionalFeature optional_feature) {
            this->missing_optional_feature_mask.SetBit(static_cast<u32>(optional_feature));
        }

        constexpr bool HasRequiredFeatures(const OptionalFeatureMask *required_optional_features) {
            if (this->missing_required_feature_mask.begin() != this->missing_required_feature_mask.end()) { return false; }
            for (const u32 &index : *required_optional_features) {
                if (this->missing_optional_feature_mask.GetBit(index)) { return false; }
            }
            return true;
        }
        constexpr u32 CountPreferredFeatures(const OptionalFeatureMask *pref_optional_features) {
            u32 pref_count = 0;
            for (const u32 &index : *pref_optional_features) {
                if (!this->missing_optional_feature_mask.GetBit(index)) { ++pref_count; }
            }
            return pref_count;
        }
    };
}

#pragma once

namespace awn::gfx {

    enum class RequiredFeature : u32 {
        TargetApiVersion,
        TargetUniformBufferSize,
        GraphicsQueue,
        SupportsPresent,
        TargetMemoryMapAlignment,
        TargetUniformBufferAlignment,
        TargetStorageBufferAlignment,
        TargetTexelBufferAlignment,
        TargetMaxTextureDescriptors,
        TargetMaxSamplerDescriptors,
        SupportsFullDrawU32,
        SupportsIndependentBlend,
        SupportsGeometryShaders,
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
        SupportsMultiView,
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
        SupportsBufferDeviceAddressCaptureReplay,
        SupportsSynchronization2,
        SupportsDynamicRendering,
        SupportsMaintenance4,
        SupportsVertexInputDynamicState,
        SupportsExtendedDynamicState2,
        SupportsIndexTypeU8,
        SupportsTessellationShaders,
        SupportsHostImageCopy,
        SupportsMeshShaders,
        SupportsCooperativeMatrix,
        SupportsVideoDecode,
        SupportsVideoEncode,
        SupportsOpticalFlow,
        SupportsAstc,
        SupportsDescriptorBuffer,
        SupportsSparseBinding,
        SupportsNestedCommandBuffers,
        SupportsExtendedDynamicState3,
        SupportsMaintenance5,
        SupportsMaintenance6,
        SupportsDynamicRenderingUnusedAttachments,
        SupportsDynamicRenderingLocalRead,
        SupportsShaderClock,
        SupportsShaderExpectAssume,
        SupportsShaderFloatControls2,
        SupportsShaderMaximalReconvergence,
        SupportsShaderQuadControl,
        SupportsShaderSubgroupRotate,
        SupportsShaderSubgroupUniformControlFlow,
        SupportsDepthBounds,
        SupportsShaderObject,
        SupportsMutableDescriptors,
        SupportsPerStageDescriptorSets,
        SupportsCommandBufferInheritance,
    };
    VP_ENUM_TRAITSV(
        awn::gfx::RequiredFeature, RequiredFeature,
        TargetApiVersion,
        TargetUniformBufferSize,
        GraphicsQueue,
        SupportsPresent,
        TargetMemoryMapAlignment,
        TargetUniformBufferAlignment,
        TargetStorageBufferAlignment,
        TargetTexelBufferAlignment,
        TargetMaxTextureDescriptors,
        TargetMaxSamplerDescriptors,
        SupportsFullDrawU32,
        SupportsIndependentBlend,
        SupportsGeometryShaders,
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
        SupportsMultiView,
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
        SupportsBufferDeviceAddressCaptureReplay,
        SupportsSynchronization2,
        SupportsDynamicRendering,
        SupportsMaintenance4,
        SupportsVertexInputDynamicState,
        SupportsExtendedDynamicState2,
        SupportsIndexTypeU8,
        SupportsTessellationShaders,
        SupportsHostImageCopy,
        SupportsMeshShaders,
        SupportsCooperativeMatrix,
        SupportsVideoDecode,
        SupportsVideoEncode,
        SupportsOpticalFlow,
        SupportsAstc,
        SupportsDescriptorBuffer,
        SupportsSparseBinding,
        SupportsNestedCommandBuffers,
        SupportsExtendedDynamicState3,
        SupportsMaintenance5,
        SupportsMaintenance6,
        SupportsDynamicRenderingUnusedAttachments,
        SupportsDynamicRenderingLocalRead,
        SupportsShaderClock,
        SupportsShaderExpectAssume,
        SupportsShaderFloatControls2,
        SupportsShaderMaximalReconvergence,
        SupportsShaderQuadControl,
        SupportsShaderSubgroupRotate,
        SupportsShaderSubgroupUniformControlFlow,
        SupportsDepthBounds,
        SupportsShaderObject,
        SupportsMutableDescriptors,
        SupportsPerStageDescriptorSets,
        SupportsCommandBufferInheritance
    );

    static constexpr u32 cRequiredFeatureCount = RequiredFeatureTraits::cEnumCount;
    using RequiredFeatureMask = vp::util::FixedConnection<u64, cRequiredFeatureCount>;
    
    enum class OptionalFeature : u32 {
        SupportsSeparateTextureAndSeparateSampler,
    };
    VP_ENUM_TRAITSV(
        awn::gfx::OptionalFeature, OptionalFeature,
        SupportsSeparateTextureAndSeparateSampler
    );

    static constexpr u32 cOptionalFeatureCount = OptionalFeatureTraits::cEnumCount;
    using OptionalFeatureMask = vp::util::FixedConnection<u32, cOptionalFeatureCount>;

    struct MissingFeatures {
        RequiredFeatureMask missing_required_feature_mask;
        OptionalFeatureMask missing_optional_feature_mask;

        constexpr bool HasRequiredFeatures(const OptionalFeatureMask *required_optional_features) {
            for ([[maybe_unused]] const u32 &index : this->missing_required_feature_mask) {
                return false;
            }
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

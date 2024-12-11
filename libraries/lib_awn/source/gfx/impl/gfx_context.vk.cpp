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

    AWN_SINGLETON_TRAITS_IMPL(Context);

    namespace {

#if defined(AWN_VK_DEBUG)
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, [[maybe_unused]] void* user_data) {

            ::puts(callback_data->pMessage);
            ::fflush(stdout);

            return VK_FALSE;
        }
#endif

        constexpr VkQueueFlagBits GfxQueueTypeToVkQueueFlagBits(QueueType queue_type) {
            switch (queue_type) {
                case QueueType::Graphics:
                    return VK_QUEUE_GRAPHICS_BIT;
                case QueueType::Compute:
                    return VK_QUEUE_COMPUTE_BIT;
                case QueueType::Transfer:
                    return VK_QUEUE_TRANSFER_BIT;
                case QueueType::VideoDecode:
                    return VK_QUEUE_VIDEO_DECODE_BIT_KHR;
                case QueueType::VideoEncode:
                    return VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
                case QueueType::OpticalFlow:
                    return VK_QUEUE_OPTICAL_FLOW_BIT_NV;
                default:
                    break;
            };
            VP_ASSERT(false);
        }

        void FindQueueFamilyIndices(VkPhysicalDevice vk_physical_device, const QueueType *queue_type_array, u32 queue_count, u32 *out_index_array) {

            /* Query queue family properties count */
            u32 queue_family_count = 0;
            ::pfn_vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, std::addressof(queue_family_count), nullptr);
            VP_ASSERT(queue_family_count != 0);

            /* Allocate a temporary queue family property holder */
            VkQueueFamilyProperties *queue_properties = reinterpret_cast<VkQueueFamilyProperties*>(::malloc(sizeof(VkQueueFamilyProperties) * queue_family_count));
            VP_ASSERT(queue_properties != nullptr);
            ON_SCOPE_EXIT {
                ::free(queue_properties);
            };

            /* Query queue family properties */
            ::pfn_vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, std::addressof(queue_family_count), queue_properties);

            /* Clear family indices */
            ::memset(out_index_array, 0xff, queue_count * sizeof(u32));

            /* Acquire queue family indices */
            for (u32 i = 0; i < queue_count; ++i) {
                const VkQueueFlagBits queue_flag = GfxQueueTypeToVkQueueFlagBits(queue_type_array[i]);
                u32 least_bit = 0xffff'ffff;
                for (u32 j = 0; j < queue_family_count; ++j) {
                    if ((queue_properties[j].queueFlags & queue_flag) != queue_flag) { continue; }
                    const u32 bit_count = vp::util::CountOneBits32(queue_properties[j].queueFlags);
                    if (least_bit < bit_count) { continue; }
                    least_bit = bit_count;
                    out_index_array[i] = j;
                }
            }

            return;
        }

        void FindMissingFeatures(MissingFeatures *missing_features, VkPhysicalDevice vk_physical_device, const ContextInfo *context_info) {

            /* Property structs */
            VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT,
                .pNext = nullptr,
            };
            VkPhysicalDeviceRobustness2PropertiesEXT robustness_2_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT,
                .pNext = std::addressof(mesh_shader_properties),
            };
            VkPhysicalDeviceNestedCommandBufferPropertiesEXT nested_command_buffer_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT,
                .pNext = std::addressof(robustness_2_properties),
            };
            VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
                .pNext = std::addressof(nested_command_buffer_properties),
            };
            VkPhysicalDeviceShaderObjectPropertiesEXT shader_object_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT,
                .pNext = std::addressof(descriptor_buffer_properties),
            };
            VkPhysicalDeviceVulkan14Properties properties_14 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES,
                .pNext = std::addressof(shader_object_properties),
            };
            VkPhysicalDeviceVulkan13Properties properties_13 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES,
                .pNext = std::addressof(properties_14),
            };
            VkPhysicalDeviceVulkan12Properties properties_12 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES,
                .pNext = std::addressof(properties_13),
            };
            VkPhysicalDeviceVulkan11Properties properties_11 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
                .pNext = std::addressof(properties_12),
            };
            VkPhysicalDeviceProperties2 properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                .pNext = std::addressof(properties_11),
            };

            /* Feature structs */
            VkPhysicalDevicePerStageDescriptorSetFeaturesNV per_stage_descriptor_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV,
                .pNext = nullptr,
            };
            VkPhysicalDeviceCommandBufferInheritanceFeaturesNV command_buffer_inheritance_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV,
                .pNext = std::addressof(per_stage_descriptor_features),
            };
            VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
                .pNext = std::addressof(command_buffer_inheritance_features),
            };
            VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT vertex_attribute_robustness_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT,
                .pNext = std::addressof(mesh_shader_features),
            };
            VkPhysicalDeviceRobustness2FeaturesEXT robustness_2_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
                .pNext = std::addressof(vertex_attribute_robustness_features),
            };
            VkPhysicalDeviceNestedCommandBufferFeaturesEXT nested_command_buffer_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
                .pNext = std::addressof(robustness_2_features),
            };
            VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutable_descriptor_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT,
                .pNext = std::addressof(nested_command_buffer_features),
            };
            VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptor_buffer_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
                .pNext = std::addressof(mutable_descriptor_features),
            };
            VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
                .pNext = std::addressof(descriptor_buffer_features),
            };
            VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR shader_maximal_reconvergence_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR,
                .pNext = std::addressof(shader_object_features),
            };
            VkPhysicalDeviceVulkan14Features features_14 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
                .pNext = std::addressof(shader_maximal_reconvergence_features),
            };
            VkPhysicalDeviceVulkan13Features features_13 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                .pNext = std::addressof(features_14),
            };
            VkPhysicalDeviceVulkan12Features features_12 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext = std::addressof(features_13),
            };
            VkPhysicalDeviceVulkan11Features features_11 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                .pNext = std::addressof(features_12),
            };
            VkPhysicalDeviceFeatures2 features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = std::addressof(features_11),
            };

            /* Query supported extensions */
            u32 property_count = 0;
            const VkResult result0 = ::pfn_vkEnumerateDeviceExtensionProperties(vk_physical_device, nullptr, std::addressof(property_count), nullptr);
            VP_ASSERT(result0 == VK_SUCCESS || result0 == VK_INCOMPLETE);

            VkExtensionProperties *property_array = reinterpret_cast<VkExtensionProperties*>(::malloc(sizeof(VkExtensionProperties) * property_count));
            VP_ASSERT(property_array != nullptr);
            ON_SCOPE_EXIT { ::free(property_array); };

            const VkResult result1 = ::pfn_vkEnumerateDeviceExtensionProperties(vk_physical_device, nullptr, std::addressof(property_count), property_array);
            VP_ASSERT(result1 == VK_SUCCESS);

            struct DeviceExtensionEntry {
                const char *extension = "";
                union {
                    RequiredFeature required_feature;
                    OptionalFeature optional_feature;
                };
                u32 min_revision = 0;
                bool is_optional;
            };
            static constexpr const DeviceExtensionEntry cAllDeviceExtensionArray[] = {
                {
                    .extension = VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME,
                    .required_feature = RequiredFeature::SupportsExternalMemoryHost,
                    .is_optional = false,
                },
#ifdef VP_TARGET_PLATFORM_win32
                {
                    .extension = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
                    .required_feature = RequiredFeature::SupportsExternalMemoryHost,
                    .is_optional = false,
                },
                {
                    .extension = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
                    .required_feature = RequiredFeature::SupportsExternalMemoryHost,
                    .is_optional = false,
                },
#endif
                {
                    .extension = VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
                    .required_feature = RequiredFeature::SupportsShaderObject,
                    .is_optional = false,
                },
                {
                    .extension = VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
                    .required_feature = RequiredFeature::SupportsDescriptorBuffer,
                    .is_optional = false,
                },
                {
                    .extension = VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
                    .required_feature = RequiredFeature::SupportsMutableDescriptors,
                    .is_optional = false,
                },
                {
                    .extension = VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
                    .required_feature = RequiredFeature::SupportsNestedCommandBuffers,
                    .is_optional = false,
                },
                {
                    .extension = VK_KHR_SHADER_MAXIMAL_RECONVERGENCE_EXTENSION_NAME,
                    .optional_feature = OptionalFeature::SupportsShaderMaximalReconvergence,
                    .is_optional = true,
                },
                {
                    .extension = VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
                    .optional_feature = OptionalFeature::SupportsRobustness2,
                    .is_optional = true,
                },
                {
                    .extension = VK_EXT_VERTEX_ATTRIBUTE_ROBUSTNESS_EXTENSION_NAME,
                    .optional_feature = OptionalFeature::SupportsVertexAttributeRobustness,
                    .is_optional = true,
                },
                {
                    .extension = VK_EXT_MESH_SHADER_EXTENSION_NAME,
                    .optional_feature = OptionalFeature::SupportsMeshShaders,
                    .is_optional = true,
                },
                {
                    .extension = VK_NV_PER_STAGE_DESCRIPTOR_SET_EXTENSION_NAME,
                    .optional_feature = OptionalFeature::SupportsPerStageDescriptorSets,
                    .is_optional = true,
                },
                {
                    .extension = VK_NV_COMMAND_BUFFER_INHERITANCE_EXTENSION_NAME,
                    .optional_feature = OptionalFeature::SupportsCommandBufferInheritance,
                    .is_optional = true,
                },
                {
                    .extension = VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME,
                    .optional_feature = OptionalFeature::SupportsNvBindlessTexture,
                    .is_optional = true,
                },
            };
            static constexpr const u32 cAllDeviceExtensionCount = sizeof(cAllDeviceExtensionArray) / sizeof(DeviceExtensionEntry);
            for (u32 j = 0; j < cAllDeviceExtensionCount; ++j) {
                const DeviceExtensionEntry *entry = std::addressof(cAllDeviceExtensionArray[j]);
                bool is_fail = true;
                for (u32 i = 0; i < property_count; ++i) {
                    const char *extension = property_array[i].extensionName;
                    if (!vp::util::StringCompare(extension, entry->extension)) { continue; }
                    if (property_array[i].specVersion < entry->min_revision) { is_fail = true; }
                    is_fail = false;
                    break;
                }
                if (is_fail) {                    
                    if (entry->is_optional) {
                        missing_features->SetMissing(entry->optional_feature);
                    } else {
                        missing_features->SetMissing(entry->required_feature);
                    }
                }
            }

            /* Query Physical Device */
            ::pfn_vkGetPhysicalDeviceProperties2(vk_physical_device, std::addressof(properties));
            //::pfn_vkGetPhysicalDeviceMemoryProperties(vk_physical_device, std::addressof(memory_properties));
            ::pfn_vkGetPhysicalDeviceFeatures2(vk_physical_device, std::addressof(features));

            /* GraphicsQueue checks */
            {
                const QueueType queue_type = QueueType::Graphics;
                u32 graphics_queue_family_index = 0;
                FindQueueFamilyIndices(vk_physical_device, std::addressof(queue_type), 1, std::addressof(graphics_queue_family_index));
                if (graphics_queue_family_index == Context::cInvalidQueueFamilyIndex) { missing_features->SetMissing(RequiredFeature::GraphicsQueue); };
            }

            /* TargetApiVersion checks */
            if (!(Context::cTargetMinimumApiVersion <= properties.properties.apiVersion)) { missing_features->SetMissing(RequiredFeature::TargetApiVersion); }

            /* TargetUniformBufferSize checks */
            if (properties.properties.limits.maxUniformBufferRange < Context::cTargetMaxUniformBufferSize) { missing_features->SetMissing(RequiredFeature::TargetUniformBufferSize); }

            /* TargetMemoryMapAlignment checks */
            {
                const size_t vk_alignment = properties.properties.limits.minMemoryMapAlignment;
                if (Context::cTargetMemoryPoolAlignment % vk_alignment != 0 || !(Context::cTargetMemoryPoolAlignment / vk_alignment > 0)) { missing_features->SetMissing(RequiredFeature::TargetMemoryMapAlignment); }
            }

            /* TargetUniformBufferAlignment checks */
            {
                const size_t vk_alignment = properties.properties.limits.minUniformBufferOffsetAlignment;
                if (Context::cTargetUniformBufferAlignment % vk_alignment != 0 || !(Context::cTargetUniformBufferAlignment / vk_alignment > 0)) { missing_features->SetMissing(RequiredFeature::TargetUniformBufferAlignment); }
            }

            /* TargetStorageBufferAlignment checks */
            {
                const size_t vk_alignment = properties.properties.limits.minStorageBufferOffsetAlignment;
                if (Context::cTargetStorageBufferAlignment % vk_alignment != 0 || !(Context::cTargetStorageBufferAlignment / vk_alignment > 0)) { missing_features->SetMissing(RequiredFeature::TargetStorageBufferAlignment); }
            }

            /* TargetMaxSamplerCount checks */
            const bool is_nv_bindless_texture = !missing_features->missing_optional_feature_mask.GetBit(static_cast<u32>(OptionalFeature::SupportsNvBindlessTexture));
            if (properties.properties.limits.maxSamplerAllocationCount < context_info->sampler_descriptor_count) { missing_features->SetMissing(RequiredFeature::TargetMaxSamplerCount); }
            if (!is_nv_bindless_texture && properties_12.maxDescriptorSetUpdateAfterBindSamplers < context_info->sampler_descriptor_count) { missing_features->SetMissing(RequiredFeature::TargetMaxSamplerCount); }
            if (!is_nv_bindless_texture && properties_12.maxDescriptorSetUpdateAfterBindSampledImages < context_info->texture_view_descriptor_count) { missing_features->SetMissing(RequiredFeature::TargetMaxTextureViewCount); }

            /* Base feature checks */
            if (!features.features.fullDrawIndexUint32)                     { missing_features->SetMissing(RequiredFeature::SupportsFullDrawU32); }
            if (!features.features.independentBlend)                        { missing_features->SetMissing(RequiredFeature::SupportsIndependentBlend); }
            if (!features.features.geometryShader)                          { missing_features->SetMissing(RequiredFeature::SupportsGeometryShaders); }
            if (!features.features.tessellationShader)                      { missing_features->SetMissing(RequiredFeature::SupportsTessellationShaders); }
            if (!features.features.logicOp)                                 { missing_features->SetMissing(RequiredFeature::SupportsLogicOperation); }
            if (!features.features.depthClamp)                              { missing_features->SetMissing(RequiredFeature::SupportsDepthClamp); }
            if (!features.features.depthBiasClamp)                          { missing_features->SetMissing(RequiredFeature::SupportsDepthBiasClamp); }
            if (!features.features.fillModeNonSolid)                        { missing_features->SetMissing(RequiredFeature::SupportsFillModeNonSolid); }
            if (!features.features.wideLines)                               { missing_features->SetMissing(RequiredFeature::SupportsWideLines); }
            if (!features.features.largePoints)                             { missing_features->SetMissing(RequiredFeature::SupportsLargePoints); }
            if (!features.features.multiViewport)                           { missing_features->SetMissing(RequiredFeature::SupportsMultiViewport); }
            if (!features.features.samplerAnisotropy)                       { missing_features->SetMissing(RequiredFeature::SupportsSamplerAnisotropy); }
            if (!features.features.textureCompressionBC)                    { missing_features->SetMissing(RequiredFeature::SupportsBC); }
            if (!features.features.textureCompressionASTC_LDR)              { missing_features->SetMissing(OptionalFeature::SupportsAstcLdr); }
            if (!features.features.fragmentStoresAndAtomics)                { missing_features->SetMissing(RequiredFeature::SupportsFragmentStoresAndAtomics); }
            if (!features.features.shaderStorageImageExtendedFormats)       { missing_features->SetMissing(RequiredFeature::SupportsShaderStorageImageExtendedFormats); }
            if (!features.features.shaderUniformBufferArrayDynamicIndexing) { missing_features->SetMissing(RequiredFeature::SupportsShaderUniformBufferArrayDynamicIndexing); }
            if (!features.features.shaderSampledImageArrayDynamicIndexing)  { missing_features->SetMissing(RequiredFeature::SupportsShaderSampledImageArrayDynamicIndexing); }
            if (!features.features.shaderStorageBufferArrayDynamicIndexing) { missing_features->SetMissing(RequiredFeature::SupportsShaderStorageBufferArrayDynamicIndexing); }
            if (!features.features.shaderStorageImageArrayDynamicIndexing)  { missing_features->SetMissing(RequiredFeature::SupportsShaderStorageImageArrayDynamicIndexing); }
            if (!features.features.shaderInt64)                             { missing_features->SetMissing(RequiredFeature::SupportsShaderInt64); }

            /* 1.1 feature checks */
            if (!features_11.variablePointersStorageBuffer) { missing_features->SetMissing(RequiredFeature::SupportsVariablePointersStorageBuffer); }
            if (!features_11.variablePointers)              { missing_features->SetMissing(RequiredFeature::SupportsVariablePointers); }

            /* 1.2 feature checks */
            if (!features_12.descriptorIndexing)                            { missing_features->SetMissing(RequiredFeature::SupportsDescriptorIndexing); }
            if (!features_12.shaderInputAttachmentArrayDynamicIndexing)     { missing_features->SetMissing(RequiredFeature::SupportsShaderInputAttachmentArrayDynamicIndexing); }
            if (!features_12.shaderUniformBufferArrayNonUniformIndexing)    { missing_features->SetMissing(RequiredFeature::SupportsShaderUniformBufferArrayNonUniformIndexing); }
            if (!features_12.shaderSampledImageArrayNonUniformIndexing)     { missing_features->SetMissing(RequiredFeature::SupportsShaderSampledImageArrayNonUniformIndexing); }
            if (!features_12.shaderStorageBufferArrayNonUniformIndexing)    { missing_features->SetMissing(RequiredFeature::SupportsShaderStorageBufferArrayNonUniformIndexing); }
            if (!features_12.shaderStorageImageArrayNonUniformIndexing)     { missing_features->SetMissing(RequiredFeature::SupportsShaderStorageImageArrayNonUniformIndexing); }
            if (!features_12.descriptorBindingUniformBufferUpdateAfterBind) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBindingUniformBufferArrayUpdateAfterBind); }
            if (!features_12.descriptorBindingSampledImageUpdateAfterBind)  { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBindingSampledImageArrayUpdateAfterBind); }
            if (!features_12.descriptorBindingStorageImageUpdateAfterBind)  { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBindingStorageBufferArrayUpdateAfterBind); }
            if (!features_12.descriptorBindingStorageBufferUpdateAfterBind) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBindingStorageImageArrayUpdateAfterBind); }
            if (!features_12.descriptorBindingUpdateUnusedWhilePending)     { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBindingUpdateUnusedWhilePending); }
            if (!features_12.descriptorBindingPartiallyBound)               { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBindingPartiallyBound); }
            if (!features_12.descriptorBindingVariableDescriptorCount)      { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBindingVariableDescriptorCount); }
            if (!features_12.runtimeDescriptorArray)                        { missing_features->SetMissing(RequiredFeature::SupportsRuntimeDescriptorArray); }
            if (!features_12.scalarBlockLayout)                             { missing_features->SetMissing(RequiredFeature::SupportsScalarBlockLayout); }
            if (!features_12.uniformBufferStandardLayout)                   { missing_features->SetMissing(RequiredFeature::SupportsUniformBufferStandardLayout); }
            if (!features_12.timelineSemaphore)                             { missing_features->SetMissing(RequiredFeature::SupportsTimelineSemaphore); }
            if (!features_12.bufferDeviceAddress)                           { missing_features->SetMissing(RequiredFeature::SupportsBufferDeviceAddress); }

            /* 1.3 feature checks */
            if (!features_13.synchronization2) { missing_features->SetMissing(RequiredFeature::SupportsSynchronization2); }
            if (!features_13.dynamicRendering) { missing_features->SetMissing(RequiredFeature::SupportsDynamicRendering); }
            if (!features_13.maintenance4)     { missing_features->SetMissing(RequiredFeature::SupportsMaintenance4); }

            /* 1.4 feature checks */
            if (!features_14.indexTypeUint8)            { missing_features->SetMissing(RequiredFeature::SupportsIndexTypeU8); }
            if (!features_14.dynamicRenderingLocalRead) { missing_features->SetMissing(RequiredFeature::SupportsDynamicRenderingLocalRead); }
            if (!features_14.maintenance5)              { missing_features->SetMissing(RequiredFeature::SupportsMaintenance5); }
            if (!features_14.maintenance6)              { missing_features->SetMissing(RequiredFeature::SupportsMaintenance6); }
            if (!features_14.hostImageCopy)             { missing_features->SetMissing(RequiredFeature::SupportsHostImageCopy); }

            /* SupportsShaderObject checks */
            if (!shader_object_features.shaderObject) { missing_features->SetMissing(RequiredFeature::SupportsShaderObject); }

            /* SupportsDescriptorBuffer checks */
            if (!descriptor_buffer_features.descriptorBuffer)                   { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
            if (!descriptor_buffer_features.descriptorBufferImageLayoutIgnored) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
            //if (!descriptor_buffer_features.descriptorBufferPushDescriptors)    { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
            if (descriptor_buffer_properties.combinedImageSamplerDescriptorSize > Context::cTargetMaxTextureSamplerDescriptorSize) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
            if (descriptor_buffer_properties.robustUniformBufferDescriptorSize > Context::cTargetMaxUniformBufferDescriptorSize)   { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
            if (descriptor_buffer_properties.robustStorageBufferDescriptorSize > Context::cTargetMaxStorageBufferDescriptorSize)   { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
            if (descriptor_buffer_properties.storageImageDescriptorSize > Context::cTargetMaxImageDescriptorSize)                  { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
            if (descriptor_buffer_properties.sampledImageDescriptorSize > Context::cTargetMaxSeparateTextureDescriptorSize)        { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
            if (descriptor_buffer_properties.samplerDescriptorSize > Context::cTargetMaxSeparateSamplerDescriptorSize)             { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }

            /* SupportsMutableDescriptors checks */
            if (!mutable_descriptor_features.mutableDescriptorType) { missing_features->SetMissing(RequiredFeature::SupportsMutableDescriptors); }

            /* SupportsNestedCommandBuffers checks */
            if (!nested_command_buffer_features.nestedCommandBuffer)                { missing_features->SetMissing(RequiredFeature::SupportsNestedCommandBuffers); }
            if (!nested_command_buffer_features.nestedCommandBufferRendering)       { missing_features->SetMissing(RequiredFeature::SupportsNestedCommandBuffers); }
            if (!nested_command_buffer_features.nestedCommandBufferSimultaneousUse) { missing_features->SetMissing(RequiredFeature::SupportsNestedCommandBuffers); }

            /* SupportsShaderMaximalReconvergence checks */
            if (!shader_maximal_reconvergence_features.shaderMaximalReconvergence) { missing_features->SetMissing(OptionalFeature::SupportsShaderMaximalReconvergence); }

            /* SupportsRobustness2 checks */
            if (!features.features.robustBufferAccess) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
            if (!properties_12.robustBufferAccessUpdateAfterBind) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
            if (!features_13.robustImageAccess) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
            if (!robustness_2_features.robustBufferAccess2) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
            if (!robustness_2_features.robustImageAccess2)  { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
            if (!robustness_2_features.nullDescriptor)      { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
            if (robustness_2_properties.robustStorageBufferAccessSizeAlignment > cTargetStorageBufferAlignment) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
            if (robustness_2_properties.robustUniformBufferAccessSizeAlignment > cTargetUniformBufferAlignment) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }

            /* SupportsVertexAttributeRobustness checks */
            if (!vertex_attribute_robustness_features.vertexAttributeRobustness) { missing_features->SetMissing(OptionalFeature::SupportsVertexAttributeRobustness); }

            /* SupportsMeshShaders checks  */
            if (!mesh_shader_features.taskShader) { missing_features->SetMissing(OptionalFeature::SupportsMeshShaders); }
            if (!mesh_shader_features.meshShader) { missing_features->SetMissing(OptionalFeature::SupportsMeshShaders); }

            /* SupportsPerStageDescriptorSets checks */
            if (!per_stage_descriptor_features.perStageDescriptorSet) { missing_features->SetMissing(OptionalFeature::SupportsPerStageDescriptorSets); missing_features->SetMissing(OptionalFeature::SupportsNvBindlessTexture); }
            if (!per_stage_descriptor_features.dynamicPipelineLayout) { missing_features->SetMissing(OptionalFeature::SupportsPerStageDescriptorSets); missing_features->SetMissing(OptionalFeature::SupportsNvBindlessTexture); }

            /* SupportsCommandBufferInheritance checks */
            if (!command_buffer_inheritance_features.commandBufferInheritance) { missing_features->SetMissing(OptionalFeature::SupportsCommandBufferInheritance); }

            return;
        }

        VkPhysicalDevice FindDefaultPhysicalDevice(VkInstance vk_instance, const ContextInfo *context_info) {
            u32 vk_physical_device_count = 0;
            VkPhysicalDevice *vk_physical_device_array = nullptr;
            MissingFeatures *missing_features_array = nullptr;

            /* Get count of physical devices */
            const u32 result2 = ::pfn_vkEnumeratePhysicalDevices(vk_instance, std::addressof(vk_physical_device_count), nullptr);
            VP_ASSERT(result2 == VK_SUCCESS);
            VP_ASSERT(vk_physical_device_count != 0);

            /* Create physical device array */
            void *memory = ::malloc((sizeof(MissingFeatures) + sizeof(VkPhysicalDevice)) * vk_physical_device_count);
            VP_ASSERT(memory != nullptr);
            ON_SCOPE_EXIT {
                ::free(memory);
            };
            
            vk_physical_device_array = reinterpret_cast<VkPhysicalDevice*>(memory);
            missing_features_array = reinterpret_cast<MissingFeatures*>(reinterpret_cast<uintptr_t>(memory) + sizeof(VkPhysicalDevice) * vk_physical_device_count);

            /* Query physical devices */
            const u32 result3 = ::pfn_vkEnumeratePhysicalDevices(vk_instance, std::addressof(vk_physical_device_count), vk_physical_device_array);
            VP_ASSERT(result3 == VK_SUCCESS);

            /* Build physical device features */
            for (u32 i = 0; i < vk_physical_device_count; ++i) {
                MissingFeatures *feature = std::addressof(missing_features_array[i]);
                std::construct_at(feature);
                FindMissingFeatures(feature, vk_physical_device_array[i], context_info);
            }

            /* Select device with all required and most preferred features */
            VkPhysicalDevice vk_physical_device = VK_NULL_HANDLE;
            u32 max_pref_count = 0;
            for (u32 i = 0; i < vk_physical_device_count; ++i) {
                if (!missing_features_array[i].HasRequiredFeatures(std::addressof(context_info->required_optional_feature_mask))) { continue; }
                const u32 pref_count = missing_features_array[i].CountPreferredFeatures(std::addressof(context_info->preferred_optional_feature_mask));
                if (pref_count < max_pref_count) { continue; }
                max_pref_count = pref_count;
                vk_physical_device = vk_physical_device_array[i];
            }

            return vk_physical_device;
        }
    }

    void Context::InitializeInstance(const ContextInfo *context_info) {

        /* VKInstance feature enables */
        #if defined(AWN_VK_DEBUG)
            constexpr const char *cDebugLayerArray[] = {
                "VK_LAYER_KHRONOS_validation"
            };
            constexpr const u32 cDebugLayerCount = sizeof(cDebugLayerArray) / sizeof(const char*);
        #endif

        constexpr const char *cInstanceExtensionArray[] = {
            #if defined(AWN_VK_DEBUG)
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            #endif
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
        };
        constexpr const u32 cInstanceExtensionCount = sizeof(cInstanceExtensionArray) / sizeof(const char*);

        #if defined(AWN_VK_DEBUG)
        //const VkValidationFeatureEnableEXT validation_feature_enable_array[] = {
        //    VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
        //};
        //const VkValidationFeaturesEXT validation_features = {
        //    .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        //    .enabledValidationFeatureCount = sizeof(validation_feature_enable_array) / sizeof(VkValidationFeatureEnableEXT),
        //    .pEnabledValidationFeatures = validation_feature_enable_array
        //};
        #endif

        const VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = context_info->app_name,
            .applicationVersion = 1,
            .pEngineName = context_info->engine_name,
            .engineVersion = 1,
            .apiVersion = cTargetMinimumApiVersion
        };

        const VkInstanceCreateInfo instance_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            #if defined(AWN_VK_DEBUG)
            //    .pNext = std::addressof(validation_features),
            #endif
            .pApplicationInfo = std::addressof(app_info),
            #if defined(AWN_VK_DEBUG)
                .enabledLayerCount = cDebugLayerCount,
                .ppEnabledLayerNames = cDebugLayerArray,
            #endif
            .enabledExtensionCount = cInstanceExtensionCount,
            .ppEnabledExtensionNames = cInstanceExtensionArray
        };
        const u32 result = ::pfn_vkCreateInstance(std::addressof(instance_info), this->GetVkAllocationCallbacks(), std::addressof(m_vk_instance));
        VP_ASSERT(result == VK_SUCCESS);

        /* Load instance procs */
        ::LoadVkCProcsInstance(m_vk_instance);

        return;
    }

    void Context::FinalizeInstance() {
        if (m_vk_instance != VK_NULL_HANDLE) {
            ::pfn_vkDestroyInstance(m_vk_instance, this->GetVkAllocationCallbacks());
            m_vk_instance = VK_NULL_HANDLE;
        }
    }

    void Context::SetVkPhysicalDevice(VkPhysicalDevice vk_physical_device, const ContextInfo *context_info) {

        /* Get properties and features */
        FindMissingFeatures(std::addressof(m_missing_features), vk_physical_device, context_info);
        ::pfn_vkGetPhysicalDeviceProperties2(vk_physical_device, std::addressof(m_vk_physical_device_properties));
        ::pfn_vkGetPhysicalDeviceMemoryProperties(vk_physical_device, std::addressof(m_vk_physical_device_memory_properties));
        m_is_ext_bindless_nv = (this->GetTextureSamplerDescriptorSize() == sizeof(size_t));

        /* Get queue family indices in enum order */
        const QueueType queue_type_array[] = {
            QueueType::Graphics,
            QueueType::Compute,
            QueueType::Transfer,
            QueueType::VideoDecode,
            QueueType::VideoEncode,
            QueueType::OpticalFlow,
        };
        const u32 queue_count = sizeof(queue_type_array) / sizeof(QueueType);
        FindQueueFamilyIndices(vk_physical_device, queue_type_array, queue_count, m_queue_family_index_array_enum);

        /* Build linear queue family indice array */
        m_queue_family_index_count = 0;
        for (u32 i = 0; i < queue_count; ++i) {
            if (m_queue_family_index_array_enum[i] == cInvalidQueueFamilyIndex) { continue; }
            m_queue_family_index_array_linear[m_queue_family_index_count] = m_queue_family_index_array_enum[i];
            ++m_queue_family_index_count;
        }

        m_vk_physical_device = vk_physical_device;

        return;
    }

    void Context::InitializeLogicalDevice(const ContextInfo *context_info) {

        /* Build extension array */
        static constexpr const char *cRequiredExtensionArray[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_6_EXTENSION_NAME,
            VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME,
#ifdef VP_TARGET_PLATFORM_win32
            VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
#endif
            VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
            VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
            VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
        };
        struct DeviceOptionalExtension {
            const char *extension = "";
            OptionalFeature optional_feature;
            bool is_enable_by_default = true;
        };
        static constexpr DeviceOptionalExtension cOptionalExtensionArray[] = {
            {
                .extension = VK_KHR_SHADER_MAXIMAL_RECONVERGENCE_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsShaderMaximalReconvergence,
            },
            {
                .extension = VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsRobustness2,
            },
            {
                .extension = VK_EXT_VERTEX_ATTRIBUTE_ROBUSTNESS_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsVertexAttributeRobustness,
            },
            {
                .extension = VK_EXT_MESH_SHADER_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsMeshShaders,
            },
            {
                .extension = VK_NV_PER_STAGE_DESCRIPTOR_SET_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsCommandBufferInheritance,
            },
            {
                .extension = VK_NV_COMMAND_BUFFER_INHERITANCE_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsCommandBufferInheritance,
            },
            {
                .extension = VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsNvBindlessTexture,
                .is_enable_by_default = false,
            },
        };
        static constexpr u32 cRequiredExtensionCount = sizeof(cRequiredExtensionArray) / sizeof(const char*);
        static constexpr u32 cOptionalExtensionCount = sizeof(cOptionalExtensionArray) / sizeof(DeviceOptionalExtension);
        static constexpr u32 cAllExtensionCount = cRequiredExtensionCount + cOptionalExtensionCount;

        const char *concat_extension_array[cAllExtensionCount];
        u32 concat_count = 0;
        for (u32 i = 0; i < cRequiredExtensionCount; ++i) {
            concat_extension_array[concat_count] = cRequiredExtensionArray[i];
            ++concat_count;
        }
        for (u32 i = 0; i < cOptionalExtensionCount; ++i) {
            /* Enforce feature exists */
            const u32 opt_feature = static_cast<u32>(cOptionalExtensionArray[i].optional_feature);
            if (m_missing_features.missing_optional_feature_mask.GetBit(opt_feature)) { continue; }
            /* Enable if better by default, or preferred by application */
            if (!cOptionalExtensionArray[i].is_enable_by_default && !context_info->preferred_optional_feature_mask.GetBit(opt_feature)) { continue; }

            m_optional_features.SetBit(opt_feature);
            concat_extension_array[concat_count] = cOptionalExtensionArray[i].extension;
            ++concat_count;
        }

        /* Build rest of optional features */
        struct DeviceOptionalNonExtension {
            OptionalFeature optional_feature;
            bool is_enable_by_default;
        };
        static constexpr DeviceOptionalNonExtension cOptionalNonExtensionArray[] = {
            {
                .optional_feature = OptionalFeature::SupportsSeparateTextureAndSeparateSampler,
                .is_enable_by_default = false,
            },
            {
                .optional_feature = OptionalFeature::SupportsAstcLdr,
                .is_enable_by_default = true,
            },
        };
        static constexpr u32 cOptionalNonExtensionCount = sizeof(cOptionalNonExtensionArray) / sizeof(DeviceOptionalNonExtension);
        for (u32 i = 0; i < cOptionalNonExtensionCount; ++i) {
            /* Enforce feature exists */
            if (m_missing_features.missing_optional_feature_mask.GetBit(i)) { continue; }
            /* Enable if preferred by application */
            if (!cOptionalNonExtensionArray[i].is_enable_by_default && !context_info->preferred_optional_feature_mask.GetBit(i)) { continue; }
            m_optional_features.SetBit(i);
        }

        /* VkDevice feature enables */
        const bool is_command_buffer_inheritance = this->IsOptionalFeature(OptionalFeature::SupportsCommandBufferInheritance);
        VkPhysicalDeviceCommandBufferInheritanceFeaturesNV command_buffer_inheritance_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV,
            .pNext = nullptr,
            .commandBufferInheritance = is_command_buffer_inheritance,
        };
        const bool is_per_stage_desc = this->IsOptionalFeature(OptionalFeature::SupportsPerStageDescriptorSets);
        VkPhysicalDevicePerStageDescriptorSetFeaturesNV per_stage_descriptor_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV,
            .pNext = std::addressof(command_buffer_inheritance_features),
            .perStageDescriptorSet = is_per_stage_desc,
            .dynamicPipelineLayout = is_per_stage_desc,
        };
        const bool is_mesh_shaders = this->IsOptionalFeature(OptionalFeature::SupportsMeshShaders);
        VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
            .pNext = std::addressof(per_stage_descriptor_features),
            .taskShader = is_mesh_shaders,
            .meshShader = is_mesh_shaders,
            //.multiviewMeshShader = is_mesh_shaders,
            //.primitiveFragmentShadingRateMeshShader = is_mesh_shaders,
            //.meshShaderQueries = is_mesh_shaders,
        };
        const bool is_vertex_attribute_robustness = this->IsOptionalFeature(OptionalFeature::SupportsVertexAttributeRobustness);
        VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT vertex_attribute_robustness_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT,
            .pNext = std::addressof(mesh_shader_features),
            .vertexAttributeRobustness = is_vertex_attribute_robustness,
        };
        const bool is_robustness2 = this->IsOptionalFeature(OptionalFeature::SupportsRobustness2);
        VkPhysicalDeviceRobustness2FeaturesEXT robustness_2_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
            .pNext = std::addressof(vertex_attribute_robustness_features),
            .robustBufferAccess2 = is_robustness2,
            .robustImageAccess2 = is_robustness2,
            .nullDescriptor = is_robustness2,
        };
        const bool is_maximal_reconvergance = this->IsOptionalFeature(OptionalFeature::SupportsShaderMaximalReconvergence);
        VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR shader_maximal_reconvergence_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR,
            .pNext = std::addressof(robustness_2_features),
            .shaderMaximalReconvergence = is_maximal_reconvergance,
        };
        VkPhysicalDeviceNestedCommandBufferFeaturesEXT nested_command_buffer_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
            .pNext = std::addressof(shader_maximal_reconvergence_features),
            .nestedCommandBuffer = VK_TRUE,
            .nestedCommandBufferRendering = VK_TRUE,
            .nestedCommandBufferSimultaneousUse = VK_TRUE,
        };
        VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutable_descriptor_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT,
            .pNext = std::addressof(nested_command_buffer_features),
            .mutableDescriptorType = VK_TRUE,
        };
        VkPhysicalDeviceDescriptorBufferFeaturesEXT target_descriptor_buffer_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
            .pNext = std::addressof(mutable_descriptor_features),
            .descriptorBuffer = VK_TRUE,
            .descriptorBufferImageLayoutIgnored = VK_TRUE,
            //.descriptorBufferPushDescriptors = VK_TRUE,
        };
        VkPhysicalDeviceShaderObjectFeaturesEXT target_shader_object_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
            .pNext = std::addressof(target_descriptor_buffer_features),
            .shaderObject = VK_TRUE,
        };
        VkPhysicalDeviceVulkan14Features target_1_4_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
            .pNext = std::addressof(target_shader_object_features),
            .indexTypeUint8 = VK_TRUE,
            .dynamicRenderingLocalRead = VK_TRUE,
            .maintenance5 = VK_TRUE,
            .maintenance6 = VK_TRUE,
            .hostImageCopy = VK_TRUE,
        };
        VkPhysicalDeviceVulkan13Features target_1_3_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = std::addressof(target_1_4_features),
            .robustImageAccess = is_robustness2,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
            .maintenance4 = VK_TRUE,
        };
        VkPhysicalDeviceVulkan12Features target_1_2_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = std::addressof(target_1_3_features),
            .descriptorIndexing                            = VK_TRUE,
            .shaderInputAttachmentArrayDynamicIndexing     = VK_TRUE,
            .shaderUniformBufferArrayNonUniformIndexing    = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing     = VK_TRUE,
            .shaderStorageBufferArrayNonUniformIndexing    = VK_TRUE,
            .shaderStorageImageArrayNonUniformIndexing     = VK_TRUE,
            .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind  = VK_TRUE,
            .descriptorBindingStorageImageUpdateAfterBind  = VK_TRUE,
            .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingUpdateUnusedWhilePending     = VK_TRUE,
            .descriptorBindingPartiallyBound               = VK_TRUE,
            .descriptorBindingVariableDescriptorCount      = VK_TRUE,
            .runtimeDescriptorArray                        = VK_TRUE,
            .scalarBlockLayout                             = VK_TRUE,
            .uniformBufferStandardLayout                   = VK_TRUE,
            .timelineSemaphore                             = VK_TRUE,
            .bufferDeviceAddress                           = VK_TRUE,
        };
        VkPhysicalDeviceVulkan11Features target_1_1_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = std::addressof(target_1_2_features),
            .multiview = VK_TRUE,
            .variablePointersStorageBuffer = VK_TRUE,
            .variablePointers              = VK_TRUE,
        };
        const VkPhysicalDeviceFeatures2 target_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = std::addressof(target_1_1_features),
            .features = {
                .robustBufferAccess                      = is_robustness2,
                .fullDrawIndexUint32                     = VK_TRUE,
                .independentBlend                        = VK_TRUE,
                .geometryShader                          = VK_TRUE,
                .tessellationShader                      = VK_TRUE,
                .logicOp                                 = VK_TRUE,
                .depthClamp                              = VK_TRUE,
                .depthBiasClamp                          = VK_TRUE,
                .fillModeNonSolid                        = VK_TRUE,
                .wideLines                               = VK_TRUE,
                .largePoints                             = VK_TRUE,
                .multiViewport                           = VK_TRUE,
                .samplerAnisotropy                       = VK_TRUE,
                .textureCompressionASTC_LDR              = this->IsOptionalFeature(OptionalFeature::SupportsAstcLdr),
                .textureCompressionBC                    = VK_TRUE,
                .fragmentStoresAndAtomics                = VK_TRUE,
                .shaderStorageImageExtendedFormats       = VK_TRUE,
                .shaderUniformBufferArrayDynamicIndexing = VK_TRUE,
                .shaderSampledImageArrayDynamicIndexing  = VK_TRUE,
                .shaderStorageBufferArrayDynamicIndexing = VK_TRUE,
                .shaderStorageImageArrayDynamicIndexing  = VK_TRUE,
                .shaderInt64                             = VK_TRUE,
            },
        };

        /* Build queue info */
        static constexpr const float priority = 1.0f;
        VkDeviceQueueCreateInfo device_queue_info_array[cTargetMaxQueueCount] = {};
        for (u32 i = 0; i < m_queue_family_index_count; ++i) {
            device_queue_info_array[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            device_queue_info_array[i].queueFamilyIndex = m_queue_family_index_array_linear[i];
            device_queue_info_array[i].queueCount = 1;
            device_queue_info_array[i].pQueuePriorities = std::addressof(priority);
        }

        /* Create logical device */
        const VkDeviceCreateInfo device_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = std::addressof(target_features),
            .queueCreateInfoCount = m_queue_family_index_count,
            .pQueueCreateInfos = device_queue_info_array,
            .enabledExtensionCount = concat_count,
            .ppEnabledExtensionNames = concat_extension_array,
            .pEnabledFeatures = nullptr,
        };
        const s32 result = ::pfn_vkCreateDevice(m_vk_physical_device, std::addressof(device_info), this->GetVkAllocationCallbacks(), std::addressof(m_vk_device));
        VP_ASSERT(result == VK_SUCCESS);

        /* Load device procs */
        ::LoadVkCProcsDevice(m_vk_device);

		/* Get queues */
        for (u32 i = 0; i < cTargetMaxQueueCount; ++i) {
            if (m_queue_family_index_array_enum[i] == cInvalidQueueFamilyIndex) { continue; }
            ::pfn_vkGetDeviceQueue(m_vk_device, m_queue_family_index_array_enum[i], 0, std::addressof(m_vk_queue_array_enum[i]));
        }

        return;
    }

    void Context::FinalizeLogicalDevice() {
        if (m_vk_device != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDevice(m_vk_device, this->GetVkAllocationCallbacks());
            m_vk_device = VK_NULL_HANDLE;
        }
    }

    void Context::InitializePipelineLayout(const ContextInfo *context_info) {

		/* Create bindless descriptor layout */
        const auto init_2_binding_multi_desc_desc_layout_lambda = [this](VkDescriptorSetLayout *set_layout, VkShaderStageFlags stage, VkDescriptorType type0, u32 desc_count0, VkDescriptorType type1, u32 desc_count1) {
            const VkDescriptorBindingFlags  bindless_set_binding_flag_array[] = {
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            };
            const VkDescriptorSetLayoutBindingFlagsCreateInfo bindless_set_binding_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = sizeof(bindless_set_binding_flag_array) / sizeof(VkDescriptorBindingFlags),
                .pBindingFlags = bindless_set_binding_flag_array,
            };
            const VkDescriptorSetLayoutBinding bindless_set_binding_array[] = {
                {
                    .binding         = 0,
                    .descriptorType  = type0,
                    .descriptorCount = desc_count0,
                    .stageFlags      = stage,
                },
                {
                    .binding         = 1,
                    .descriptorType  = type1,
                    .descriptorCount = desc_count1,
                    .stageFlags      = stage,
                },
            };
            const u32 ext_flags = (!this->IsOptionalFeature(OptionalFeature::SupportsPerStageDescriptorSets)) ? 0 : VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV;
            const VkDescriptorSetLayoutCreateInfo bindless_set_layout_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = std::addressof(bindless_set_binding_info),
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT | ext_flags,
                .bindingCount = sizeof(bindless_set_binding_array) / sizeof(VkDescriptorSetLayoutBinding),
                .pBindings = bindless_set_binding_array,
            };
            const u32 result0 = ::pfn_vkCreateDescriptorSetLayout(m_vk_device, std::addressof(bindless_set_layout_info), this->GetVkAllocationCallbacks(), set_layout);
            VP_ASSERT(result0 == VK_SUCCESS);

            return;
        };
        const bool is_nv_bindless = this->IsOptionalFeature(OptionalFeature::SupportsNvBindlessTexture);
        if (!is_nv_bindless) {            
            init_2_binding_multi_desc_desc_layout_lambda(std::addressof(m_vk_bindless_descriptor_set_layout), VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, context_info->texture_view_descriptor_count, VK_DESCRIPTOR_TYPE_SAMPLER, context_info->sampler_descriptor_count);
        }

        /* Build per stage descriptor set layout arrays */
        struct MultiBindInfo {
            VkDescriptorType type;
            u32 count;
        };
        const auto init_multi_binding_single_desc_desc_layout_lambda = [this](VkDescriptorSetLayout *set_layout, VkShaderStageFlags stage, const MultiBindInfo *bind_info, u32 bind_count) {

            /* Combine descriptor types for mutable (max 8) */
            VkDescriptorType desc_type_array[8];
            u32 desc_count = 0;
            for (u32 i = 0; i < bind_count; ++i) {
                desc_count += bind_info[i].count;
                desc_type_array[i] = bind_info[i].type;
            }
            const u32 desc_type_count = bind_count;

            /* Create binding array (max 256) */
            const u32 total_count = desc_count;
            VP_ASSERT(total_count < 0x100);
            VkMutableDescriptorTypeListEXT  per_stage_set_mutable_desc_list_array[0x100];
            VkDescriptorBindingFlags  bindless_set_binding_flag_array[0x100];
            VkDescriptorSetLayoutBinding bindless_set_binding_array[0x100];
            for (u32 i = 0; i < total_count; ++i) {
                bindless_set_binding_flag_array[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                bindless_set_binding_array[i].binding            = i;
                bindless_set_binding_array[i].descriptorType     = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
                bindless_set_binding_array[i].descriptorCount    = 1;
                bindless_set_binding_array[i].stageFlags         = stage;
                bindless_set_binding_array[i].pImmutableSamplers = nullptr;
                per_stage_set_mutable_desc_list_array[i].descriptorTypeCount = desc_type_count;
                per_stage_set_mutable_desc_list_array[i].pDescriptorTypes = desc_type_array;
            }

            /* Create set layout */
            const VkMutableDescriptorTypeCreateInfoEXT mutable_desc_info = {
                .sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT,
                .mutableDescriptorTypeListCount = total_count,
                .pMutableDescriptorTypeLists = per_stage_set_mutable_desc_list_array,
            };
            const VkDescriptorSetLayoutBindingFlagsCreateInfo bindless_set_binding_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .pNext = std::addressof(mutable_desc_info),
                .bindingCount = total_count,
                .pBindingFlags = bindless_set_binding_flag_array,
            };
            const u32 ext_flags = (!this->IsOptionalFeature(OptionalFeature::SupportsPerStageDescriptorSets)) ? 0 : VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV;
            const VkDescriptorSetLayoutCreateInfo bindless_set_layout_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = std::addressof(bindless_set_binding_info),
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT | ext_flags,
                .bindingCount = total_count,
                .pBindings = bindless_set_binding_array,
            };
            const u32 result0 = ::pfn_vkCreateDescriptorSetLayout(this->GetVkDevice(), std::addressof(bindless_set_layout_info), this->GetVkAllocationCallbacks(), set_layout);
            VP_ASSERT(result0 == VK_SUCCESS);

            return;
        };

        static constexpr MultiBindInfo cTsIStSSBindInfoArray[] = {
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .count = cTargetMaxPerStageTextureSamplerCount,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .count = cTargetMaxPerStageImageCount,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .count = cTargetMaxPerStageSeparateTextureCount,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLER,
                .count = cTargetMaxPerStageSeparateSamplerCount,
            },
        };
        static constexpr MultiBindInfo cUboSboBindInfoArray[] = {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .count = cTargetMaxPerStageUniformBufferCount,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .count = cTargetMaxPerStageStorageBufferCount,
            },
        };
        static constexpr u32 cTsIStSSBindInfoCount = sizeof(cTsIStSSBindInfoArray) / sizeof(MultiBindInfo);
        static constexpr u32 cUboSboBindInfoCount = sizeof(cUboSboBindInfoArray) / sizeof(MultiBindInfo);
        static_assert(cTsIStSSBindInfoCount <= 8);
        static_assert(cUboSboBindInfoCount <= 8);

        const auto set_desc_layout_lambda = [=, this](PerStageDescriptorLayout *layout, gfx::ShaderStage shader_stage) {

            /* Set pipeline layout info */
            layout->vk_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layout->vk_pipeline_layout_create_info.setLayoutCount = cMaxPerStageSetCount - is_nv_bindless;
            layout->vk_pipeline_layout_create_info.pSetLayouts = layout->vk_descriptor_set_layout_array;
            //layout->vk_pipeline_layout_create_info.pushConstantRangeCount = 1;
            //layout->vk_pipeline_layout_create_info.pPushConstantRanges = std::addressof(layout->vk_push_constant_range);

            /* Create descriptor sets */
            VkShaderStageFlags vk_stage_flags = vp::res::GfxShaderStageToVkShaderStageFlags(shader_stage);
            if (!is_nv_bindless) {                
                layout->vk_descriptor_set_layout_array[cTargetBindlessLayoutSet] = m_vk_bindless_descriptor_set_layout;
            }
            init_multi_binding_single_desc_desc_layout_lambda(std::addressof(layout->vk_descriptor_set_layout_array[cTargetPerStageTextureSamplerLayoutSet - is_nv_bindless]), vk_stage_flags, cTsIStSSBindInfoArray, cTsIStSSBindInfoCount);
            init_multi_binding_single_desc_desc_layout_lambda(std::addressof(layout->vk_descriptor_set_layout_array[cTargetPerStageBufferLayoutSet - is_nv_bindless]), vk_stage_flags, cUboSboBindInfoArray, cUboSboBindInfoCount);

            /* Set pipeline layout */
            //layout->vk_push_constant_range.stageFlags = vk_stage_flags;
            //layout->vk_push_constant_range.offset = 0;
            //layout->vk_push_constant_range.size = 0x100;

            return;
        };
        set_desc_layout_lambda(std::addressof(m_per_stage_descriptor_layout_vertex), ShaderStage::Vertex);
        set_desc_layout_lambda(std::addressof(m_per_stage_descriptor_layout_tessellation_control), ShaderStage::TessellationControl);
        set_desc_layout_lambda(std::addressof(m_per_stage_descriptor_layout_tessellation_evaluation), ShaderStage::TessellationEvaluation);
        set_desc_layout_lambda(std::addressof(m_per_stage_descriptor_layout_geometry), ShaderStage::Geometry);
        set_desc_layout_lambda(std::addressof(m_per_stage_descriptor_layout_fragment), ShaderStage::Fragment);
        set_desc_layout_lambda(std::addressof(m_per_stage_descriptor_layout_compute), ShaderStage::Compute);
        const bool is_mesh_shader = this->IsOptionalFeature(OptionalFeature::SupportsMeshShaders);
        if (is_mesh_shader) {
            set_desc_layout_lambda(std::addressof(m_per_stage_descriptor_layout_task), ShaderStage::Task);
            set_desc_layout_lambda(std::addressof(m_per_stage_descriptor_layout_mesh), ShaderStage::Mesh);
        }

        if (!this->IsOptionalFeature(OptionalFeature::SupportsPerStageDescriptorSets)) {

            /* Build combined descriptor layout arrays */
            if (!is_nv_bindless) {
                m_vk_combined_descriptor_set_layout_graphics[cTargetBindlessLayoutSet] = m_vk_bindless_descriptor_set_layout;
                m_vk_combined_descriptor_set_layout_compute[cTargetBindlessLayoutSet] = m_vk_bindless_descriptor_set_layout;
            }
            const u32 stage_count = (is_mesh_shader) ? cTargetMaxShaderStageCount : cTargetMaxShaderStageCount - 2;
            for (u32 i = 0; i < stage_count; ++i) {
                if (i == 5) { continue; }
                PerStageDescriptorLayout *per_stage_layout = std::addressof(m_per_stage_descriptor_layout_array[i]);
                const gfx::ShaderStage stage = static_cast<ShaderStage>(i);
                m_vk_combined_descriptor_set_layout_graphics[this->GetPerStageSetCombinedTextureSampler(stage)] = per_stage_layout->vk_descriptor_set_layout_array[cTargetPerStageTextureSamplerLayoutSet - is_nv_bindless];
                m_vk_combined_descriptor_set_layout_graphics[this->GetPerStageSetCombinedBuffer(stage)] = per_stage_layout->vk_descriptor_set_layout_array[cTargetPerStageBufferLayoutSet - is_nv_bindless];
            }
            PerStageDescriptorLayout *per_stage_layout = std::addressof(m_per_stage_descriptor_layout_array[5]);
            m_vk_combined_descriptor_set_layout_compute[this->GetPerStageSetCombinedTextureSampler(ShaderStage::Compute)] = per_stage_layout->vk_descriptor_set_layout_array[cTargetPerStageTextureSamplerLayoutSet - is_nv_bindless];
            m_vk_combined_descriptor_set_layout_compute[this->GetPerStageSetCombinedBuffer(ShaderStage::Compute)] = per_stage_layout->vk_descriptor_set_layout_array[cTargetPerStageBufferLayoutSet - is_nv_bindless];

            /* Create combined pipeline layouts */
            const u32 layout_count_graphics = cMaxCombinedSetCountGraphics - (is_mesh_shader * 4) - is_nv_bindless;
            const VkPipelineLayoutCreateInfo layout_info_graphics = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = layout_count_graphics,
                .pSetLayouts = m_vk_combined_descriptor_set_layout_graphics,
            };
            const VkResult result0 = ::pfn_vkCreatePipelineLayout(this->GetVkDevice(), std::addressof(layout_info_graphics), this->GetVkAllocationCallbacks(), std::addressof(m_vk_combined_pipeline_layout_graphics));
            VP_ASSERT(result0 == VK_SUCCESS);

            const VkPipelineLayoutCreateInfo layout_info_compute = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = static_cast<u32>(cMaxCombinedSetCountCompute) - is_nv_bindless,
                .pSetLayouts = m_vk_combined_descriptor_set_layout_compute,
            };
            const VkResult result1 = ::pfn_vkCreatePipelineLayout(this->GetVkDevice(), std::addressof(layout_info_compute), this->GetVkAllocationCallbacks(), std::addressof(m_vk_combined_pipeline_layout_compute));
            VP_ASSERT(result1 == VK_SUCCESS);
        }

        return;
    }

    void Context::FinalizePipelineLayout() {

        /* Destroy combined pipeline layouts */
        if (!this->IsOptionalFeature(OptionalFeature::SupportsPerStageDescriptorSets)) {
            if (m_vk_combined_pipeline_layout_graphics != VK_NULL_HANDLE) {
                ::pfn_vkDestroyPipelineLayout(this->GetVkDevice(), m_vk_combined_pipeline_layout_graphics, this->GetVkAllocationCallbacks());
                m_vk_combined_pipeline_layout_graphics = VK_NULL_HANDLE;
            }
            if (m_vk_combined_pipeline_layout_compute != VK_NULL_HANDLE) {
                ::pfn_vkDestroyPipelineLayout(this->GetVkDevice(), m_vk_combined_pipeline_layout_compute, this->GetVkAllocationCallbacks());
                m_vk_combined_pipeline_layout_compute = VK_NULL_HANDLE;
            }
        }

        /* Destroy descriptor set layouts */
        if (m_vk_bindless_descriptor_set_layout != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(this->GetVkDevice(), m_vk_bindless_descriptor_set_layout, this->GetVkAllocationCallbacks());
            m_vk_bindless_descriptor_set_layout = VK_NULL_HANDLE;
        }

        const bool is_nv_bindless = this->IsOptionalFeature(OptionalFeature::SupportsNvBindlessTexture);
        const auto finalize_per_stage_desc_lambda = [this, is_nv_bindless](PerStageDescriptorLayout *layout) {
            if (layout->vk_descriptor_set_layout_array[cTargetPerStageTextureSamplerLayoutSet - is_nv_bindless] != VK_NULL_HANDLE) {
                ::pfn_vkDestroyDescriptorSetLayout(this->GetVkDevice(), layout->vk_descriptor_set_layout_array[cTargetPerStageTextureSamplerLayoutSet - is_nv_bindless], this->GetVkAllocationCallbacks());
            }
            if (layout->vk_descriptor_set_layout_array[cTargetPerStageBufferLayoutSet - is_nv_bindless] != VK_NULL_HANDLE) {
                ::pfn_vkDestroyDescriptorSetLayout(this->GetVkDevice(), layout->vk_descriptor_set_layout_array[cTargetPerStageBufferLayoutSet - is_nv_bindless], this->GetVkAllocationCallbacks());
            }
            ::memset(layout, 0, sizeof(PerStageDescriptorLayout));
        };
        finalize_per_stage_desc_lambda(std::addressof(m_per_stage_descriptor_layout_vertex));
        finalize_per_stage_desc_lambda(std::addressof(m_per_stage_descriptor_layout_tessellation_control));
        finalize_per_stage_desc_lambda(std::addressof(m_per_stage_descriptor_layout_tessellation_evaluation));
        finalize_per_stage_desc_lambda(std::addressof(m_per_stage_descriptor_layout_geometry));
        finalize_per_stage_desc_lambda(std::addressof(m_per_stage_descriptor_layout_fragment));
        finalize_per_stage_desc_lambda(std::addressof(m_per_stage_descriptor_layout_compute));
        finalize_per_stage_desc_lambda(std::addressof(m_per_stage_descriptor_layout_task));
        finalize_per_stage_desc_lambda(std::addressof(m_per_stage_descriptor_layout_mesh));

        return;
    }

	void Context::Initialize(const ContextInfo *context_info) {
        VP_UNUSED(context_info);

        /* Load initial vulkan procs */
        ::LoadVkCProcsInitial();

        /* Check supported Vulkan Api version */
        {
            u32 api_version = 0;
            const u32 result0 = ::pfn_vkEnumerateInstanceVersion(std::addressof(api_version));
            VP_ASSERT(result0 == VK_SUCCESS);
            VP_ASSERT(cTargetMinimumApiVersion <= api_version);
        }

		/* Initialize default instance */
        this->InitializeInstance(context_info);

        #if defined(AWN_VK_DEBUG)
        /* Create debug messenger */
        {
            const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = DebugCallback,
                .pUserData = nullptr
            };

            const u32 result = ::pfn_vkCreateDebugUtilsMessengerEXT(m_vk_instance, std::addressof(debug_messenger_info), this->GetVkAllocationCallbacks(), std::addressof(m_debug_messenger));
            VP_ASSERT(result == VK_SUCCESS);
        }
        #endif

        /* Find our default physical device */
        VkPhysicalDevice vk_physical_device = FindDefaultPhysicalDevice(m_vk_instance, context_info);
        VP_ASSERT(vk_physical_device != VK_NULL_HANDLE);
        this->SetVkPhysicalDevice(vk_physical_device, context_info);

		/* Initialize device */
        this->InitializeLogicalDevice(context_info);

        /* Initialize pipeline layout */
        this->InitializePipelineLayout(context_info);

        return;
	}

    void Context::Finalize() {

        /* Ensure all device operations complete */
        ::pfn_vkDeviceWaitIdle(m_vk_device);

        /* Finalize pipeline layout */
        this->FinalizePipelineLayout();

		/* Finalize device */
        this->FinalizeLogicalDevice();

        m_vk_physical_device = VK_NULL_HANDLE;

        #if defined(AWN_VK_DEBUG)
            /* Destroy debugging utilities */
            if (m_debug_messenger != VK_NULL_HANDLE) {
                ::pfn_vkDestroyDebugUtilsMessengerEXT(m_vk_instance, m_debug_messenger, this->GetVkAllocationCallbacks());
                m_debug_messenger = VK_NULL_HANDLE;
            }
        #endif

        /* Finalize instance */
        this->FinalizeInstance();

        return;
    }

    void Context::SubmitCommandList(CommandList command_list, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count) {

        /* Integrity checks */
        VP_ASSERT(wait_sync_count < cMaxQueueWaitSyncCount);
        VP_ASSERT(signal_sync_count < cMaxQueueSignalSyncCount);

        /* Setup command buffer submit info, wait and signal semaphore infos */
        VkCommandBufferSubmitInfo command_buffer_submit_info = {
            .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext         = nullptr,
            .commandBuffer = command_list.vk_command_buffer,
            .deviceMask    = 0,
        };
        VkSemaphoreSubmitInfo wait_semaphore_array[cMaxQueueWaitSyncCount];
        for (u32 i = 0; i < wait_sync_count; ++i) {
            wait_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            wait_semaphore_array[i].pNext       = nullptr;
            wait_semaphore_array[i].semaphore   = wait_sync_array[i]->GetVkSemaphore();
            wait_semaphore_array[i].value       = wait_sync_array[i]->GetExpectedValue();
            wait_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            wait_semaphore_array[i].deviceIndex = 0;
        }
        VkSemaphoreSubmitInfo signal_semaphore_array[cMaxQueueSignalSyncCount];
        for (u32 i = 0; i < signal_sync_count; ++i) {
            signal_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            signal_semaphore_array[i].pNext       = nullptr;
            signal_semaphore_array[i].semaphore   = signal_sync_array[i]->GetVkSemaphore();
            signal_semaphore_array[i].value       = signal_sync_array[i]->GetExpectedValue();
            signal_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
            signal_semaphore_array[i].deviceIndex = 0;
        }

        /* Submit command list */
        const VkSubmitInfo2 graphics_submit_info = {
            .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount   = wait_sync_count,
            .pWaitSemaphoreInfos      = wait_semaphore_array,
            .commandBufferInfoCount   = 1,
            .pCommandBufferInfos      = std::addressof(command_buffer_submit_info),
            .signalSemaphoreInfoCount = signal_sync_count,
            .pSignalSemaphoreInfos    = signal_semaphore_array,
        };
        const u32 result0 = ::pfn_vkQueueSubmit2(this->GetVkQueue(command_list.GetQueueType()), 1, std::addressof(graphics_submit_info), VK_NULL_HANDLE);
        VP_ASSERT(result0 == VK_SUCCESS);

        return;
    }

    void Context::SubmitCommandLists(CommandList **command_list_array, u32 command_list_count, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count) {

        /* Integrity checks */
        VP_ASSERT(command_list_count < cMaxQueueCommandListCount);
        VP_ASSERT(wait_sync_count < cMaxQueueWaitSyncCount);
        VP_ASSERT(signal_sync_count < cMaxQueueSignalSyncCount);

        /* Setup command buffer submit info, wait and signal semaphore infos */
        VkCommandBufferSubmitInfo command_buffer_submit_info_array[cMaxQueueCommandListCount];
        u32 queue_type = static_cast<u32>(command_list_array[0]->GetQueueType());
        for (u32 i = 0; i < command_list_count; ++i) {
            VP_ASSERT(queue_type == static_cast<u32>(command_list_array[i]->GetQueueType()));
            command_buffer_submit_info_array[i].sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            command_buffer_submit_info_array[i].pNext         = nullptr;
            command_buffer_submit_info_array[i].commandBuffer = command_list_array[i]->vk_command_buffer;
            command_buffer_submit_info_array[i].deviceMask    = 0;
        }
        VkSemaphoreSubmitInfo wait_semaphore_array[cMaxQueueWaitSyncCount];
        for (u32 i = 0; i < wait_sync_count; ++i) {
            wait_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            wait_semaphore_array[i].pNext       = nullptr;
            wait_semaphore_array[i].semaphore   = wait_sync_array[i]->GetVkSemaphore();
            wait_semaphore_array[i].value       = wait_sync_array[i]->GetExpectedValue();
            wait_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            wait_semaphore_array[i].deviceIndex = 0;
        }
        VkSemaphoreSubmitInfo signal_semaphore_array[cMaxQueueSignalSyncCount];
        for (u32 i = 0; i < signal_sync_count; ++i) {
            signal_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            signal_semaphore_array[i].pNext       = nullptr;
            signal_semaphore_array[i].semaphore   = signal_sync_array[i]->GetVkSemaphore();
            signal_semaphore_array[i].value       = signal_sync_array[i]->GetExpectedValue();
            signal_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
            signal_semaphore_array[i].deviceIndex = 0;
        }

        /* Submit command lists */
        const VkSubmitInfo2 graphics_submit_info = {
            .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount   = wait_sync_count,
            .pWaitSemaphoreInfos      = wait_semaphore_array,
            .commandBufferInfoCount   = command_list_count,
            .pCommandBufferInfos      = command_buffer_submit_info_array,
            .signalSemaphoreInfoCount = signal_sync_count,
            .pSignalSemaphoreInfos    = signal_semaphore_array,
        };
        const u32 result0 = ::pfn_vkQueueSubmit2(this->GetVkQueue(static_cast<QueueType>(queue_type)), 1, std::addressof(graphics_submit_info), VK_NULL_HANDLE);
        VP_ASSERT(result0 == VK_SUCCESS);

        return;
    }
}

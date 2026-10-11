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

    Result ConvertVkResultToResult(VkResult vk_result) {
        switch (vk_result) {
            case VK_SUCCESS:
                RESULT_RETURN_SUCCESS;
            case VK_NOT_READY:
                RESULT_RETURN(ResultNotReady);
            case VK_TIMEOUT :
                RESULT_RETURN(ResultTimeout);
            case VK_INCOMPLETE:
                RESULT_RETURN(ResultIncomplete);
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                RESULT_RETURN(ResultOutOfCpuMemory);
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                RESULT_RETURN(ResultOutOfGpuMemory);
            case VK_ERROR_DEVICE_LOST:
#ifdef AWN_VK_DEBUG
                impl::Context::GetInstance()->ReportDeviceLostReason();
#endif
                RESULT_RETURN(ResultGpuLost);
            case VK_ERROR_MEMORY_MAP_FAILED:
                RESULT_RETURN(ResultFailedToMapMemory);
            case VK_SUBOPTIMAL_KHR:
                RESULT_RETURN(ResultSubOptimal);
            case VK_ERROR_OUT_OF_DATE_KHR:
                RESULT_RETURN(ResultOutOfDate);
            case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
                RESULT_RETURN(ResultInvalidShaderCode);
            default:
                break;
        };
        return ResultUnknownApiError;
    }

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
            std::memset(out_index_array, 0xff, queue_count * sizeof(u32));

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

#ifdef AWN_VK_DEBUG
        void ReportMissingFeatures(const MissingFeatures *features) {
            using Traits = vp::util::TEnumTraits<RequiredFeature>;
            for (const u32 i : features->missing_required_feature_mask) {
                const char *string = Traits::GetStringByIndex(i);
                ::printf("  %s\n", string);
            }
        }
#endif

        VkPhysicalDevice FindDefaultPhysicalDevice(VkInstance vk_instance, const ContextInfo *context_info) {
            u32 vk_physical_device_count = 0;
            VkPhysicalDevice *vk_physical_device_array = nullptr;
            MissingFeatures *missing_features_array = nullptr;

            /* Get count of physical devices */
            const VkResult result2 = ::pfn_vkEnumeratePhysicalDevices(vk_instance, std::addressof(vk_physical_device_count), nullptr);
            RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result2));
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
            const VkResult result3 = ::pfn_vkEnumeratePhysicalDevices(vk_instance, std::addressof(vk_physical_device_count), vk_physical_device_array);
            RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result3));

            /* Build physical device features */
            for (u32 i = 0; i < vk_physical_device_count; ++i) {
                MissingFeatures *feature = std::addressof(missing_features_array[i]);
                std::construct_at(feature);
                FindMissingFeatures(feature, vk_physical_device_array[i], std::addressof(context_info->features_info));
            }

            /* Select device with all required and most preferred features */
            VkPhysicalDevice vk_physical_device = VK_NULL_HANDLE;
            u32 max_pref_count = 0;
            for (u32 i = 0; i < vk_physical_device_count; ++i) {
                if (!missing_features_array[i].HasRequiredFeatures(std::addressof(context_info->required_optional_feature_mask))) {
#ifdef AWN_VK_DEBUG
                    ::printf("Missing Features Detected (device: %d):\n", i);
                    ReportMissingFeatures(std::addressof(missing_features_array[i]));
#endif
                    continue;
                }
                const u32 pref_count = missing_features_array[i].CountPreferredFeatures(std::addressof(context_info->preferred_optional_feature_mask));
                if (pref_count < max_pref_count) { continue; }
                max_pref_count = pref_count;
                vk_physical_device = vk_physical_device_array[i];
            }

            return vk_physical_device;
        }
    }

    void FindMissingFeatures(MissingFeatures *missing_features, VkPhysicalDevice vk_physical_device, const FeaturesInfo *features_info) {

        /* Query supported extensions */
        u32 property_count = 0;
        const VkResult result0 = ::pfn_vkEnumerateDeviceExtensionProperties(vk_physical_device, nullptr, std::addressof(property_count), nullptr);
        VP_ASSERT(result0 == VK_SUCCESS || result0 == VK_INCOMPLETE);

        VkExtensionProperties *property_array = reinterpret_cast<VkExtensionProperties*>(::malloc(sizeof(VkExtensionProperties) * property_count));
        VP_ASSERT(property_array != nullptr);
        ON_SCOPE_EXIT { ::free(property_array); };

        const VkResult result1 = ::pfn_vkEnumerateDeviceExtensionProperties(vk_physical_device, nullptr, std::addressof(property_count), property_array);
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result1));

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
#elifdef VP_TARGET_PLATFORM_linux
            {
                .extension = VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME,
                .required_feature = RequiredFeature::SupportsExternalMemoryHost,
                .is_optional = false,
            },
            {
                .extension = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
                .required_feature = RequiredFeature::SupportsExternalMemoryHost,
                .is_optional = false,
            },
            {
                .extension = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
                .required_feature = RequiredFeature::SupportsExternalMemoryHost,
                .is_optional = false,
            },
            {
                .extension = VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
                .required_feature = RequiredFeature::SupportsExternalMemoryHost,
                .is_optional = false,
            },
            {
                .extension = VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
                .required_feature = RequiredFeature::SupportsExternalMemoryHost,
                .is_optional = false,
            },
            {
                .extension = VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
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
                .extension = VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME,
                .required_feature = RequiredFeature::SupportsDescriptorBuffer,
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
                .extension = VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME,
                .required_feature = RequiredFeature::SupportsDeviceAddressCommands,
                .is_optional = false,
            },
            {
                .extension = VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME,
                .required_feature = RequiredFeature::SupportsConditionalRendering,
                .is_optional = false,
            },
            {
                .extension = VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsFragmentShadingRate,
                .is_optional = true,
            },
            {
                .extension = VK_EXT_IMAGE_TILING_CONTROL_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsImageTilingControl,
                .is_optional = true,
            },
            {
                .extension = VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsImageCompressionControl,
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

        /* Property structs */
        void *prev_property = nullptr;
        VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT,
            .pNext = prev_property,
        };
        prev_property = (missing_features->IsOptionalFeature(OptionalFeature::SupportsMeshShaders)) ? std::addressof(mesh_shader_properties) : prev_property;
        VkPhysicalDeviceRobustness2PropertiesEXT robustness_2_properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT,
            .pNext = prev_property,
        };
        prev_property = (missing_features->IsOptionalFeature(OptionalFeature::SupportsRobustness2)) ? std::addressof(robustness_2_properties) : prev_property;
#ifdef VP_TARGET_PLATFORM_linux
        VkPhysicalDeviceDrmPropertiesEXT drm_props = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT,
            .pNext = prev_property,
        };
        prev_property = std::addressof(drm_props);
#endif
        VkPhysicalDeviceNestedCommandBufferPropertiesEXT nested_command_buffer_properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT,
            .pNext = prev_property,
        };
        prev_property = std::addressof(nested_command_buffer_properties);
        VkPhysicalDeviceDescriptorHeapPropertiesEXT descriptor_heap_properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT,
            .pNext = prev_property,
        };
        prev_property = std::addressof(descriptor_heap_properties);
        VkPhysicalDeviceShaderObjectPropertiesEXT shader_object_properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT,
            .pNext = prev_property,
        };
        prev_property = std::addressof(shader_object_properties);
        VkPhysicalDeviceVulkan14Properties properties_14 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES,
            .pNext = prev_property,
        };
        prev_property = std::addressof(properties_14);
        VkPhysicalDeviceVulkan13Properties properties_13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES,
            .pNext = prev_property,
        };
        prev_property = std::addressof(properties_13);
        VkPhysicalDeviceVulkan12Properties properties_12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES,
            .pNext = prev_property,
        };
        prev_property = std::addressof(properties_12);
        VkPhysicalDeviceVulkan11Properties properties_11 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
            .pNext = prev_property,
        };
        prev_property = std::addressof(properties_11);
        VkPhysicalDeviceProperties2 properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = prev_property,
        };

        /* Feature structs */
        void *prev_feature = nullptr;
        VkPhysicalDeviceConditionalRenderingFeaturesEXT conditional_rendering_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(conditional_rendering_features);
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
            .pNext = prev_feature,
        };
        prev_feature = (missing_features->IsOptionalFeature(OptionalFeature::SupportsFragmentShadingRate)) ? std::addressof(fragment_shading_rate_features) : prev_feature;
        VkPhysicalDeviceImageTilingControlFeaturesEXT image_tiling_control_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_TILING_CONTROL_FEATURES_EXT,
            .pNext = prev_feature,
        };
        prev_feature = (missing_features->IsOptionalFeature(OptionalFeature::SupportsImageTilingControl)) ? std::addressof(image_tiling_control_features) : prev_feature;
        VkPhysicalDeviceImageCompressionControlFeaturesEXT image_compression_control_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT,
            .pNext = prev_feature,
        };
        prev_feature = (missing_features->IsOptionalFeature(OptionalFeature::SupportsImageCompressionControl)) ? std::addressof(image_compression_control_features) : prev_feature;
        VkPhysicalDeviceDeviceAddressCommandsFeaturesKHR device_address_commands_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_ADDRESS_COMMANDS_FEATURES_KHR,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(device_address_commands_features);
        VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
            .pNext = prev_feature,
        };
        prev_feature = (missing_features->IsOptionalFeature(OptionalFeature::SupportsMeshShaders)) ? std::addressof(mesh_shader_features) : prev_feature;
        VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT vertex_attribute_robustness_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT,
            .pNext = prev_feature,
        };
        prev_feature = (missing_features->IsOptionalFeature(OptionalFeature::SupportsVertexAttributeRobustness)) ? std::addressof(vertex_attribute_robustness_features) : prev_feature;
        VkPhysicalDeviceRobustness2FeaturesEXT robustness_2_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
            .pNext = prev_feature,
        };
        prev_feature = (missing_features->IsOptionalFeature(OptionalFeature::SupportsRobustness2)) ? std::addressof(robustness_2_features) : prev_feature;

        VkPhysicalDeviceNestedCommandBufferFeaturesEXT nested_command_buffer_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(nested_command_buffer_features);
        VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptor_heap_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(descriptor_heap_features);
        VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(shader_object_features);
        VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR shader_maximal_reconvergence_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(shader_maximal_reconvergence_features);
        VkPhysicalDeviceVulkan14Features features_14 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(features_14);
        VkPhysicalDeviceVulkan13Features features_13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(features_13);
        VkPhysicalDeviceVulkan12Features features_12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(features_12);
        VkPhysicalDeviceVulkan11Features features_11 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = prev_feature,
        };
        prev_feature = std::addressof(features_11);
        VkPhysicalDeviceFeatures2 features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = prev_feature,
        };

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
        if (!features_11.storageBuffer16BitAccess)           { missing_features->SetMissing(RequiredFeature::SupportsStorageBuffer16BitAccess); }
        if (!features_11.uniformAndStorageBuffer16BitAccess) { missing_features->SetMissing(RequiredFeature::SupportsUniformAndStorageBuffer16BitAccess); }
        //if (!features_11.storageInputOutput16)               { missing_features->SetMissing(RequiredFeature::SupportsStorageInputOutput16); }
        if (!features_11.variablePointersStorageBuffer)      { missing_features->SetMissing(RequiredFeature::SupportsVariablePointersStorageBuffer); }
        if (!features_11.variablePointers)                   { missing_features->SetMissing(RequiredFeature::SupportsVariablePointers); }

        /* 1.2 feature checks */
        if (!features_12.shaderFloat16)                                 { missing_features->SetMissing(RequiredFeature::SupportsShaderFloat16); }
        if (!features_12.descriptorIndexing)                            { missing_features->SetMissing(RequiredFeature::SupportsDescriptorIndexing); }
        //if (!features_12.shaderInputAttachmentArrayDynamicIndexing)     { missing_features->SetMissing(RequiredFeature::SupportsShaderInputAttachmentArrayDynamicIndexing); }
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
        if (!features_13.shaderTerminateInvocation) { missing_features->SetMissing(RequiredFeature::SupportsTerminateInvocation); }
        if (!features_13.shaderDemoteToHelperInvocation) { missing_features->SetMissing(RequiredFeature::SupportsDemoteToHelper); }
        if (!features_13.synchronization2) { missing_features->SetMissing(RequiredFeature::SupportsSynchronization2); }
        if (!features_13.dynamicRendering) { missing_features->SetMissing(RequiredFeature::SupportsDynamicRendering); }
        if (!features_13.maintenance4)     { missing_features->SetMissing(RequiredFeature::SupportsMaintenance4); }

        /* 1.4 feature checks */
        if (!features_14.vertexAttributeInstanceRateDivisor) { missing_features->SetMissing(RequiredFeature::SupportsVertexAttributeInstanceRateDivisor); }
        if (!features_14.vertexAttributeInstanceRateZeroDivisor) { missing_features->SetMissing(RequiredFeature::SupportsVertexAttributeInstanceRateZeroDivisor); }
        if (!features_14.indexTypeUint8)                         { missing_features->SetMissing(RequiredFeature::SupportsIndexTypeU8); }
        if (!features_14.dynamicRenderingLocalRead)              { missing_features->SetMissing(RequiredFeature::SupportsDynamicRenderingLocalRead); }
        if (!features_14.maintenance5)                           { missing_features->SetMissing(RequiredFeature::SupportsMaintenance5); }
        if (!features_14.maintenance6)                           { missing_features->SetMissing(RequiredFeature::SupportsMaintenance6); }
        if (!features_14.hostImageCopy)                          { missing_features->SetMissing(RequiredFeature::SupportsHostImageCopy); }

        /* SupportsShaderObject checks */
        if (!shader_object_features.shaderObject) { missing_features->SetMissing(RequiredFeature::SupportsShaderObject); }

        /* SupportsDescriptorBuffer checks (heap) */
        if (!descriptor_heap_features.descriptorHeap) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
        if (descriptor_heap_properties.imageDescriptorSize > Context::cTargetMaxSeparateTextureDescriptorSize) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
        if (descriptor_heap_properties.imageDescriptorSize > Context::cTargetMaxImageDescriptorSize) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
        if (descriptor_heap_properties.samplerDescriptorSize > Context::cTargetMaxSeparateSamplerDescriptorSize) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
        const size_t max_sampler_desc = (descriptor_heap_properties.maxSamplerHeapSize - descriptor_heap_properties.minSamplerHeapReservedRange) / descriptor_heap_properties.samplerDescriptorSize;
        if (max_sampler_desc < features_info->sampler_descriptor_count) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }
        const size_t max_texture_view_desc = (descriptor_heap_properties.maxResourceHeapSize - descriptor_heap_properties.minResourceHeapReservedRange) / descriptor_heap_properties.imageDescriptorSize;
        if (max_texture_view_desc < features_info->texture_view_descriptor_count) { missing_features->SetMissing(RequiredFeature::SupportsDescriptorBuffer); }

        /* SupportsNestedCommandBuffers checks */
        if (!nested_command_buffer_features.nestedCommandBuffer)                { missing_features->SetMissing(RequiredFeature::SupportsNestedCommandBuffers); }
        if (!nested_command_buffer_features.nestedCommandBufferRendering)       { missing_features->SetMissing(RequiredFeature::SupportsNestedCommandBuffers); }
        //if (!nested_command_buffer_features.nestedCommandBufferSimultaneousUse) { missing_features->SetMissing(RequiredFeature::SupportsNestedCommandBuffers); }

        /* SupportsShaderMaximalReconvergence checks */
        if (!shader_maximal_reconvergence_features.shaderMaximalReconvergence) { missing_features->SetMissing(OptionalFeature::SupportsShaderMaximalReconvergence); }

        /* SupportsRobustness2 checks */
        if (!features.features.robustBufferAccess) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
        if (!properties_12.robustBufferAccessUpdateAfterBind) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
        if (!features_13.robustImageAccess) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
        if (!robustness_2_features.robustBufferAccess2) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
        if (!robustness_2_features.robustImageAccess2)  { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
        if (!robustness_2_features.nullDescriptor)      { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
        if (robustness_2_properties.robustStorageBufferAccessSizeAlignment > Context::cTargetStorageBufferAlignment) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }
        if (robustness_2_properties.robustUniformBufferAccessSizeAlignment > Context::cTargetUniformBufferAlignment) { missing_features->SetMissing(OptionalFeature::SupportsRobustness2); }

        /* SupportsVertexAttributeRobustness checks */
        if (!vertex_attribute_robustness_features.vertexAttributeRobustness) { missing_features->SetMissing(OptionalFeature::SupportsVertexAttributeRobustness); }

        /* SupportsMeshShaders checks  */
        if (!mesh_shader_features.taskShader) { missing_features->SetMissing(OptionalFeature::SupportsMeshShaders); }
        if (!mesh_shader_features.meshShader) { missing_features->SetMissing(OptionalFeature::SupportsMeshShaders); }

        /* SupportsDeviceAddressCommands checks */
        if (!device_address_commands_features.deviceAddressCommands) { missing_features->SetMissing(RequiredFeature::SupportsDeviceAddressCommands); }

        /* SupportsConditionalRendering checks */
        if (!conditional_rendering_features.conditionalRendering) { missing_features->SetMissing(RequiredFeature::SupportsConditionalRendering); }
        if (!conditional_rendering_features.inheritedConditionalRendering) { missing_features->SetMissing(RequiredFeature::SupportsConditionalRendering); }

        /* SupportsFragmentShadingRate checks */
        if (!fragment_shading_rate_features.attachmentFragmentShadingRate) { missing_features->SetMissing(OptionalFeature::SupportsFragmentShadingRate); }

        /* SupportsImageCompressionControl checks */
        if (!image_compression_control_features.imageCompressionControl) { missing_features->SetMissing(OptionalFeature::SupportsImageCompressionControl); }

        /* SupportsImageTilingControl checks */
        if (!image_tiling_control_features.imageTilingControl) { missing_features->SetMissing(OptionalFeature::SupportsImageTilingControl); }

#ifdef VP_TARGET_PLATFORM_linux
        /* Ensure device is compatible with the underlying windowing system TODO; move to application defined IFunction filter */
        if (!IsWaylandCompatibleVkPhysicalDevice(std::addressof(drm_props))) { missing_features->SetMissing(RequiredFeature::SupportsExternalMemoryHost); }
#endif

        return;
    }

#ifdef AWN_VK_DEBUG
    void Context::ReportDeviceLostReason() {

        sys::SleepThread(TimeSpan::FromMilliSeconds(1000));
        VkDeviceFaultCountsEXT fault_counts = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT,
        };
        VkDeviceFaultInfoEXT fault_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT,
        };
        const VkResult result0 = ::pfn_vkGetDeviceFaultInfoEXT(m_vk_device, std::addressof(fault_counts), nullptr);
        if (result0 != VK_INCOMPLETE) {
            RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result0));
        }

        fault_info.pAddressInfos = reinterpret_cast<VkDeviceFaultAddressInfoEXT*>(::malloc(sizeof(VkDeviceFaultAddressInfoEXT) * fault_counts.addressInfoCount));
        fault_info.pVendorInfos = reinterpret_cast<VkDeviceFaultVendorInfoEXT*>(::malloc(sizeof(VkDeviceFaultVendorInfoEXT) * fault_counts.vendorInfoCount));
        VP_ASSERT(fault_info.pAddressInfos != nullptr);
        VP_ASSERT(fault_info.pVendorInfos != nullptr);
        ON_SCOPE_EXIT {
            ::free(fault_info.pVendorInfos);
            ::free(fault_info.pAddressInfos);
        };

        const VkResult result1 = ::pfn_vkGetDeviceFaultInfoEXT(m_vk_device, std::addressof(fault_counts), std::addressof(fault_info));
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result1));

        ::puts(fault_info.description);

        return;
    }
#endif


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
#ifdef VP_TARGET_PLATFORM_win32
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elifdef VP_TARGET_PLATFORM_linux
            VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
            VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
        };
        constexpr const u32 cInstanceExtensionCount = sizeof(cInstanceExtensionArray) / sizeof(const char*);

        #if defined(AWN_VK_DEBUG)
        /* Note; sync validation currently crashes Layer presentation on Linux as of 1.4.341 */
        const VkValidationFeatureEnableEXT validation_feature_enable_array[] = {
            //VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
        };
        const VkValidationFeaturesEXT validation_features = {
            .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
            .enabledValidationFeatureCount = sizeof(validation_feature_enable_array) / sizeof(VkValidationFeatureEnableEXT),
            .pEnabledValidationFeatures = validation_feature_enable_array
        };
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
                .pNext = std::addressof(validation_features),
            #endif
            .pApplicationInfo = std::addressof(app_info),
            #if defined(AWN_VK_DEBUG)
                .enabledLayerCount = cDebugLayerCount,
                .ppEnabledLayerNames = cDebugLayerArray,
            #endif
            .enabledExtensionCount = cInstanceExtensionCount,
            .ppEnabledExtensionNames = cInstanceExtensionArray
        };
        const VkResult vk_result = ::pfn_vkCreateInstance(std::addressof(instance_info), this->GetVkAllocationCallbacks(), std::addressof(m_vk_instance));
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(vk_result));

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

    void Context::SetVkPhysicalDevice(VkPhysicalDevice vk_physical_device, const FeaturesInfo *features_info) {

        /* Get properties and features */
        FindMissingFeatures(std::addressof(m_missing_features), vk_physical_device, features_info);
        ::pfn_vkGetPhysicalDeviceProperties2(vk_physical_device, std::addressof(m_vk_physical_device_properties));
        ::pfn_vkGetPhysicalDeviceMemoryProperties(vk_physical_device, std::addressof(m_vk_physical_device_memory_properties));
        m_is_unified_memory = (m_vk_physical_device_memory_properties.memoryHeapCount == 1);
        //m_is_ext_bindless_nv = (this->GetTextureSamplerDescriptorSize() == sizeof(size_t));

        /* Get queue family indices in enum order */
        /* Todo; make sure each are unique, or combine families */
        const QueueType queue_type_array[] = {
            QueueType::Graphics,
            QueueType::Compute,
            QueueType::Transfer,
            QueueType::VideoDecode,
            QueueType::VideoEncode,
            QueueType::OpticalFlow,
        };
        const u32 queue_count = 1;
        FindQueueFamilyIndices(vk_physical_device, queue_type_array, queue_count, m_queue_family_index_array_enum);

        /* Build linear queue family indice array */
        m_queue_family_index_count = 0;
        for (u32 i = 0; i < queue_count; ++i) {
            if (m_queue_family_index_array_enum[i] == cInvalidQueueFamilyIndex) { continue; }
            m_queue_family_index_array_linear[m_queue_family_index_count] = m_queue_family_index_array_enum[i];
            ++m_queue_family_index_count;
        }

        m_vk_physical_device = vk_physical_device;

#ifdef VP_TARGET_PLATFORM_linux
        /* Create gbm device */
        //int dir_fd = ::open("/dev/dri/", O_RDONLY | O_DIRECTORY);
        //VP_ASSERT(0 <= dir_fd);
        //ON_SCOPE_EXIT {
        //    ::close(dir_fd);
        //};
        //DIR *dir = ::fdopendir(dir_fd);
        //VP_ASSERT(dir != nullptr);
        //ON_SCOPE_EXIT {
        //    ::closedir(dir);
        //};
        //const VkPhysicalDeviceDrmPropertiesEXT *drm_props = std::addressof(m_vk_physical_device_drm_properties);
        //const dev_t primary = ::makedev(drm_props->primaryMajor, drm_props->primaryMinor);
        //const dev_t render = ::makedev(drm_props->renderMajor, drm_props->renderMinor);
        //struct dirent *ent;
        //while ((ent = ::readdir(dir), ent != nullptr)) {
        //    if (ent->d_type != DT_CHR) { continue; }
        //    struct stat s;
        //    if (::fstatat(dir_fd, ent->d_name, std::addressof(s), 0) != 0) { continue; }
        //    const bool is_primary = drm_props->hasPrimary && s.st_rdev == primary;
        //    const bool is_render = drm_props->hasRender && s.st_rdev == render;
        //    ::printf("dev_t(%s): %d, %d\n", ent->d_name, ::major(s.st_rdev), ::minor(s.st_rdev));
        //    if (is_primary || is_render) {
        //        errno = 0;
        //        const int fd = ::openat(dir_fd, ent->d_name, O_RDWR);
        //        if (fd < 0) {
        //            ::printf("errno: %d\n", errno);
        //            continue;
        //        }
        //        auto fail_guard0 = SCOPE_GUARD {
        //            ::close(fd);
        //        };
        //        struct gbm_device *device = ::gbm_create_device(fd);
        //        VP_ASSERT(device != nullptr);
        //        fail_guard0.Cancel();
        //        m_drm_fd = fd;
        //        m_gbm_device = device;
        //        break;
        //    }
        //}
        //VP_ASSERT(0 <= m_drm_fd);
        //VP_ASSERT(m_gbm_device != nullptr);
#endif

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
#ifdef VP_TARGET_PLATFORM_linux
            VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
            VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
            VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
            VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
#endif
#ifdef AWN_VK_DEBUG
            //VK_EXT_DEVICE_FAULT_EXTENSION_NAME,
#endif
            VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME,
            VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME,
            VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
            VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME,
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
                .extension = VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsFragmentShadingRate,
            },
            {
                .extension = VK_EXT_IMAGE_TILING_CONTROL_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsImageTilingControl,
            },
            {
                .extension = VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME,
                .optional_feature = OptionalFeature::SupportsImageCompressionControl,
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
        void *prev_feature = nullptr;
        VkPhysicalDeviceConditionalRenderingFeaturesEXT conditional_rendering_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT,
            .pNext = prev_feature,
            .conditionalRendering = VK_TRUE,
            .inheritedConditionalRendering = VK_TRUE,
        };
        prev_feature = std::addressof(conditional_rendering_features);
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
            .pNext = prev_feature,
            .attachmentFragmentShadingRate = VK_TRUE,
        };
        prev_feature = (this->IsOptionalFeature(OptionalFeature::SupportsFragmentShadingRate)) ? std::addressof(fragment_shading_rate_features) : prev_feature;
        VkPhysicalDeviceImageTilingControlFeaturesEXT image_tiling_control_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_TILING_CONTROL_FEATURES_EXT,
            .pNext = prev_feature,
            .imageTilingControl = VK_TRUE,
        };
        prev_feature = (this->IsOptionalFeature(OptionalFeature::SupportsImageTilingControl)) ? std::addressof(image_tiling_control_features) : prev_feature;
        VkPhysicalDeviceImageCompressionControlFeaturesEXT image_compression_control_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT,
            .pNext = prev_feature,
            .imageCompressionControl = VK_TRUE,
        };
        prev_feature = (this->IsOptionalFeature(OptionalFeature::SupportsImageCompressionControl)) ? std::addressof(image_compression_control_features) : prev_feature;
        VkPhysicalDeviceDeviceAddressCommandsFeaturesKHR device_address_commands_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_ADDRESS_COMMANDS_FEATURES_KHR,
            .pNext = prev_feature,
            .deviceAddressCommands = VK_TRUE,
        };
        prev_feature = std::addressof(device_address_commands_features);
        const bool is_mesh_shaders = this->IsOptionalFeature(OptionalFeature::SupportsMeshShaders);
        VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
            .pNext = prev_feature,
            .taskShader = VK_TRUE,
            .meshShader = VK_TRUE,
            //.multiviewMeshShader = is_mesh_shaders,
            //.primitiveFragmentShadingRateMeshShader = is_mesh_shaders,
            //.meshShaderQueries = is_mesh_shaders,
        };
        prev_feature = (is_mesh_shaders) ? std::addressof(mesh_shader_features) : prev_feature;
        const bool is_vertex_attribute_robustness = this->IsOptionalFeature(OptionalFeature::SupportsVertexAttributeRobustness);
        VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT vertex_attribute_robustness_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT,
            .pNext = prev_feature,
            .vertexAttributeRobustness = true,
        };
        prev_feature = (is_vertex_attribute_robustness) ? std::addressof(vertex_attribute_robustness_features) : prev_feature;
        const bool is_robustness2 = this->IsOptionalFeature(OptionalFeature::SupportsRobustness2);
        VkPhysicalDeviceRobustness2FeaturesEXT robustness_2_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
            .pNext = prev_feature,
            .robustBufferAccess2 = true,
            .robustImageAccess2 = true,
            .nullDescriptor = true,
        };
        prev_feature = (is_robustness2) ? std::addressof(robustness_2_features) : prev_feature;
        const bool is_maximal_reconvergance = this->IsOptionalFeature(OptionalFeature::SupportsShaderMaximalReconvergence);
        VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR shader_maximal_reconvergence_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR,
            .pNext = prev_feature,
            .shaderMaximalReconvergence = true,
        };
        prev_feature = (is_maximal_reconvergance) ? std::addressof(shader_maximal_reconvergence_features) : prev_feature;
        
#ifdef AWN_VK_DEBUG
        //VkPhysicalDeviceFaultFeaturesEXT fault_features = {
        //    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT,
        //    .pNext = prev_feature,
        //    .deviceFault = VK_TRUE,
        //};
        //prev_feature = std::addressof(fault_features);
#endif
        VkPhysicalDeviceNestedCommandBufferFeaturesEXT nested_command_buffer_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
            .pNext = prev_feature,
            .nestedCommandBuffer = VK_TRUE,
            .nestedCommandBufferRendering = VK_TRUE,
            //.nestedCommandBufferSimultaneousUse = VK_TRUE,
        };
        prev_feature = std::addressof(nested_command_buffer_features);
        VkPhysicalDeviceDescriptorHeapFeaturesEXT target_descriptor_heap_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT,
            .pNext = prev_feature,
            .descriptorHeap = VK_TRUE,
        };
        prev_feature = std::addressof(target_descriptor_heap_features);
        VkPhysicalDeviceShaderObjectFeaturesEXT target_shader_object_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
            .pNext = prev_feature,
            .shaderObject = VK_TRUE,
        };
        prev_feature = std::addressof(target_shader_object_features);
        VkPhysicalDeviceVulkan14Features target_1_4_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
            .pNext = prev_feature,
            .vertexAttributeInstanceRateDivisor = VK_TRUE,
            .vertexAttributeInstanceRateZeroDivisor = VK_TRUE,
            .indexTypeUint8 = VK_TRUE,
            .dynamicRenderingLocalRead = VK_TRUE,
            .maintenance5 = VK_TRUE,
            .maintenance6 = VK_TRUE,
            .hostImageCopy = VK_TRUE,
        };
        prev_feature = std::addressof(target_1_4_features);
        VkPhysicalDeviceVulkan13Features target_1_3_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = prev_feature,
            .robustImageAccess = is_robustness2,
            .shaderDemoteToHelperInvocation = VK_TRUE,
            .shaderTerminateInvocation = VK_TRUE,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
            .maintenance4 = VK_TRUE,
        };
        prev_feature = std::addressof(target_1_3_features);
        VkPhysicalDeviceVulkan12Features target_1_2_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = prev_feature,
            //.storageBuffer8BitAccess                       = VK_TRUE,
            //.uniformAndStorageBuffer8BitAccess             = VK_TRUE,
            .shaderFloat16                                 = VK_TRUE,
            .descriptorIndexing                            = VK_TRUE,
            //.shaderInputAttachmentArrayDynamicIndexing     = VK_TRUE,
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
        prev_feature = std::addressof(target_1_2_features);
        VkPhysicalDeviceVulkan11Features target_1_1_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = prev_feature,
            .storageBuffer16BitAccess = VK_TRUE,
            .uniformAndStorageBuffer16BitAccess = VK_TRUE,
            //.storageInputOutput16 = VK_TRUE,
            //.multiview = VK_TRUE,
            .variablePointersStorageBuffer = VK_TRUE,
            .variablePointers = VK_TRUE,
        };
        prev_feature = std::addressof(target_1_1_features);
        const VkPhysicalDeviceFeatures2 target_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = prev_feature,
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
        const VkResult vk_result = ::pfn_vkCreateDevice(m_vk_physical_device, std::addressof(device_info), this->GetVkAllocationCallbacks(), std::addressof(m_vk_device));
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(vk_result));

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

	void Context::Initialize(const ContextInfo *context_info) {

        /* Load initial vulkan procs */
        ::LoadVkCProcsInitial();

        /* Check supported Vulkan Api version */
        {
            u32 api_version = 0;
            const VkResult result0 = ::pfn_vkEnumerateInstanceVersion(std::addressof(api_version));
            RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result0));
            VP_ASSERT(cTargetMinimumApiVersion <= api_version);
        }

		/* Initialize default instance */
        this->InitializeInstance(context_info);

        #if defined(AWN_VK_DEBUG)
        /* Create debug messenger */
        {
            const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = DebugCallback,
                .pUserData = nullptr
            };

            const VkResult result = ::pfn_vkCreateDebugUtilsMessengerEXT(m_vk_instance, std::addressof(debug_messenger_info), this->GetVkAllocationCallbacks(), std::addressof(m_debug_messenger));
            RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result));
        }
        #endif

        /* Find our default physical device */
        VkPhysicalDevice vk_physical_device = FindDefaultPhysicalDevice(m_vk_instance, context_info);
        VP_ASSERT(vk_physical_device != VK_NULL_HANDLE);
        this->SetVkPhysicalDevice(vk_physical_device, std::addressof(context_info->features_info));

		/* Initialize device */
        this->InitializeLogicalDevice(context_info);

        return;
	}

    void Context::Finalize() {

        /* Ensure all device operations complete */
        ::pfn_vkDeviceWaitIdle(m_vk_device);

		/* Finalize device */
        this->FinalizeLogicalDevice();

        m_vk_physical_device = VK_NULL_HANDLE;
#ifdef VP_TARGET_PLATFORM_linux
        //if (m_gbm_device != nullptr) {
        //    ::gbm_device_destroy(m_gbm_device);
        //    m_gbm_device = nullptr;
        //}
        //if (0 <= m_drm_fd) {
        //    ::close(m_drm_fd);
        //    m_drm_fd = -1;
        //}
#endif

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

    void Context::Import(const ContextImportInfo *import_info) {
        m_vk_instance = import_info->vk_instance;
        this->SetVkPhysicalDevice(import_info->vk_physical_device, std::addressof(import_info->features_info));
        m_vk_device = import_info->vk_device;
        m_optional_features = import_info->optional_feature_mask;
        for (u32 i = 0; i < cTargetMaxQueueCount; ++i) {
            if (m_queue_family_index_array_enum[i] == cInvalidQueueFamilyIndex) { continue; }
            ::pfn_vkGetDeviceQueue(m_vk_device, m_queue_family_index_array_enum[i], 0, std::addressof(m_vk_queue_array_enum[i]));
        }
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
            .commandBuffer = command_list.GetVkCommandBuffer(),
            .deviceMask    = 0,
        };
        VkSemaphoreSubmitInfo wait_semaphore_array[cMaxQueueWaitSyncCount];
        for (u32 i = 0; i < wait_sync_count; ++i) {
            wait_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            wait_semaphore_array[i].pNext       = nullptr;
            wait_semaphore_array[i].semaphore   = wait_sync_array[i]->GetVkSemaphore();
            wait_semaphore_array[i].value       = wait_sync_array[i]->GetExpectedValue();
            wait_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            wait_semaphore_array[i].deviceIndex = 0;
        }
        VkSemaphoreSubmitInfo signal_semaphore_array[cMaxQueueSignalSyncCount];
        for (u32 i = 0; i < signal_sync_count; ++i) {
            signal_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            signal_semaphore_array[i].pNext       = nullptr;
            signal_semaphore_array[i].semaphore   = signal_sync_array[i]->GetVkSemaphore();
            signal_semaphore_array[i].value       = signal_sync_array[i]->GetExpectedValue();
            signal_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
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
        const VkResult result0 = ::pfn_vkQueueSubmit2(this->GetVkQueue(command_list.GetQueueType()), 1, std::addressof(graphics_submit_info), VK_NULL_HANDLE);
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result0));

        return;
    }

    void Context::SubmitCommandLists(CommandList *command_list_array, u32 command_list_count, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count) {

        /* Integrity checks */
        VP_ASSERT(command_list_count < cMaxQueueCommandListCount);
        VP_ASSERT(wait_sync_count < cMaxQueueWaitSyncCount);
        VP_ASSERT(signal_sync_count < cMaxQueueSignalSyncCount);

        /* Setup command buffer submit info, wait and signal semaphore infos */
        VkCommandBufferSubmitInfo command_buffer_submit_info_array[cMaxQueueCommandListCount];
        u32 queue_type = static_cast<u32>(command_list_array[0].GetQueueType());
        for (u32 i = 0; i < command_list_count; ++i) {
            VP_ASSERT(queue_type == static_cast<u32>(command_list_array[i].GetQueueType()));
            command_buffer_submit_info_array[i].sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            command_buffer_submit_info_array[i].pNext         = nullptr;
            command_buffer_submit_info_array[i].commandBuffer = command_list_array[i].GetVkCommandBuffer();
            command_buffer_submit_info_array[i].deviceMask    = 0;
        }
        VkSemaphoreSubmitInfo wait_semaphore_array[cMaxQueueWaitSyncCount];
        for (u32 i = 0; i < wait_sync_count; ++i) {
            wait_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            wait_semaphore_array[i].pNext       = nullptr;
            wait_semaphore_array[i].semaphore   = wait_sync_array[i]->GetVkSemaphore();
            wait_semaphore_array[i].value       = wait_sync_array[i]->GetExpectedValue();
            wait_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            wait_semaphore_array[i].deviceIndex = 0;
        }
        VkSemaphoreSubmitInfo signal_semaphore_array[cMaxQueueSignalSyncCount];
        for (u32 i = 0; i < signal_sync_count; ++i) {
            signal_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            signal_semaphore_array[i].pNext       = nullptr;
            signal_semaphore_array[i].semaphore   = signal_sync_array[i]->GetVkSemaphore();
            signal_semaphore_array[i].value       = signal_sync_array[i]->GetExpectedValue();
            signal_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
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
        const VkResult result0 = ::pfn_vkQueueSubmit2(this->GetVkQueue(static_cast<QueueType>(queue_type)), 1, std::addressof(graphics_submit_info), VK_NULL_HANDLE);
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result0));

        return;
    }
}

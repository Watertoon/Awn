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

        void FindMissingFeatures(MissingFeatures *missing_features, VkPhysicalDevice vk_physical_device) {

            /* Property structs */
            VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
                .pNext = nullptr,
            };
            VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT,
                .pNext = std::addressof(descriptor_buffer_properties),
            };
            VkPhysicalDeviceShaderObjectPropertiesEXT shader_object_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT,
                .pNext = std::addressof(mesh_shader_properties),
            };
            VkPhysicalDeviceHostImageCopyPropertiesEXT host_image_copy_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES_EXT,
                .pNext = std::addressof(shader_object_properties),
            };
            VkPhysicalDeviceMaintenance5PropertiesKHR maintenance_5_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR,
                .pNext = std::addressof(host_image_copy_properties),
            };
            VkPhysicalDeviceNestedCommandBufferPropertiesEXT nested_command_buffer_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT,
                .pNext = std::addressof(maintenance_5_properties),
            };
            VkPhysicalDeviceCooperativeMatrixPropertiesKHR cooperative_matrix_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR,
                .pNext = std::addressof(nested_command_buffer_properties),
            };
            VkPhysicalDeviceExtendedDynamicState3PropertiesEXT extended_dynamic_state_3_properties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT,
                .pNext = std::addressof(cooperative_matrix_properties),
            };
            VkPhysicalDeviceVulkan13Properties properties_13 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES,
                .pNext = std::addressof(extended_dynamic_state_3_properties),
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
            VkPhysicalDevicePresentWaitFeaturesKHR present_wait_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR,
                .pNext = nullptr,
            };
            VkPhysicalDevicePresentIdFeaturesKHR present_id_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR,
                .pNext = std::addressof(present_wait_features),
            };
            VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchain_1_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT,
                .pNext = std::addressof(present_id_features),
            };
            VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperative_matrix_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR,
                .pNext = std::addressof(swapchain_1_features),
            };
            VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR shader_subgroup_uniform_control_flow_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR,
                .pNext = std::addressof(cooperative_matrix_features),
            };
            VkPhysicalDeviceShaderSubgroupRotateFeaturesKHR shader_subgroup_rotate_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES_KHR,
                .pNext = std::addressof(shader_subgroup_uniform_control_flow_features),
            };
            VkPhysicalDeviceShaderQuadControlFeaturesKHR   shader_quad_control_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR,
                .pNext = std::addressof(shader_subgroup_rotate_features),
            };
            VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR shader_maximal_reconvergence_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR,
                .pNext = std::addressof(shader_quad_control_features),
            };
            VkPhysicalDeviceShaderFloatControls2FeaturesKHR shader_float_controls_2_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES_KHR,
                .pNext = std::addressof(shader_maximal_reconvergence_features),
            };
            VkPhysicalDeviceShaderExpectAssumeFeaturesKHR shader_expect_assume_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES_KHR,
                .pNext = std::addressof(shader_float_controls_2_features),
            };
            VkPhysicalDeviceShaderClockFeaturesKHR  shader_clock_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR,
                .pNext = std::addressof(shader_expect_assume_features),
            };
            VkPhysicalDeviceIndexTypeUint8FeaturesKHR index_type_u8_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_KHR,
                .pNext = std::addressof(shader_clock_features),
            };
            VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamic_rendering_local_read_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR,
                .pNext = std::addressof(index_type_u8_features),
            };
            VkPhysicalDeviceMaintenance6FeaturesKHR maintenance_6_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES_KHR,
                .pNext = std::addressof(dynamic_rendering_local_read_features),
            };
            VkPhysicalDeviceNestedCommandBufferFeaturesEXT nested_command_buffer_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
                .pNext = std::addressof(maintenance_6_features),
            };
            VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamic_rendering_unused_attachment_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT,
                .pNext = std::addressof(nested_command_buffer_features),
            };
            VkPhysicalDeviceMaintenance5FeaturesKHR maintenance_5_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
                .pNext = std::addressof(dynamic_rendering_unused_attachment_features),
            };
            VkPhysicalDeviceHostImageCopyFeaturesEXT host_image_copy_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT,
                .pNext = std::addressof(maintenance_5_features),
            };
            VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
                .pNext = std::addressof(host_image_copy_features),
            };
            VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
                .pNext = std::addressof(shader_object_features),
            };
            VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptor_buffer_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
                .pNext = std::addressof(mesh_shader_features),
            };
            VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state_3_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
                .pNext = std::addressof(descriptor_buffer_features),
            };
            VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state_2_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                .pNext = std::addressof(extended_dynamic_state_3_features),
            };
            VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_state_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                .pNext = std::addressof(extended_dynamic_state_2_features),
            };
            VkPhysicalDeviceVulkan13Features features_13 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                .pNext = std::addressof(vertex_input_dynamic_state_features),
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

            ///* Memory property structs */
            //VkPhysicalDeviceMemoryProperties memory_properties = {
            //    .sType = ,
            //};

            /* Query Physical Device */
            ::pfn_vkGetPhysicalDeviceProperties2(vk_physical_device, std::addressof(properties));
            //::pfn_vkGetPhysicalDeviceMemoryProperties(vk_physical_device, std::addressof(memory_properties));
            ::pfn_vkGetPhysicalDeviceFeatures2(vk_physical_device, std::addressof(features));

            /*  Ensure Vulkan 1.3 support */
            if (!(Context::cTargetMinimumApiVersion <= properties.properties.apiVersion)) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::TargetApiVersion)); }

            /* Ensure resource ubos meet our size requirements */
            if (properties.properties.limits.maxUniformBufferRange < Context::cTargetMaxUniformBufferSize) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::TargetUniformBufferSize)); }

            /* Ensure graphics queue */
            {
                const QueueType queue_type = QueueType::Graphics;
                u32 graphics_queue_family_index = 0;
                FindQueueFamilyIndices(vk_physical_device, std::addressof(queue_type), 1, std::addressof(graphics_queue_family_index));
                if (graphics_queue_family_index == Context::cInvalidQueueFamilyIndex) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::GraphicsQueue)); };

                //const u32 result = ::pfn_vkGetPhysicalDeviceWin32PresentationSupportKHR(vk_physical_device, graphics_queue_family_index);
                //if (!result) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsPresent)); }
            }

            /* Ensure our memory alignment is compatible with out TargetMemoryAlignment */
            {
                const size_t vk_alignment = properties.properties.limits.minMemoryMapAlignment;
                if (Context::cTargetMemoryPoolAlignment % vk_alignment != 0 || !(Context::cTargetMemoryPoolAlignment / vk_alignment > 0)) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::TargetMemoryMapAlignment)); }
            }
            {
                const size_t vk_alignment = properties.properties.limits.minUniformBufferOffsetAlignment;
                if (Context::cTargetUniformBufferAlignment % vk_alignment != 0 || !(Context::cTargetUniformBufferAlignment / vk_alignment > 0)) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::TargetUniformBufferAlignment)); }
            }
            {
                const size_t vk_alignment = properties.properties.limits.minStorageBufferOffsetAlignment;
                if (Context::cTargetStorageBufferAlignment % vk_alignment != 0 || !(Context::cTargetStorageBufferAlignment / vk_alignment > 0)) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::TargetStorageBufferAlignment)); }
            }
            {
                const size_t vk_alignment = properties.properties.limits.minTexelBufferOffsetAlignment;
                if (Context::cTargetTexelBufferAlignment % vk_alignment != 0 || !(Context::cTargetTexelBufferAlignment / vk_alignment > 0)) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::TargetTexelBufferAlignment)); }
            }

            /* Property checks */
            if (properties_12.maxDescriptorSetUpdateAfterBindSampledImages  < Context::cTargetMaxTextureDescriptorCount) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::TargetMaxTextureDescriptors)); }
            if (properties_12.maxDescriptorSetUpdateAfterBindSamplers       < Context::cTargetMaxSamplerDescriptorCount) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::TargetMaxSamplerDescriptors)); }
            if (!extended_dynamic_state_3_properties.dynamicPrimitiveTopologyUnrestricted) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::TargetMaxSamplerDescriptors)); }
            //if ((cooperative_matrix_properties.cooperativeMatrixSupportedStages & VK_SHADER_STAGE_COMPUTE_BIT) != VK_SHADER_STAGE_COMPUTE_BIT) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsCooperativeMatrix)); }

            /* Base feature checks */
            if (!features.features.fullDrawIndexUint32)                     { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsFullDrawU32)); }
            if (!features.features.independentBlend)                        { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsIndependentBlend)); }
            if (!features.features.geometryShader)                          { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsGeometryShaders)); }
            if (!features.features.tessellationShader)                      { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsTessellationShaders)); }
            if (!features.features.logicOp)                                 { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsLogicOperation)); }
            if (!features.features.depthClamp)                              { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDepthClamp)); }
            if (!features.features.depthBiasClamp)                          { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDepthBiasClamp)); }
            if (!features.features.fillModeNonSolid)                        { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsFillModeNonSolid)); }
            //if (!features.features.depthBounds)                             { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDepthBounds)); }
            if (!features.features.wideLines)                               { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsWideLines)); }
            if (!features.features.largePoints)                             { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsLargePoints)); }
            if (!features.features.multiViewport)                           { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsMultiViewport)); }
            if (!features.features.samplerAnisotropy)                       { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsSamplerAnisotropy)); }
            if (!features.features.textureCompressionBC)                    { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsBC)); }
            if (!features.features.fragmentStoresAndAtomics)                { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsFragmentStoresAndAtomics)); }
            if (!features.features.shaderStorageImageExtendedFormats)       { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderStorageImageExtendedFormats)); }
            if (!features.features.shaderUniformBufferArrayDynamicIndexing) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderUniformBufferArrayDynamicIndexing)); }
            if (!features.features.shaderSampledImageArrayDynamicIndexing)  { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderSampledImageArrayDynamicIndexing)); }
            if (!features.features.shaderStorageBufferArrayDynamicIndexing) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderStorageBufferArrayDynamicIndexing)); }
            if (!features.features.shaderStorageImageArrayDynamicIndexing)  { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderStorageImageArrayDynamicIndexing)); }
            //if (!features.features.sparseBinding)                           { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsSparseBinding)); }

            /* 1.1 feature checks */
            if (!features_11.multiview)                                     { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsMultiView));  }
            if (!features_11.variablePointersStorageBuffer)                 { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsVariablePointersStorageBuffer)); }
            if (!features_11.variablePointers)                              { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsVariablePointers)); }

            /* 1.2 feature checks */
            if (!features_12.descriptorIndexing)                                 { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorIndexing)); }
            if (!features_12.shaderInputAttachmentArrayDynamicIndexing)          { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderInputAttachmentArrayDynamicIndexing)); }
            //if (!m_vk_physical_device_supported_features_12.shaderUniformTexelBufferArrayDynamicIndexing)       { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::Supports)); }
            //if (!m_vk_physical_device_supported_features_12.shaderStorageTexelBufferArrayDynamicIndexing)       { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::Supports)); }
            if (!features_12.shaderUniformBufferArrayNonUniformIndexing)         { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderUniformBufferArrayNonUniformIndexing)); }
            if (!features_12.shaderSampledImageArrayNonUniformIndexing)          { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderSampledImageArrayNonUniformIndexing)); }
            if (!features_12.shaderStorageBufferArrayNonUniformIndexing)         { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderStorageBufferArrayNonUniformIndexing)); }
            if (!features_12.shaderStorageImageArrayNonUniformIndexing)          { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderStorageImageArrayNonUniformIndexing)); }
            //if (!m_vk_physical_device_supported_features_12.shaderInputAttachmentArrayNonUniformIndexing)       { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::Supports)); }
            //if (!m_vk_physical_device_supported_features_12.shaderUniformTexelBufferArrayNonUniformIndexing)    { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::Supports)); }
            //if (!m_vk_physical_device_supported_features_12.shaderStorageTexelBufferArrayNonUniformIndexing)    { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::Supports)); }
            if (!features_12.descriptorBindingUniformBufferUpdateAfterBind)      { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBindingUniformBufferArrayUpdateAfterBind)); }
            if (!features_12.descriptorBindingSampledImageUpdateAfterBind)       { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBindingSampledImageArrayUpdateAfterBind)); }
            if (!features_12.descriptorBindingStorageImageUpdateAfterBind)       { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBindingStorageBufferArrayUpdateAfterBind)); }
            if (!features_12.descriptorBindingStorageBufferUpdateAfterBind)      { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBindingStorageImageArrayUpdateAfterBind)); }
            //if (!m_vk_physical_device_supported_features_12.descriptorBindingUniformTexelBufferUpdateAfterBind) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::Supports)); }
            //if (!m_vk_physical_device_supported_features_12.descriptorBindingStorageTexelBufferUpdateAfterBind) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::Supports)); }
            if (!features_12.descriptorBindingUpdateUnusedWhilePending)          { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBindingUpdateUnusedWhilePending)); }
            if (!features_12.descriptorBindingPartiallyBound)                    { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBindingPartiallyBound)); }
            if (!features_12.descriptorBindingVariableDescriptorCount)           { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBindingVariableDescriptorCount)); }
            if (!features_12.runtimeDescriptorArray)                             { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsRuntimeDescriptorArray)); }
            if (!features_12.scalarBlockLayout)                                  { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsScalarBlockLayout)); }
            if (!features_12.uniformBufferStandardLayout)                        { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsUniformBufferStandardLayout)); }
            if (!features_12.timelineSemaphore)                                  { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsTimelineSemaphore)); }
            if (!features_12.bufferDeviceAddress)                                { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsBufferDeviceAddress)); }
            //if (!features_12.bufferDeviceAddressCaptureReplay)                   { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsBufferDeviceAddressCaptureReplay)); }

            /* 1.3 feature checks */
            if (!features_13.synchronization2) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsSynchronization2)); }
            if (!features_13.dynamicRendering) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDynamicRendering)); }
            if (!features_13.maintenance4)     { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsMaintenance4)); }

            /* Vertex Input Dynamic State feature checks */
            if (!vertex_input_dynamic_state_features.vertexInputDynamicState) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsVertexInputDynamicState)); }

            /* Extended Dynamic State 2 feature checks */
            if (!extended_dynamic_state_2_features.extendedDynamicState2LogicOp) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsExtendedDynamicState2)); }

            /* Extended Dynamic State 3 feature checks */
            if (!extended_dynamic_state_3_features.extendedDynamicState3DepthClampEnable)   { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsExtendedDynamicState3)); }
            if (!extended_dynamic_state_3_features.extendedDynamicState3PolygonMode)        { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsExtendedDynamicState3)); }
            if (!extended_dynamic_state_3_features.extendedDynamicState3LogicOpEnable)      { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsExtendedDynamicState3)); }
            if (!extended_dynamic_state_3_features.extendedDynamicState3ColorBlendEnable)   { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsExtendedDynamicState3)); }
            if (!extended_dynamic_state_3_features.extendedDynamicState3ColorBlendEquation) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsExtendedDynamicState3)); }
            if (!extended_dynamic_state_3_features.extendedDynamicState3ColorWriteMask)     { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsExtendedDynamicState3)); }

            /* Descriptor Buffer feature checks */
            if (!descriptor_buffer_features.descriptorBuffer)                   { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBuffer)); }
            if (!descriptor_buffer_features.descriptorBufferImageLayoutIgnored) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBuffer)); }
            if (!descriptor_buffer_features.descriptorBufferPushDescriptors)    { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDescriptorBuffer)); }

            /* Mesh Shader feature checks  */
            if (!mesh_shader_features.taskShader) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsMeshShaders)); }
            if (!mesh_shader_features.meshShader) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsMeshShaders)); }

            /* Shader Object feature checks */
            if (!shader_object_features.shaderObject) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderObject)); }

            /* Host Image Copy feature checks */
            if (!host_image_copy_features.hostImageCopy) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsHostImageCopy)); }

            /* Maintenance 5 feature checks */
            if (!maintenance_5_features.maintenance5) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsMaintenance5)); }

            /* Unused attachments feature checks */
            if (!dynamic_rendering_unused_attachment_features.dynamicRenderingUnusedAttachments) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDynamicRenderingUnusedAttachments)); }
            
            /* Nested command buffer feature checks */
            if (!nested_command_buffer_features.nestedCommandBuffer)                { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsNestedCommandBuffers)); }
            if (!nested_command_buffer_features.nestedCommandBufferRendering)       { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsNestedCommandBuffers)); }
            if (!nested_command_buffer_features.nestedCommandBufferSimultaneousUse) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsNestedCommandBuffers)); }

            /* Maintenance 6 feature checks */
            if (!maintenance_6_features.maintenance6) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsMaintenance6)); }

            /* Dynamic Rendering Local Read features checks */
            if (!dynamic_rendering_local_read_features.dynamicRenderingLocalRead) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsDynamicRenderingLocalRead)); }

            /* Index type u8 features checks */
            if (!index_type_u8_features.indexTypeUint8) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsIndexTypeU8)); }

            /* Shader clock checks */
            if (!shader_clock_features.shaderSubgroupClock) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderClock)); }
            if (!shader_clock_features.shaderDeviceClock) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderClock)); }

            /* Shader expect assume checks */
            if (!shader_expect_assume_features.shaderExpectAssume) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderExpectAssume)); }

            /* Shader float controls 2 checks */
            //if (!shader_float_controls_2_features.shaderFloatControls2) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderFloatControls2)); }

            /* Shader maximal reconvergence checks */
            if (!shader_maximal_reconvergence_features.shaderMaximalReconvergence) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderMaximalReconvergence)); }

            /* Shader quad control checks */
            //if (!shader_quad_control_features.shaderQuadControl) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderQuadControl)); }

            /* Shader subgroup rotate checks */
            //if (!shader_subgroup_rotate_features.shaderSubgroupRotate)          { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderSubgroupRotate)); }
            //if (!shader_subgroup_rotate_features.shaderSubgroupRotateClustered) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderSubgroupRotate)); }

            /* Shader subgroup uniform control flow checks */
            //if (!shader_subgroup_uniform_control_flow_features.shaderSubgroupUniformControlFlow) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsShaderSubgroupUniformControlFlow)); }

            /* Shader cooperative matrix checks */
            //if (!cooperative_matrix_features.cooperativeMatrix) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsCooperativeMatrix)); }

            /* Swapchain maintenance 1 checks */
            if (!swapchain_1_features.swapchainMaintenance1) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsPresent)); }

            /* Present id checks */
            //if (!present_id_features.presentId) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsPresent)); }

            /* Present wait checks */
            //if (!present_wait_features.presentWait) { missing_features->missing_required_feature_mask.SetBit(static_cast<u32>(RequiredFeature::SupportsPresent)); }

            return;
        }

        VkPhysicalDevice FindDefaultPhysicalDevice(VkInstance vk_instance, const ContextInfo *context_info) {
            u32               vk_physical_device_count = 0;
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
                FindMissingFeatures(feature, vk_physical_device_array[i]);
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
            constexpr const char *debug_layer_array[] = {
                "VK_LAYER_KHRONOS_validation"
            };
            constexpr const u32 debug_layer_count = sizeof(debug_layer_array) / sizeof(const char*);
        #endif

        constexpr const char *instance_extension_array[] = {
            #if defined(AWN_VK_DEBUG)
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            #endif
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
        };
        constexpr const u32 instance_extension_count = sizeof(instance_extension_array) / sizeof(const char*);

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
                .enabledLayerCount = debug_layer_count,
                .ppEnabledLayerNames = debug_layer_array,
            #endif
            .enabledExtensionCount = instance_extension_count,
            .ppEnabledExtensionNames = instance_extension_array
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

    void Context::SetVkPhysicalDevice(VkPhysicalDevice vk_physical_device) {

        /* Get properties and features */
        ::pfn_vkGetPhysicalDeviceProperties2(vk_physical_device, std::addressof(m_vk_physical_device_properties));
        ::pfn_vkGetPhysicalDeviceMemoryProperties(vk_physical_device, std::addressof(m_vk_physical_device_memory_properties));
        ::pfn_vkGetPhysicalDeviceFeatures2(vk_physical_device, std::addressof(m_vk_physical_device_supported_features));

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

    void Context::InitializeLogicalDevice() {

        /* VkDevice feature enables */
        //static constinit VkPhysicalDevicePresentWaitFeaturesKHR present_wait_features = {
        //    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR,
        //    .pNext = nullptr,
        //    .presentWait = VK_TRUE,
        //};
        //static constinit VkPhysicalDevicePresentIdFeaturesKHR present_id_features = {
        //    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR,
        //    .pNext = std::addressof(present_wait_features),
        //    .presentId = VK_TRUE,
        //};
        static constinit VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchain_1_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT,
        //    .pNext = std::addressof(present_id_features),
            .pNext = nullptr,
            .swapchainMaintenance1 = VK_TRUE,
        };
        //static constinit VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR shader_subgroup_uniform_control_flow_features = {
        //    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR,
        //    .pNext = std::addressof(swapchain_1_features),
        //    .shaderSubgroupUniformControlFlow = VK_TRUE,
        //};
        //static constinit VkPhysicalDeviceShaderSubgroupRotateFeaturesKHR shader_subgroup_rotate_features = {
        //    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES_KHR,
        //    .pNext = std::addressof(shader_subgroup_uniform_control_flow_features),
        //    .shaderSubgroupRotate = VK_TRUE,
        //    .shaderSubgroupRotateClustered = VK_TRUE,
        //};
        //static constinit VkPhysicalDeviceShaderQuadControlFeaturesKHR   shader_quad_control_features = {
        //    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR,
        //    .pNext = std::addressof(shader_subgroup_rotate_features),
        //    .shaderQuadControl = VK_TRUE,
        //};
        static constinit VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR shader_maximal_reconvergence_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR,
            .pNext = std::addressof(swapchain_1_features),
        //    .pNext = std::addressof(shader_quad_control_features),
            .shaderMaximalReconvergence = VK_TRUE,
        };
        //static constinit VkPhysicalDeviceShaderFloatControls2FeaturesKHR shader_float_controls_2_features = {
        //    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES_KHR,
        //    .pNext = std::addressof(shader_maximal_reconvergence_features),
        //    .shaderFloatControls2 = VK_TRUE,
        //};
        static constinit VkPhysicalDeviceShaderExpectAssumeFeaturesKHR shader_expect_assume_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES_KHR,
            .pNext = std::addressof(shader_maximal_reconvergence_features),
        //    .pNext = std::addressof(shader_float_controls_2_features),
            .shaderExpectAssume = VK_TRUE,
        };
        static constinit VkPhysicalDeviceShaderClockFeaturesKHR  shader_clock_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR,
            .pNext = std::addressof(shader_expect_assume_features),
            .shaderSubgroupClock = VK_TRUE,
            .shaderDeviceClock = VK_TRUE,
        };
        static constinit VkPhysicalDeviceIndexTypeUint8FeaturesKHR index_type_u8_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_KHR,
            .pNext = std::addressof(shader_clock_features),
            .indexTypeUint8 = VK_TRUE,
        };
        static constinit VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamic_rendering_local_read_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR,
            .pNext = std::addressof(index_type_u8_features),
            .dynamicRenderingLocalRead = VK_TRUE,
        };
        static constinit VkPhysicalDeviceMaintenance6FeaturesKHR maintenance_6_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES_KHR,
            .pNext = std::addressof(dynamic_rendering_local_read_features),
            .maintenance6 = VK_TRUE,
        };
        static constinit VkPhysicalDeviceNestedCommandBufferFeaturesEXT nested_command_buffer_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
            .pNext = std::addressof(maintenance_6_features),
            .nestedCommandBuffer = VK_TRUE,
            .nestedCommandBufferRendering = VK_TRUE,
            .nestedCommandBufferSimultaneousUse = VK_TRUE,
        };
        static constinit VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT target_dynamic_rendering_unused_attachment_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT,
            .pNext = std::addressof(nested_command_buffer_features),
            .dynamicRenderingUnusedAttachments = VK_TRUE,
        };
        static constinit VkPhysicalDeviceMaintenance5FeaturesKHR target_maintenance_5_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
            .pNext = std::addressof(target_dynamic_rendering_unused_attachment_features),
            .maintenance5 = VK_TRUE,
        };
        static constinit VkPhysicalDeviceHostImageCopyFeaturesEXT target_host_image_copy_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT,
            .pNext = std::addressof(target_maintenance_5_features),
            .hostImageCopy = VK_TRUE,
        };
        static constinit VkPhysicalDeviceShaderObjectFeaturesEXT target_shader_object_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
            .pNext = std::addressof(target_host_image_copy_features),
            .shaderObject = VK_TRUE,
        };
        static constinit VkPhysicalDeviceMeshShaderFeaturesEXT target_mesh_shader_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
            .pNext = std::addressof(target_shader_object_features),
            .taskShader = VK_TRUE,
            .meshShader = VK_TRUE,
        };
        static constinit VkPhysicalDeviceDescriptorBufferFeaturesEXT target_descriptor_buffer_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
            .pNext = std::addressof(target_mesh_shader_features),
            .descriptorBuffer = VK_TRUE,
            .descriptorBufferImageLayoutIgnored = VK_TRUE,
            .descriptorBufferPushDescriptors = VK_TRUE,
        };
        static constinit VkPhysicalDeviceExtendedDynamicState3FeaturesEXT target_extended_dynamic_state_3_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
            .pNext = std::addressof(target_descriptor_buffer_features),
            .extendedDynamicState3DepthClampEnable = VK_TRUE,
            .extendedDynamicState3PolygonMode = VK_TRUE,
            .extendedDynamicState3LogicOpEnable = VK_TRUE,
            .extendedDynamicState3ColorBlendEnable = VK_TRUE,
            .extendedDynamicState3ColorBlendEquation = VK_TRUE,
            .extendedDynamicState3ColorWriteMask = VK_TRUE,
        };
        static constinit VkPhysicalDeviceExtendedDynamicState2FeaturesEXT target_extended_dynamic_state_2_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
            .pNext = std::addressof(target_extended_dynamic_state_3_features),
            .extendedDynamicState2LogicOp = VK_TRUE,
        };
        static constinit VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT target_extended_dynamic_state_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = std::addressof(target_extended_dynamic_state_2_features),
            .vertexInputDynamicState = VK_TRUE,
        };
        static constinit VkPhysicalDeviceVulkan13Features target_1_3_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = std::addressof(target_extended_dynamic_state_features),
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
            .maintenance4 = VK_TRUE,
        };
        static constinit VkPhysicalDeviceVulkan12Features target_1_2_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = std::addressof(target_1_3_features),
            .descriptorIndexing                                 = VK_TRUE,
            .shaderInputAttachmentArrayDynamicIndexing          = VK_TRUE,
            //.shaderUniformTexelBufferArrayDynamicIndexing       = VK_TRUE,
            //.shaderStorageTexelBufferArrayDynamicIndexing       = VK_TRUE,
            .shaderUniformBufferArrayNonUniformIndexing         = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing          = VK_TRUE,
            .shaderStorageBufferArrayNonUniformIndexing         = VK_TRUE,
            .shaderStorageImageArrayNonUniformIndexing          = VK_TRUE,
            //.shaderInputAttachmentArrayNonUniformIndexing       = VK_TRUE,
            //.shaderUniformTexelBufferArrayNonUniformIndexing    = VK_TRUE,
            //.shaderStorageTexelBufferArrayNonUniformIndexing    = VK_TRUE,
            .descriptorBindingUniformBufferUpdateAfterBind      = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind       = VK_TRUE,
            .descriptorBindingStorageImageUpdateAfterBind       = VK_TRUE,
            .descriptorBindingStorageBufferUpdateAfterBind      = VK_TRUE,
            //.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
            //.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingUpdateUnusedWhilePending          = VK_TRUE,
            .descriptorBindingPartiallyBound                    = VK_TRUE,
            .descriptorBindingVariableDescriptorCount           = VK_TRUE,
            .runtimeDescriptorArray                             = VK_TRUE,
            .scalarBlockLayout                                  = VK_TRUE,
            .uniformBufferStandardLayout                        = VK_TRUE,
            .timelineSemaphore                                  = VK_TRUE,
            .bufferDeviceAddress                                = VK_TRUE,
            //.bufferDeviceAddressCaptureReplay                   = VK_TRUE,
        };
        static constinit VkPhysicalDeviceVulkan11Features target_1_1_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = std::addressof(target_1_2_features),
            .multiview = VK_TRUE,
            .variablePointersStorageBuffer = VK_TRUE,
            .variablePointers              = VK_TRUE,
        };
        static constinit const VkPhysicalDeviceFeatures2 target_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = std::addressof(target_1_1_features),
            .features = {
                .fullDrawIndexUint32                     = VK_TRUE,
                .independentBlend                        = VK_TRUE,
                .geometryShader                          = VK_TRUE,
                .tessellationShader                      = VK_TRUE,
                .logicOp                                 = VK_TRUE,
                .depthClamp                              = VK_TRUE,
                .depthBiasClamp                          = VK_TRUE,
                .fillModeNonSolid                        = VK_TRUE,
                //.depthBounds                             = VK_TRUE,
                .wideLines                               = VK_TRUE,
                .largePoints                             = VK_TRUE,
                .multiViewport                           = VK_TRUE,
                .samplerAnisotropy                       = VK_TRUE,
                .textureCompressionBC                    = VK_TRUE,
                .fragmentStoresAndAtomics                = VK_TRUE,
                .shaderStorageImageExtendedFormats       = VK_TRUE,
                .shaderUniformBufferArrayDynamicIndexing = VK_TRUE,
                .shaderSampledImageArrayDynamicIndexing  = VK_TRUE,
                .shaderStorageBufferArrayDynamicIndexing = VK_TRUE,
                .shaderStorageImageArrayDynamicIndexing  = VK_TRUE,
                //.sparseBinding                           = VK_TRUE,
            }
        };

        static constexpr const char *device_extension_array[] = {
            /* Required extensions */
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_MAP_MEMORY_2_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
            VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
            VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME,
            VK_KHR_INDEX_TYPE_UINT8_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
            //VK_KHR_PRESENT_ID_EXTENSION_NAME,
            //VK_KHR_PRESENT_WAIT_EXTENSION_NAME,
            
            VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
            VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
            VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME,
            VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME,
            VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_6_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME,
            VK_KHR_SHADER_CLOCK_EXTENSION_NAME,
            VK_KHR_SHADER_EXPECT_ASSUME_EXTENSION_NAME,
            //VK_KHR_SHADER_FLOAT_CONTROLS_2_EXTENSION_NAME,
            VK_KHR_SHADER_MAXIMAL_RECONVERGENCE_EXTENSION_NAME,
            //VK_KHR_SHADER_QUAD_CONTROL_EXTENSION_NAME,
            //VK_KHR_SHADER_SUBGROUP_ROTATE_EXTENSION_NAME,
           // VK_KHR_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_EXTENSION_NAME,
            VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME,
            //VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME,
        };
        [[maybe_unused]] constexpr const char *cOptionalExtensions[] = {
            VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
            VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
            VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME,
            VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME,
            VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_6_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME,
            VK_KHR_SHADER_CLOCK_EXTENSION_NAME,
            VK_KHR_SHADER_EXPECT_ASSUME_EXTENSION_NAME,
            VK_KHR_SHADER_FLOAT_CONTROLS_2_EXTENSION_NAME,
            VK_KHR_SHADER_MAXIMAL_RECONVERGENCE_EXTENSION_NAME,
            VK_KHR_SHADER_QUAD_CONTROL_EXTENSION_NAME,
            VK_KHR_SHADER_SUBGROUP_ROTATE_EXTENSION_NAME,
            VK_KHR_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_EXTENSION_NAME,
            VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME,
        };
        constexpr u32 device_extension_count = sizeof(device_extension_array) / sizeof(const char*);

        static constexpr const float priority = 1.0f;
        VkDeviceQueueCreateInfo device_queue_info_array[cTargetMaxQueueCount] = {};
        for (u32 i = 0; i < m_queue_family_index_count; ++i) {
            device_queue_info_array[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            device_queue_info_array[i].queueFamilyIndex = m_queue_family_index_array_linear[i];
            device_queue_info_array[i].queueCount = 1;
            device_queue_info_array[i].pQueuePriorities = std::addressof(priority);
        }
        const VkDeviceCreateInfo device_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = std::addressof(target_features),
            .queueCreateInfoCount = m_queue_family_index_count,
            .pQueueCreateInfos = device_queue_info_array,
            .enabledExtensionCount = device_extension_count,
            .ppEnabledExtensionNames = device_extension_array,
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

    void Context::InitializePipelineLayout() {

		/* Initialize bindless descriptor layout */
        {
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
                    .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .descriptorCount = cTargetMaxTextureDescriptorCount,
                    .stageFlags      = VK_SHADER_STAGE_ALL,
                },
                {
                    .binding         = 1,
                    .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .descriptorCount = cTargetMaxSamplerDescriptorCount,
                    .stageFlags      = VK_SHADER_STAGE_ALL,
                },
            };

            const VkDescriptorSetLayoutCreateInfo bindless_set_layout_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = std::addressof(bindless_set_binding_info),
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                .bindingCount = sizeof(bindless_set_binding_array) / sizeof(VkDescriptorSetLayoutBinding),
                .pBindings = bindless_set_binding_array,
            };
            const u32 result7 = ::pfn_vkCreateDescriptorSetLayout(m_vk_device, std::addressof(bindless_set_layout_info), this->GetVkAllocationCallbacks(), std::addressof(m_vk_bindless_descriptor_set_layout));
            VP_ASSERT(result7 == VK_SUCCESS);
        }

        /* Initialize per stage combined descriptor layout for tessellation */
        const auto init_per_stage_combined_desc_layout_lambda = [this](VkDescriptorSetLayout *set_layout, u32 stage_bit) {

            /* Initialize descriptor set layout */
            VkDescriptorBindingFlags  per_stage_set_binding_flag_array[cTargetMaxPerStageCombinedCount];
            const VkDescriptorSetLayoutBindingFlagsCreateInfo per_stage_set_binding_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = sizeof(per_stage_set_binding_flag_array) / sizeof(VkDescriptorBindingFlags),
                .pBindingFlags = per_stage_set_binding_flag_array
            };
            VkDescriptorSetLayoutBinding per_stage_set_binding_array[cTargetMaxPerStageCombinedCount];
            for (u32 i = cCombinedBindingBaseTextureSampler; i < cTargetMaxPerStageTextureSamplerCount; ++i) {
                per_stage_set_binding_flag_array[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                per_stage_set_binding_array[i].binding = i;
                per_stage_set_binding_array[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                per_stage_set_binding_array[i].descriptorCount = 1;
                per_stage_set_binding_array[i].stageFlags = stage_bit;
                per_stage_set_binding_array[i].pImmutableSamplers = nullptr;
            }
            for (u32 i = cCombinedBindingBaseImage; i < cCombinedBindingBaseImage + cTargetMaxPerStageImageCount; ++i) {
                per_stage_set_binding_flag_array[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                per_stage_set_binding_array[i].binding = i;
                per_stage_set_binding_array[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                per_stage_set_binding_array[i].descriptorCount = 1;
                per_stage_set_binding_array[i].stageFlags = stage_bit;
                per_stage_set_binding_array[i].pImmutableSamplers = nullptr;
            }
            for (u32 i = cCombinedBindingBaseUniformBuffer; i < cCombinedBindingBaseUniformBuffer + cTargetMaxPerStageUniformBufferCount; ++i) {
                per_stage_set_binding_flag_array[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                per_stage_set_binding_array[i].binding = i;
                per_stage_set_binding_array[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                per_stage_set_binding_array[i].descriptorCount = 1;
                per_stage_set_binding_array[i].stageFlags = stage_bit;
                per_stage_set_binding_array[i].pImmutableSamplers = nullptr;
            }
            for (u32 i = cCombinedBindingBaseStorageBuffer; i < cCombinedBindingBaseStorageBuffer + cTargetMaxPerStageStorageBufferCount; ++i) {
                per_stage_set_binding_flag_array[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                per_stage_set_binding_array[i].binding = i;
                per_stage_set_binding_array[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                per_stage_set_binding_array[i].descriptorCount = 1;
                per_stage_set_binding_array[i].stageFlags = stage_bit;
                per_stage_set_binding_array[i].pImmutableSamplers = nullptr;
            }

            const VkDescriptorSetLayoutCreateInfo per_stage_set_layout_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = std::addressof(per_stage_set_binding_info),
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                .bindingCount = sizeof(per_stage_set_binding_array) / sizeof(VkDescriptorSetLayoutBinding),
                .pBindings = per_stage_set_binding_array
            };
            const u32 result0 = ::pfn_vkCreateDescriptorSetLayout(m_vk_device, std::addressof(per_stage_set_layout_info), this->GetVkAllocationCallbacks(), set_layout);
            VP_ASSERT(result0 == VK_SUCCESS);

            const u32    desc_size     = this->GetStorageBufferDescriptorSize();
            const u32    binding       = 69;
            VkDeviceSize desc_offset   = 0;
            ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(m_vk_device, *set_layout, binding, std::addressof(desc_offset));
            VP_ASSERT(desc_offset + desc_size <= cTargetMaxPerStageMemorySizeCombined);

            return;
        };

        init_per_stage_combined_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_combined_vertex),                  VK_SHADER_STAGE_VERTEX_BIT);
        init_per_stage_combined_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_combined_tessellation_control),    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        init_per_stage_combined_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_combined_tessellation_evaluation), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        init_per_stage_combined_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_combined_geometry),                VK_SHADER_STAGE_GEOMETRY_BIT);
        init_per_stage_combined_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_combined_fragment),                VK_SHADER_STAGE_FRAGMENT_BIT);
        init_per_stage_combined_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_combined_compute),                 VK_SHADER_STAGE_COMPUTE_BIT);
        init_per_stage_combined_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_combined_task),                    VK_SHADER_STAGE_TASK_BIT_EXT);
        init_per_stage_combined_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_combined_mesh),                    VK_SHADER_STAGE_MESH_BIT_EXT);

        /* Initialize per stage separate texture and separate sampler descriptor layout */
        const auto init_per_stage_separate_texture_sampler_desc_layout_lambda = [this](VkDescriptorSetLayout *set_layout, u32 stage_bit) {

            /* Initialize descriptor set layout */
            VkDescriptorBindingFlags  per_stage_set_binding_flag_array[cTargetMaxPerStageSeparateTextureCount + cTargetMaxPerStageSeparateSamplerCount];
            const VkDescriptorSetLayoutBindingFlagsCreateInfo per_stage_set_binding_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = sizeof(per_stage_set_binding_flag_array) / sizeof(VkDescriptorBindingFlags),
                .pBindingFlags = per_stage_set_binding_flag_array
            };
            VkDescriptorSetLayoutBinding per_stage_set_binding_array[cTargetMaxPerStageSeparateTextureCount + cTargetMaxPerStageSeparateSamplerCount];
            for (u32 i = 0; i < cTargetMaxPerStageSeparateTextureCount; ++i) {
                per_stage_set_binding_flag_array[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                per_stage_set_binding_array[i].binding = i;
                per_stage_set_binding_array[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                per_stage_set_binding_array[i].descriptorCount = 1;
                per_stage_set_binding_array[i].stageFlags = stage_bit;
                per_stage_set_binding_array[i].pImmutableSamplers = nullptr;
            }
            for (u32 i = cTargetMaxPerStageSeparateTextureCount; i < cTargetMaxPerStageSeparateTextureCount + cTargetMaxPerStageSeparateSamplerCount; ++i) {
                per_stage_set_binding_flag_array[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                per_stage_set_binding_array[i].binding = i;
                per_stage_set_binding_array[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                per_stage_set_binding_array[i].descriptorCount = 1;
                per_stage_set_binding_array[i].stageFlags = stage_bit;
                per_stage_set_binding_array[i].pImmutableSamplers = nullptr;
            }

            const VkDescriptorSetLayoutCreateInfo per_stage_set_layout_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = std::addressof(per_stage_set_binding_info),
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                .bindingCount = sizeof(per_stage_set_binding_array) / sizeof(VkDescriptorSetLayoutBinding),
                .pBindings = per_stage_set_binding_array
            };
            const u32 result0 = ::pfn_vkCreateDescriptorSetLayout(m_vk_device, std::addressof(per_stage_set_layout_info), this->GetVkAllocationCallbacks(), set_layout);
            VP_ASSERT(result0 == VK_SUCCESS);

            const u32    desc_size     = this->GetSeparateSamplerDescriptorSize();
            const u32    binding       = 159;
            VkDeviceSize desc_offset   = 0;
            ::pfn_vkGetDescriptorSetLayoutBindingOffsetEXT(m_vk_device, *set_layout, binding, std::addressof(desc_offset));
            VP_ASSERT(desc_offset + desc_size <= cTargetMaxPerStageMemorySizeSeparateTextureSampler);

            return;
        };

        init_per_stage_separate_texture_sampler_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_vertex),                  VK_SHADER_STAGE_VERTEX_BIT);
        init_per_stage_separate_texture_sampler_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_control),    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        init_per_stage_separate_texture_sampler_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_evaluation), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        init_per_stage_separate_texture_sampler_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_geometry),                VK_SHADER_STAGE_GEOMETRY_BIT);
        init_per_stage_separate_texture_sampler_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_fragment),                VK_SHADER_STAGE_FRAGMENT_BIT);
        init_per_stage_separate_texture_sampler_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_compute),                 VK_SHADER_STAGE_COMPUTE_BIT);
        init_per_stage_separate_texture_sampler_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_task),                    VK_SHADER_STAGE_TASK_BIT_EXT);
        init_per_stage_separate_texture_sampler_desc_layout_lambda(std::addressof(m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_mesh),                    VK_SHADER_STAGE_MESH_BIT_EXT);

        /* Build descriptor set layout arrays */
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetBindlessLayoutSet] = this->GetBindlessVkDescriptorSetLayout();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetCombinedVertex]                 = this->GetPerStageVkDescriptorSetLayoutCombinedVertex();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetCombinedTessellationControl]    = this->GetPerStageVkDescriptorSetLayoutCombinedTessellationControl();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetCombinedTessellationEvaluation] = this->GetPerStageVkDescriptorSetLayoutCombinedTessellationEvaluation();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetCombinedGeometry]               = this->GetPerStageVkDescriptorSetLayoutCombinedGeometry();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetCombinedFragment]               = this->GetPerStageVkDescriptorSetLayoutCombinedFragment();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetCombinedTask]                   = this->GetPerStageVkDescriptorSetLayoutCombinedTask();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetCombinedMesh]                   = this->GetPerStageVkDescriptorSetLayoutCombinedMesh();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetSeparateTextureSamplerVertex]                 = this->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerVertex();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetSeparateTextureSamplerTessellationControl]    = this->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerTessellationControl();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetSeparateTextureSamplerTessellationEvaluation] = this->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerTessellationEvaluation();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetSeparateTextureSamplerGeometry]               = this->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerGeometry();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetSeparateTextureSamplerFragment]               = this->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerFragment();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetSeparateTextureSamplerTask]                   = this->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerTask();
        m_vk_descriptor_set_layout_array_graphics[Context::cTargetPerStageLayoutSetSeparateTextureSamplerMesh]                   = this->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerMesh();

        m_vk_descriptor_set_layout_array_compute[Context::cTargetBindlessLayoutSet] = this->GetBindlessVkDescriptorSetLayout();
        m_vk_descriptor_set_layout_array_compute[Context::cTargetPerStageLayoutSetCombinedCompute]               = this->GetPerStageVkDescriptorSetLayoutCombinedCompute();
        m_vk_descriptor_set_layout_array_compute[Context::cTargetPerStageLayoutSetSeparateTextureSamplerCompute] = this->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerCompute();

        /* Initialize graphics pipeline layout */
        {
            const VkPipelineLayoutCreateInfo pipeline_layout_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = (sizeof(m_vk_descriptor_set_layout_array_graphics) / sizeof(VkDescriptorSetLayout)),
                .pSetLayouts = m_vk_descriptor_set_layout_array_graphics,
            };

            const u32 result9 = ::pfn_vkCreatePipelineLayout(m_vk_device, std::addressof(pipeline_layout_info), this->GetVkAllocationCallbacks(), std::addressof(m_vk_pipeline_layout_graphics));
            VP_ASSERT(result9 == VK_SUCCESS);
        }

        /* Initialize compute pipeline layout */
        {
            const VkPipelineLayoutCreateInfo pipeline_layout_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = (sizeof(m_vk_descriptor_set_layout_array_compute) / sizeof(VkDescriptorSetLayout)),
                .pSetLayouts = m_vk_descriptor_set_layout_array_compute,
            };

            const u32 result10 = ::pfn_vkCreatePipelineLayout(m_vk_device, std::addressof(pipeline_layout_info), this->GetVkAllocationCallbacks(), std::addressof(m_vk_pipeline_layout_compute));
            VP_ASSERT(result10 == VK_SUCCESS);
        }

        return;
    }

    void Context::FinalizePipelineLayout() {

        /* Destroy pipeline layouts */
        if (m_vk_pipeline_layout_graphics != VK_NULL_HANDLE) {
            ::pfn_vkDestroyPipelineLayout(m_vk_device, m_vk_pipeline_layout_graphics, this->GetVkAllocationCallbacks());
            m_vk_pipeline_layout_graphics = VK_NULL_HANDLE;
        }
        if (m_vk_pipeline_layout_compute != VK_NULL_HANDLE) {
            ::pfn_vkDestroyPipelineLayout(m_vk_device, m_vk_pipeline_layout_compute, this->GetVkAllocationCallbacks());
            m_vk_pipeline_layout_compute = VK_NULL_HANDLE;
        }

        /* Destroy descriptor set layouts */
        if (m_vk_bindless_descriptor_set_layout != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_bindless_descriptor_set_layout, this->GetVkAllocationCallbacks());
            m_vk_bindless_descriptor_set_layout = VK_NULL_HANDLE;
        }

        if (m_vk_per_stage_descriptor_set_layout_combined_vertex != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_combined_vertex, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_combined_vertex = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_combined_tessellation_control != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_combined_tessellation_control, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_combined_tessellation_control = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_combined_tessellation_evaluation != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_combined_tessellation_evaluation, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_combined_tessellation_evaluation = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_combined_geometry != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_combined_geometry, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_combined_geometry = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_combined_fragment != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_combined_fragment, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_combined_fragment = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_combined_compute != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_combined_compute, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_combined_compute = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_combined_task != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_combined_task, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_combined_task = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_combined_mesh != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_combined_mesh, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_combined_mesh = VK_NULL_HANDLE;
        }

        if (m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_vertex != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_vertex, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_vertex = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_control != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_control, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_control = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_evaluation != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_evaluation, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_evaluation = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_geometry != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_geometry, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_geometry = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_fragment != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_fragment, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_fragment = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_compute != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_compute, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_compute = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_task != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_task, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_task = VK_NULL_HANDLE;
        }
        if (m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_mesh != VK_NULL_HANDLE) {
            ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_mesh, this->GetVkAllocationCallbacks());
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_mesh = VK_NULL_HANDLE;
        }

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
        this->SetVkPhysicalDevice(vk_physical_device);

		/* Initialize device */
        this->InitializeLogicalDevice();

        /* Initialize pipeline layout */
        this->InitializePipelineLayout();

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
        VP_ASSERT(wait_sync_count < cMaxQueueWaitSyncCount && signal_sync_count < cMaxQueueSignalSyncCount);

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
        VP_ASSERT(command_list_count < cMaxQueueCommandListCount && wait_sync_count < cMaxQueueWaitSyncCount && signal_sync_count < cMaxQueueSignalSyncCount);

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

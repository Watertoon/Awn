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

    AWN_SINGLETON_TRAITS_IMPL(Context);

    namespace {
        #if defined(VP_DEBUG)
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, [[maybe_unused]] void* user_data) {

            ::puts(callback_data->pMessage);
            ::fflush(stdout);

            return VK_FALSE;
        }
        #endif

        bool FindGraphicsQueueFamilyIndex(u32 *out_index, VkPhysicalDevice vk_physical_device) {

            /* Query queue family properties count */
            u32 queue_family_count = 0;
            ::pfn_vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, std::addressof(queue_family_count), nullptr);
            VP_ASSERT(queue_family_count != 0);

            /* Allocate a temporary queue family property holder */
            VkQueueFamilyProperties *queue_properties = reinterpret_cast<VkQueueFamilyProperties*>(::malloc(sizeof(VkQueueFamilyProperties) * queue_family_count));
            VP_ASSERT(queue_properties != nullptr);

            /* Query queue family properties */
            ::pfn_vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, std::addressof(queue_family_count), queue_properties);

            /* Acquire graphics queue family index */
            u32 index = 0xffff'ffff;
            for (u32 i = 0; i < queue_family_count; ++i) {

                if ((queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
                    index = i;
                    break;
                }
            }

            ::free(queue_properties);

            *out_index = index;

            return (index != 0xffff'ffff);
        }
    }

    bool Context::SetVkPhysicalDevice(VkPhysicalDevice vk_physical_device, [[maybe_unused]]const ContextOptionalFeatureInfo *optional_feature_info) {

        /* Query Physical Device */
        ::pfn_vkGetPhysicalDeviceProperties2(vk_physical_device, std::addressof(m_vk_physical_device_properties));
        ::pfn_vkGetPhysicalDeviceMemoryProperties(vk_physical_device, std::addressof(m_vk_physical_device_memory_properties));
        ::pfn_vkGetPhysicalDeviceFeatures2(vk_physical_device, std::addressof(m_vk_physical_device_supported_features));

        /*  Ensure Vulkan 1.3 support */
        if (!(cTargetMinimumApiVersion <= m_vk_physical_device_properties.properties.apiVersion)) { VP_ASSERT(false); return false; }

        /* Ensure resource ubos meet our size requirements */
        if (m_vk_physical_device_properties.properties.limits.maxUniformBufferRange < cTargetMaxUniformBufferSize) { VP_ASSERT(false); return false; }

        /* Ensure present support */
        {
            u32 graphics_queue_family_index = 0;
            bool b_result1 = FindGraphicsQueueFamilyIndex(std::addressof(graphics_queue_family_index), vk_physical_device);
            VP_ASSERT(b_result1 == true);

            const u32 result = ::pfn_vkGetPhysicalDeviceWin32PresentationSupportKHR(vk_physical_device, graphics_queue_family_index);
            VP_ASSERT(result == VK_TRUE);
        }

        /* Ensure support for targeted surface format */
        {
            //u32 surface_format_count = 0;
            //const u32 result0 = ::pfn_vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, m_vk_surface, std::addressof(surface_format_count), nullptr);
            //VP_ASSERT(result0 == VK_SUCCESS);
            //
            //VkSurfaceFormatKHR *surface_formats = reinterpret_cast<VkSurfaceFormatKHR*>(::malloc(sizeof(VkSurfaceFormatKHR) * surface_format_count));
            //const u32 result1 = ::pfn_vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, m_vk_surface, std::addressof(surface_format_count), surface_formats);
            //VP_ASSERT(result1 == VK_SUCCESS);

            //for (u32 i = 0; i < surface_format_count; ++i) {
            //    if (surface_formats[i].format == TargetSurfaceFormat.format && surface_formats[i].colorSpace == TargetSurfaceFormat.colorSpace) {
            //        ::free(surface_formats);
            //        surface_formats = nullptr;
            //        break;
            //    }
            //}
            //if (surface_formats != nullptr) {
            //    ::free(surface_formats);
            //    VP_ASSERT(false); return false;
            //}
        }

        /* Ensure support for targeted present mode */
        {
            //u32 present_mode_count = 0;
            //const u32 result0 = ::pfn_vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, m_vk_surface, std::addressof(present_mode_count), nullptr);
            //VP_ASSERT(result0 == VK_SUCCESS);
            //
            //VkPresentModeKHR *present_modes = reinterpret_cast<VkPresentModeKHR*>(::malloc(sizeof(VkPresentModeKHR) * present_mode_count));
            //VP_ASSERT(present_modes != nullptr);
            //
            //const u32 result1 = ::pfn_vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, m_vk_surface, std::addressof(present_mode_count), present_modes);
            //VP_ASSERT(result1 == VK_SUCCESS);

            //for (u32 i = 0; i < present_mode_count; ++i) {
            //    if (present_modes[i] == TargetPresentMode) {
            //        ::free(present_modes);
            //        present_modes = nullptr;
            //        break;
            //    }
            //}
            //if (present_modes != nullptr) {
            //    ::free(present_modes);
            //    VP_ASSERT(false); return false;
            //}
        }

        /* Ensure our memory alignment is compatible with out TargetMemoryAlignment */
        {
            const size_t vk_alignment = m_vk_physical_device_properties.properties.limits.minMemoryMapAlignment;
            if (cTargetMemoryPoolAlignment % vk_alignment != 0)   { VP_ASSERT(false); return false; }
            if (!(cTargetMemoryPoolAlignment / vk_alignment > 0)) { VP_ASSERT(false); return false; }
        }
        {
            const size_t vk_alignment = m_vk_physical_device_properties.properties.limits.minUniformBufferOffsetAlignment;
            if (cTargetConstantBufferAlignment % vk_alignment != 0)   { VP_ASSERT(false); return false; }
            if (!(cTargetConstantBufferAlignment / vk_alignment > 0)) { VP_ASSERT(false); return false; }
        }
        {
            const size_t vk_alignment = m_vk_physical_device_properties.properties.limits.minStorageBufferOffsetAlignment;
            if (cTargetStorageBufferAlignment % vk_alignment != 0)   { VP_ASSERT(false); return false; }
            if (!(cTargetStorageBufferAlignment / vk_alignment > 0)) { VP_ASSERT(false); return false; }
        }
        {
            const size_t vk_alignment = m_vk_physical_device_properties.properties.limits.minTexelBufferOffsetAlignment;
            if (cTargetTexelBufferAlignment % vk_alignment != 0)   { VP_ASSERT(false); return false; }
            if (!(cTargetTexelBufferAlignment / vk_alignment > 0)) { VP_ASSERT(false); return false; }
        }

        /* Property checks */
        if (m_vk_physical_device_properties_12.maxUpdateAfterBindDescriptorsInAllPools       < cTargetMaxDescriptorCount)        { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_properties_12.maxDescriptorSetUpdateAfterBindSampledImages  < cTargetMaxTextureDescriptorCount) { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_properties_12.maxDescriptorSetUpdateAfterBindSamplers       < cTargetMaxSamplerDescriptorCount) { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_push_descriptor_properties.maxPushDescriptors               < cTargetMaxPushDescriptorCount)    { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_extended_dynamic_state_3_properties.dynamicPrimitiveTopologyUnrestricted == false)              { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_nested_command_buffer_properties.maxCommandBufferNestingLevel < cTargetCommandBufferNestingLevel)              { VP_ASSERT(false); return false; }
        //if (m_vk_physical_device_host_image_copy_properties.identicalMemoryTypeRequirements == false)                            { VP_ASSERT(false); return false; }

        /* Base feature checks */
        if (m_vk_physical_device_supported_features.features.independentBlend == false)                        { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.geometryShader == false)                          { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.tessellationShader == false)                      { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.logicOp == false)                                 { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.depthClamp == false)                              { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.depthBiasClamp == false)                          { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.fillModeNonSolid == false)                        { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.depthBounds == false)                             { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.wideLines == false)                               { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.largePoints == false)                             { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.multiViewport == false)                           { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.samplerAnisotropy == false)                       { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.textureCompressionBC == false)                    { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.fragmentStoresAndAtomics == false)                { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.shaderStorageImageExtendedFormats == false)       { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.shaderUniformBufferArrayDynamicIndexing == false) { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.shaderSampledImageArrayDynamicIndexing == false)  { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.shaderStorageBufferArrayDynamicIndexing == false) { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features.features.shaderStorageImageArrayDynamicIndexing == false)  { VP_ASSERT(false); return false; }

        /* 1.1 feature checks */
        if (m_vk_physical_device_supported_features_11.variablePointersStorageBuffer == false)                 { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_11.variablePointers == false)                              { VP_ASSERT(false); return false; }

        /* 1.2 feature checks */
        if (m_vk_physical_device_supported_features_12.descriptorIndexing == false)                                 { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderInputAttachmentArrayDynamicIndexing == false)          { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderUniformTexelBufferArrayDynamicIndexing == false)       { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderStorageTexelBufferArrayDynamicIndexing == false)       { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderUniformBufferArrayNonUniformIndexing == false)         { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderSampledImageArrayNonUniformIndexing == false)          { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderStorageBufferArrayNonUniformIndexing == false)         { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderStorageImageArrayNonUniformIndexing == false)          { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderInputAttachmentArrayNonUniformIndexing == false)       { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderUniformTexelBufferArrayNonUniformIndexing == false)    { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.shaderStorageTexelBufferArrayNonUniformIndexing == false)    { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.descriptorBindingUniformBufferUpdateAfterBind == false)      { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.descriptorBindingSampledImageUpdateAfterBind == false)       { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.descriptorBindingStorageImageUpdateAfterBind == false)       { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.descriptorBindingStorageBufferUpdateAfterBind == false)      { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.descriptorBindingUniformTexelBufferUpdateAfterBind == false) { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.descriptorBindingStorageTexelBufferUpdateAfterBind == false) { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.descriptorBindingUpdateUnusedWhilePending == false)          { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.descriptorBindingPartiallyBound == false)                    { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.descriptorBindingVariableDescriptorCount == false)           { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.runtimeDescriptorArray == false)                             { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.scalarBlockLayout == false)                                  { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.uniformBufferStandardLayout  == false)                       { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.timelineSemaphore  == false)                                 { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.bufferDeviceAddress == false)                                { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_12.bufferDeviceAddressCaptureReplay == false)                   { VP_ASSERT(false); return false; }

        /* 1.3 feature checks */
        if (m_vk_physical_device_supported_features_13.synchronization2 == false)                              { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_13.dynamicRendering == false)                              { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_supported_features_13.maintenance4 == false)                                  { VP_ASSERT(false); return false; }

        /* Vertex Input Dynamic State feature checks */
        if (m_vk_physical_device_vertex_input_dynamic_state_features.vertexInputDynamicState == false)         { VP_ASSERT(false); return false; }

        /* Extended Dynamic State 2 feature checks */
        if (m_vk_physical_device_extended_dynamic_state2_features.extendedDynamicState2LogicOp == false)       { VP_ASSERT(false); return false; }

        /* Extended Dynamic State 3 feature checks */
        if (m_vk_physical_device_extended_dynamic_state3_features.extendedDynamicState3DepthClampEnable == false)   { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_extended_dynamic_state3_features.extendedDynamicState3PolygonMode == false)        { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_extended_dynamic_state3_features.extendedDynamicState3LogicOpEnable == false)      { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable == false)   { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation == false) { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask == false)     { VP_ASSERT(false); return false; }

        /* Descriptor Buffer feature checks */
        if (m_vk_physical_device_descriptor_buffer_features.descriptorBuffer == false)                   { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_descriptor_buffer_features.descriptorBufferImageLayoutIgnored == false) { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_descriptor_buffer_features.descriptorBufferPushDescriptors == false)    { VP_ASSERT(false); return false; }

        /* Mesh Shader feature checks  */
        if (m_vk_physical_device_mesh_shader_features.taskShader == false) { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_mesh_shader_features.meshShader == false) { VP_ASSERT(false); return false; }

        /* Shader Object feature checks */
        if (m_vk_physical_device_shader_object_features.shaderObject == false) { VP_ASSERT(false); return false; }

        /* Host Image Copy feature checks */
        if (m_vk_physical_device_host_image_copy_features.hostImageCopy == false) { VP_ASSERT(false); return false; }

        /* Maintenance 5 feature checks */
        if (m_vk_physical_device_maintenance_5_features.maintenance5 == false) { VP_ASSERT(false); return false; }

        /* Unused attachments feature checks */
        if (m_vk_physical_device_dynamic_rendering_unused_attachments_features.dynamicRenderingUnusedAttachments == false) { VP_ASSERT(false); return false; }
        
        /* Nested command buffer feature checks */
        if (m_vk_physical_device_nested_command_buffer_features.nestedCommandBuffer == false)                { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_nested_command_buffer_features.nestedCommandBufferRendering == false)       { VP_ASSERT(false); return false; }
        if (m_vk_physical_device_nested_command_buffer_features.nestedCommandBufferSimultaneousUse == false) { VP_ASSERT(false); return false; }

        /* Maintenance 6 feature checks */
        if (m_vk_physical_device_maintenance_6_features.maintenance6 == false) { VP_ASSERT(false); return false; }

        m_vk_physical_device = vk_physical_device;

        /* Find queue families */
        return this->SetAllQueueFamilyIndices();
    }

    bool Context::SetAllQueueFamilyIndices() {
        
        /* Query queue family properties count */
        u32 queue_family_count = 0;
        ::pfn_vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, std::addressof(queue_family_count), nullptr);
        VP_ASSERT(queue_family_count != 0);

        /* Allocate a temporary queue family property holder */
        VkQueueFamilyProperties *queue_properties = reinterpret_cast<VkQueueFamilyProperties*>(::malloc(sizeof(VkQueueFamilyProperties) * queue_family_count));
        VP_ASSERT(queue_properties != nullptr);

        /* Query queue family properties */
        ::pfn_vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, std::addressof(queue_family_count), queue_properties);

        /* Clear family indices */
        m_graphics_queue_family_index     = 0xffff'ffff;
        m_compute_queue_family_index      = 0xffff'ffff;
        m_transfer_queue_family_index     = 0xffff'ffff;
        m_video_decode_queue_family_index = 0xffff'ffff;
        m_video_encode_queue_family_index = 0xffff'ffff;
        m_optical_flow_queue_family_index = 0xffff'ffff;
        m_queue_family_count              = 0;
        
        /* Acquire graphics queue family index */
        for (u32 i = 0; i < queue_family_count; ++i) {

            if ((queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
                m_graphics_queue_family_index = i;
                ++m_queue_family_count;
                break;
            }
        }

        /* Acquire compute queue family index */
        for (u32 i = 0; i < queue_family_count; ++i) {
            if (i == m_graphics_queue_family_index) { continue; }

            if ((queue_properties[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == VK_QUEUE_COMPUTE_BIT) {
                m_compute_queue_family_index = i;
                ++m_queue_family_count;
                break;
            }
        }

        /* Acquire transfer queue family index */
        for (u32 i = 0; i < queue_family_count; ++i) {
            if (i == m_graphics_queue_family_index || i == m_compute_queue_family_index) { continue; }

            if ((queue_properties[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)) == VK_QUEUE_TRANSFER_BIT) {
                m_transfer_queue_family_index = i;
                ++m_queue_family_count;
                break;
            }
        }

        /* Acquire video decode queue family index */
        for (u32 i = 0; i < queue_family_count; ++i) {
            if (i == m_graphics_queue_family_index || i == m_compute_queue_family_index || i == m_transfer_queue_family_index) { continue; }

            if ((queue_properties[i].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) == VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
                m_video_decode_queue_family_index = i;
                ++m_queue_family_count;
                break;
            }
        }

        /* Acquire video encode queue family index */
        for (u32 i = 0; i < queue_family_count; ++i) {
            if (i == m_graphics_queue_family_index || i == m_compute_queue_family_index || i == m_transfer_queue_family_index) { continue; }

            if ((queue_properties[i].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) == VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
                m_video_encode_queue_family_index = i;
                ++m_queue_family_count;
                break;
            }
        }

        /* Acquire optical flow queue family index */
        for (u32 i = 0; i < queue_family_count; ++i) {
            if (i == m_graphics_queue_family_index || i == m_compute_queue_family_index || i == m_transfer_queue_family_index) { continue; }

            if ((queue_properties[i].queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) == VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
                m_optical_flow_queue_family_index = i;
                ++m_queue_family_count;
                break;
            }
        }

        ::free(queue_properties);
        return (m_graphics_queue_family_index != 0xffff'ffff) & (m_compute_queue_family_index != 0xffff'ffff) & (m_transfer_queue_family_index != 0xffff'ffff) & (m_video_decode_queue_family_index != 0xffff'ffff) & (m_video_encode_queue_family_index != 0xffff'ffff) & (m_optical_flow_queue_family_index != 0xffff'ffff);
    }

	void Context::Initialize(const ContextInfo *context_info) {

        /* Load initial vulkan procs */
        ::LoadVkCProcsInitial();

        /* Check supported Vulkan Api version */
        {
            u32 api_version = 0;
            const u32 result0 = ::pfn_vkEnumerateInstanceVersion(std::addressof(api_version));
            VP_ASSERT(result0 == VK_SUCCESS);
            VP_ASSERT(cTargetMinimumApiVersion <= api_version);
        }

		/* Initialize instance */
        {
            #if defined(VP_DEBUG)
            const char *debug_layer_array[] = {
                "VK_LAYER_KHRONOS_validation"
            };
            u32 debug_layer_count = sizeof(debug_layer_array) / sizeof(const char*);
            #endif

            constexpr const char *instance_extension_array[] = {
                #if defined(VP_DEBUG)
                    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                #endif
                VK_KHR_SURFACE_EXTENSION_NAME,
                VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME
            };
            constexpr u32 instance_extension_count = sizeof(instance_extension_array) / sizeof(const char*);
            
            #if defined(VP_DEBUG)
            const VkValidationFeatureEnableEXT validation_feature_enable_array[] = {
                VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
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
                #if defined(VP_DEBUG)
                    .pNext = std::addressof(validation_features),
                #endif
                .pApplicationInfo = std::addressof(app_info),
                #if defined(VP_DEBUG)
                    .enabledLayerCount = debug_layer_count,
                    .ppEnabledLayerNames = debug_layer_array,
                #endif
                .enabledExtensionCount = instance_extension_count,
                .ppEnabledExtensionNames = instance_extension_array
            };

            const u32 result = ::pfn_vkCreateInstance(std::addressof(instance_info), nullptr, std::addressof(m_vk_instance));
            VP_ASSERT(result == VK_SUCCESS);
        }

        /* Load instance procs */
        ::LoadVkCProcsInstance(m_vk_instance);

        #if defined(VP_DEBUG)
        /* Create debug messenger */
        {
            const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = DebugCallback,
                .pUserData = nullptr
            };

            const u32 result = ::pfn_vkCreateDebugUtilsMessengerEXT(m_vk_instance, std::addressof(debug_messenger_info), nullptr, std::addressof(m_debug_messenger));
            VP_ASSERT(result == VK_SUCCESS);
        }
        #endif

        /* Set preferred physical device, with fallback if it's invalid */
        if (context_info->preferred_vk_physical_device == VK_NULL_HANDLE || this->SetVkPhysicalDevice(context_info->preferred_vk_physical_device, std::addressof(context_info->optional_feature_info)) == false) {
            
            u32               vk_physical_device_count = 0;
            VkPhysicalDevice *vk_physical_device_array = nullptr;
            
            /* Get count of physical devices */
            {
                const u32 result2 = ::pfn_vkEnumeratePhysicalDevices(m_vk_instance, std::addressof(vk_physical_device_count), nullptr);
                VP_ASSERT(result2 == VK_SUCCESS);
                VP_ASSERT(vk_physical_device_count != 0);
            }

            /* Create physical device array */
            {
                vk_physical_device_array = reinterpret_cast<VkPhysicalDevice*>(::malloc(sizeof(VkPhysicalDevice) * vk_physical_device_count));
                VP_ASSERT(vk_physical_device_array != nullptr);
            }

            /* Query physical devices */
            {
                const u32 result3 = ::pfn_vkEnumeratePhysicalDevices(m_vk_instance, std::addressof(vk_physical_device_count), vk_physical_device_array);
                VP_ASSERT(result3 == VK_SUCCESS);
            }

            /* Pick first valid Physical Device */
            {
                u32 i = 0;
                while (i != vk_physical_device_count && this->SetVkPhysicalDevice(vk_physical_device_array[i], std::addressof(context_info->optional_feature_info)) == false) { ++i; }
                VP_ASSERT(m_vk_physical_device != VK_NULL_HANDLE);
            }

            ::free(vk_physical_device_array);
        }

		/* Initialize device */
        {
            /* VkDevice feature enables */
            VkPhysicalDeviceMaintenance6FeaturesKHR maintenance_6_features = {
                .sType        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES_KHR,
                .maintenance6 = VK_TRUE,
            };
            VkPhysicalDeviceNestedCommandBufferFeaturesEXT nested_command_buffer_features = {
                .sType                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
                .pNext                              = std::addressof(maintenance_6_features),
                .nestedCommandBuffer                = VK_TRUE,
                .nestedCommandBufferRendering       = VK_TRUE,
                .nestedCommandBufferSimultaneousUse = VK_TRUE,
            };
            VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT target_dynamic_rendering_unused_attachment_features = {
                .sType                             = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT,
                .pNext                             = std::addressof(nested_command_buffer_features),
                .dynamicRenderingUnusedAttachments = VK_TRUE,
            };
            VkPhysicalDeviceMaintenance5FeaturesKHR target_maintenance_5_features = {
                .sType        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
                .pNext        = std::addressof(target_dynamic_rendering_unused_attachment_features),
                .maintenance5 = VK_TRUE,
            };
            VkPhysicalDeviceHostImageCopyFeaturesEXT target_host_image_copy_features = {
                .sType         = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT,
                .pNext        = std::addressof(target_maintenance_5_features),
                .hostImageCopy = VK_TRUE,
            };
            VkPhysicalDeviceShaderObjectFeaturesEXT target_shader_object_features = {
                .sType        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
                .pNext        = std::addressof(target_host_image_copy_features),
                .shaderObject = VK_TRUE,
            };
            VkPhysicalDeviceMeshShaderFeaturesEXT target_mesh_shader_features = {
                .sType      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
                .pNext      = std::addressof(target_shader_object_features),
                .taskShader = VK_TRUE,
                .meshShader = VK_TRUE,
            };
            VkPhysicalDeviceDescriptorBufferFeaturesEXT target_descriptor_buffer_features = {
                .sType                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
                .pNext                              = std::addressof(target_mesh_shader_features),
                .descriptorBuffer                   = VK_TRUE,
                .descriptorBufferImageLayoutIgnored = VK_TRUE,
                .descriptorBufferPushDescriptors    = VK_TRUE,
            };
            VkPhysicalDeviceExtendedDynamicState3FeaturesEXT target_extended_dynamic_state_3_features = {
                .sType                                   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
                .pNext                                   = std::addressof(target_descriptor_buffer_features),
                .extendedDynamicState3DepthClampEnable   = VK_TRUE,
                .extendedDynamicState3PolygonMode        = VK_TRUE,
                .extendedDynamicState3LogicOpEnable      = VK_TRUE,
                .extendedDynamicState3ColorBlendEnable   = VK_TRUE,
                .extendedDynamicState3ColorBlendEquation = VK_TRUE,
                .extendedDynamicState3ColorWriteMask     = VK_TRUE,
            };
            VkPhysicalDeviceExtendedDynamicState2FeaturesEXT target_extended_dynamic_state_2_features = {
                .sType                        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                .pNext                        = std::addressof(target_extended_dynamic_state_3_features),
                .extendedDynamicState2LogicOp = VK_TRUE,
            };
            VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT target_extended_dynamic_state_features = {
                .sType                   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                .pNext                   = std::addressof(target_extended_dynamic_state_2_features),
                .vertexInputDynamicState = VK_TRUE,
            };
            VkPhysicalDeviceVulkan13Features target_1_3_features = {
                .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                .pNext            = std::addressof(target_extended_dynamic_state_features),
                .synchronization2 = VK_TRUE,
                .dynamicRendering = VK_TRUE,
                .maintenance4     = VK_TRUE,
            };
            VkPhysicalDeviceVulkan12Features target_1_2_features = {
                .sType                                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext                                              = std::addressof(target_1_3_features),
                .descriptorIndexing                                 = VK_TRUE,
                .shaderInputAttachmentArrayDynamicIndexing          = VK_TRUE,
                .shaderUniformTexelBufferArrayDynamicIndexing       = VK_TRUE,
                .shaderStorageTexelBufferArrayDynamicIndexing       = VK_TRUE,
                .shaderUniformBufferArrayNonUniformIndexing         = VK_TRUE,
                .shaderSampledImageArrayNonUniformIndexing          = VK_TRUE,
                .shaderStorageBufferArrayNonUniformIndexing         = VK_TRUE,
                .shaderStorageImageArrayNonUniformIndexing          = VK_TRUE,
                .shaderInputAttachmentArrayNonUniformIndexing       = VK_TRUE,
                .shaderUniformTexelBufferArrayNonUniformIndexing    = VK_TRUE,
                .shaderStorageTexelBufferArrayNonUniformIndexing    = VK_TRUE,
                .descriptorBindingUniformBufferUpdateAfterBind      = VK_TRUE,
                .descriptorBindingSampledImageUpdateAfterBind       = VK_TRUE,
                .descriptorBindingStorageImageUpdateAfterBind       = VK_TRUE,
                .descriptorBindingStorageBufferUpdateAfterBind      = VK_TRUE,
                .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
                .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
                .descriptorBindingUpdateUnusedWhilePending          = VK_TRUE,
                .descriptorBindingPartiallyBound                    = VK_TRUE,
                .descriptorBindingVariableDescriptorCount           = VK_TRUE,
                .runtimeDescriptorArray                             = VK_TRUE,
                .scalarBlockLayout                                  = VK_TRUE,
                .uniformBufferStandardLayout                        = VK_TRUE,
                .timelineSemaphore                                  = VK_TRUE,
                .bufferDeviceAddress                                = VK_TRUE,
                .bufferDeviceAddressCaptureReplay                   = VK_TRUE,
            };
            VkPhysicalDeviceVulkan11Features target_1_1_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                .pNext = std::addressof(target_1_2_features),
                .variablePointersStorageBuffer = VK_TRUE,
                .variablePointers              = VK_TRUE,
            };
            const VkPhysicalDeviceFeatures2 target_features = {
                .sType                                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext                                       = std::addressof(target_1_1_features),
                .features = {
                    .independentBlend                        = VK_TRUE,
                    .geometryShader                          = VK_TRUE,
                    .tessellationShader                      = VK_TRUE,
                    .logicOp                                 = VK_TRUE,
                    .depthClamp                              = VK_TRUE,
                    .depthBiasClamp                          = VK_TRUE,
                    .fillModeNonSolid                        = VK_TRUE,
                    .depthBounds                             = VK_TRUE,
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
                }
            };

            const char *device_extension_array[] = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_MAP_MEMORY_2_EXTENSION_NAME,
                VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
                VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
                VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME,
                VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
                VK_EXT_MESH_SHADER_EXTENSION_NAME,
                VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
                VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
                VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME,
                VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME,
                VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
                VK_KHR_MAINTENANCE_6_EXTENSION_NAME,
            };
            const u32 device_extension_count = sizeof(device_extension_array) / sizeof(const char*);
            
            const float priority = 1.0f;
            const VkDeviceQueueCreateInfo device_queue_info_array[cTargetMaxQueueCount] {
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = m_graphics_queue_family_index,
                    .queueCount = 1,
                    .pQueuePriorities = std::addressof(priority)
                },
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = m_compute_queue_family_index,
                    .queueCount = 1,
                    .pQueuePriorities = std::addressof(priority)
                },
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = m_transfer_queue_family_index,
                    .queueCount = 1,
                    .pQueuePriorities = std::addressof(priority)
                },
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = m_video_decode_queue_family_index,
                    .queueCount = 1,
                    .pQueuePriorities = std::addressof(priority)
                },
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = m_video_encode_queue_family_index,
                    .queueCount = 1,
                    .pQueuePriorities = std::addressof(priority)
                },
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = m_optical_flow_queue_family_index,
                    .queueCount = 1,
                    .pQueuePriorities = std::addressof(priority)
                },
            };

            const VkDeviceCreateInfo device_info = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = std::addressof(target_features),
                .queueCreateInfoCount = cTargetMaxQueueCount,
                .pQueueCreateInfos = device_queue_info_array,
                .enabledExtensionCount = device_extension_count,
                .ppEnabledExtensionNames = device_extension_array,
                .pEnabledFeatures = nullptr
            };

            s32 result = ::pfn_vkCreateDevice(m_vk_physical_device, std::addressof(device_info), nullptr, std::addressof(m_vk_device));
            VP_ASSERT(result == VK_SUCCESS);
        }

        /* Load device procs */
        ::LoadVkCProcsDevice(m_vk_device);

		/* Initialize queues */
        ::pfn_vkGetDeviceQueue(m_vk_device, m_graphics_queue_family_index, 0, std::addressof(m_vk_graphics_queue));
        ::pfn_vkGetDeviceQueue(m_vk_device, m_compute_queue_family_index, 0, std::addressof(m_vk_compute_queue));
        ::pfn_vkGetDeviceQueue(m_vk_device, m_transfer_queue_family_index, 0, std::addressof(m_vk_transfer_queue));
        ::pfn_vkGetDeviceQueue(m_vk_device, m_video_decode_queue_family_index, 0, std::addressof(m_vk_video_decode_queue));
        ::pfn_vkGetDeviceQueue(m_vk_device, m_video_encode_queue_family_index, 0, std::addressof(m_vk_video_encode_queue));
        ::pfn_vkGetDeviceQueue(m_vk_device, m_optical_flow_queue_family_index, 0, std::addressof(m_vk_optical_flow_queue));
		
		/* Initialize texture descriptor layout */
        {
            const VkDescriptorBindingFlags  texture_set_binding_flag_array[] = {
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
            };
            const VkDescriptorSetLayoutBindingFlagsCreateInfo texture_set_binding_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = sizeof(texture_set_binding_flag_array) / sizeof(VkDescriptorBindingFlags),
                .pBindingFlags = texture_set_binding_flag_array
            };

            const VkDescriptorSetLayoutBinding texture_set_binding_array[] = { 
                {
                    .binding         = cTargetTextureLayoutBinding,
                    .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .descriptorCount = cTargetMaxTextureDescriptorCount,
                    .stageFlags      = VK_SHADER_STAGE_ALL,
                }
            };

            const VkDescriptorSetLayoutCreateInfo texture_set_layout_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = std::addressof(texture_set_binding_info),
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                .bindingCount = sizeof(texture_set_binding_array) / sizeof(VkDescriptorSetLayoutBinding),
                .pBindings = texture_set_binding_array
            };
            const u32 result7 = ::pfn_vkCreateDescriptorSetLayout(m_vk_device, std::addressof(texture_set_layout_info), nullptr, std::addressof(m_vk_texture_descriptor_set_layout));
            VP_ASSERT(result7 == VK_SUCCESS);
        }

        /* Initialize sampler descriptor layout */
        {
            const VkDescriptorBindingFlags  sampler_set_binding_flag_array[] = {
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
            };
            const VkDescriptorSetLayoutBindingFlagsCreateInfo sampler_set_binding_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = sizeof(sampler_set_binding_flag_array) / sizeof(VkDescriptorBindingFlags),
                .pBindingFlags = sampler_set_binding_flag_array
            };

            const VkDescriptorSetLayoutBinding sampler_set_binding_array[] = { 
                {
                    .binding         = cTargetSamplerLayoutBinding,
                    .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .descriptorCount = cTargetMaxSamplerDescriptorCount,
                    .stageFlags      = VK_SHADER_STAGE_ALL,
                }
            };

            const VkDescriptorSetLayoutCreateInfo sampler_set_layout_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = std::addressof(sampler_set_binding_info),
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                .bindingCount = sizeof(sampler_set_binding_array) / sizeof(VkDescriptorSetLayoutBinding),
                .pBindings = sampler_set_binding_array
            };
            const u32 result6 = ::pfn_vkCreateDescriptorSetLayout(m_vk_device, std::addressof(sampler_set_layout_info), nullptr, std::addressof(m_vk_sampler_descriptor_set_layout));
            VP_ASSERT(result6 == VK_SUCCESS);
        }

        /* Initialize storage buffer descriptor layout */
        {
            const VkDescriptorBindingFlags  storage_buffer_set_binding_flag_array[] = {
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
            };
            const VkDescriptorSetLayoutBindingFlagsCreateInfo storage_buffer_set_binding_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = sizeof(storage_buffer_set_binding_flag_array) / sizeof(VkDescriptorBindingFlags),
                .pBindingFlags = storage_buffer_set_binding_flag_array
            };

            const VkDescriptorSetLayoutBinding storage_buffer_set_binding_array[] = { 
                {
                    .binding         = cTargetStorageBufferLayoutBinding,
                    .descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = cTargetMaxSetStorageBufferCount,
                    .stageFlags      = VK_SHADER_STAGE_ALL,
                }
            };

            const VkDescriptorSetLayoutCreateInfo storage_buffer_set_layout_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = std::addressof(storage_buffer_set_binding_info),
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                .bindingCount = sizeof(storage_buffer_set_binding_array) / sizeof(VkDescriptorSetLayoutBinding),
                .pBindings = storage_buffer_set_binding_array
            };
            const u32 result6 = ::pfn_vkCreateDescriptorSetLayout(m_vk_device, std::addressof(storage_buffer_set_layout_info), nullptr, std::addressof(m_vk_storage_buffer_descriptor_set_layout));
            VP_ASSERT(result6 == VK_SUCCESS);
        }

        /* Initialize uniform buffer descriptor layout */
        {
            const VkDescriptorBindingFlags  uniform_buffer_set_binding_flag_array[] = {
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
            };
            const VkDescriptorSetLayoutBindingFlagsCreateInfo uniform_buffer_set_binding_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = sizeof(uniform_buffer_set_binding_flag_array) / sizeof(VkDescriptorBindingFlags),
                .pBindingFlags = uniform_buffer_set_binding_flag_array
            };

            const VkDescriptorSetLayoutBinding uniform_buffer_set_binding_array[] = { 
                {
                    .binding         = cTargetUniformBufferLayoutBinding,
                    .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = cTargetMaxSetUniformBufferCount,
                    .stageFlags      = VK_SHADER_STAGE_ALL,
                }
            };

            const VkDescriptorSetLayoutCreateInfo uniform_buffer_set_layout_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = std::addressof(uniform_buffer_set_binding_info),
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                .bindingCount = sizeof(uniform_buffer_set_binding_array) / sizeof(VkDescriptorSetLayoutBinding),
                .pBindings = uniform_buffer_set_binding_array
            };
            const u32 result6 = ::pfn_vkCreateDescriptorSetLayout(m_vk_device, std::addressof(uniform_buffer_set_layout_info), nullptr, std::addressof(m_vk_uniform_buffer_descriptor_set_layout));
            VP_ASSERT(result6 == VK_SUCCESS);
        }

        /* Initialize pipeline layout */
        {
            const VkDescriptorSetLayout descriptor_set_layout_array[] = {
                m_vk_texture_descriptor_set_layout,
                m_vk_sampler_descriptor_set_layout,
                m_vk_storage_buffer_descriptor_set_layout,
                m_vk_uniform_buffer_descriptor_set_layout,
            };

            const VkPushConstantRange push_constant_range_array[] = {
                {
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .offset = cTargetPushConstantOffset,
                    .size   = cTargetPushConstantSize,
                }
            };

            const VkPipelineLayoutCreateInfo pipeline_layout_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = sizeof(descriptor_set_layout_array) / sizeof(VkDescriptorSetLayout),
                .pSetLayouts = descriptor_set_layout_array,
                .pushConstantRangeCount = sizeof(push_constant_range_array) / sizeof(VkPushConstantRange),
                .pPushConstantRanges = push_constant_range_array
            };

            const u32 result9 = ::pfn_vkCreatePipelineLayout(m_vk_device, std::addressof(pipeline_layout_info), nullptr, std::addressof(m_vk_pipeline_layout));
            VP_ASSERT(result9 == VK_SUCCESS);
        }

        return;
	}

    void Context::Finalize() {

        /* Finalize Vulkan objects */
        ::pfn_vkQueueWaitIdle(m_vk_graphics_queue);
        ::pfn_vkQueueWaitIdle(m_vk_compute_queue);
        ::pfn_vkQueueWaitIdle(m_vk_transfer_queue);
        ::pfn_vkQueueWaitIdle(m_vk_video_decode_queue);
        ::pfn_vkQueueWaitIdle(m_vk_video_encode_queue);
        ::pfn_vkQueueWaitIdle(m_vk_optical_flow_queue);
        ::pfn_vkDeviceWaitIdle(m_vk_device);

        ::pfn_vkDestroyPipelineLayout(m_vk_device, m_vk_pipeline_layout, nullptr);
        ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_texture_descriptor_set_layout, nullptr);
        ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_sampler_descriptor_set_layout, nullptr);
        ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_storage_buffer_descriptor_set_layout, nullptr);
        ::pfn_vkDestroyDescriptorSetLayout(m_vk_device, m_vk_uniform_buffer_descriptor_set_layout, nullptr);

        ::pfn_vkDestroyDevice(m_vk_device, nullptr);

        #if defined(VP_DEBUG)
        ::pfn_vkDestroyDebugUtilsMessengerEXT(m_vk_instance, m_debug_messenger, nullptr);
        #endif

        ::pfn_vkDestroyInstance(m_vk_instance, nullptr);

        return;
    }
    
    void Context::SubmitCommandList(CommandList command_list, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count) {

        /* Integrity checks */
        VP_ASSERT(wait_sync_count < cMaxQueueWaitSyncCount && signal_sync_count < cMaxQueueSignalSyncCount);

        /* Setup command buffer submit info, wait and signal semaphore infos */
        VkCommandBufferSubmitInfo command_buffer_submit_info = {
            .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = command_list.vk_command_buffer,
            .deviceMask    = 0,
        };
        VkSemaphoreSubmitInfo wait_semaphore_array[cMaxQueueWaitSyncCount] = {};
        for (u32 i = 0; i < wait_sync_count; ++i) {
            wait_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            wait_semaphore_array[i].semaphore   = wait_sync_array[i]->GetVkSemaphore();
            wait_semaphore_array[i].value       = wait_sync_array[i]->GetExpectedValue();
            wait_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            wait_semaphore_array[i].deviceIndex = 0;
        }
        VkSemaphoreSubmitInfo signal_semaphore_array[cMaxQueueSignalSyncCount] = {};
        for (u32 i = 0; i < signal_sync_count; ++i) {
            signal_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
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
        const u32 result0 = ::pfn_vkQueueSubmit2(this->GetVkQueue(command_list.queue_type), 1, std::addressof(graphics_submit_info), VK_NULL_HANDLE);
        VP_ASSERT(result0 == VK_SUCCESS);

        return;
    }

    void Context::SubmitCommandLists(CommandList **command_list_array, u32 command_list_count, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count) {

        /* Integrity checks */
        VP_ASSERT(command_list_count < cMaxQueueCommandListCount && wait_sync_count < cMaxQueueWaitSyncCount && signal_sync_count < cMaxQueueSignalSyncCount);

        /* Setup command buffer submit info, wait and signal semaphore infos */
        VkCommandBufferSubmitInfo command_buffer_submit_info_array[cMaxQueueCommandListCount] = {};
        u32 queue_type = static_cast<u32>(command_list_array[0]->queue_type);
        for (u32 i = 0; i < command_list_count; ++i) {
            VP_ASSERT(queue_type == static_cast<u32>(command_list_array[i]->queue_type));
            command_buffer_submit_info_array[i].sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            command_buffer_submit_info_array[i].commandBuffer = command_list_array[i]->vk_command_buffer;
            command_buffer_submit_info_array[i].deviceMask    = 0;
        }
        VkSemaphoreSubmitInfo wait_semaphore_array[cMaxQueueWaitSyncCount] = {};
        for (u32 i = 0; i < wait_sync_count; ++i) {
            wait_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            wait_semaphore_array[i].semaphore   = wait_sync_array[i]->GetVkSemaphore();
            wait_semaphore_array[i].value       = wait_sync_array[i]->GetExpectedValue();
            wait_semaphore_array[i].stageMask   = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            wait_semaphore_array[i].deviceIndex = 0;
        }
        VkSemaphoreSubmitInfo signal_semaphore_array[cMaxQueueSignalSyncCount] = {};
        for (u32 i = 0; i < signal_sync_count; ++i) {
            signal_semaphore_array[i].sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
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
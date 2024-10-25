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
#pragma once

namespace awn::gfx::impl {

    struct ContextInfo {
        const char *app_name = "AwnDefault";
        const char *engine_name = "AwnDefault";
        OptionalFeatureMask preferred_optional_feature_mask;
        OptionalFeatureMask required_optional_feature_mask;
        size_t texture_descriptor_count;
        size_t sampler_descriptor_count;
    };

    struct ContextImportInfo {
        VkInstance vk_instance;
        VkPhysicalDevice vk_physical_device;
        VkDevice vk_device;
    };

#define AWN_VK_DEBUG

    class Context {
        public:
            /* Support Vulkan 1.3 */
            static constexpr u32 cTargetMinimumApiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

            /* Max simulataneous queues (1 for each QueueType) */
            static constexpr size_t cTargetMaxQueueCount = 0x6;
            static constexpr u32 cInvalidQueueFamilyIndex = 0xffff'ffff;

            /* Queue submission limits */
            static constexpr size_t cMaxQueueCommandListCount = 0x10;
            static constexpr size_t cMaxQueueWaitSyncCount    = 0x10;
            static constexpr size_t cMaxQueueSignalSyncCount  = 0x10;
            static constexpr size_t cMaxQueueSwapchainCount   = 0x10;

            /* Present modes */
            static constexpr VkPresentModeKHR cTargetPresentModeArray[] = {
                VK_PRESENT_MODE_FIFO_KHR,
                VK_PRESENT_MODE_IMMEDIATE_KHR,
            };
            static constexpr size_t cTargetPresentModeCount = sizeof(cTargetPresentModeArray) / sizeof(VkPresentModeKHR);

            /* Vertex buffer limits */
            static constexpr size_t cTargetMaxVertexBufferCount = 0x8;

            /* Render Target limits */
            static constexpr size_t cTargetMaxBoundRenderTargetColorCount = 0x8;
            static constexpr size_t cTargetMaxColorBlendEquationCount     = 0x8;
            static constexpr size_t cTargetMaxViewportScissorCount        = 0x8;

            /* Texture limits */
            static constexpr size_t cTargetMaxMipLevelCount = 0x10;

            /* Memory pools must always be page aligned */
            static constexpr size_t cTargetMemoryPoolAlignment = vp::util::c4KB;

            /* Buffer alignments */
            static constexpr size_t cTargetVertexBufferAlignment     = 0x4;
            static constexpr size_t cTargetIndexBufferAlignment      = 0x4;
            static constexpr size_t cTargetUniformBufferAlignment    = 0x40;
            static constexpr size_t cTargetStorageBufferAlignment    = 0x40;
            static constexpr size_t cTargetTexelBufferAlignment      = 0x40;
            static constexpr size_t cTargetDescriptorBufferAlignment = 0x40;

            /* Max resource sizes */
            static constexpr size_t cTargetMaxUniformBufferSize             = vp::util::c64KB;
            static constexpr size_t cTargetMaxTextureDescriptorSize         = 0x4;
            static constexpr size_t cTargetMaxSamplerDescriptorSize         = 0x4;

            static constexpr size_t cTargetMaxTextureSamplerDescriptorSize  = 0x4;
            static constexpr size_t cTargetMaxImageDescriptorSize           = 0x4;
            static constexpr size_t cTargetMaxUniformBufferDescriptorSize   = 0x8;
            static constexpr size_t cTargetMaxStorageBufferDescriptorSize   = 0x10;
            static constexpr size_t cTargetMaxSeparateTextureDescriptorSize = 0x4;
            static constexpr size_t cTargetMaxSeparateSamplerDescriptorSize = 0x4;

            /* Global descriptor resource limits */
            static constexpr size_t cTargetMaxTextureDescriptorCount = 0x1'0000;
            static constexpr size_t cTargetMaxSamplerDescriptorCount = 0x1000;

            /* Descriptor buffer index */
            static constexpr u32 cTargetDescriptorBufferIndexBindless = 0;
            static constexpr u32 cTargetDescriptorBufferIndexPerStage = 1;

            /* Base descriptor set layout bindings */
            static constexpr size_t cBindlessBindingBaseTexture = 0;
            static constexpr size_t cBindlessBindingBaseSampler = 1;

            /* Layout sets */
            static constexpr size_t cTargetBindlessLayoutSet = 0;
            static constexpr size_t cTargetTextureSamplerImageSeparateLayoutSet = 1;
            static constexpr size_t cTargetUniformBufferStorageBufferLayoutSet = 2;

            static constexpr size_t cTargetPerStageSetCount = 3;
            static constexpr size_t cTargetPerStagePushConstantRangeCount = 1;

            /* Per Shader stage resource limits */
            static constexpr size_t cTargetMaxShaderStageCount = 8;
            static constexpr size_t cTargetMaxSimultaneousShaderStageCount = 5;
            static constexpr size_t cTargetMaxMeshShaderStageCount         = 3;
            static constexpr size_t cTargetMaxPrimitiveShaderStageCount    = 5;
            static constexpr size_t cTargetMaxComputeShaderStageCount      = 1;

            static constexpr size_t cTargetMaxPerStageInputCount                  = 32;
            static constexpr size_t cTargetMaxPerStageOutputCount                 = 32;
            static constexpr size_t cTargetMaxPerStageTextureSamplerCount         = 32;
            static constexpr size_t cTargetMaxPerStageImageCount                  = 8;
            static constexpr size_t cTargetMaxPerStageUniformBufferCount          = 14;
            static constexpr size_t cTargetMaxPerStageStorageBufferCount          = 16;
            static constexpr size_t cTargetMaxPerStageSeparateTextureCount        = 128;
            static constexpr size_t cTargetMaxPerStageSeparateSamplerCount        = 32;
            static constexpr size_t cTargetMaxPerStageCombinedCount               = cTargetMaxPerStageTextureSamplerCount + cTargetMaxPerStageImageCount + cTargetMaxPerStageUniformBufferCount + cTargetMaxPerStageStorageBufferCount;
            static constexpr size_t cTargetMaxPerStageSeparateTextureSamplerCount = cTargetMaxPerStageSeparateTextureCount + cTargetMaxPerStageSeparateSamplerCount;

            static constexpr size_t cTargetMaxPerStageMemorySizeTextureSamplerImageSeparate = 0xc0 + 0x280;
            static constexpr size_t cTargetMaxPerStageMemorySizeUniformBufferStorageBuffer = 0x200;

            static constexpr size_t cTargetMaxPerStageMemorySizeTotal = cTargetMaxPerStageMemorySizeTextureSamplerImageSeparate + cTargetMaxPerStageMemorySizeUniformBufferStorageBuffer;
        private:
            void InitializeInstance(const ContextInfo *context_info);
            void FinalizeInstance();
            void SetVkPhysicalDevice(VkPhysicalDevice vk_physical_device);
            void InitializeLogicalDevice();
            void FinalizeLogicalDevice();
            void InitializePipelineLayout(const ContextInfo *context_info);
            void FinalizePipelineLayout();
        public:
            struct PerStageDescriptorLayout {
                VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info;
                VkDescriptorSetLayout vk_descriptor_set_layout_array[cTargetPerStageSetCount];
                VkPushConstantRange vk_push_constant_range;
            };
        private:
            /* Vulkan current physical device objects */
            VkInstance            m_vk_instance;
            VkPhysicalDevice      m_vk_physical_device;
            VkDevice              m_vk_device;
            u32                   m_queue_family_index_count;
            u32                   m_queue_family_index_array_linear[cTargetMaxQueueCount];
            u32                   m_queue_family_index_array_enum[cTargetMaxQueueCount];
            VkQueue               m_vk_queue_array_enum[cTargetMaxQueueCount];
            VkDescriptorSetLayout m_vk_bindless_descriptor_set_layout;
            union {
                PerStageDescriptorLayout m_per_stage_descriptor_layout_array[cTargetMaxShaderStageCount];
                struct {                    
                    PerStageDescriptorLayout m_per_stage_descriptor_layout_vertex;
                    PerStageDescriptorLayout m_per_stage_descriptor_layout_tessellation_control;
                    PerStageDescriptorLayout m_per_stage_descriptor_layout_tessellation_evaluation;
                    PerStageDescriptorLayout m_per_stage_descriptor_layout_geometry;
                    PerStageDescriptorLayout m_per_stage_descriptor_layout_fragment;
                    PerStageDescriptorLayout m_per_stage_descriptor_layout_compute;
                    PerStageDescriptorLayout m_per_stage_descriptor_layout_task;
                    PerStageDescriptorLayout m_per_stage_descriptor_layout_mesh;
                };
            };

            /* Vulkan physical device properties */
            VkPhysicalDeviceProperties2                        m_vk_physical_device_properties;
            VkPhysicalDeviceVulkan11Properties                 m_vk_physical_device_properties_11;
            VkPhysicalDeviceVulkan12Properties                 m_vk_physical_device_properties_12;
            VkPhysicalDeviceVulkan13Properties                 m_vk_physical_device_properties_13;
            VkPhysicalDeviceExtendedDynamicState3PropertiesEXT m_vk_physical_device_extended_dynamic_state_3_properties;
            VkPhysicalDeviceDescriptorBufferPropertiesEXT      m_vk_physical_device_descriptor_buffer_properties;
            VkPhysicalDeviceMeshShaderPropertiesEXT            m_vk_physical_device_mesh_shader_properties;
            VkPhysicalDeviceShaderObjectPropertiesEXT          m_vk_physical_device_shader_object_properties;
            VkPhysicalDeviceHostImageCopyPropertiesEXT         m_vk_physical_device_host_image_copy_properties;
            VkPhysicalDeviceMaintenance5PropertiesKHR          m_vk_physical_device_maintenance_5_properties;
            VkPhysicalDeviceNestedCommandBufferPropertiesEXT   m_vk_physical_device_nested_command_buffer_properties;
            VkPhysicalDeviceCooperativeMatrixPropertiesKHR     m_vk_physical_device_cooperative_matrix_properties;

            /* Vulkan physical device features */
            VkPhysicalDeviceFeatures2                                    m_vk_physical_device_supported_features;
            VkPhysicalDeviceVulkan11Features                             m_vk_physical_device_supported_features_11;
            VkPhysicalDeviceVulkan12Features                             m_vk_physical_device_supported_features_12;
            VkPhysicalDeviceVulkan13Features                             m_vk_physical_device_supported_features_13;
            VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT           m_vk_physical_device_vertex_input_dynamic_state_features;
            VkPhysicalDeviceExtendedDynamicState2FeaturesEXT             m_vk_physical_device_extended_dynamic_state2_features;
            VkPhysicalDeviceExtendedDynamicState3FeaturesEXT             m_vk_physical_device_extended_dynamic_state3_features;
            VkPhysicalDeviceDescriptorBufferFeaturesEXT                  m_vk_physical_device_descriptor_buffer_features;
            VkPhysicalDeviceMeshShaderFeaturesEXT                        m_vk_physical_device_mesh_shader_features;
            VkPhysicalDeviceShaderObjectFeaturesEXT                      m_vk_physical_device_shader_object_features;
            VkPhysicalDeviceHostImageCopyFeaturesEXT                     m_vk_physical_device_host_image_copy_features;
            VkPhysicalDeviceMaintenance5FeaturesKHR                      m_vk_physical_device_maintenance_5_features;
            VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT m_vk_physical_device_dynamic_rendering_unused_attachments_features;
            VkPhysicalDeviceNestedCommandBufferFeaturesEXT               m_vk_physical_device_nested_command_buffer_features;
            VkPhysicalDeviceMaintenance6FeaturesKHR                      m_vk_physical_device_maintenance_6_features;
            VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR         m_vk_physical_device_dynamic_rendering_local_read_features;
            VkPhysicalDeviceIndexTypeUint8FeaturesKHR                    m_vk_physical_device_index_type_u8_features;
            VkPhysicalDeviceShaderClockFeaturesKHR                       m_vk_physical_device_shader_clock_features;
            VkPhysicalDeviceShaderExpectAssumeFeaturesKHR                m_vk_physical_device_shader_expect_assume_features;
            VkPhysicalDeviceShaderFloatControls2FeaturesKHR              m_vk_physical_device_shader_float_controls_2_features;
            VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR        m_vk_physical_device_shader_maximal_reconvergence_features;
            VkPhysicalDeviceShaderQuadControlFeaturesKHR                 m_vk_physical_device_shader_quad_control_features;
            VkPhysicalDeviceShaderSubgroupRotateFeaturesKHR              m_vk_physical_device_shader_subgroup_rotate_features;
            VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR  m_vk_physical_device_shader_subgroup_uniform_control_flow_features;
            VkPhysicalDeviceCooperativeMatrixFeaturesKHR                 m_vk_physical_device_cooperative_matrix_features;
            VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT             m_vk_physical_device_mutable_descriptor_features;
            VkPhysicalDevicePerStageDescriptorSetFeaturesNV              m_vk_physical_device_per_stage_descriptor_set_features;
            VkPhysicalDeviceCommandBufferInheritanceFeaturesNV           m_vk_physical_device_command_buffer_inheritance_features;

            VkPhysicalDeviceMemoryProperties m_vk_physical_device_memory_properties;

            #if defined(AWN_VK_DEBUG)                               
                VkDebugUtilsMessengerEXT m_debug_messenger;
                //VkAllocationCallbacks m_debug_callbacks;
            #endif
        public:
            AWN_SINGLETON_TRAITS(Context);
        private:
            //static void *DebugAllocCallback(void *user_data, size_t size, size_t alignment, VkSystemAllocationScope scope) {
            //    VP_UNUSED(user_data);
            //    void *memory = ::_aligned_malloc(size, alignment);
            //    ::printf("VkAlloc; addr %p, 0x%xll bytes, align 0x%xll, scope %d\n", memory, size, alignment, scope);
            //    return memory;
            //}
            //static void DebugFreeCallback(void *user_data, void *memory) {
            //    VP_UNUSED(user_data);
            //    ::printf("VkFree; addr %p\n", memory);
            //    ::_aligned_free(memory);
            //}
            //static void *DebugReallocCallback(void *user_data, void *orig, size_t size, size_t alignment, VkSystemAllocationScope scope) {
            //    VP_UNUSED(user_data);
            //    void *memory = ::_aligned_realloc(orig, size, alignment);
            //    ::printf("VkRealloc; prev_addr %p, addr %p, size 0x%xll, align 0x%xll, scope %d\n", orig, memory, size, alignment, scope);
            //    return memory;
            //}
            //static void DebugAllocationNotifCallback(void *user_data, size_t size, VkInternalAllocationType alloc_type, VkSystemAllocationScope scope) {
            //    VP_UNUSED(user_data);
            //    ::printf("VkInternalAlloc; 0x%xll bytes, type %d, scope %d\n", size, alloc_type, scope);
            //}
            //static void DebugFreeNotifCallback(void *user_data, size_t size, VkInternalAllocationType alloc_type, VkSystemAllocationScope scope) {
            //    VP_UNUSED(user_data);
            //    ::printf("VkInternalFree; 0x%xll bytes, type %d, scope %d\n", size, alloc_type, scope);
            //}
        public:
            constexpr ALWAYS_INLINE Context() :
            m_vk_instance(), m_vk_physical_device(), m_vk_device(),
            m_queue_family_index_count(), m_queue_family_index_array_linear(), m_queue_family_index_array_enum(), m_vk_queue_array_enum(),
            m_vk_bindless_descriptor_set_layout(),
            m_per_stage_descriptor_layout_array{},
            m_vk_physical_device_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                .pNext = std::addressof(m_vk_physical_device_properties_11),
            },
            m_vk_physical_device_properties_11 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
                .pNext = std::addressof(m_vk_physical_device_properties_12),
            },
            m_vk_physical_device_properties_12 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES,
                .pNext = std::addressof(m_vk_physical_device_properties_13),
            },
            m_vk_physical_device_properties_13 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES,
                .pNext = std::addressof(m_vk_physical_device_extended_dynamic_state_3_properties),
            },
            m_vk_physical_device_extended_dynamic_state_3_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_descriptor_buffer_properties),
            },
            m_vk_physical_device_descriptor_buffer_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_mesh_shader_properties),
            },
            m_vk_physical_device_mesh_shader_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_shader_object_properties),
            },
            m_vk_physical_device_shader_object_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_host_image_copy_properties),
            },
            m_vk_physical_device_host_image_copy_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_maintenance_5_properties),
            },
            m_vk_physical_device_maintenance_5_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR,
                .pNext = std::addressof(m_vk_physical_device_nested_command_buffer_properties),
            },
            m_vk_physical_device_nested_command_buffer_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_cooperative_matrix_properties),
            },
            m_vk_physical_device_cooperative_matrix_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR,
                .pNext = nullptr,
            },
            m_vk_physical_device_supported_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = std::addressof(m_vk_physical_device_supported_features_11),
            },
            m_vk_physical_device_supported_features_11 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                .pNext = std::addressof(m_vk_physical_device_supported_features_12),
            },
            m_vk_physical_device_supported_features_12 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext = std::addressof(m_vk_physical_device_supported_features_13),
            },
            m_vk_physical_device_supported_features_13 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                .pNext = std::addressof(m_vk_physical_device_vertex_input_dynamic_state_features),
            },
            m_vk_physical_device_vertex_input_dynamic_state_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_extended_dynamic_state2_features),
            },
            m_vk_physical_device_extended_dynamic_state2_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_extended_dynamic_state3_features),
            },
            m_vk_physical_device_extended_dynamic_state3_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_descriptor_buffer_features),
            },
            m_vk_physical_device_descriptor_buffer_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_mesh_shader_features),
            },
            m_vk_physical_device_mesh_shader_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_shader_object_features),
            },
            m_vk_physical_device_shader_object_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_host_image_copy_features),
            },
            m_vk_physical_device_host_image_copy_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_maintenance_5_features),
            },
            m_vk_physical_device_maintenance_5_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_dynamic_rendering_unused_attachments_features),
            },
            m_vk_physical_device_dynamic_rendering_unused_attachments_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_nested_command_buffer_features),
            },
            m_vk_physical_device_nested_command_buffer_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_maintenance_6_features),
            },
            m_vk_physical_device_maintenance_6_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_dynamic_rendering_local_read_features),
            },
            m_vk_physical_device_dynamic_rendering_local_read_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_index_type_u8_features),
            },
            m_vk_physical_device_index_type_u8_features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_shader_clock_features),
            },
            m_vk_physical_device_shader_clock_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_shader_expect_assume_features),
            },
            m_vk_physical_device_shader_expect_assume_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_shader_float_controls_2_features),
            },
            m_vk_physical_device_shader_float_controls_2_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_shader_maximal_reconvergence_features),
            },
            m_vk_physical_device_shader_maximal_reconvergence_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_shader_quad_control_features),
            },
            m_vk_physical_device_shader_quad_control_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_shader_subgroup_rotate_features),
            },
            m_vk_physical_device_shader_subgroup_rotate_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_shader_subgroup_uniform_control_flow_features),
            },
            m_vk_physical_device_shader_subgroup_uniform_control_flow_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_cooperative_matrix_features),
            },
            m_vk_physical_device_cooperative_matrix_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR,
                .pNext = std::addressof(m_vk_physical_device_mutable_descriptor_features),
            },
            m_vk_physical_device_mutable_descriptor_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_per_stage_descriptor_set_features),
            },
            m_vk_physical_device_per_stage_descriptor_set_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV,
                .pNext = std::addressof(m_vk_physical_device_command_buffer_inheritance_features),
            },
            m_vk_physical_device_command_buffer_inheritance_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV,
                .pNext = nullptr,
            }
            //m_debug_callbacks {
            //    .pUserData = this,
            //    .pfnAllocation = DebugAllocCallback,
            //    .pfnReallocation = DebugReallocCallback,
            //    .pfnFree = DebugFreeCallback,
            //    .pfnInternalAllocation = DebugAllocationNotifCallback,
            //    .pfnInternalFree = DebugFreeNotifCallback,
            //}
            {/*...*/}

            constexpr ALWAYS_INLINE ~Context() {/*...*/}

            void Initialize(const ContextInfo *context_info);
            void Finalize();

            void SubmitCommandList(CommandList command_list, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count);
            void SubmitCommandLists(CommandList **command_list_array, u32 command_list_count, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count);

        public:
            constexpr ALWAYS_INLINE u32 GetVkMemoryTypeMask(u32 vk_memory_properties) const {

                u32 memory_type_mask = 0;
                const u32 memory_type_count = m_vk_physical_device_memory_properties.memoryTypeCount;
                for (u32 i = 0; i < memory_type_count; ++i) {
                    if ((m_vk_physical_device_memory_properties.memoryTypes[i].propertyFlags & vk_memory_properties) != vk_memory_properties) { continue; }

                    memory_type_mask |= (1 << i);
                }

                return memory_type_mask;
            }
            static_assert(VK_MAX_MEMORY_TYPES == 32);

            constexpr ALWAYS_INLINE u32 GetVkMemoryTypeIndex(MemoryPropertyFlags memory_property_flags) const {

                u32 vk_memory_properties    = vp::res::GfxMemoryPoolFlagsToVkMemoryPropertyFlags(memory_property_flags);
                u32 closest_type            = 0xffff'ffff;
                u32 closest_extra_props     = 0xffff'ffff;
                const u32 memory_type_count = m_vk_physical_device_memory_properties.memoryTypeCount;
                for (u32 i = 0; i < memory_type_count; ++i) {

                    /* Exact match */
                    if (m_vk_physical_device_memory_properties.memoryTypes[i].propertyFlags == vk_memory_properties) { return i; }

                    /* Narrow properties as much as possible */
                    const u32 masked_properties    = (m_vk_physical_device_memory_properties.memoryTypes[i].propertyFlags & vk_memory_properties);
                    const u32 extra_property_count = vp::util::CountOneBits32(m_vk_physical_device_memory_properties.memoryTypes[i].propertyFlags & (~vk_memory_properties));
                    if (masked_properties != vk_memory_properties || closest_extra_props < extra_property_count) { continue; }
                    closest_type        = i;
                    closest_extra_props = extra_property_count;
                }

                return closest_type;
            }

            constexpr ALWAYS_INLINE VkInstance             GetVkInstance()            const { return m_vk_instance; }
            constexpr ALWAYS_INLINE VkPhysicalDevice       GetVkPhysicalDevice()      const { return m_vk_physical_device; }
            constexpr ALWAYS_INLINE VkDevice               GetVkDevice()              const { return m_vk_device; }
            constexpr ALWAYS_INLINE const VkAllocationCallbacks *GetVkAllocationCallbacks() const { return nullptr; } //std::addressof(m_debug_callbacks); }

            constexpr ALWAYS_INLINE VkQueue GetVkQueue(QueueType queue_type) const { return m_vk_queue_array_enum[static_cast<u32>(queue_type)]; }
            constexpr ALWAYS_INLINE VkQueue GetVkQueueGraphics()    const { return m_vk_queue_array_enum[static_cast<u32>(QueueType::Graphics)]; }
            constexpr ALWAYS_INLINE VkQueue GetVkQueueCompute()     const { return m_vk_queue_array_enum[static_cast<u32>(QueueType::Compute)]; }
            constexpr ALWAYS_INLINE VkQueue GetVkQueueTransfer()    const { return m_vk_queue_array_enum[static_cast<u32>(QueueType::Transfer)]; }
            constexpr ALWAYS_INLINE VkQueue GetVkQueueVideoDecode() const { return m_vk_queue_array_enum[static_cast<u32>(QueueType::VideoDecode)]; }
            constexpr ALWAYS_INLINE VkQueue GetVkQueueVideoEncode() const { return m_vk_queue_array_enum[static_cast<u32>(QueueType::VideoEncode)]; }
            constexpr ALWAYS_INLINE VkQueue GetVkQueueOpticalFlow() const { return m_vk_queue_array_enum[static_cast<u32>(QueueType::OpticalFlow)]; }

            constexpr ALWAYS_INLINE const u32 *GetQueueFamilyIndiceArray() const { return m_queue_family_index_array_linear; }
            constexpr ALWAYS_INLINE       u32  GetQueueFamilyCount()       const { return m_queue_family_index_count; }

            constexpr ALWAYS_INLINE u32 GetQueueFamilyIndex(QueueType queue_type) const { return m_queue_family_index_array_enum[static_cast<u32>(queue_type)]; }
            constexpr ALWAYS_INLINE u32 GetGraphicsQueueFamilyIndex()    const { return m_queue_family_index_array_enum[static_cast<u32>(QueueType::Graphics)]; }
            constexpr ALWAYS_INLINE u32 GetComputeQueueFamilyIndex()     const { return m_queue_family_index_array_enum[static_cast<u32>(QueueType::Compute)]; }
            constexpr ALWAYS_INLINE u32 GetTransferQueueFamilyIndex()    const { return m_queue_family_index_array_enum[static_cast<u32>(QueueType::Transfer)]; }
            constexpr ALWAYS_INLINE u32 GetVideoDecodeQueueFamilyIndex() const { return m_queue_family_index_array_enum[static_cast<u32>(QueueType::VideoDecode)]; }
            constexpr ALWAYS_INLINE u32 GetVideoEncodeQueueFamilyIndex() const { return m_queue_family_index_array_enum[static_cast<u32>(QueueType::VideoEncode)]; }
            constexpr ALWAYS_INLINE u32 GetOpticalFlowQueueFamilyIndex() const { return m_queue_family_index_array_enum[static_cast<u32>(QueueType::OpticalFlow)]; }

            constexpr ALWAYS_INLINE VkDescriptorSetLayout GetBindlessVkDescriptorSetLayout() const { return m_vk_bindless_descriptor_set_layout; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout GetPerStageVkDescriptorSetLayoutTextureSamplerImageSeparate(ShaderStage shader_stage) const { return this->GetPerStageDescriptorLayout(shader_stage)->vk_descriptor_set_layout_array[cTargetTextureSamplerImageSeparateLayoutSet]; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout GetPerStageVkDescriptorSetLayoutUniformBufferStorageBuffer(ShaderStage shader_stage) const { return this->GetPerStageDescriptorLayout(shader_stage)->vk_descriptor_set_layout_array[cTargetUniformBufferStorageBufferLayoutSet]; }
           constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayout(ShaderStage shader_stage) const { return std::addressof(m_per_stage_descriptor_layout_array[static_cast<u32>(shader_stage)]); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutVertex()                   const { return std::addressof(m_per_stage_descriptor_layout_vertex); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutTessellationEvaluation()   const { return std::addressof(m_per_stage_descriptor_layout_tessellation_evaluation); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutTessellationControl()      const { return std::addressof(m_per_stage_descriptor_layout_tessellation_control); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutGeometry()                 const { return std::addressof(m_per_stage_descriptor_layout_geometry); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutFragment()                 const { return std::addressof(m_per_stage_descriptor_layout_fragment); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutCompute()                  const { return std::addressof(m_per_stage_descriptor_layout_compute); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutTask()                     const { return std::addressof(m_per_stage_descriptor_layout_task); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutMesh()                     const { return std::addressof(m_per_stage_descriptor_layout_mesh); }

            static constexpr ALWAYS_INLINE u32 GetPerStageSetTextureSampler(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return cTargetTextureSamplerImageSeparateLayoutSet;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageSetImage(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return cTargetTextureSamplerImageSeparateLayoutSet;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageSetUniformBuffer(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return cTargetUniformBufferStorageBufferLayoutSet;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageSetStorageBuffer(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return cTargetUniformBufferStorageBufferLayoutSet;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageSetSeparateTexture(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return cTargetTextureSamplerImageSeparateLayoutSet;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageSetSeparateSampler(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return cTargetTextureSamplerImageSeparateLayoutSet;
            }

            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseTextureSampler(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return 0;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseImage(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return 0;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseUniformBuffer(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return 0;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseStorageBuffer(ShaderStage shader_stage) {
                VP_UNUSED(shader_stage);
                return 0;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseSeparateTexture(ShaderStage shader_stage) { 
                VP_UNUSED(shader_stage);
                return 0;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseSeparateSampler(ShaderStage shader_stage) { 
                VP_UNUSED(shader_stage);
                return 0;
            }

            constexpr ALWAYS_INLINE const VkPhysicalDeviceProperties2 *GetPhysicalDeviceProperties() const { return std::addressof(m_vk_physical_device_properties); }
            constexpr ALWAYS_INLINE const void *GetPhysicalDeviceLuid() const { return m_vk_physical_device_properties_11.deviceLUID; }

            constexpr ALWAYS_INLINE size_t GetSeparateTextureDescriptorSize() const { return m_vk_physical_device_descriptor_buffer_properties.sampledImageDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetSeparateSamplerDescriptorSize() const { return m_vk_physical_device_descriptor_buffer_properties.samplerDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetTextureSamplerDescriptorSize()  const { return m_vk_physical_device_descriptor_buffer_properties.combinedImageSamplerDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetImageDescriptorSize()           const { return m_vk_physical_device_descriptor_buffer_properties.storageImageDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetUniformBufferDescriptorSize()   const { return m_vk_physical_device_descriptor_buffer_properties.uniformBufferDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetStorageBufferDescriptorSize()   const { return m_vk_physical_device_descriptor_buffer_properties.storageBufferDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetDescriptorOffsetAlignment()     const { return m_vk_physical_device_descriptor_buffer_properties.descriptorBufferOffsetAlignment; }
    };
}

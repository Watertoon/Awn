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

namespace awn::gfx {

    /* TODO */
    struct ContextOptionalFeatureInfo {
        bool require_ray_tracing;
        bool require_optical_flow;

        constexpr ALWAYS_INLINE void SetDefaults() {
            require_ray_tracing  = false;
            require_optical_flow = false;
        }
    };

    struct ContextInfo {
        const char                 *app_name;
        const char                 *engine_name;
        VkPhysicalDevice            preferred_vk_physical_device;
        ContextOptionalFeatureInfo  optional_feature_info;

        constexpr ALWAYS_INLINE void SetDefaults() {
            app_name                  = "AwnAppNameDefault";
            engine_name               = "AwnEngineNameDefault";
            preferred_vk_physical_device = VK_NULL_HANDLE;
            optional_feature_info.SetDefaults();
        }
    };

    enum class QueueType {
		Graphics    = 0x0,
		Compute     = 0x1,
		Transfer    = 0x2,
		VideoDecode = 0x3,
		VideoEncode = 0x4,
		OpticalFlow = 0x5,
	};

    class CommandList;

    class Context {
        public:
            /* Support Vulkan 1.3 */
            static constexpr u32    cTargetMinimumApiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

            /* Max simulataneous queues (1 for each QueueType) */
            static constexpr size_t cTargetMaxQueueCount = 0x6;

            /* Queue submission limits */
            static constexpr size_t cMaxQueueCommandListCount = 0x8;
            static constexpr size_t cMaxQueueWaitSyncCount    = 0x8;
            static constexpr size_t cMaxQueueSignalSyncCount  = 0x8;

            /* Vertex buffer limits */
            static constexpr size_t cTargetMaxVertexBufferCount = 0x8;

            /* Render Target limits */
            static constexpr size_t cTargetMaxBoundRenderTargetColorCount = 0x8;
            static constexpr size_t cTargetMaxColorBlendEquationCount     = 0x8;
            static constexpr size_t cTargetMaxViewportScissorCount        = 0x8;

            /* Memory pools must always be page aligned */
            static constexpr size_t cTargetMemoryPoolAlignment = 0x1000;

            /* Buffer alignments */
            static constexpr size_t cTargetVertexBufferAlignment     = 0x4;
            static constexpr size_t cTargetIndexBufferAlignment      = 0x4;
            static constexpr size_t cTargetConstantBufferAlignment   = 0x40;
            static constexpr size_t cTargetStorageBufferAlignment    = 0x40;
            static constexpr size_t cTargetTexelBufferAlignment      = 0x40;
            static constexpr size_t cTargetDescriptorBufferAlignment = 0x40;

            /* Max resource sizes */
            static constexpr size_t cTargetMaxUniformBufferSize             = 0x10000;
            static constexpr size_t cTargetMaxTextureDescriptorSize         = 0x4;
            static constexpr size_t cTargetMaxSamplerDescriptorSize         = 0x4;

            static constexpr size_t cTargetMaxTextureSamplerDescriptorSize  = 0x4;
            static constexpr size_t cTargetMaxImageDescriptorSize           = 0x4;
            static constexpr size_t cTargetMaxUniformBufferDescriptorSize   = 0x8;
            static constexpr size_t cTargetMaxStorageBufferDescriptorSize   = 0x10;
            static constexpr size_t cTargetMaxSeparateTextureDescriptorSize = 0x4;
            static constexpr size_t cTargetMaxSeparateSamplerDescriptorSize = 0x4;

            /* Global descriptor resource limits */
            static constexpr size_t cTargetMaxTextureDescriptorCount    = 0x2000;
            static constexpr size_t cTargetMaxSamplerDescriptorCount    = 0xfa0;
            static constexpr u32    cTargetDescriptorSetLayoutCountGraphics = 31;
            static constexpr u32    cTargetDescriptorSetLayoutCountCompute  = 7;

            /* Descriptor buffer index */
            static constexpr u32 cTargetDescriptorBufferIndexTexture  = 0;
            static constexpr u32 cTargetDescriptorBufferIndexSampler  = 1;
            static constexpr u32 cTargetDescriptorBufferIndexPerStage = 2;

            /* Base descriptor set layout bindings */
            static constexpr size_t cCombinedBindingBaseTextureSampler = 0;
            static constexpr size_t cCombinedBindingBaseImage          = 32;
            static constexpr size_t cCombinedBindingBaseUniformBuffer  = 40;
            static constexpr size_t cCombinedBindingBaseStorageBuffer  = 54;

            /* Layout sets */
            static constexpr size_t cTargetTextureLayoutSet = 0;
            static constexpr size_t cTargetSamplerLayoutSet = 1;

            static constexpr size_t cTargetPerStageLayoutSetTextureSamplerVertex                         = 2;
            static constexpr size_t cTargetPerStageLayoutSetTextureSamplerGeometry                       = 3;
            static constexpr size_t cTargetPerStageLayoutSetTextureSamplerFragment                       = 4;
            static constexpr size_t cTargetPerStageLayoutSetTextureSamplerTask                           = 5;
            static constexpr size_t cTargetPerStageLayoutSetTextureSamplerMesh                           = 6;
            static constexpr size_t cTargetPerStageLayoutSetImageVertex                                  = 7;
            static constexpr size_t cTargetPerStageLayoutSetImageGeometry                                = 8;
            static constexpr size_t cTargetPerStageLayoutSetImageFragment                                = 9;
            static constexpr size_t cTargetPerStageLayoutSetImageTask                                    = 10;
            static constexpr size_t cTargetPerStageLayoutSetImageMesh                                    = 11;
            static constexpr size_t cTargetPerStageLayoutSetUniformBufferVertex                          = 12;
            static constexpr size_t cTargetPerStageLayoutSetUniformBufferGeometry                        = 13;
            static constexpr size_t cTargetPerStageLayoutSetUniformBufferFragment                        = 14;
            static constexpr size_t cTargetPerStageLayoutSetUniformBufferTask                            = 15;
            static constexpr size_t cTargetPerStageLayoutSetUniformBufferMesh                            = 16;
            static constexpr size_t cTargetPerStageLayoutSetStorageBufferVertex                          = 17;
            static constexpr size_t cTargetPerStageLayoutSetStorageBufferGeometry                        = 18;
            static constexpr size_t cTargetPerStageLayoutSetStorageBufferFragment                        = 19;
            static constexpr size_t cTargetPerStageLayoutSetStorageBufferTask                            = 20;
            static constexpr size_t cTargetPerStageLayoutSetStorageBufferMesh                            = 21;
            static constexpr size_t cTargetPerStageLayoutSetCombinedTessellationEvaluation               = 22;
            static constexpr size_t cTargetPerStageLayoutSetCombinedTessellationControl                  = 23;
            static constexpr size_t cTargetPerStageLayoutSetSeparateTextureSamplerVertex                 = 24;
            static constexpr size_t cTargetPerStageLayoutSetSeparateTextureSamplerTessellationEvaluation = 25;
            static constexpr size_t cTargetPerStageLayoutSetSeparateTextureSamplerTessellationControl    = 26;
            static constexpr size_t cTargetPerStageLayoutSetSeparateTextureSamplerGeometry               = 27;
            static constexpr size_t cTargetPerStageLayoutSetSeparateTextureSamplerFragment               = 28;
            static constexpr size_t cTargetPerStageLayoutSetSeparateTextureSamplerTask                   = 29;
            static constexpr size_t cTargetPerStageLayoutSetSeparateTextureSamplerMesh                   = 30;

            static constexpr size_t cTargetPerStageLayoutSetTextureSamplerCompute         = 2;
            static constexpr size_t cTargetPerStageLayoutSetImageCompute                  = 3;
            static constexpr size_t cTargetPerStageLayoutSetUniformBufferCompute          = 4;
            static constexpr size_t cTargetPerStageLayoutSetStorageBufferCompute          = 5;
            static constexpr size_t cTargetPerStageLayoutSetSeparateTextureSamplerCompute = 6;

            /* Push constant ranges */
            static constexpr size_t cTargetAllStagePushConstantRangeCount = 1;
            static constexpr size_t cTargetPushConstantOffset             = 0;
            static constexpr size_t cTargetPushConstantSize               = 0x80;

            /* Per Shader stage resource limits */
            static constexpr size_t cTargetMaxSimultaneousShaderStageCount = 5;
            static constexpr size_t cTargetMaxMeshShaderStageCount         = 3;
            static constexpr size_t cTargetMaxPrimitiveShaderStageCount    = 5;
            static constexpr size_t cTargetMaxComputeShaderStageCount      = 1;

            static constexpr size_t cTargetMaxPerStageInputCount           = 32;
            static constexpr size_t cTargetMaxPerStageOutputCount          = 32;
            static constexpr size_t cTargetMaxPerStageTextureSamplerCount  = 32;
            static constexpr size_t cTargetMaxPerStageImageCount           = 8;
            static constexpr size_t cTargetMaxPerStageUniformBufferCount   = 14;
            static constexpr size_t cTargetMaxPerStageStorageBufferCount   = 16;
            static constexpr size_t cTargetMaxPerStageSeparateTextureCount = 128;
            static constexpr size_t cTargetMaxPerStageSeparateSamplerCount = 32;

            static constexpr size_t cTargetMaxPerStageMemorySizeTextureSampler  = 0x80;
            static constexpr size_t cTargetMaxPerStageMemorySizeImage           = 0x40;
            static constexpr size_t cTargetMaxPerStageMemorySizeUniformBuffer   = 0x80;
            static constexpr size_t cTargetMaxPerStageMemorySizeStorageBuffer   = 0x100;
            static constexpr size_t cTargetMaxPerStageMemorySizeSeparateTexture = 0x200;
            static constexpr size_t cTargetMaxPerStageMemorySizeSeparateSampler = 0x80;

            static constexpr size_t cTargetMaxPerStageMemorySizeCombined               = cTargetMaxPerStageMemorySizeTextureSampler + cTargetMaxPerStageMemorySizeImage + cTargetMaxPerStageMemorySizeUniformBuffer + cTargetMaxPerStageMemorySizeStorageBuffer;
            static constexpr size_t cTargetMaxPerStageMemorySizeSeparateTextureSampler = cTargetMaxPerStageMemorySizeSeparateTexture + cTargetMaxPerStageMemorySizeSeparateSampler;

            static constexpr size_t cTargetMaxPerStageMemorySizeTotal = cTargetMaxPerStageMemorySizeCombined + cTargetMaxPerStageMemorySizeSeparateTextureSampler;

            /* Command buffer nesting limits */
            static constexpr size_t cTargetCommandBufferNestingLevel = 256;
        private:
            /* Vulkan current physical device objects */
            VkInstance                                                   m_vk_instance;
            VkPhysicalDevice                                             m_vk_physical_device;
            VkDevice                                                     m_vk_device;
            u32                                                          m_queue_family_count;
            u32                                                          m_graphics_queue_family_index;
            u32                                                          m_compute_queue_family_index;
            u32                                                          m_transfer_queue_family_index;
            u32                                                          m_video_decode_queue_family_index;
            u32                                                          m_video_encode_queue_family_index;
            u32                                                          m_optical_flow_queue_family_index;
            VkQueue                                                      m_vk_graphics_queue;
            VkQueue                                                      m_vk_compute_queue;
            VkQueue                                                      m_vk_transfer_queue;
            VkQueue                                                      m_vk_video_decode_queue;
            VkQueue                                                      m_vk_video_encode_queue;
            VkQueue                                                      m_vk_optical_flow_queue;
            VkDescriptorSetLayout                                        m_vk_texture_descriptor_set_layout;
            VkDescriptorSetLayout                                        m_vk_sampler_descriptor_set_layout;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_texture_sampler_vertex;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_texture_sampler_geometry;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_texture_sampler_fragment;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_texture_sampler_compute;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_texture_sampler_task;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_texture_sampler_mesh;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_image_vertex;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_image_geometry;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_image_fragment;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_image_compute;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_image_task;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_image_mesh;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_uniform_buffer_vertex;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_uniform_buffer_geometry;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_uniform_buffer_fragment;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_uniform_buffer_compute;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_uniform_buffer_task;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_uniform_buffer_mesh;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_storage_buffer_vertex;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_storage_buffer_geometry;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_storage_buffer_fragment;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_storage_buffer_compute;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_storage_buffer_task;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_storage_buffer_mesh;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_combined_tessellation_control;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_combined_tessellation_evaluation;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_vertex;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_control;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_evaluation;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_geometry;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_fragment;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_compute;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_task;
            VkDescriptorSetLayout                                        m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_mesh;
            VkPipelineLayout                                             m_vk_pipeline_layout_graphics;
            VkPipelineLayout                                             m_vk_pipeline_layout_compute;
            VkPushConstantRange                                          m_vk_push_constant_range;

            /* Vulkan all physical device properties and features */
            VkPhysicalDeviceProperties2                                  m_vk_physical_device_properties;
            VkPhysicalDeviceVulkan11Properties                           m_vk_physical_device_properties_11;
            VkPhysicalDeviceVulkan12Properties                           m_vk_physical_device_properties_12;
            VkPhysicalDeviceVulkan13Properties                           m_vk_physical_device_properties_13;
            VkPhysicalDevicePushDescriptorPropertiesKHR                  m_vk_physical_device_push_descriptor_properties;
            VkPhysicalDeviceExtendedDynamicState3PropertiesEXT           m_vk_physical_device_extended_dynamic_state_3_properties;
            VkPhysicalDeviceDescriptorBufferPropertiesEXT                m_vk_physical_device_descriptor_buffer_properties;
            VkPhysicalDeviceMeshShaderPropertiesEXT                      m_vk_physical_device_mesh_shader_properties;
            VkPhysicalDeviceShaderObjectPropertiesEXT                    m_vk_physical_device_shader_object_properties;
            VkPhysicalDeviceHostImageCopyPropertiesEXT                   m_vk_physical_device_host_image_copy_properties;
            VkPhysicalDeviceMaintenance5PropertiesKHR                    m_vk_physical_device_maintenance_5_properties;
            VkPhysicalDeviceNestedCommandBufferPropertiesEXT             m_vk_physical_device_nested_command_buffer_properties;

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

            VkPhysicalDeviceMemoryProperties                             m_vk_physical_device_memory_properties;

            #if defined(VP_DEBUG)                               
                VkDebugUtilsMessengerEXT                                 m_debug_messenger;
            #endif
        public:
            AWN_SINGLETON_TRAITS(Context);
        public:
            constexpr ALWAYS_INLINE Context() :
            m_vk_instance(VK_NULL_HANDLE),
            m_vk_physical_device(VK_NULL_HANDLE),
            m_vk_device(VK_NULL_HANDLE),
            m_queue_family_count(),
            m_graphics_queue_family_index(),
            m_compute_queue_family_index(),
            m_transfer_queue_family_index(),
            m_video_decode_queue_family_index(),
            m_video_encode_queue_family_index(),
            m_optical_flow_queue_family_index(),
            m_vk_graphics_queue(VK_NULL_HANDLE),
            m_vk_compute_queue(VK_NULL_HANDLE),
            m_vk_transfer_queue(VK_NULL_HANDLE),
            m_vk_video_decode_queue(VK_NULL_HANDLE),
            m_vk_video_encode_queue(VK_NULL_HANDLE),
            m_vk_optical_flow_queue(VK_NULL_HANDLE),
            m_vk_texture_descriptor_set_layout(VK_NULL_HANDLE),
            m_vk_sampler_descriptor_set_layout(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_texture_sampler_vertex(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_texture_sampler_geometry(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_texture_sampler_fragment(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_texture_sampler_compute(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_texture_sampler_task(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_texture_sampler_mesh(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_image_vertex(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_image_geometry(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_image_fragment(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_image_compute(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_image_task(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_image_mesh(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_uniform_buffer_vertex(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_uniform_buffer_geometry(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_uniform_buffer_fragment(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_uniform_buffer_compute(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_uniform_buffer_task(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_uniform_buffer_mesh(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_storage_buffer_vertex(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_storage_buffer_geometry(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_storage_buffer_fragment(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_storage_buffer_compute(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_storage_buffer_task(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_storage_buffer_mesh(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_combined_tessellation_control(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_combined_tessellation_evaluation(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_vertex(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_control(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_evaluation(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_geometry(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_fragment(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_compute(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_task(VK_NULL_HANDLE),
            m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_mesh(VK_NULL_HANDLE),
            m_vk_pipeline_layout_graphics(VK_NULL_HANDLE),
            m_vk_pipeline_layout_compute(VK_NULL_HANDLE),
            m_vk_push_constant_range {
                .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT,
                .offset     = cTargetPushConstantOffset,
                .size       = cTargetPushConstantSize
            },
            m_vk_physical_device_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                .pNext = std::addressof(m_vk_physical_device_properties_11)
            },
            m_vk_physical_device_properties_11 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
                .pNext = std::addressof(m_vk_physical_device_properties_12)
            },
            m_vk_physical_device_properties_12 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES,
                .pNext = std::addressof(m_vk_physical_device_properties_13)
            },
            m_vk_physical_device_properties_13 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES,
                .pNext = std::addressof(m_vk_physical_device_push_descriptor_properties)
            },
            m_vk_physical_device_push_descriptor_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR,
                .pNext = std::addressof(m_vk_physical_device_extended_dynamic_state_3_properties)
            },
            m_vk_physical_device_extended_dynamic_state_3_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_descriptor_buffer_properties)
            },
            m_vk_physical_device_descriptor_buffer_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_mesh_shader_properties)
            },
            m_vk_physical_device_mesh_shader_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_shader_object_properties)
            },
            m_vk_physical_device_shader_object_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_host_image_copy_properties)
            },
            m_vk_physical_device_host_image_copy_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_maintenance_5_properties)
            },
            m_vk_physical_device_maintenance_5_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR,
                .pNext = std::addressof(m_vk_physical_device_nested_command_buffer_properties)
            },
            m_vk_physical_device_nested_command_buffer_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT
            },
            m_vk_physical_device_supported_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = std::addressof(m_vk_physical_device_supported_features_11)
            },
            m_vk_physical_device_supported_features_11 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                .pNext = std::addressof(m_vk_physical_device_supported_features_12)
            },
            m_vk_physical_device_supported_features_12 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext = std::addressof(m_vk_physical_device_supported_features_13)
            },
            m_vk_physical_device_supported_features_13 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                .pNext = std::addressof(m_vk_physical_device_vertex_input_dynamic_state_features)
            },
            m_vk_physical_device_vertex_input_dynamic_state_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_extended_dynamic_state2_features)
            },
            m_vk_physical_device_extended_dynamic_state2_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_extended_dynamic_state3_features)
            },
            m_vk_physical_device_extended_dynamic_state3_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_descriptor_buffer_features)
            },
            m_vk_physical_device_descriptor_buffer_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_mesh_shader_features)
            },
            m_vk_physical_device_mesh_shader_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_shader_object_features)
            },
            m_vk_physical_device_shader_object_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_host_image_copy_features)
            },
            m_vk_physical_device_host_image_copy_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT,
                .pNext = std::addressof(m_vk_physical_device_maintenance_5_features)
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
                .pNext = nullptr,
            }
            {/*...*/}

            constexpr ALWAYS_INLINE ~Context() {/*...*/}

            void Initialize(const ContextInfo *context_info);
            void Finalize();

            void SubmitCommandList(CommandList command_list, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count);
            void SubmitCommandLists(CommandList **command_list_array, u32 command_list_count, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count);

            bool SetVkPhysicalDevice(VkPhysicalDevice vk_physical_device, const ContextOptionalFeatureInfo *optional_feature_info);

            bool SetAllQueueFamilyIndices();

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
            static_assert(VK_MAX_MEMORY_TYPES == 32);

            constexpr ALWAYS_INLINE VkInstance             GetVkInstance()            const { return m_vk_instance; }
            constexpr ALWAYS_INLINE VkPhysicalDevice       GetVkPhysicalDevice()      const { return m_vk_physical_device; }
            constexpr ALWAYS_INLINE VkDevice               GetVkDevice()              const { return m_vk_device; }
            constexpr ALWAYS_INLINE VkAllocationCallbacks *GetVkAllocationCallbacks() const { return nullptr; }

            constexpr ALWAYS_INLINE VkQueue                GetVkQueue(QueueType queue_type) const { return std::addressof(m_vk_graphics_queue)[static_cast<u32>(queue_type)]; }
            constexpr ALWAYS_INLINE VkQueue                GetVkQueueGraphics()             const { return m_vk_graphics_queue; }
            constexpr ALWAYS_INLINE VkQueue                GetVkQueueCompute()              const { return m_vk_compute_queue; }
            constexpr ALWAYS_INLINE VkQueue                GetVkQueueTransfer()             const { return m_vk_transfer_queue; }
            constexpr ALWAYS_INLINE VkQueue                GetVkQueueVideoDecode()          const { return m_vk_video_decode_queue; }
            constexpr ALWAYS_INLINE VkQueue                GetVkQueueVideoEncode()          const { return m_vk_video_encode_queue; }
            constexpr ALWAYS_INLINE VkQueue                GetVkQueueOpticalFlow()          const { return m_vk_optical_flow_queue; }

            constexpr ALWAYS_INLINE const u32             *GetQueueFamilyIndiceArray()               const { return std::addressof(m_graphics_queue_family_index); }
            constexpr ALWAYS_INLINE       u32              GetQueueFamilyCount()                     const { return m_queue_family_count; }

            constexpr ALWAYS_INLINE       u32              GetQueueFamilyIndex(QueueType queue_type) const { return std::addressof(m_graphics_queue_family_index)[static_cast<u32>(queue_type)]; }
            constexpr ALWAYS_INLINE       u32              GetGraphicsQueueFamilyIndex()             const { return m_graphics_queue_family_index; }
            constexpr ALWAYS_INLINE       u32              GetComputeQueueFamilyIndex()              const { return m_compute_queue_family_index; }
            constexpr ALWAYS_INLINE       u32              GetTransferQueueFamilyIndex()             const { return m_transfer_queue_family_index; }
            constexpr ALWAYS_INLINE       u32              GetVideoDecodeQueueFamilyIndex()          const { return m_video_decode_queue_family_index; }
            constexpr ALWAYS_INLINE       u32              GetVideoEncodeQueueFamilyIndex()          const { return m_video_encode_queue_family_index; }
            constexpr ALWAYS_INLINE       u32              GetOpticalFlowQueueFamilyIndex()          const { return m_optical_flow_queue_family_index; }

            constexpr ALWAYS_INLINE const VkPushConstantRange   *GetVkPushConstantRangeArray()   const { return std::addressof(m_vk_push_constant_range); }
            constexpr ALWAYS_INLINE const VkDescriptorSetLayout *GetVkDescriptorSetLayoutArray() const { return std::addressof(m_vk_texture_descriptor_set_layout); }

            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetTextureVkDescriptorSetLayout() const { return m_vk_texture_descriptor_set_layout; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetSamplerVkDescriptorSetLayout() const { return m_vk_sampler_descriptor_set_layout; }

            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutTextureSamplerVertex()                         const { return m_vk_per_stage_descriptor_set_layout_texture_sampler_vertex; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutTextureSamplerGeometry()                       const { return m_vk_per_stage_descriptor_set_layout_texture_sampler_geometry; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutTextureSamplerFragment()                       const { return m_vk_per_stage_descriptor_set_layout_texture_sampler_fragment; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutTextureSamplerTask()                           const { return m_vk_per_stage_descriptor_set_layout_texture_sampler_task; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutTextureSamplerMesh()                           const { return m_vk_per_stage_descriptor_set_layout_texture_sampler_mesh; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutImageVertex()                                  const { return m_vk_per_stage_descriptor_set_layout_image_vertex; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutImageGeometry()                                const { return m_vk_per_stage_descriptor_set_layout_image_geometry; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutImageFragment()                                const { return m_vk_per_stage_descriptor_set_layout_image_fragment; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutImageTask()                                    const { return m_vk_per_stage_descriptor_set_layout_image_task; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutImageMesh()                                    const { return m_vk_per_stage_descriptor_set_layout_image_mesh; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutUniformBufferVertex()                          const { return m_vk_per_stage_descriptor_set_layout_uniform_buffer_vertex; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutUniformBufferGeometry()                        const { return m_vk_per_stage_descriptor_set_layout_uniform_buffer_geometry; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutUniformBufferFragment()                        const { return m_vk_per_stage_descriptor_set_layout_uniform_buffer_fragment; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutUniformBufferTask()                            const { return m_vk_per_stage_descriptor_set_layout_uniform_buffer_task; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutUniformBufferMesh()                            const { return m_vk_per_stage_descriptor_set_layout_uniform_buffer_mesh; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutStorageBufferVertex()                          const { return m_vk_per_stage_descriptor_set_layout_storage_buffer_vertex; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutStorageBufferGeometry()                        const { return m_vk_per_stage_descriptor_set_layout_storage_buffer_geometry; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutStorageBufferFragment()                        const { return m_vk_per_stage_descriptor_set_layout_storage_buffer_fragment; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutStorageBufferTask()                            const { return m_vk_per_stage_descriptor_set_layout_storage_buffer_task; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutStorageBufferMesh()                            const { return m_vk_per_stage_descriptor_set_layout_storage_buffer_mesh; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutCombinedTessellationEvaluation()               const { return m_vk_per_stage_descriptor_set_layout_combined_tessellation_evaluation; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutCombinedTessellationControl()                  const { return m_vk_per_stage_descriptor_set_layout_combined_tessellation_control; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerVertex()                 const { return m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_vertex; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerTessellationEvaluation() const { return m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_evaluation; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerTessellationControl()    const { return m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_tessellation_control; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerGeometry()               const { return m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_geometry; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerFragment()               const { return m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_fragment; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerTask()                   const { return m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_task; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerMesh()                   const { return m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_mesh; }

            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutTextureSamplerCompute()         const { return m_vk_per_stage_descriptor_set_layout_texture_sampler_compute; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutImageCompute()                  const { return m_vk_per_stage_descriptor_set_layout_image_compute; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutUniformBufferCompute()          const { return m_vk_per_stage_descriptor_set_layout_uniform_buffer_compute; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutStorageBufferCompute()          const { return m_vk_per_stage_descriptor_set_layout_storage_buffer_compute; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerCompute() const { return m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_compute; }

            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutTextureSampler(ShaderStage shader_stage) const {

                /* Get desc layout by shader stage */
                const u32 stage_index = static_cast<u32>(shader_stage);
                VkDescriptorSetLayout desc_layout;
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    desc_layout = std::addressof(m_vk_per_stage_descriptor_set_layout_combined_tessellation_control)[stage_index - 1];
                } else {
                    desc_layout = (1 < stage_index) ? std::addressof(m_vk_per_stage_descriptor_set_layout_texture_sampler_vertex)[stage_index - 2] : m_vk_per_stage_descriptor_set_layout_texture_sampler_vertex;
                }

                return desc_layout;
            }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutImage(ShaderStage shader_stage) const {

                /* Get desc layout by shader stage */
                const u32 stage_index = static_cast<u32>(shader_stage);
                VkDescriptorSetLayout desc_layout;
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    desc_layout = std::addressof(m_vk_per_stage_descriptor_set_layout_combined_tessellation_control)[stage_index - 1];
                } else {
                    desc_layout = (1 < stage_index) ? std::addressof(m_vk_per_stage_descriptor_set_layout_image_vertex)[stage_index - 2] : m_vk_per_stage_descriptor_set_layout_image_vertex;
                }

                return desc_layout;
            }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutUniformBuffer(ShaderStage shader_stage) const {

                /* Get desc layout by shader stage */
                const u32 stage_index = static_cast<u32>(shader_stage);
                VkDescriptorSetLayout desc_layout;
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    desc_layout = std::addressof(m_vk_per_stage_descriptor_set_layout_combined_tessellation_control)[stage_index - 1];
                } else {
                    desc_layout = (1 < stage_index) ? std::addressof(m_vk_per_stage_descriptor_set_layout_uniform_buffer_vertex)[stage_index - 2] : m_vk_per_stage_descriptor_set_layout_uniform_buffer_vertex;
                }

                return desc_layout;
            }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutStorageBuffer(ShaderStage shader_stage) const {

                /* Get desc layout by shader stage */
                const u32 stage_index = static_cast<u32>(shader_stage);
                VkDescriptorSetLayout desc_layout;
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    desc_layout = std::addressof(m_vk_per_stage_descriptor_set_layout_combined_tessellation_control)[stage_index - 1];
                } else {
                    desc_layout = (1 < stage_index) ? std::addressof(m_vk_per_stage_descriptor_set_layout_storage_buffer_vertex)[stage_index - 2] : m_vk_per_stage_descriptor_set_layout_storage_buffer_vertex;
                }

                return desc_layout;
            }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout  GetPerStageVkDescriptorSetLayoutSeparateTextureSampler(ShaderStage shader_stage) const {

                /* Get desc layout by shader stage */
                const u32 stage_index = static_cast<u32>(shader_stage);
                VkDescriptorSetLayout desc_layout;
                if (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) {
                    desc_layout = std::addressof(m_vk_per_stage_descriptor_set_layout_combined_tessellation_control)[stage_index - 1];
                } else {
                    desc_layout = (1 < stage_index) ? std::addressof(m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_vertex)[stage_index - 2] : m_vk_per_stage_descriptor_set_layout_separate_texture_sampler_vertex;
                }

                return desc_layout;
            }

            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseTextureSampler(ShaderStage shader_stage) {
                /* Get desc layout by shader stage */
                const u32 binding_base = (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) ? cCombinedBindingBaseTextureSampler : 0;
                return binding_base;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseImage(ShaderStage shader_stage) {
                /* Get desc layout by shader stage */
                const u32 binding_base = (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) ? cCombinedBindingBaseImage : 0;
                return binding_base;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseUniformBuffer(ShaderStage shader_stage) {
                /* Get desc layout by shader stage */
                const u32 binding_base = (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) ? cCombinedBindingBaseUniformBuffer : 0;
                return binding_base;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseStorageBuffer(ShaderStage shader_stage) {
                /* Get desc layout by shader stage */
                const u32 binding_base = (shader_stage == ShaderStage::TessellationEvaluation || shader_stage == ShaderStage::TessellationControl) ? cCombinedBindingBaseStorageBuffer : 0;
                return binding_base;
            }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseSeparateTexture(ShaderStage shader_stage) { VP_UNUSED(shader_stage); return 0; }
            static constexpr ALWAYS_INLINE u32 GetPerStageBindingBaseSeparateSampler(ShaderStage shader_stage) { VP_UNUSED(shader_stage); return 128; }


            constexpr ALWAYS_INLINE VkPipelineLayout GetVkPipelineLayoutGraphics() const { return m_vk_pipeline_layout_graphics; }
            constexpr ALWAYS_INLINE VkPipelineLayout GetVkPipelineLayoutCompute()  const { return m_vk_pipeline_layout_compute; }

            constexpr ALWAYS_INLINE const VkPhysicalDeviceProperties2 *GetPhysicalDeviceProperties() const { return std::addressof(m_vk_physical_device_properties); }

            constexpr ALWAYS_INLINE size_t GetSeparateTextureDescriptorSize() const { return m_vk_physical_device_descriptor_buffer_properties.sampledImageDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetSeparateSamplerDescriptorSize() const { return m_vk_physical_device_descriptor_buffer_properties.samplerDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetTextureSamplerDescriptorSize()  const { return m_vk_physical_device_descriptor_buffer_properties.combinedImageSamplerDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetImageDescriptorSize()           const { return m_vk_physical_device_descriptor_buffer_properties.storageImageDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetUniformBufferDescriptorSize()   const { return m_vk_physical_device_descriptor_buffer_properties.uniformBufferDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetStorageBufferDescriptorSize()   const { return m_vk_physical_device_descriptor_buffer_properties.storageBufferDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetDescriptorOffsetAlignment()     const { return m_vk_physical_device_descriptor_buffer_properties.descriptorBufferOffsetAlignment; }
    };
}

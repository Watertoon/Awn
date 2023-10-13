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
            static constexpr size_t cTargetMaxUniformBufferSize     = 0x10000;
            static constexpr size_t cTargetMaxTextureDescriptorSize = 0x20;
            static constexpr size_t cTargetMaxSamplerDescriptorSize = 0x20;

            /* Global descriptor resource limits */
            static constexpr size_t cTargetMaxDescriptorCount                = 0x4fa0;
            static constexpr size_t cTargetMaxTextureDescriptorCount         = 0x4000;
            static constexpr size_t cTargetMaxSamplerDescriptorCount         = 0xfa0;
            static constexpr size_t cTargetMaxUniformBufferDescriptorCount   = 16;
            static constexpr size_t cTargetMaxPushDescriptorCount            = cTargetMaxUniformBufferDescriptorCount;
            static constexpr size_t cTargetDescriptorSetLayoutCount          = 3;
            static constexpr size_t cTargetTextureSamplerDescriptorIndexSize = sizeof(u32);
            static constexpr size_t cTargetTextureDescriptorIndexBits        = 20;
            static constexpr size_t cTargetSamplerDescriptorIndexBits        = 12;

            /* Texture descriptor binding */
            static constexpr size_t cTargetTextureDescriptorBinding       = 0;
            static constexpr size_t cTargetSamplerDescriptorBinding       = 0;
            static constexpr size_t cTargetUniformBufferDescriptorBinding = 0;
            static constexpr size_t cTargetStorageBufferDescriptorBinding = 0;
            
            /* Push constant ranges */
            static constexpr size_t cTargetAllStagePushConstantRangeCount = 1;
            static constexpr size_t cTargetPushConstantOffset             = 0;
            static constexpr size_t cTargetPushConstantSize               = 256;

            /* Per Shader stage resource limits */
            static constexpr size_t cTargetMaxSimultaneousShaderStageCount = 5;
            static constexpr size_t cTargetMaxMeshShaderStageCount         = 3;
            static constexpr size_t cTargetMaxPrimitiveShaderStageCount    = 5;
            static constexpr size_t cTargetMaxComputeShaderStageCount      = 1;
            static constexpr size_t cTargetMaxPerStageUniformBufferCount   = 14;
            static constexpr size_t cTargetMaxPerStageStorageBufferCount   = 16;
            static constexpr size_t cTargetMaxPerStageTextureCount         = 32;
            static constexpr size_t cTargetMaxPerStageSamplerCount         = 32;
            static constexpr size_t cTargetMaxPerStageStorageImageCount    = 8;
            
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
            VkDescriptorSetLayout                                        m_vk_uniform_buffer_descriptor_set_layout;
            VkPipelineLayout                                             m_vk_pipeline_layout;
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

            VkPhysicalDeviceMemoryProperties                             m_vk_physical_device_memory_properties;

            #if defined(VP_DEBUG)                               
                VkDebugUtilsMessengerEXT                                 m_debug_messenger;
            #endif
        public:
            AWN_SINGLETON_TRAITS(Context);
        public:
            constexpr ALWAYS_INLINE Context() :
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

            constexpr ALWAYS_INLINE const VkPushConstantRange   *GetVkPushConstantRangeArray()     const { return std::addressof(m_vk_push_constant_range); }
            constexpr ALWAYS_INLINE const VkDescriptorSetLayout *GetVkDescriptorSetLayoutArray()   const { return std::addressof(m_vk_texture_descriptor_set_layout); }
            constexpr ALWAYS_INLINE       VkDescriptorSetLayout  GetTextureVkDescriptorSetLayout() const { return m_vk_texture_descriptor_set_layout; }
            constexpr ALWAYS_INLINE       VkDescriptorSetLayout  GetSamplerVkDescriptorSetLayout() const { return m_vk_sampler_descriptor_set_layout; }
            constexpr ALWAYS_INLINE       VkPipelineLayout       GetVkPipelineLayout()             const { return m_vk_pipeline_layout; }

            constexpr ALWAYS_INLINE const VkPhysicalDeviceProperties2 *GetPhysicalDeviceProperties() const { return std::addressof(m_vk_physical_device_properties); }

            constexpr ALWAYS_INLINE size_t GetTextureDescriptorSize() const { return m_vk_physical_device_descriptor_buffer_properties.sampledImageDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetSamplerDescriptorSize() const { return m_vk_physical_device_descriptor_buffer_properties.samplerDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetDescriptorAlignment()   const { return m_vk_physical_device_descriptor_buffer_properties.descriptorBufferOffsetAlignment; }
    };
}

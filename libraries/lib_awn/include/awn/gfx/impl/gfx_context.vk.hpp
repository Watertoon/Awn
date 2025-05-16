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

    Result ConvertVkResultToResult(VkResult vk_result);

    struct ContextInfo {
        const char *app_name = "AwnDefault";
        const char *engine_name = "Awn";
        OptionalFeatureMask preferred_optional_feature_mask;
        OptionalFeatureMask required_optional_feature_mask;
        size_t texture_view_descriptor_count;
        size_t sampler_descriptor_count;
    };

    struct ContextImportInfo {
        VkInstance vk_instance;
        VkPhysicalDevice vk_physical_device;
        VkDevice vk_device;
    };

    class Context {
        public:
            /* Support Vulkan 1.3 */
            static constexpr u32 cTargetMinimumApiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0);

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
            static constexpr size_t cTargetDescriptorBufferAlignment = 0x100;
            static constexpr size_t cTargetDescriptorBufferOffsetAlignment = 0x40;

            /* Max resource sizes */
            static constexpr size_t cTargetMaxUniformBufferSize = vp::util::c64KB;

            /* Global descriptor resource limits */
            static constexpr size_t cTargetMaxTextureViewCount = 0xffff;
            static constexpr size_t cTargetMaxSamplerCount = 0xfa0;

            static constexpr size_t cTargetMaxTextureSamplerDescriptorSize  = 0x80;
            static constexpr size_t cTargetMaxUniformBufferDescriptorSize   = 0x40;
            static constexpr size_t cTargetMaxStorageBufferDescriptorSize   = 0x40;
            static constexpr size_t cTargetMaxImageDescriptorSize           = 0x40;
            static constexpr size_t cTargetMaxSeparateTextureDescriptorSize = 0x40;
            static constexpr size_t cTargetMaxSeparateSamplerDescriptorSize = 0x40;

            /* Descriptor buffer index */
            static constexpr u32 cTargetDescriptorBufferIndexBindless = 0;
            static constexpr u32 cTargetDescriptorBufferIndexPerStage = 0;

            /* Base descriptor set layout bindings */
            static constexpr size_t cBindlessBindingBaseTexture = 0;
            static constexpr size_t cBindlessBindingBaseSampler = 1;

            /* Per Shader stage resource limits */
            static constexpr size_t cTargetMaxShaderStageCount = 8;
            static constexpr size_t cTargetMaxSimultaneousShaderStageCount = 5;

            static constexpr size_t cTargetMaxMeshShaderStageCount         = 3;
            static constexpr size_t cTargetMaxPrimitiveShaderStageCount    = 5;
            static constexpr size_t cTargetMaxComputeShaderStageCount      = 1;
        private:
            static constexpr size_t cTargetBindlessLayoutSet = 0;

            /* NV Per Stage layout sets */
            static constexpr size_t cTargetPerStageTextureSamplerLayoutSet = 1;
            static constexpr size_t cTargetPerStageBufferLayoutSet = 2;

            static constexpr size_t cMaxPerStageSetCount = 3;

            /* Global pipeline layout */
            static constexpr size_t cTargetPerStageTextureSamplerLayoutSetFragment = 1;
            static constexpr size_t cTargetPerStageBufferLayoutSetFragment = 2;
            static constexpr size_t cTargetPerStageTextureSamplerLayoutSetVertex = 3;
            static constexpr size_t cTargetPerStageBufferLayoutSetVertex = 4;
            static constexpr size_t cTargetPerStageTextureSamplerLayoutSetGeometry = 5;
            static constexpr size_t cTargetPerStageBufferLayoutSetGeometry = 6;
            static constexpr size_t cTargetPerStageTextureSamplerLayoutSetTessellationControl = 7;
            static constexpr size_t cTargetPerStageBufferLayoutSetTessellationControl = 8;
            static constexpr size_t cTargetPerStageTextureSamplerLayoutSetTessellationEvaluation = 9;
            static constexpr size_t cTargetPerStageBufferLayoutSetTessellationEvaluation = 10;
            static constexpr size_t cTargetPerStageTextureSamplerLayoutSetTask = 11;
            static constexpr size_t cTargetPerStageBufferLayoutSetTask = 12;
            static constexpr size_t cTargetPerStageTextureSamplerLayoutSetMesh = 13;
            static constexpr size_t cTargetPerStageBufferLayoutSetMesh = 14;

            static constexpr size_t cMaxCombinedSetCountGraphics = 15;

            static constexpr size_t cTargetPerStageTextureSamplerLayoutSetCompute = 1;
            static constexpr size_t cTargetPerStageBufferLayoutSetCompute = 2;

            static constexpr size_t cMaxCombinedSetCountCompute = 3;

            static constexpr size_t cTargetPerStageTextureSamplerLayoutSetArray[cTargetMaxShaderStageCount] = {
                cTargetPerStageTextureSamplerLayoutSetVertex,
                cTargetPerStageTextureSamplerLayoutSetTessellationControl,
                cTargetPerStageTextureSamplerLayoutSetTessellationEvaluation,
                cTargetPerStageTextureSamplerLayoutSetGeometry,
                cTargetPerStageTextureSamplerLayoutSetFragment,
                cTargetPerStageTextureSamplerLayoutSetCompute,
                cTargetPerStageTextureSamplerLayoutSetTask,
                cTargetPerStageTextureSamplerLayoutSetMesh,
            };
            static constexpr size_t cTargetPerStageBufferLayoutSetArray[cTargetMaxShaderStageCount] = {
                cTargetPerStageBufferLayoutSetVertex,
                cTargetPerStageBufferLayoutSetTessellationControl,
                cTargetPerStageBufferLayoutSetTessellationEvaluation,
                cTargetPerStageBufferLayoutSetGeometry,
                cTargetPerStageBufferLayoutSetFragment,
                cTargetPerStageBufferLayoutSetCompute,
                cTargetPerStageBufferLayoutSetTask,
                cTargetPerStageBufferLayoutSetMesh,
            };
        public:
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
        private:
            void InitializeInstance(const ContextInfo *context_info);
            void FinalizeInstance();
            void SetVkPhysicalDevice(VkPhysicalDevice vk_physical_device, const ContextInfo *context_info);
            void InitializeLogicalDevice(const ContextInfo *context_info);
            void FinalizeLogicalDevice();
            void InitializePipelineLayout(const ContextInfo *context_info);
            void FinalizePipelineLayout();
        public:
            struct PerStageDescriptorLayout {
                VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info;
                VkDescriptorSetLayout vk_descriptor_set_layout_array[cMaxPerStageSetCount];
                //VkPushConstantRange vk_push_constant_range;
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
            VkDescriptorSetLayout m_vk_combined_descriptor_set_layout_graphics[cMaxCombinedSetCountGraphics];
            VkDescriptorSetLayout m_vk_combined_descriptor_set_layout_compute[cMaxCombinedSetCountCompute];
            VkPipelineLayout m_vk_combined_pipeline_layout_graphics;
            VkPipelineLayout m_vk_combined_pipeline_layout_compute;

            /* Vulkan physical device properties */
            VkPhysicalDeviceProperties2                        m_vk_physical_device_properties;
            VkPhysicalDeviceVulkan11Properties                 m_vk_physical_device_properties_11;
            VkPhysicalDeviceVulkan12Properties                 m_vk_physical_device_properties_12;
            VkPhysicalDeviceVulkan13Properties                 m_vk_physical_device_properties_13;
            VkPhysicalDeviceVulkan14Properties                 m_vk_physical_device_properties_14;
            VkPhysicalDeviceShaderObjectPropertiesEXT          m_vk_physical_device_shader_object_properties;
            VkPhysicalDeviceDescriptorBufferPropertiesEXT      m_vk_physical_device_descriptor_buffer_properties;
            VkPhysicalDeviceNestedCommandBufferPropertiesEXT   m_vk_physical_device_nested_command_buffer_properties;
            VkPhysicalDeviceRobustness2PropertiesEXT           m_vk_physical_device_robustness_2_properties;
            VkPhysicalDeviceMeshShaderPropertiesEXT            m_vk_physical_device_mesh_shader_properties;

            /* Vulkan physical device features */
            MissingFeatures     m_missing_features;
            OptionalFeatureMask m_optional_features;
            bool                m_is_unified_memory;
            //bool                m_is_ext_bindless_nv;

            VkPhysicalDeviceMemoryProperties m_vk_physical_device_memory_properties;

            #if defined(AWN_VK_DEBUG)                               
                VkDebugUtilsMessengerEXT m_debug_messenger;
                //VkAllocationCallbacks m_debug_callbacks;
            #endif
        public:
            AWN_SINGLETON_TRAITS(Context);
        public:
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
#ifdef AWN_VK_DEBUG
            void ReportDeviceLostReason();
#endif
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
                .pNext = std::addressof(m_vk_physical_device_properties_14),
            },
            m_vk_physical_device_properties_14 {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES,
                .pNext = std::addressof(m_vk_physical_device_shader_object_properties),
            },
            m_vk_physical_device_shader_object_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_descriptor_buffer_properties),
            },
            m_vk_physical_device_descriptor_buffer_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_nested_command_buffer_properties),
            },
            m_vk_physical_device_nested_command_buffer_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_robustness_2_properties),
            },
            m_vk_physical_device_robustness_2_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT,
                .pNext = std::addressof(m_vk_physical_device_mesh_shader_properties),
            },
            m_vk_physical_device_mesh_shader_properties {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT,
                .pNext = nullptr,
            },
            m_missing_features(),
            m_optional_features(),
            m_is_unified_memory()
            //m_is_ext_bindless_nv()
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
            void SubmitCommandLists(CommandList *command_list_array, u32 command_list_count, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count);
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

            constexpr ALWAYS_INLINE VkInstance GetVkInstance() const { return m_vk_instance; }
            constexpr ALWAYS_INLINE VkPhysicalDevice GetVkPhysicalDevice() const { return m_vk_physical_device; }
            constexpr ALWAYS_INLINE VkDevice GetVkDevice() const { return m_vk_device; }
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
            constexpr ALWAYS_INLINE VkDescriptorSetLayout GetPerStageVkDescriptorSetLayoutCombinedTextureSampler(ShaderStage shader_stage) const { return this->GetPerStageDescriptorLayout(shader_stage)->vk_descriptor_set_layout_array[cTargetPerStageTextureSamplerLayoutSet]; }
            constexpr ALWAYS_INLINE VkDescriptorSetLayout GetPerStageVkDescriptorSetLayoutCombinedBuffer(ShaderStage shader_stage) const { return this->GetPerStageDescriptorLayout(shader_stage)->vk_descriptor_set_layout_array[cTargetPerStageBufferLayoutSet]; }
            constexpr ALWAYS_INLINE const VkDescriptorSetLayout *GetPerStageDescriptorLayoutArray(u32 *out_count, ShaderStage shader_stage) const {
                const VkDescriptorSetLayout *array;
                if (shader_stage == ShaderStage::Compute) {
                    array = m_vk_combined_descriptor_set_layout_compute;
                    *out_count = cMaxCombinedSetCountCompute;
                } else {
                    const bool is_mesh_shaders = this->IsOptionalFeature(OptionalFeature::SupportsMeshShaders);
                    array = m_vk_combined_descriptor_set_layout_graphics;
                    *out_count = cMaxCombinedSetCountGraphics - (is_mesh_shaders * 4);
                }
                return array;
            }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayout(ShaderStage shader_stage) const { return std::addressof(m_per_stage_descriptor_layout_array[static_cast<u32>(shader_stage)]); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutVertex()                   const { return std::addressof(m_per_stage_descriptor_layout_vertex); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutTessellationEvaluation()   const { return std::addressof(m_per_stage_descriptor_layout_tessellation_evaluation); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutTessellationControl()      const { return std::addressof(m_per_stage_descriptor_layout_tessellation_control); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutGeometry()                 const { return std::addressof(m_per_stage_descriptor_layout_geometry); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutFragment()                 const { return std::addressof(m_per_stage_descriptor_layout_fragment); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutCompute()                  const { return std::addressof(m_per_stage_descriptor_layout_compute); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutTask()                     const { return std::addressof(m_per_stage_descriptor_layout_task); }
            constexpr ALWAYS_INLINE const PerStageDescriptorLayout *GetPerStageDescriptorLayoutMesh()                     const { return std::addressof(m_per_stage_descriptor_layout_mesh); }

            constexpr ALWAYS_INLINE VkPipelineLayout GetVkPipelineLayoutGraphics() const { return m_vk_combined_pipeline_layout_graphics; }
            constexpr ALWAYS_INLINE VkPipelineLayout GetVkPipelineLayoutCompute() const { return m_vk_combined_pipeline_layout_compute; }

            constexpr ALWAYS_INLINE u32 GetBindlessSet() {
                return cTargetBindlessLayoutSet;
            }
            constexpr ALWAYS_INLINE u32 GetPerStageSetCombinedTextureSampler(ShaderStage shader_stage) {
                const u32 base_set = cTargetPerStageTextureSamplerLayoutSetArray[static_cast<u32>(shader_stage)];
                return base_set;
            }
            constexpr ALWAYS_INLINE u32 GetPerStageSetCombinedBuffer(ShaderStage shader_stage) {
                const u32 base_set = cTargetPerStageBufferLayoutSetArray[static_cast<u32>(shader_stage)];
                return base_set;
            }
            constexpr ALWAYS_INLINE u32 GetPerStageSetTextureSampler(ShaderStage shader_stage) {
                return this->GetPerStageSetCombinedTextureSampler(shader_stage);
            }
            constexpr ALWAYS_INLINE u32 GetPerStageSetImage(ShaderStage shader_stage) {
                return this->GetPerStageSetCombinedTextureSampler(shader_stage);
            }
            constexpr ALWAYS_INLINE u32 GetPerStageSetUniformBuffer(ShaderStage shader_stage) {
                return this->GetPerStageSetCombinedBuffer(shader_stage);
            }
            constexpr ALWAYS_INLINE u32 GetPerStageSetStorageBuffer(ShaderStage shader_stage) {
                return this->GetPerStageSetCombinedBuffer(shader_stage);
            }
            constexpr ALWAYS_INLINE u32 GetPerStageSetSeparateTexture(ShaderStage shader_stage) {
                return this->GetPerStageSetCombinedTextureSampler(shader_stage);
            }
            constexpr ALWAYS_INLINE u32 GetPerStageSetSeparateSampler(ShaderStage shader_stage) {
                return this->GetPerStageSetCombinedTextureSampler(shader_stage);
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

            constexpr ALWAYS_INLINE size_t GetBindlessDescriptorLayoutSize()  const {
                size_t layout_size = 0;
                ::pfn_vkGetDescriptorSetLayoutSizeEXT(this->GetVkDevice(), this->GetBindlessVkDescriptorSetLayout(), std::addressof(layout_size));
                return vp::util::AlignUp(layout_size, this->GetDescriptorOffsetAlignment());
            }

            constexpr ALWAYS_INLINE size_t GetCombinedTextureSamplerDescriptorLayoutSize()  const {
                size_t layout_size = 0;
                ::pfn_vkGetDescriptorSetLayoutSizeEXT(this->GetVkDevice(), this->GetPerStageVkDescriptorSetLayoutCombinedTextureSampler(ShaderStage::Vertex), std::addressof(layout_size));
                return vp::util::AlignUp(layout_size, this->GetDescriptorOffsetAlignment());
            }

            constexpr ALWAYS_INLINE size_t GetCombinedBufferDescriptorLayoutSize()  const {
                size_t layout_size = 0;
                ::pfn_vkGetDescriptorSetLayoutSizeEXT(this->GetVkDevice(), this->GetPerStageVkDescriptorSetLayoutCombinedBuffer(ShaderStage::Vertex), std::addressof(layout_size));
                return vp::util::AlignUp(layout_size, this->GetDescriptorOffsetAlignment());
            }

            constexpr ALWAYS_INLINE size_t GetMaxDescriptorSize()  const {
                const size_t max0 = vp::util::Max(this->GetTextureSamplerDescriptorSize(), this->GetUniformBufferDescriptorSize());
                const size_t max1 = vp::util::Max(max0, this->GetStorageBufferDescriptorSize());
                const size_t max2 = vp::util::Max(max1, this->GetImageDescriptorSize());
                const size_t max3 = vp::util::Max(max2, this->GetSeparateTextureDescriptorSize());
                const size_t max4 = vp::util::Max(max3, this->GetSeparateSamplerDescriptorSize());
                return max4;
            }
            constexpr ALWAYS_INLINE size_t GetTextureSamplerDescriptorSize()  const { return m_vk_physical_device_descriptor_buffer_properties.combinedImageSamplerDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetUniformBufferDescriptorSize()   const { return m_vk_physical_device_descriptor_buffer_properties.uniformBufferDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetStorageBufferDescriptorSize()   const { return m_vk_physical_device_descriptor_buffer_properties.storageBufferDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetImageDescriptorSize()           const { return m_vk_physical_device_descriptor_buffer_properties.storageImageDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetSeparateTextureDescriptorSize() const { return m_vk_physical_device_descriptor_buffer_properties.sampledImageDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetSeparateSamplerDescriptorSize() const { return m_vk_physical_device_descriptor_buffer_properties.samplerDescriptorSize; }
            constexpr ALWAYS_INLINE size_t GetDescriptorOffsetAlignment()     const { return m_vk_physical_device_descriptor_buffer_properties.descriptorBufferOffsetAlignment; }

            constexpr bool IsOptionalFeature(OptionalFeature optional_feature) const {
                return m_optional_features.GetBit(static_cast<u32>(optional_feature));
            }
            constexpr bool IsUnifiedMemory() const {
                return m_is_unified_memory;
            }
    };
}

namespace awn::gfx {

    constexpr bool IsOptionalFeature(OptionalFeature optional_feature) {
        impl::Context *context = impl::Context::GetInstance();
        VP_ASSERT(context != nullptr);
        return context->IsOptionalFeature(optional_feature);
    }
}

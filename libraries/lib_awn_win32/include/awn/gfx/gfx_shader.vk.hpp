#pragma once

namespace awn::gfx {

    enum class ShaderStage {
        Vertex                 = (1 << 1),
        TessellationEvaluation = (1 << 2),
        TessellationControl    = (1 << 3),
        Geometry               = (1 << 4),
        Fragment               = (1 << 5),
        Compute                = (1 << 6),
        Task                   = (1 << 7),
        Mesh                   = (1 << 8),
        Ray                    = (1 << 9),
        AnyHit                 = (1 << 10),
        ClosestHit             = (1 << 11),
        Miss                   = (1 << 12),
        Callable               = (1 << 13),
    };

    struct MeshShaderInfo {
        u32   vertex_code_size;
        u32   tessellation_control_code_size;
        u32   tessellation_evaluation_code_size;
        u32   geometry_code_size;
        u32   fragment_code_size;
        void *vertex_code;
        void *tessellation_control_code;
        void *tessellation_evaluation_code;
        void *geometry_code;
        void *fragment_code;
    };
    struct PrimitiveShaderInfo {
        u32   task_code_size;
        u32   mesh_code_size;
        u32   fragment_code_size;
        void *task_code;
        void *mesh_code;
        void *fragment_code;
    };
    struct ComputeShaderInfo {
        u32   compute_code_size;
        void *compute_code;
    };

    class Shader {
        public:
            friend class CommandBuffer;
        private:
            u32         m_shader_count;
            ShaderStage m_shader_stage_mask;
            VkShaderExt m_vk_shader_array[Context::TargetMaxSimultaneousShaderStageCount];
        public:
            constexpr ALWAYS_INLINE Shader() : m_shader_count(0), m_shader_stage_mask(0), m_vk_shader_array{} {/*...*/}

            void Initialize(MeshShaderInfo *shader_info, const bool is_binary = false) {

                /* Integrity checks */
                VP_ASSERT(shader_info->mesh_code != nullptr & shader_info->fragment_code != nullptr);

                /* Setup mesh, fragment, and the optional task shader info */
                const VkShaderCreateInfoExt vk_shader_create_info_array[Context::TargetMaxMeshShaderStageCount] = {
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flag                   = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0),
                        .stage                  = VK_SHADER_STAGE_MESH_BIT,
                        .nextStage              = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = shader_info->mesh_code_size,
                        .pCode                  = shader_info->mesh_code,
                        .pName                  = "main",
                        .setLayoutCount         = Context::TargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetDescriporSetLayoutArray(),
                        .pushConstantRangeCount = Context::TargetMeshShaderPushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetMeshShaderPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flag                   = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0),
                        .stage                  = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .nextStage              = 0,
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = shader_info->fragment_code_size,
                        .pCode                  = shader_info->fragment_code,
                        .pName                  = "main",
                        .setLayoutCount         = Context::TargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetDescriporSetLayoutArray(),
                        .pushConstantRangeCount = Context::TargetMeshShaderPushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetMeshShaderPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flag                   = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0),
                        .stage                  = VK_SHADER_STAGE_TASK_BIT,
                        .nextStage              = VK_SHADER_STAGE_MESH_BIT,
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = shader_info->task_code_size,
                        .pCode                  = shader_info->task_code,
                        .pName                  = "main",
                        .setLayoutCount         = Context::TargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetDescriporSetLayoutArray(),
                        .pushConstantRangeCount = Context::TargetMeshShaderPushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetMeshShaderPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                };

                /* Create shaders */
                const u32 shader_count = (shader_info->task_code == nullptr) ? 2 : 3;
                const u32 vk_result    = ::pfn_vkCreateShadersExt(Context::GetInstance()->GetVkDevice(), shader_count, vk_shader_create_info_array, Context::GetInstance()->GetVkAllocationCallbacks(), m_vk_shader_array);
                VP_ASSERT(vk_result == VK_SUCCESS);

                /* Set state */
                m_shader_count      = shader_count;
                m_shader_stage_mask = ShaderStage::Mesh | ShaderStage::Fragment | ((shader_info->task_code) ? VK_SHADER_STAGE_TASK_BIT : 0);

                return;
            }

            void Initialize(PrimitiveShaderInfo *shader_info, const bool is_binary = true) {

                /* Integrity checks */
                VP_ASSERT(shader_info->vertex_code != nullptr & shader_info->fragment_code != nullptr);

                /* Calculate shader stages */
                u32 stage_array[Context::TargetMaxPrimitiveShaderStageCount] = {
                    VK_SHADER_STAGE_VERTEX_BIT,
                    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                    VK_SHADER_STAGE_GEOMETRY_BIT,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                };
                u32 next_stage_array[Context::TargetMaxPrimitiveShaderStageCount] = {
                    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                    VK_SHADER_STAGE_GEOMETRY_BIT,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                };
                u32 code_size_array[Context::TargetMaxPrimitiveShaderStageCount] = {
                    shader_info->vertex_code_size,
                    shader_info->tessellation_control_code_size,
                    shader_info->tessellation_evaluation_code_size,
                    shader_info->geometry_code_size,
                    shader_info->fragment_code_size,
                };
                void *code_array[Context::TargetMaxPrimitiveShaderStageCount] = {
                    shader_info->vertex_code,
                    shader_info->tessellation_control_code,
                    shader_info->tessellation_evaluation_code,
                    shader_info->geometry_code,
                    shader_info->fragment_code,
                };

                /* TODO; refactor to reduce redundancy? */
                u32 shader_count      = 5;
                u32 shader_stage_mask = ShaderStage::Vertex | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Geometry | ShaderStage::Fragment;
                if (shader_info->geometry_code == nullptr && (shader_info->tessellation_control_code == nullptr || shader_info->tesselation_evaluation_code == nullptr)) {
                    stage_array[1]      = VK_SHADER_STAGE_FRAGMENT_BIT;
                    next_stage_array[1] = 0;
                    code_size_array[1]  = shader_info->fragment_code_size;
                    code_array[1]       = shader_info->fragment_code;
                    shader_count        = 2;
                    shader_stage_mask   = ShaderStage::Vertex | ShaderStage::Fragment;
                } else if (shader_info->tessellation_control_code == nullptr || shader_info->tessellation_evaluation_code == nullptr) {
                    stage_array[1]      = VK_SHADER_STAGE_GEOMETRY_BIT;
                    stage_array[2]      = VK_SHADER_STAGE_FRAGMENT_BIT;
                    next_stage_array[1] = VK_SHADER_STAGE_FRAGMENT_BIT;
                    next_stage_array[2] = 0;
                    code_size_array[1]  = shader_info->geometry_code_size;
                    code_size_array[2]  = shader_info->fragment_code_size;
                    code_array[1]       = shader_info->geometry_code;
                    code_array[2]       = shader_info->fragment_code;
                    shader_count        = 3;
                    shader_stage_mask   = ShaderStage::Vertex | ShaderStage::Geometry | ShaderStage::Fragment;
                } else if (geometry->geometry_code == nullptr) {
                    stage_array[3]      = VK_SHADER_STAGE_FRAGMENT_BIT;
                    next_stage_array[3] = 0;
                    code_size_array[3]  = shader_info->fragment_code_size;
                    code_array[3]       = shader_info->fragment_code;
                    shader_count        = 4;
                    shader_stage_mask   = ShaderStage::Vertex | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Fragment;
                }

                /* Setup mesh, fragment, and the optional task shader info */
                const VkShaderCreateInfoExt vk_shader_create_info_array[Context::TargetMaxPrimitiveShaderStageCount] = {
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flag                   = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0),
                        .stage                  = stage_array[0],
                        .nextStage              = next_stage_array[0],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[0],
                        .pCode                  = code_array[0],
                        .pName                  = "main",
                        .setLayoutCount         = Context::TargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetDescriporSetLayoutArray(),
                        .pushConstantRangeCount = Context::TargetMeshShaderPushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetMeshShaderPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flag                   = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0),
                        .stage                  = stage_array[1],
                        .nextStage              = next_stage_array[1],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[1],
                        .pCode                  = code_array[1],
                        .pName                  = "main",
                        .setLayoutCount         = Context::TargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetDescriporSetLayoutArray(),
                        .pushConstantRangeCount = Context::TargetMeshShaderPushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetMeshShaderPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flag                   = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0),
                        .stage                  = stage_array[2],
                        .nextStage              = next_stage_array[2],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[2],
                        .pCode                  = code_array[2],
                        .pName                  = "main",
                        .setLayoutCount         = Context::TargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetDescriporSetLayoutArray(),
                        .pushConstantRangeCount = Context::TargetMeshShaderPushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetMeshShaderPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flag                   = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0),
                        .stage                  = stage_array[3],
                        .nextStage              = next_stage_array[3],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[3],
                        .pCode                  = code_array[3],
                        .pName                  = "main",
                        .setLayoutCount         = Context::TargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetDescriporSetLayoutArray(),
                        .pushConstantRangeCount = Context::TargetMeshShaderPushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetMeshShaderPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flag                   = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0),
                        .stage                  = stage_array[4],
                        .nextStage              = next_stage_array[4],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[4],
                        .pCode                  = code_array[4],
                        .pName                  = "main",
                        .setLayoutCount         = Context::TargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetDescriporSetLayoutArray(),
                        .pushConstantRangeCount = Context::TargetMeshShaderPushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetMeshShaderPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                };

                /* Create shaders */
                const u32 vk_result = ::pfn_vkCreateShadersExt(Context::GetInstance()->GetVkDevice(), shader_count, vk_shader_create_info_array, Context::GetInstance()->GetVkAllocationCallbacks(), m_vk_shader_array);
                VP_ASSERT(vk_result == VK_SUCCESS);

                /* Set state */
                m_shader_count      = shader_count;
                m_shader_stage_mask = shader_stage_mask;

                return;
            }

            void Initialize(ComputeShaderInfo *shader_info, const bool is_binary = true) {

                /* Integrity checks */
                VP_ASSERT(shader_info->compute_code != nullptr);

                /* Setup compute shader info */
                const VkShaderCreateInfoExt vk_shader_create_info_array[Context::TargetMaxComputeShaderStageCount] = {
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flag                   = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0),
                        .stage                  = VK_SHADER_STAGE_COMPUTE_BIT,
                        .nextStage              = 0,
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = shader_info->compute_code_size,
                        .pCode                  = shader_info->compute_code,
                        .pName                  = "main",
                        .setLayoutCount         = Context::TargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetDescriporSetLayoutArray(),
                        .pushConstantRangeCount = Context::TargetMeshShaderPushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetMeshShaderPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    }
                };

                /* Create shaders */
                const u32 vk_result = ::pfn_vkCreateShadersEXT(Context::GetInstance()->GetVkDevice(), Context::TargetMaxComputeShaderStageCount, vk_shader_create_info_array, Context::GetInstance()->GetVkAllocationCallbacks(), m_vk_shader_array);
                VP_ASSERT(vk_result == VK_SUCCESS);

                /* Set state */
                m_shader_count      = Context::TargetMaxComputeShaderStageCount;
                m_shader_stage_mask = ShaderStage::Compute;

                return;
            }

            void Finalize() {

                /* Delete shaders */
                for (u32 i = 0; i < m_shader_count; ++i) {
                    ::pfn_vkDestroyShaderEXT(Context::GetInstance()->GetVkDevice(), m_vk_shader_array[i], Context::GetInstance()->GetVkAllocationCallbacks());
                }

                /* Reset state */
                m_vk_shader_array[0] = VK_NULL_HANDLE;
                m_vk_shader_array[1] = VK_NULL_HANDLE;
                m_vk_shader_array[2] = VK_NULL_HANDLE;
                m_vk_shader_array[3] = VK_NULL_HANDLE;
                m_vk_shader_array[4] = VK_NULL_HANDLE;
                m_shader_count       = 0;
                m_shader_stage_mask  = 0;
            }
    };
}

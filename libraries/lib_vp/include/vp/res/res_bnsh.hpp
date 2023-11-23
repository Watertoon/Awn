#pragma once

namespace vp::res {

    struct ResBnshShaderCodeInfo {
        void *runtime_user_shader_code_buffer_address;
        void *shader_control;
        void *shader_code;
        u32   shader_code_size;
        u32   shader_control_size;
        void *shader_control_extra_info;
        u32   exact_scratch_memory;
        u32   scaled_scratch_memory;
        void *shader_ext_data;
        u64   reserve0;
    };
    static_assert(sizeof(ResBnshShaderCodeInfo) == 0x40);

    struct ResBnshExtendedInterfaceInfo {
        ResNintendoWareDictionary *reserve1_dictionary;
        ResNintendoWareDictionary *reserve2_dictionary;
        u32                        reserve1_base_index;
        u32                        reserve2_base_index;
    };
    static_assert(sizeof(ResBnshExtendedInterfaceInfo) == 0x18);

    struct ResBnshShaderInterfaceInfo {
        ResNintendoWareDictionary    *input_attribute_dictionary;
        ResNintendoWareDictionary    *output_attribute_dictionary;
        ResNintendoWareDictionary    *sampler_dictionary;
        ResNintendoWareDictionary    *constant_buffer_dictionary;
        ResNintendoWareDictionary    *storage_buffer_dictionary;
        u32                           output_base_index;
        u32                           sampler_base_index;
        u32                           constant_buffer_base_index;
        u32                           storage_buffer_base_index;
        u32                          *interface_slot_array;
        s32                           work_group_x;
        s32                           work_group_y;
        s32                           work_group_z;
        u32                           reserve0_base_index;
        ResNintendoWareDictionary    *reserve0_dictionary;
        ResBnshExtendedInterfaceInfo *extended_interface_info;
    };
    static_assert(sizeof(ResBnshShaderInterfaceInfo) == 0x60);

    enum class GfxShaderStage : u32 {
        Vertex                = 0,
        TesselationControl    = 1,
        TessleationEvaluation = 2,
        Geometry              = 3,
        Fragment              = 4,
        Compute               = 5,
    };

    enum class GfxShaderCodeType : u8 {
        Binary         = 0x0,
        Intermediate   = 0x1,
        Source         = 0x2,
    };

    enum class GfxShaderInterfaceType : u32 {
        InputAttribute  = 0x0,
        OutputAttribute = 0x1,
        Sampler         = 0x2,
        ConstantBuffer  = 0x3,
        StorageBuffer   = 0x4,
        Reserve0        = 0x5,
        Reserve1        = 0x6,
        Reserve2        = 0x7,
    };

    struct ResBnshShaderInterfaceInfoTable {
        ResBnshShaderInterfaceInfo *vertex_interface_info;
        ResBnshShaderInterfaceInfo *tessellation_evaluation_interface_info;
        ResBnshShaderInterfaceInfo *tessellation_control_interface_info;
        ResBnshShaderInterfaceInfo *geometry_interface_info;
        ResBnshShaderInterfaceInfo *fragment_interface_info;
        ResBnshShaderInterfaceInfo *compute_interface_info;
        u64                         reserve0[2];
    };
    static_assert(sizeof(ResBnshShaderInterfaceInfoTable) == 0x40);

    struct ResBnshShaderVariation;

    struct ResBnshShaderProgram {
        union {
            u8 option;
            struct {
                u8 reserve0               : 1;
                u8 is_nvn_program_ext_api : 1;
                u8 reserve1               : 6;
            };
        };
        u8                               code_type;
        u16                              reserve2;
        u32                              reserve3;
        ResBnshShaderCodeInfo           *vertex_info;
        ResBnshShaderCodeInfo           *tessellation_control_info;
        ResBnshShaderCodeInfo           *tessellation_evaluation_info;
        ResBnshShaderCodeInfo           *geometry_info;
        ResBnshShaderCodeInfo           *fragment_info;
        ResBnshShaderCodeInfo           *compute_info;
        u64                              reserve4[5];
        u32                              object_size;
        u32                              reserve5;
        void                            *runtime_shader;
        ResBnshShaderVariation          *parent_variation;
        ResBnshShaderInterfaceInfoTable *shader_interface_info_table;
        u64                              reserve6[4];
    };
    static_assert(sizeof(ResBnshShaderProgram) == 0xa0);

    struct ResBnshShaderContainer;
    
    struct ResBnshShaderVariation {
        ResBnshShaderProgram   *program_source;
        ResBnshShaderProgram   *program_intermediate;
        ResBnshShaderProgram   *program_binary;
        ResBnshShaderContainer *parent_container;
        u64                     reserve0[4];
    };

    struct ResBnshShaderContainerData {
        ResGfxMemoryPoolInfo  memory_pool_info;
        u64                   reserve0;
        u64                   reserve1;
        void                 *runtime_memory_pool;
        u64                   reserve2[5];
    };
    static_assert(sizeof(ResBnshShaderContainerData) == 0x50);

    enum class BnshTargetApiType : u16 {
        Nvn = 4,
    };

    struct ResBnshShaderContainer : public ResNintendoWareSubHeader {
        u16                         target_api_type;
        u16                         target_api_version;
        u8                          target_code_type;
        u8                          reserve0;
        u16                         reserve1;
        u32                         compiler_version;
        u32                         shader_variation_count;
        ResBnshShaderVariation     *shader_variation_array;
        ResBnshShaderContainerData *shader_container_data;
        u16                         reserve2[4];
        u64                         reserve3[5];

        static constexpr u32 cMagic = util::TCharCode32("grsc");
    };
    static_assert(sizeof(ResBnshShaderContainer) == 0x60);

    struct ResBnsh : public ResNintendoWareFileHeader {
        u64 reserve0[8];

        static constexpr u64 cMagic              = util::TCharCode32("BNSH");
        static constexpr u16 cTargetVersionMajor = 2;
        static constexpr u8  cTargetVersionMinor = 2;
        static constexpr u8  cTargetVersionMicro = 1;
    };
    static_assert(sizeof(ResBnsh) == 0x60);
}

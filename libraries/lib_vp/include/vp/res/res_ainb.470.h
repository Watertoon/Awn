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

namespace vp::res {

    /* Command */
    struct ResAinbCommand {
        u32 name_offset;
        util::Guid guid;
        s16 node_index;
        s16 secondary_node_index;
    };
    static_assert(sizeof(ResAinbCommand) == 0x18);

    /* T param */
    template <typename T>
    constexpr bool TIsAinbParameter() {
        return false;
    }
    template <typename T>
        requires (std::is_same<T, s32>::value) || (std::is_same<T, float>::value) || (std::is_same<T, bool>::value) || (std::is_same<T, vp::util::Float3>::value) || (std::is_pointer<T>::value)
    constexpr bool TIsAinbParameter() {
        return true;
    }

    /* Node parameter entry  */
    struct ResAinbLocalParameterEntry {
        u32 base_index;
        u32 count;
    };

    template <typename T>
        requires(TIsAinbParameter<T>())
    constexpr u32 TGetAinbPropertyEntryIndex() {
        if constexpr (std::is_same<s32, T>::value) {
            return 0;
        }
        if constexpr (std::is_same<bool, T>::value) {
            return 1;
        }
        if constexpr (std::is_same<float, T>::value) {
            return 2;
        }
        if constexpr (std::is_same<const char*, T>::value) {
            return 3;
        }
        if constexpr (std::is_same<vp::util::Float3, T>::value) {
            return 4;
        }
        if constexpr (std::is_pointer<T>::value) {
            return 5;
        }
    }

    struct ResAinbLocalParameterPropertyEntry {
        union {
            ResAinbLocalParameterEntry entry_array[6];
            struct {                
                ResAinbLocalParameterEntry entry_s32;
                ResAinbLocalParameterEntry entry_bool;
                ResAinbLocalParameterEntry entry_float;
                ResAinbLocalParameterEntry entry_string;
                ResAinbLocalParameterEntry entry_vector3f;
                ResAinbLocalParameterEntry entry_pointer;
            };
        };
    };

    template <typename T>
        requires(TIsAinbParameter<T>())
    constexpr u32 TGetAinbInputEntryIndex() {
        if constexpr (std::is_same<s32, T>::value) {
            return 0;
        }
        if constexpr (std::is_same<bool, T>::value) {
            return 2;
        }
        if constexpr (std::is_same<float, T>::value) {
            return 4;
        }
        if constexpr (std::is_same<const char*, T>::value) {
            return 6;
        }
        if constexpr (std::is_same<vp::util::Float3, T>::value) {
            return 8;
        }
        if constexpr (std::is_pointer<T>::value) {
            return 10;
        }
    }

    template <typename T>
        requires(TIsAinbParameter<T>())
    constexpr u32 TGetAinbOutputEntryIndex() {
        if constexpr (std::is_same<s32, T>::value) {
            return 1;
        }
        if constexpr (std::is_same<bool, T>::value) {
            return 3;
        }
        if constexpr (std::is_same<float, T>::value) {
            return 5;
        }
        if constexpr (std::is_same<const char*, T>::value) {
            return 7;
        }
        if constexpr (std::is_same<vp::util::Float3, T>::value) {
            return 9;
        }
        if constexpr (std::is_pointer<T>::value) {
            return 11;
        }
    }

    struct ResAinbLocalParameterInputOutputEntry {
        union {
            ResAinbLocalParameterEntry entry_array[12];
            struct {                
                ResAinbLocalParameterEntry entry_input_s32;
                ResAinbLocalParameterEntry entry_output_s32;
                ResAinbLocalParameterEntry entry_input_bool;
                ResAinbLocalParameterEntry entry_output_bool;
                ResAinbLocalParameterEntry entry_input_float;
                ResAinbLocalParameterEntry entry_output_float;
                ResAinbLocalParameterEntry entry_input_string;
                ResAinbLocalParameterEntry entry_output_string;
                ResAinbLocalParameterEntry entry_input_vector3f;
                ResAinbLocalParameterEntry entry_output_vector3f;
                ResAinbLocalParameterEntry entry_input_pointer;
                ResAinbLocalParameterEntry entry_output_pointer;
            };
        };

        static constexpr u32 cInvalidOutputOffset = 0xffff'ffff;

        constexpr u32 GetBaseOffsetOutputS32() const {
            return 0;
        }
        constexpr u32 GetBaseOffsetOutputBool() const {
            return (entry_output_s32.count) * sizeof(u32);
        }
        constexpr u32 GetBaseOffsetOutputFloat() const {
            return (entry_output_s32.count + entry_output_bool.count) * sizeof(u32);
        }
        constexpr u32 GetBaseOffsetOutputString() const {
            return (entry_output_s32.count + entry_output_bool.count + entry_output_float.count) * sizeof(u32);
        }
        constexpr u32 GetBaseOffsetOutputVector3f() const {
            return ((entry_output_s32.count + entry_output_bool.count + entry_output_float.count) * sizeof(u32)) + (entry_output_string.count * sizeof(size_t));
        }
        constexpr u32 GetBaseOffsetOutputPointer() const {
            return ((entry_output_s32.count + entry_output_bool.count + entry_output_float.count) * sizeof(u32)) + (entry_output_string.count * sizeof(size_t)) + (entry_output_vector3f.count * sizeof(vp::util::Float3));
        }

        template <typename T>
            requires (TIsAinbParameter<T>())
        constexpr u32 GetBaseOffsetOutputAuto() const {
            if constexpr (std::is_same<s32, T>::value) {
                return this->GetBaseOffsetOutputS32();
            }
            if constexpr (std::is_same<bool, T>::value) {
                return this->GetBaseOffsetOutputBool();
            }
            if constexpr (std::is_same<float, T>::value) {
                return this->GetBaseOffsetOutputFloat();
            }
            if constexpr (std::is_same<const char*, T>::value) {
                return this->GetBaseOffsetOutputString();
            }
            if constexpr (std::is_same<vp::util::Float3, T>::value) {
                return this->GetBaseOffsetOutputVector3f();
            }
            if constexpr (std::is_pointer<T>::value) {
                return this->GetBaseOffsetOutputPointer();
            }
        }
    };

    struct ResAinbParameterInlineEntry {
        u8 count;
        u8 base_index;
    };

    struct ResAinbParameterInlineChild {
        u16 node_index;
        u16 reserve0;
        u32 name_offset;
    };

    struct ResAinbNodeParameterInlineEntry {
        ResAinbParameterInlineEntry entry_reserve0;
        ResAinbParameterInlineEntry entry_reserve1;
        ResAinbParameterInlineEntry entry_child;
        ResAinbParameterInlineEntry entry_root_change;
        ResAinbParameterInlineEntry entry_reserve2;
        ResAinbParameterInlineEntry entry_reserve3;
        ResAinbParameterInlineEntry entry_reserve4;
        ResAinbParameterInlineEntry entry_reserve5;
        ResAinbParameterInlineEntry entry_reserve6;
        ResAinbParameterInlineEntry entry_reserve7;
    };

    struct ResAinbNodeParameter {
        ResAinbLocalParameterPropertyEntry property_parameter;
        ResAinbLocalParameterInputOutputEntry input_output_parameter;
        ResAinbNodeParameterInlineEntry inline_parameter;
        u32 inline_offset_array[];
    };

    /* Node */
	enum AinbBuiltInType : u16 {
		UserDefined                    = 0,
		Element_S32Selector            = 1,
		Element_Sequential             = 2,
		Element_Simultaneous           = 3,
		Element_F32Selector            = 4,
		Element_StringSelector         = 5,
		Element_RandomSelector         = 6,
		Element_BoolSelector           = 7,
		Element_Fork                   = 8,
		Element_Join                   = 9,
		Element_Alert                  = 10,
		Element_Expression             = 20,
		Element_ModuleIF_Input_S32     = 100,
		Element_ModuleIF_Input_F32     = 101,
		Element_ModuleIF_Input_Vec3f   = 102,
		Element_ModuleIF_Input_String  = 103,
		Element_ModuleIF_Input_Bool    = 104,
		Element_ModuleIF_Input_Ptr     = 105,
		Element_ModuleIF_Output_S32    = 200,
		Element_ModuleIF_Output_F32    = 201,
		Element_ModuleIF_Output_Vec3f  = 202,
		Element_ModuleIF_Output_String = 203,
		Element_ModuleIF_Output_Bool   = 204,
		Element_ModuleIF_Output_Ptr    = 205,
		Element_ModuleIF_Child         = 300,
		Element_StateEnd               = 400,
		Element_SplitTiming            = 500,
	};

    struct ResAinbNode {
        u16 built_in_node_type;
        u16 node_index;
        u16 attachment_count;
        union {
            u8 node_flags;
            struct {
                u8 is_query_node : 1;
                u8 is_module_caller : 1;
                u8 is_resident_initialized : 1;
                u8 is_query_multi_param : 1;
                u8 reserve0 : 4;
            };
        };
        u8 reserve1;
        u32 name_offset;
        u32 name_murmur32;
        u32 reserve2;
        u32 node_parameter_offset;
        u16 exb_field_count;
        u16 exb_value_size;
        u16 multi_param_count;
        u16 reserve3;
        u32 base_attachment;
        u16 base_query_node;
        u16 query_node_count;
        u16 reserve4;
        u16 reserve5;
        util::Guid guid;
    };
    static_assert(sizeof(ResAinbNode) == 0x3c);

    /* Local blackboard */
    struct ResAinbLocalBlackboardParameterPath {
        u32 path_offset;
        u32 path_murmur3_hash;
        u32 hash1;
        u32 hash2;
    };
    static_assert(sizeof(ResAinbLocalBlackboardParameterPath) == 0x10);

    template <typename T>
        requires(TIsAinbParameter<T>())
    consteval u32 TGetAinbLocalBBTypeIndex() {
        if constexpr (std::is_same<const char*, T>::value) {
            return 0;
        }
        if constexpr (std::is_same<s32, T>::value) {
            return 1;
        }
        if constexpr (std::is_same<float, T>::value) {
            return 2;
        }
        if constexpr (std::is_same<bool, T>::value) {
            return 3;
        }
        if constexpr (std::is_same<vp::util::Float3, T>::value) {
            return 4;
        }
        if constexpr (std::is_pointer<T>::value) {
            return 5;
        }
    }

    struct ResAinbLocalBlackboardParameter {
        union {
            u32 info_bitpack;
            struct {
                u32 name_offset : 22;
                u32 reserve0 : 1;
                u32 reserve1 : 1;
                u32 bgyml_index : 7;
                u32 is_valid_index : 1;
            };
        };
        u32 description_offset;

        static constexpr u32 cNoLinkMask = 0x80c0'0000;

        constexpr bool IsNoLink() const {
            return (info_bitpack & cNoLinkMask) != 0;
        }
    };
    static_assert(sizeof(ResAinbLocalBlackboardParameter) == 0x8);

    struct ResAinbLocalBlackboardParameterEntry {
        u16 count;
        u16 base_index;
        u16 base_offset;
        u16 reserve0;
    };
    static_assert(sizeof(ResAinbLocalBlackboardParameterEntry) == 0x8);

    struct ResAinbLocalBlackboardParameterTable {
        union {
            ResAinbLocalBlackboardParameterEntry entry_array[6];
            struct {                
                ResAinbLocalBlackboardParameterEntry entry_string;
                ResAinbLocalBlackboardParameterEntry entry_s32;
                ResAinbLocalBlackboardParameterEntry entry_float;
                ResAinbLocalBlackboardParameterEntry entry_bool;
                ResAinbLocalBlackboardParameterEntry entry_vector3f;
                ResAinbLocalBlackboardParameterEntry entry_pointer;
            };
        };
        ResAinbLocalBlackboardParameter parameter_array[];

        const vp::res::ResAinbLocalBlackboardParameter *GetParameter(u32 entry_index, u32 param_index) const {
            const vp::res::ResAinbLocalBlackboardParameterEntry *param_entry = std::addressof(this->entry_array[entry_index]);
            const u32 index = param_entry->base_index + param_index;
            const vp::res::ResAinbLocalBlackboardParameter *param = std::addressof(this->parameter_array[index]);
            return param;
        }

        constexpr u32 GetDefaultArrayOffset() const {
            return sizeof(ResAinbLocalBlackboardParameterTable) + (this->entry_pointer.base_offset + this->entry_pointer.count) * sizeof(ResAinbLocalBlackboardParameter);
        }

        u32 *GetDefaultStringOffsetArray() const {
            u32 *offset_array = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(this) + this->entry_string.base_offset + this->GetDefaultArrayOffset());
            return offset_array;
        }
        s32 *GetDefaultS32Array() const {
            s32 *default_array = reinterpret_cast<s32*>(reinterpret_cast<uintptr_t>(this) + this->entry_s32.base_offset + this->GetDefaultArrayOffset());
            return default_array;
        }
        float *GetDefaultFloatArray() const {
            float *default_array = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + this->entry_float.base_offset + this->GetDefaultArrayOffset());
            return default_array;
        }
        bool *GetDefaultBoolArray() const {
            bool *default_array = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + this->entry_bool.base_offset + this->GetDefaultArrayOffset());
            return default_array;
        }
        vp::util::Float3 *GetDefaultVector3fArray() const {
            vp::util::Float3 *default_array = reinterpret_cast<vp::util::Float3*>(reinterpret_cast<uintptr_t>(this) + this->entry_vector3f.base_offset + this->GetDefaultArrayOffset());
            return default_array;
        }

        constexpr u32 GetPathArrayOffset() const {
            return this->GetDefaultArrayOffset() + this->entry_vector3f.base_offset + (this->entry_vector3f.count * sizeof(vp::util::Float3));
        }

        ResAinbLocalBlackboardParameterPath *GetParameterPathArray() const {
            return reinterpret_cast<ResAinbLocalBlackboardParameterPath*>(reinterpret_cast<uintptr_t>(this) + this->GetPathArrayOffset());
        }

        ResAinbLocalBlackboardParameterPath *GetParameterPath(const vp::res::ResAinbLocalBlackboardParameter *parameter) const {
            if (!parameter->is_valid_index) { return nullptr; }
            return reinterpret_cast<ResAinbLocalBlackboardParameterPath*>(reinterpret_cast<uintptr_t>(this) + this->GetPathArrayOffset() + parameter->bgyml_index * sizeof(ResAinbLocalBlackboardParameterPath));
        }
    };
    static_assert(sizeof(ResAinbLocalBlackboardParameterTable) == 0x30);

    struct ResAinbEnumRelocation {
        u32 enum_value_result_offset;
        u32 enum_class_name_offset;
        u32 enum_value_name_offset;
    };
    static_assert(sizeof(ResAinbEnumRelocation) == 0xc);

    struct ResAinbEnumRelocationTable {
        u32 relocation_count;
        ResAinbEnumRelocation relocation_array[];
    };
    static_assert(sizeof(ResAinbEnumRelocationTable) == 0x4);

    template <typename T>
        requires(TIsAinbParameter<T>())
    consteval u32 TGetAinbPropertyOffsetIndex() {
        if constexpr (std::is_same<s32, T>::value) {
            return 0;
        }
        if constexpr (std::is_same<bool, T>::value) {
            return 1;
        }
        if constexpr (std::is_same<float, T>::value) {
            return 2;
        }
        if constexpr (std::is_same<const char*, T>::value) {
            return 3;
        }
        if constexpr (std::is_same<vp::util::Float3, T>::value) {
            return 4;
        }
        if constexpr (std::is_pointer<T>::value) {
            return 5;
        }
    }

    struct ResAinbParameterPropertyTable {
        union {
            u32 offset_array[6];
            struct {                
                u32 offset_s32;
                u32 offset_bool;
                u32 offset_float;
                u32 offset_string;
                u32 offset_vector3f;
                u32 offset_pointer;
            };
        };
    };
    static_assert(sizeof(ResAinbParameterPropertyTable) == 0x18);

    struct ResAinbActiveUpdate {
        u32 is_valid            : 8;
        u32 reserve0            : 23;
        u32 is_update_post_calc : 1;
    };

    template <typename T>
        requires(TIsAinbParameter<T>())
    consteval u32 TGetAinbInputOffsetIndex() {
        if constexpr (std::is_same<s32, T>::value) {
            return 0;
        }
        if constexpr (std::is_same<bool, T>::value) {
            return 2;
        }
        if constexpr (std::is_same<float, T>::value) {
            return 4;
        }
        if constexpr (std::is_same<const char*, T>::value) {
            return 6;
        }
        if constexpr (std::is_same<vp::util::Float3, T>::value) {
            return 8;
        }
        if constexpr (std::is_pointer<T>::value) {
            return 10;
        }
    }

    template <typename T>
        requires(TIsAinbParameter<T>())
    consteval u32 TGetAinbOutputOffsetIndex() {
        if constexpr (std::is_same<s32, T>::value) {
            return 1;
        }
        if constexpr (std::is_same<bool, T>::value) {
            return 3;
        }
        if constexpr (std::is_same<float, T>::value) {
            return 5;
        }
        if constexpr (std::is_same<const char*, T>::value) {
            return 7;
        }
        if constexpr (std::is_same<vp::util::Float3, T>::value) {
            return 9;
        }
        if constexpr (std::is_pointer<T>::value) {
            return 11;
        }
    }

    struct ResAinbParameterInputOutputTable {
        union {
            u32 offset_array[12];
            struct {                
                u32 offset_input_s32;
                u32 offset_output_s32;
                u32 offset_input_bool;
                u32 offset_output_bool;
                u32 offset_input_float;
                u32 offset_output_float;
                u32 offset_input_string;
                u32 offset_output_string;
                u32 offset_input_vector3f;
                u32 offset_output_vector3f;
                u32 offset_input_pointer;
                u32 offset_output_pointer;
            };
        };
    };
    static_assert(sizeof(ResAinbParameterInputOutputTable) == 0x30);

    struct ResAinbMultiParam {
        u32 reserve0;
        u32 reserve1;
    };
    static_assert(sizeof(ResAinbMultiParam) == 0x8);

    struct ResAinbAttachmentParameter {
        u8 is_update_multi;
        u8 reserve0;
        u16 reserve1;
        ResAinbLocalParameterPropertyEntry property_parameter;
    };
    static_assert(sizeof(ResAinbAttachmentParameter) == 0x34);

    struct ResAinbAttachment {
        u32 name_offset;
        u32 attachment_parameter_offset;
        u16 exb_field_count;
        u16 exb_value_total_size;
        u32 name_murmur3;
    };
    static_assert(sizeof(ResAinbAttachment) == 0x10);

    enum class AinbReplacementType : u8 {
        RemoveChild = 0,
        ReplaceChild = 1,
        RemoveAttachment = 2,
    };

    struct ResAinbChildReplacement {
        u8  replacement_type;
        u8  reserve0;
        u16 replacement_node_index;
        u16 base_index;
        u16 new_node_index;
    };

    struct ResAinbChildReplacementTable {
        u8 runtime_init_guard;
        u8 reserve0;
        u16 child_replacement_count;
        u16 override_node_count;
        u16 override_attachment_count;
        ResAinbChildReplacement child_replacement_array[];
    };

    struct ResAinbQueryNode {
        s16 query_node_index;
        u16 reserve0;
    };

    struct ResAinbEmbedAinb {
        u32 file_path_offset;
        u32 category_offset;
        u32 ainb_count;
    };
    static_assert(sizeof(ResAinbEmbedAinb) == 0xc);

    struct ResAinbEmbedAinbArray {
        u32              embed_ainb_count;
        ResAinbEmbedAinb embed_ainb_array[];
    };
    static_assert(sizeof(ResAinbEmbedAinbArray) == 0x4);

    struct ResAinbCustomAction {
        s16 node_index;
        u16 reserve0;
        u32 string_offset0;
        u32 string_offset1;
    };
    static_assert(sizeof(ResAinbCustomAction) == 0xc);

    struct ResAinbCustomActionTable {
        u32 custom_action_count;
        ResAinbCustomAction custom_action_array[];
    };
    static_assert(sizeof(ResAinbCustomActionTable) == 0x4);

    struct ResAinb {
        u32 magic;
        u32 version;
        u32 file_name_offset;
        u32 command_count;
        u32 node_count;
        u32 query_node_count;
        u32 attachment_count;
        u32 output_node_count;
        u32 local_blackboard_parameter_table_offset;
        u32 string_pool_offset;
        u32 enum_relocation_table_offset;
        u32 property_parameter_table_offset;
        u32 root_change_offset_array_offset;
        u32 input_output_parameter_table_offset;
        u32 multi_param_array_offset;
        u32 attachment_array_offset;
        u32 attachment_index_array_offset;
        u32 exb_offset;
        u32 child_replacement_table_offset;
        u32 query_node_array_offset;
        u32 reserve0;
        u32 reserve1;
        u32 reserve2;
        u32 embed_ainb_array_offset;
        u32 file_category_offset;
        u32 file_category;
        u32 custom_action_table_offset;
        u32 reserve4;
        u32 module_caller_link_offset;
        ResAinbCommand command_array[];

        static constexpr u32 cMagic = vp::util::TCharCode32("AIB ");
        static constexpr u32 cTargetVersion = 0x407;

        constexpr bool IsValid() const {
            return (this->magic == cMagic) && (this->version == cTargetVersion);
        }

        ResAinbCommand *GetCommandArray() {
            return this->command_array;
        }
        ResAinbNode *GetNodeArray() {
            return reinterpret_cast<ResAinbNode*>(reinterpret_cast<uintptr_t>(this) + sizeof(ResAinb) + sizeof(ResAinbCommand) * command_count);
        }
        ResAinbLocalBlackboardParameterTable *GetLocalBlackboardParameterTable() {
            return reinterpret_cast<ResAinbLocalBlackboardParameterTable*>(reinterpret_cast<uintptr_t>(this) + this->local_blackboard_parameter_table_offset);
        }
        const char *GetStringPool() {
            return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(this) + this->string_pool_offset);
        }
        ResAinbEnumRelocationTable *GetEnumRelocationTable() {
            return reinterpret_cast<ResAinbEnumRelocationTable*>(reinterpret_cast<uintptr_t>(this) + this->enum_relocation_table_offset);
        }
        ResAinbParameterPropertyTable *GetParameterPropertyTable() {
            return reinterpret_cast<ResAinbParameterPropertyTable*>(reinterpret_cast<uintptr_t>(this) + this->property_parameter_table_offset);
        }
        u32 *GetRootChangeOffsetArray() {
            return reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(this) + this->root_change_offset_array_offset);
        }
        ResAinbParameterInputOutputTable *GetParameterInputOutputTable() {
            return reinterpret_cast<ResAinbParameterInputOutputTable*>(reinterpret_cast<uintptr_t>(this) + this->input_output_parameter_table_offset);
        }
        ResAinbMultiParam *GetMultiParamArray() {
            return reinterpret_cast<ResAinbMultiParam*>(reinterpret_cast<uintptr_t>(this) + this->multi_param_array_offset);
        }
        ResAinbAttachment *GetAttachmentArray() {
            return reinterpret_cast<ResAinbAttachment*>(reinterpret_cast<uintptr_t>(this) + this->attachment_array_offset);
        }
        u32 *GetAttachmentIndexArray() {
            return reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(this) + this->attachment_index_array_offset);
        }
        ResExb *GetExb() {
            return (exb_offset != 0) ? reinterpret_cast<ResExb*>(reinterpret_cast<uintptr_t>(this) + this->exb_offset) : nullptr;
        }
        ResAinbChildReplacementTable *GetChildReplacementTable() {
            return (this->child_replacement_table_offset != 0) ? reinterpret_cast<ResAinbChildReplacementTable*>(reinterpret_cast<uintptr_t>(this) + this->child_replacement_table_offset) : nullptr;
        }
        ResAinbQueryNode *GetQueryNodeArray() {
            return reinterpret_cast<ResAinbQueryNode*>(reinterpret_cast<uintptr_t>(this) + this->query_node_array_offset);
        }
        ResAinbEmbedAinb *GetEmbedAinbArray() {
            return reinterpret_cast<ResAinbEmbedAinb*>(reinterpret_cast<uintptr_t>(this) + this->embed_ainb_array_offset);
        }
        ResAinbCustomActionTable *GetCustomActionTable() {
            return (this->custom_action_table_offset != 0) ? reinterpret_cast<ResAinbCustomActionTable*>(reinterpret_cast<uintptr_t>(this) + this->custom_action_table_offset) : nullptr;
        }
        const char *GetName() const {
            return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(this) + this->file_name_offset + this->string_pool_offset);
        }
        const char *GetFileCategoryName() const {
            return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(this) + this->file_category_offset + this->string_pool_offset);
        }

        template <typename T>
        T *GetStruct(u32 offset) {
            return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset);
        }
        const char *GetString(u32 string_offset) {
            return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(this) + string_offset + this->string_pool_offset);
        }
        const char *GetStringAbsolute(u32 string_absolute_offset) {
            return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(this) + string_absolute_offset);
        }
    };
    static_assert(sizeof(ResAinb) == 0x74);  

    enum class AinbFloatBBLocation : u32 {
        Float = 0,
        Vector3fX = 1,
        Vector3fY = 2,
        Vector3fZ = 3,
    };

    struct ResAinbParamFlags {
        union {
            u32 flags;
            struct {
                u32 bb_or_exb_index : 16;
                u32 reserve0 : 7;
                u32 is_set_output_dirty : 1;
                u32 is_ptr_flag_0 : 1;
                u32 is_expression_bits_0 : 1;
                u32 float_bb_location : 2;
                u32 reserve2 : 2;
                u32 is_expression_bits_1 : 2;
            };
        };

        static constexpr u32 cExpressionMagic = 0xc200'0000;
        static constexpr u32 cPointerFlagMask = 0xfe7f'ffff;

        constexpr u32 GetIndex() const {
            return this->bb_or_exb_index;
        }
        constexpr u32 GetFlagsNoPointerFlags() const {
            return this->flags & cPointerFlagMask;
        }
        constexpr AinbFloatBBLocation GetFloatBBLocation() const {
            return static_cast<AinbFloatBBLocation>(this->float_bb_location);
        }
        constexpr bool IsExpression() const {
            return (this->flags & cExpressionMagic) == cExpressionMagic;
        }
        constexpr bool IsSetOutputDirty() const {
            return this->is_set_output_dirty;
        }
        constexpr bool IsSetPointerFlag0() const {
            return this->is_ptr_flag_0;
        }
        constexpr bool IsPointerFlags() const {
            return this->is_set_output_dirty | this->is_ptr_flag_0;
        }
    };

    /* Property parameter */
    struct ResAinbParameterPropertyS32 {
        u32 name_offset;
        ResAinbParamFlags param_flags;
        s32 value;

        constexpr s32 *GetProperty(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return std::addressof(this->value);
        }
    };

    struct ResAinbParameterPropertyFloat {
        u32 name_offset;
        ResAinbParamFlags param_flags;
        float value;

        constexpr float *GetProperty(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return std::addressof(this->value);
        }
    };

    struct ResAinbParameterPropertyBool {
        u32 name_offset;
        ResAinbParamFlags param_flags;
        union {
            bool value;
            u32 storage;
        };

        constexpr bool *GetProperty(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return std::addressof(this->value);
        }
    };

    struct ResAinbParameterPropertyString {
        u32 name_offset;
        ResAinbParamFlags param_flags;
        u32 string_offset;

        const char *GetProperty(ResAinb *ainb) {
            const char *string = ainb->GetString(this->string_offset);
            return string;
        }
    };

    struct ResAinbParameterPropertyVector3f {
        u32 name_offset;
        ResAinbParamFlags param_flags;
        vp::util::Float3 value;

        constexpr vp::util::Float3 *GetProperty(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return std::addressof(this->value);
        }
    };

    struct ResAinbParameterPropertyPointer {
        u32 name_offset;
        u32 class_name_offset;
        ResAinbParamFlags param_flags;

        constexpr void *GetProperty(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return nullptr;
        }
    };

    template <typename T>
        requires (TIsAinbParameter<T>())
    struct TResAinbParameterPropertyAuto {
        using Type = ResAinbParameterPropertyPointer;
    };
    template<>
    struct TResAinbParameterPropertyAuto<s32> {
        using Type = ResAinbParameterPropertyS32;
    };
    template<>
    struct TResAinbParameterPropertyAuto<float> {
        using Type = ResAinbParameterPropertyFloat;
    };
    template<>
    struct TResAinbParameterPropertyAuto<bool> {
        using Type = ResAinbParameterPropertyBool;
    };
    template<>
    struct TResAinbParameterPropertyAuto<const char*> {
        using Type = ResAinbParameterPropertyString;
    };
    template<>
    struct TResAinbParameterPropertyAuto<vp::util::Float3> {
        using Type = ResAinbParameterPropertyVector3f;
    };

    /* Input parameter */
    static constexpr s16 cMultiBase = -100;

    struct ResAinbParameterInputS32 {
        u32 offset_name;
        s16 input_index_or_multi_base_index;
        s16 output_index_or_multi_count;
        ResAinbParamFlags param_flags;
        s32 value;

        constexpr s32 *GetImmediate(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return std::addressof(this->value);
        }
        constexpr bool IsImmediate() const {
            return static_cast<u16>(cMultiBase) < static_cast<u16>(this->input_index_or_multi_base_index);
        }
        constexpr s16 GetMultiBaseIndex() const {
            return this->input_index_or_multi_base_index - cMultiBase;
        }
        constexpr bool IsMultiParam() const {
            return this->input_index_or_multi_base_index <= cMultiBase;
        }
    };

    struct ResAinbParameterInputFloat {
        u32 name_offset;
        s16 input_index_or_multi_base_index;
        s16 output_index_or_multi_count;
        ResAinbParamFlags param_flags;
        float value;

        constexpr float *GetImmediate(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return std::addressof(this->value);
        }
        constexpr bool IsImmediate() const {
            return static_cast<u16>(cMultiBase) < static_cast<u16>(this->input_index_or_multi_base_index);
        }
        constexpr s16 GetMultiBaseIndex() const {
            return this->input_index_or_multi_base_index - cMultiBase;
        }
        constexpr bool IsMultiParam() const {
            return this->input_index_or_multi_base_index <= cMultiBase;
        }
    };

    struct ResAinbParameterInputBool {
        u32 name_offset;
        s16 input_index_or_multi_base_index;
        s16 output_index_or_multi_count;
        ResAinbParamFlags param_flags;
        union {
            bool value;
            u32 storage;
        };

        constexpr bool *GetImmediate(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return std::addressof(this->value);
        }
        constexpr bool IsImmediate() const {
            return static_cast<u16>(cMultiBase) < static_cast<u16>(this->input_index_or_multi_base_index);
        }
        constexpr s16 GetMultiBaseIndex() const {
            return this->input_index_or_multi_base_index - cMultiBase;
        }
        constexpr bool IsMultiParam() const {
            return this->input_index_or_multi_base_index <= cMultiBase;
        }
    };

    struct ResAinbParameterInputString {
        u32 name_offset;
        s16 input_index_or_multi_base_index;
        s16 output_index_or_multi_count;
        ResAinbParamFlags param_flags;
        u32 string_offset;

        const char *GetImmediate(ResAinb *ainb) {
            const char *string = ainb->GetString(this->string_offset);
            return string;
        }
        constexpr bool IsImmediate() const {
            return static_cast<u16>(cMultiBase) < static_cast<u16>(this->input_index_or_multi_base_index);
        }
        constexpr s16 GetMultiBaseIndex() const {
            return this->input_index_or_multi_base_index - cMultiBase;
        }
        constexpr bool IsMultiParam() const {
            return this->input_index_or_multi_base_index <= cMultiBase;
        }
    };

    struct ResAinbParameterInputVector3f {
        u32 name_offset;
        s16 input_index_or_multi_base_index;
        s16 output_index_or_multi_count;
        ResAinbParamFlags param_flags;
        vp::util::Float3 value;

        constexpr vp::util::Float3 *GetImmediate(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return std::addressof(this->value);
        }
        constexpr bool IsImmediate() const {
            return static_cast<u16>(cMultiBase) < static_cast<u16>(this->input_index_or_multi_base_index);
        }
        constexpr s16 GetMultiBaseIndex() const {
            return this->input_index_or_multi_base_index - cMultiBase;
        }
        constexpr bool IsMultiParam() const {
            return this->input_index_or_multi_base_index <= cMultiBase;
        }
    };

    struct ResAinbParameterInputPointer {
        u32 name_offset;
        u32 class_name_offset;
        s16 input_index_or_multi_base_index;
        s16 output_index_or_multi_count;
        ResAinbParamFlags param_flags;

        constexpr void *GetImmediate(ResAinb *ainb) {
            VP_UNUSED(ainb);
            return nullptr;
        }
        constexpr bool IsImmediate() const {
            return static_cast<u16>(cMultiBase) < static_cast<u16>(this->input_index_or_multi_base_index);
        }
        constexpr s16 GetMultiBaseIndex() const {
            return this->input_index_or_multi_base_index - cMultiBase;
        }
        constexpr bool IsMultiParam() const {
            return this->input_index_or_multi_base_index <= cMultiBase;
        }
    };

    template <typename T>
        requires (TIsAinbParameter<T>())
    struct TResAinbParameterInputAuto {
        using Type = ResAinbParameterInputPointer;
    };
    template<>
    struct TResAinbParameterInputAuto<s32> {
        using Type = ResAinbParameterInputS32;
    };
    template<>
    struct TResAinbParameterInputAuto<float> {
        using Type = ResAinbParameterInputFloat;
    };
    template<>
    struct TResAinbParameterInputAuto<bool> {
        using Type = ResAinbParameterInputBool;
    };
    template<>
    struct TResAinbParameterInputAuto<const char*> {
        using Type = ResAinbParameterInputString;
    };
    template<>
    struct TResAinbParameterInputAuto<vp::util::Float3> {
        using Type = ResAinbParameterInputVector3f;
    };

    /* Output parameter */
    struct ResAinbParameterOutput {
        union {
            u32 output_flags;
            struct {
                u32 name_offset : 30;
                u32 is_ptr_flag_0 : 1;
            };
        };

        constexpr bool IsSetPointerFlag0() const {
            return this->is_ptr_flag_0;
        }
    };

    struct ResAinbParameterOutputPointer {
        union {
            u32 output_flags;
            struct {
                u32 name_offset : 30;
                u32 is_ptr_flag_0 : 1;
            };
        };
        u32 class_name_offset;

        constexpr bool IsSetPointerFlag0() const {
            return this->is_ptr_flag_0;
        }
    };

    template <typename T>
        requires(TIsAinbParameter<T>())
    struct TResAinbParameterOutputAuto {
        using Type = ResAinbParameterOutputPointer;
    };
    template<>
    struct TResAinbParameterOutputAuto<s32> {
        using Type = ResAinbParameterOutput;
    };
    template<>
    struct TResAinbParameterOutputAuto<float> {
        using Type = ResAinbParameterOutput;
    };
    template<>
    struct TResAinbParameterOutputAuto<bool> {
        using Type = ResAinbParameterOutput;
    };
    template<>
    struct TResAinbParameterOutputAuto<const char*> {
        using Type = ResAinbParameterOutput;
    };
    template<>
    struct TResAinbParameterOutputAuto<vp::util::Float3> {
        using Type = ResAinbParameterOutput;
    };
}

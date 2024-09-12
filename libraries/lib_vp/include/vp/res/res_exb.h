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

	enum class ExbCommand : u8 {
        Terminator          = 0x1, 
        Assignment          = 0x2,
        Negate              = 0x3,
        NegateBool          = 0x4,
        Add                 = 0x5,
        Subtract            = 0x6,
        Multiply            = 0x7,
        Divide              = 0x8,
        Modulus             = 0x9,
        Increment           = 0xa,
        Decrement           = 0xb,
        ScalarMultiplyVec3f = 0xc,
        ScalarDivideVec3f   = 0xd,
        LeftShift           = 0xe,
        RightShift          = 0xf,
        LessThan            = 0x10,
        LessThanEqual       = 0x11,
        GreaterThan         = 0x12,
        GreaterThanEqual    = 0x13,
        Equal               = 0x14,
        NotEqual            = 0x15,
        And                 = 0x16,
        Xor                 = 0x17,
        Or                  = 0x18,
        LogicalAnd          = 0x19,
        LogicalOr           = 0x1a,
        UserFunction        = 0x1b,
        JumpIfLhsZero       = 0x1c,
        Jump                = 0x1d,
    };

    enum class ExbDataType : u8 {
        Bool     = 0x2,
        S32      = 0x3,
        Float    = 0x4,
        String   = 0x5,
        Vector3f = 0x6,
    };

    template <typename T>
        requires (std::is_same_v<T, bool> || std::is_same_v<T, s32> || std::is_same_v<T, float> || std::is_same_v<T, const char*> || std::is_same_v<T, vp::util::Float3>)
    ExbDataType TGetExbDataTypeAuto() {
        
        if constexpr (std::is_same_v<T, bool>) {
            return ExbDataType::Bool;
        } else if constexpr (std::is_same_v<T, s32>) {
            return ExbDataType::S32;
        } else if constexpr (std::is_same_v<T, float>) {
            return ExbDataType::Float;
        } else if constexpr (std::is_same_v<T, const char*>) {
            return ExbDataType::String;
        } else if constexpr (std::is_same_v<T, vp::util::Float3>) {
            return ExbDataType::Vector3f;
        }
    }

    enum class ExbParamSource : u8 {
        Immediate               = 0x0,
        ImmediateString         = 0x1,
        StaticMemory            = 0x2,
        ParameterTable          = 0x3,
        ParameterTableString    = 0x4,
        OutputMemory            = 0x5,
        InputMemory             = 0x6,
        ScratchMemoryU32Aligned = 0x7,
        ScratchMemoryU64Aligned = 0x8,
        UserMemoryOutput        = 0x9,
        UserMemoryInput         = 0xa,
    };

    struct ResExbCommandInfo {
        u32 base_index_pre_command;
        u32 count_pre_command;
        u32 base_index_main_command;
        u32 count_main_command;
        u32 static_memory_usage;
        u16 scratch_memory_u32_usage;
        u16 scratch_memory_u64_usage;
        u16 output_source;
        u16 input_source;
    };
    static_assert(sizeof(ResExbCommandInfo) == 0x1c);
    
    struct ResExbCommandInfoTable {
        u32               info_count;
        ResExbCommandInfo info_array[];
    };
    static_assert(sizeof(ResExbCommandInfoTable) == 0x4);

    struct ResExbCommand {
        u8  command_type;
        u8  data_type;
        union {
            struct {
                u8  lhs_source;
                u8  rhs_source;
            };
            u16 static_memory_offset;
        };
        union {
            struct {
                u16 arg0;
                u16 arg1;
            };
            u32 function_local_index;
        };
    };
    static_assert(sizeof(ResExbCommand) == 0x8);

    struct ResExbCommandTable {
        u32               command_count;
        ResExbCommandInfo command_array[];
    };
    static_assert(sizeof(ResExbCommandTable) == 0x4);

    struct ResExbSignatureTable {
        u32 signature_count;
        u32 signature_offset_array[];
    };
    static_assert(sizeof(ResExbSignatureTable) == 0x4);

    struct ResExb {
        u32 magic;
        u32 version;
        u32 static_memory_size;
        u32 reserve1;
        u32 scratch_u32_allocator_size;
        u32 scratch_u64_allocator_size;
        u32 command_info_offset;
        u32 command_table_offset;
        u32 signature_table_offset;
        u32 parameter_region_offset;
        u32 string_pool_offset;

        static constexpr u32 cMagic         = util::TCharCode32("EXB ");
        static constexpr u32 cTargetVersion = 2;

        constexpr bool IsValid() const {
            return (magic == cMagic) & (version == cTargetVersion);
        }

        ALWAYS_INLINE ResExbCommandInfoTable *GetCommandInfoTable() const {
            return reinterpret_cast<ResExbCommandInfoTable*>(reinterpret_cast<uintptr_t>(this) + command_info_offset);
        }
        ALWAYS_INLINE ResExbCommandTable *GetCommandTable() const {
            return reinterpret_cast<ResExbCommandTable*>(reinterpret_cast<uintptr_t>(this) + command_table_offset);
        }
        ALWAYS_INLINE ResExbSignatureTable *GetSignatureTable() const {
            return reinterpret_cast<ResExbSignatureTable*>(reinterpret_cast<uintptr_t>(this) + signature_table_offset);
        }
        ALWAYS_INLINE void *GetParameterRegion() const {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + parameter_region_offset);
        }
        ALWAYS_INLINE const char *GetString(u32 string_offset) const {
            return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(this) + string_pool_offset + string_offset);
        }
    };
    static_assert(sizeof(ResExb) == 0x2c);
}

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

	enum class ExbInstructionType : u8 {
        Invalid             = 0x0, 
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
        LogicalShiftLeft    = 0xe,
        LogicalShiftRight   = 0xf,
        LessThan            = 0x10,
        LessThanEqual       = 0x11,
        GreaterThan         = 0x12,
        GreaterThanEqual    = 0x13,
        EqualTo             = 0x14,
        NotEqualTo          = 0x15,
        And                 = 0x16,
        Xor                 = 0x17,
        Or                  = 0x18,
        LogicalAnd          = 0x19,
        LogicalOr           = 0x1a,
        UserFunction        = 0x1b,
        JumpZero            = 0x1c,
        Jump                = 0x1d,
    };

    enum class ExbDataType_Old : u8 {
        Invalid  = 0x0,
        Void     = 0x1,
        Bool     = 0x2,
        S32      = 0x3,
        Float    = 0x4,
        String   = 0x5,
        Vector3f = 0x6,
    };
    enum class ExbDataType : u8 {
        Invalid  = 0x0,
        Void     = 0x1,
        Bool     = 0x2,
        S32      = 0x3,
        U32      = 0x4,
        Float    = 0x5,
        String   = 0x6,
        Vector3f = 0x7,
    };

    template <typename T>
    concept IsExbDataType = (std::is_same_v<T, bool> || std::is_same_v<T, s32> || std::is_same_v<T, float> || std::is_same_v<T, const char*> || std::is_same_v<T, vp::util::Float3> || std::is_same_v<T, u32>);

    constexpr ExbDataType FixupExbDataType(u8 data_type, u32 version) {
        if (version == 1 || version == 2) {            
            return static_cast<ExbDataType>((3 < data_type) ? data_type + 1 : data_type); /* Fixup from ExbDataType_Old */
        }
        return static_cast<ExbDataType>(data_type);
    }

    template <typename T>
        requires (IsExbDataType<T>)
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
        } else if constexpr (std::is_same_v<T, u32>) {
            return ExbDataType::U32;
        }
    }

    enum class ExbSourceType : u8 {
        Immediate       = 0x0,
        ImmediateString = 0x1,
        GlobalMemory    = 0x2,
        Constant        = 0x3,
        ConstantString  = 0x4,
        Output          = 0x5,
        Input           = 0x6,
        LocalMemoryU32  = 0x7,
        LocalMemoryU64  = 0x8,
        ExternGlobal    = 0x9,
        ExternConst     = 0xa,
    };

    struct ResExbCommandInfo {
        u32 base_instruction_setup;
        u32 instruction_count_setup;
        u32 base_instruction_update;
        u32 instruction_count_update;
        u32 global_memory_size;
        u16 local_memory_size_u32;
        u16 local_memory_size_u64;
        u16 output_source;
        u16 input_source;

        static constexpr u32 cInvalidCommand = 0xffff'ffff;
    };
    static_assert(sizeof(ResExbCommandInfo) == 0x1c);

    struct ResExbCommandInfoTable {
        u32               command_count;
        ResExbCommandInfo command_array[];
    };
    static_assert(sizeof(ResExbCommandInfoTable) == 0x4);

    struct ResExbInstruction {
        u8  instruction_type;
        u8  data_type;
        union {
            struct {
                u8  lhs_source;
                u8  rhs_source;
            };
            u16 global_memory_offset;
        };
        union {
            struct {
                u16 arg0;
                u16 arg1;
            };
            u32 function_id;
        };
    };
    static_assert(sizeof(ResExbInstruction) == 0x8);

    struct ResExbInstructionTable {
        u32               instruction_count;
        ResExbInstruction instruction_array[];
    };
    static_assert(sizeof(ResExbInstructionTable) == 0x4);

    struct ResExbSignatureTable {
        u32 signature_count;
        u32 signature_offset_array[];
    };
    static_assert(sizeof(ResExbSignatureTable) == 0x4);

    struct ResExb {
        u32 magic;
        u32 version;
        u32 global_memory_size;
        u32 reserve0;
        u32 local_memory_size_u32;
        u32 local_memory_size_u64;
        u32 command_info_table_offset;
        u32 instruction_table_offset;
        u32 signature_table_offset;
        u32 constant_memory_offset;
        u32 string_pool_offset;

        static constexpr u32 cMagic = util::TCharCode32("EXB ");
        static constexpr u32 cTargetVersion = 2;

        constexpr bool IsValid() const {
            return (magic == cMagic) & (version == cTargetVersion);
        }

        ResExbCommandInfoTable *GetCommandInfoTable() const {
            return reinterpret_cast<ResExbCommandInfoTable*>(reinterpret_cast<uintptr_t>(this) + this->command_info_table_offset);
        }
        ResExbInstructionTable *GetInstructionTable() const {
            return reinterpret_cast<ResExbInstructionTable*>(reinterpret_cast<uintptr_t>(this) + this->instruction_table_offset);
        }
        ResExbSignatureTable *GetSignatureTable() const {
            return reinterpret_cast<ResExbSignatureTable*>(reinterpret_cast<uintptr_t>(this) + this->signature_table_offset);
        }
        void *GetConstantMemory() const {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + this->constant_memory_offset);
        }
        char *GetStringPool() {
            return reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(this) + this->string_pool_offset);
        }
        const char *GetString(u32 string_offset) const {
            return reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(this) + this->string_pool_offset + string_offset);
        }
    };
    static_assert(sizeof(ResExb) == 0x2c);
}

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
#include <vp.hpp>

namespace vp::res {

    void *ExbCommandProcessor::GetInputValuePointerVoid(ExbDataType data_type, u8 source_type, u16 in_arg, u64 *immediate_storage) {

        const ExbSourceType source = static_cast<ExbSourceType>(source_type);
        void *memory = nullptr;
        switch (source) {
            case ExbSourceType::ImmediateString:
                *reinterpret_cast<char**>(immediate_storage) = m_string_pool + m_instruction_iter->arg1;
                return immediate_storage;
            case ExbSourceType::Immediate:
                if (data_type == ExbDataType::Float) {
                    *reinterpret_cast<float*>(immediate_storage) = static_cast<float>(m_instruction_iter->arg1);
                } else if (data_type == ExbDataType::S32 || data_type == ExbDataType::Bool) {
                    *reinterpret_cast<u32*>(immediate_storage) = static_cast<u32>(m_instruction_iter->arg1);
                }
                return immediate_storage;
            case ExbSourceType::GlobalMemory:
                memory = m_global_memory;
                break;
            case ExbSourceType::Constant:
                memory = m_constant_memory;
                break;
            case ExbSourceType::ConstantString:
                return m_string_pool + *reinterpret_cast<s32*>(reinterpret_cast<uintptr_t>(m_constant_memory) + in_arg);
                break;
            case ExbSourceType::Output:
                memory = m_output_memory;
                break;
            case ExbSourceType::Input:
                memory = m_input_memory;
                break;
            case ExbSourceType::LocalMemoryU32:
                memory = m_local_memory_u32;
                break;
            case ExbSourceType::LocalMemoryU64:
                memory = m_local_memory_u64;
                break;
            case ExbSourceType::ExternGlobal:
                if (data_type == ExbDataType::S32) {
                    return m_user_memory_callback->GetUserMemoryGlobalS32(in_arg);
                } else if (data_type == ExbDataType::Float) {
                    return m_user_memory_callback->GetUserMemoryGlobalFloat(in_arg);
                } else if (data_type == ExbDataType::Bool) {
                    return m_user_memory_callback->GetUserMemoryGlobalBool(in_arg);
                } else if (data_type == ExbDataType::Vector3f) {
                    return m_user_memory_callback->GetUserMemoryGlobalVector3f(in_arg);
                } else if (data_type == ExbDataType::String) {
                    return m_user_memory_callback->GetUserMemoryGlobalString(in_arg);
                }

                return nullptr;
            case ExbSourceType::ExternConst:
                if (data_type == ExbDataType::S32) {
                    return m_user_memory_callback->GetUserMemoryConstS32(in_arg);
                } else if (data_type == ExbDataType::Float) {
                    if (-1 < static_cast<s16>(in_arg)) { return m_user_memory_callback->GetUserMemoryConstFloat(in_arg); }
                    void *float_region = m_user_memory_callback->GetUserMemoryConstVector3f(in_arg & 0xff);
                    return reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(float_region) + ((in_arg & 0x7f00) >> 8));
                } else if (data_type == ExbDataType::Bool) {
                    return m_user_memory_callback->GetUserMemoryConstBool(in_arg);
                } else if (data_type == ExbDataType::Vector3f) {
                    return m_user_memory_callback->GetUserMemoryConstVector3f(in_arg);
                } else if (data_type == ExbDataType::String) {
                    return m_user_memory_callback->GetUserMemoryConstString(in_arg);
                }
                return nullptr;
            default:
                return nullptr;
        }

        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + in_arg);
    }

    void *ExbCommandProcessor::GetOutputValuePointerVoid(ExbDataType data_type, u8 source_type, u16 in_arg) {

        const ExbSourceType source = static_cast<ExbSourceType>(source_type);
        void *memory = nullptr;
        switch (source) {
            case ExbSourceType::GlobalMemory:
                memory = m_global_memory;
                break;
            case ExbSourceType::Output:
                memory = m_output_memory;
                break;
            case ExbSourceType::LocalMemoryU32:
                memory = m_local_memory_u32;
                break;
            case ExbSourceType::LocalMemoryU64:
                memory = m_local_memory_u64;
                break;
            case ExbSourceType::ExternGlobal:
                if (data_type == ExbDataType::S32) {
                    return m_user_memory_callback->GetUserMemoryGlobalS32(in_arg);
                } else if (data_type == ExbDataType::Float) {
                    if (-1 < static_cast<s16>(in_arg)) { return m_user_memory_callback->GetUserMemoryGlobalFloat(in_arg); }
                    void *float_region = m_user_memory_callback->GetUserMemoryGlobalVector3f(in_arg & 0xff);
                    return reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(float_region) + ((in_arg & 0x7f00) >> 8));
                } else if (data_type == ExbDataType::Bool) {
                    return m_user_memory_callback->GetUserMemoryGlobalBool(in_arg);
                } else if (data_type == ExbDataType::Vector3f) {
                    return m_user_memory_callback->GetUserMemoryGlobalVector3f(in_arg);
                } else if (data_type == ExbDataType::String) {
                    return m_user_memory_callback->GetUserMemoryGlobalString(in_arg);
                }
                return nullptr;
            default:
                return nullptr;
        }

        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + in_arg);
    }

    void *ExbCommandProcessor::GetOutputValuePointerUserMemory(ExbDataType data_type,u16 in_arg) {

        if (data_type == ExbDataType::S32) {
            return m_user_memory_callback->GetUserMemoryGlobalS32(in_arg);
        } else if (data_type == ExbDataType::Float) {
            if (-1 < static_cast<s16>(in_arg)) { return m_user_memory_callback->GetUserMemoryGlobalFloat(in_arg); }
            void *float_region = m_user_memory_callback->GetUserMemoryGlobalVector3f(in_arg & 0xff);
            return reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(float_region) + ((in_arg & 0x7f00) >> 8));
        } else if (data_type == ExbDataType::Bool) {
            return m_user_memory_callback->GetUserMemoryGlobalBool(in_arg);
        } else if (data_type == ExbDataType::Vector3f) {
            return m_user_memory_callback->GetUserMemoryGlobalVector3f(in_arg);
        } else if (data_type == ExbDataType::String) {
            return m_user_memory_callback->GetUserMemoryGlobalString(in_arg);
        }

        return nullptr;
    }

    void ExbCommandProcessor::ProcessCommands() {

        ResExbInstruction *base_entry = m_instruction_iter;
        for (;;) {

            /* Load command */
            ExbInstructionType  exb_command   = static_cast<ExbInstructionType>(m_instruction_iter->instruction_type);
            ExbDataType exb_data_type = static_cast<ExbDataType>(m_instruction_iter->data_type);
            u8          lhs_source    = m_instruction_iter->lhs_source;
            u8          rhs_source    = m_instruction_iter->rhs_source;
            u16         arg0          = m_instruction_iter->arg0;
            u16         arg1          = m_instruction_iter->arg1;

            u64   immediate_storage = 0;
            void *lhs_ptr           = GetOutputValuePointerVoid(exb_data_type, lhs_source, arg0);

            switch (exb_command) {
                case ExbInstructionType::Terminator:
                    return;
                case ExbInstructionType::Assignment:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Assign s32, float, bool, vector3f, or string */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *lhs = *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *lhs = *rhs;
                    } else if (exb_data_type == ExbDataType::Bool) {
                        bool *lhs = reinterpret_cast<bool*>(lhs_ptr);
                        bool *rhs = reinterpret_cast<bool*>(rhs_ptr);
                        *lhs = *rhs;
                    } else if (exb_data_type == ExbDataType::Vector3f) {
                        vp::util::Float3 *lhs = reinterpret_cast<vp::util::Float3*>(lhs_ptr);
                        vp::util::Float3 *rhs = reinterpret_cast<vp::util::Float3*>(rhs_ptr);
                        *lhs = *rhs;
                    } else if (exb_data_type == ExbDataType::String) {
                        char **lhs = reinterpret_cast<char**>(lhs_ptr);
                        char **rhs = reinterpret_cast<char**>(rhs_ptr);
                        *lhs = *rhs;
                    }
                    break;
                }
                case ExbInstructionType::Negate:
                {
                    /* Negate float, s32, or Vector3f */
                    if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        *lhs = -(*lhs);
                    } else if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        *lhs = -(*lhs);
                    } else if (exb_data_type == ExbDataType::Vector3f) {
                        vp::util::Float3 *lhs = reinterpret_cast<vp::util::Float3*>(lhs_ptr);
                        *lhs = -(*lhs);
                    }
                    break;
                }
                case ExbInstructionType::NegateBool:
                {
                    /* Negate bool */
                    bool *lhs = reinterpret_cast<bool*>(lhs_ptr);
                    *lhs = *lhs ^ 1;
                    break;
                }
                case ExbInstructionType::Add:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Add s32, float, or vector3f */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *lhs = *lhs + *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *lhs = *lhs + *rhs;
                    } else if (exb_data_type == ExbDataType::Vector3f) {
                        vp::util::Float3 *lhs = reinterpret_cast<vp::util::Float3*>(lhs_ptr);
                        vp::util::Float3 *rhs = reinterpret_cast<vp::util::Float3*>(rhs_ptr);
                        *lhs = *lhs + *rhs;
                    }

                    break;
                }
                case ExbInstructionType::Subtract:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Subtract s32, float, or vector3f */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *lhs = *lhs - *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *lhs = *lhs - *rhs;
                    } else if (exb_data_type == ExbDataType::Vector3f) {
                        vp::util::Float3 *lhs = reinterpret_cast<vp::util::Float3*>(lhs_ptr);
                        vp::util::Float3 *rhs = reinterpret_cast<vp::util::Float3*>(rhs_ptr);
                        *lhs = *lhs - *rhs;
                    }
                    break;
                }
                case ExbInstructionType::Multiply:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Multiply s32 or float */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *lhs = *lhs + *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *lhs = *lhs + *rhs;
                    }
                    break;
                }
                case ExbInstructionType::Divide:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Divide s32 or float */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *lhs = *lhs / *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *lhs = *lhs / *rhs;
                    }
                    break;
                }
                case ExbInstructionType::Modulus:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Modulo s32 */
                    s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                    s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);

                    const s32 lhs_val = *lhs;
                    const s32 rhs_val = *rhs;
                    const s32 div     = (rhs_val != 0) ? (lhs_val / rhs_val) : 0;
                    *lhs = lhs_val - (div * rhs_val);

                    break;
                }
                case ExbInstructionType::Increment:
                {
                    /* Increment s32 */
                    s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);

                    *lhs = *lhs + 1;

                    break;
                }
                case ExbInstructionType::Decrement:
                {
                    /* Decrement s32 */
                    s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);

                    *lhs = *lhs - 1;

                    break;
                }
                case ExbInstructionType::ScalarMultiplyVec3f:
                {
                    float *rhs_ptr = reinterpret_cast<float*>(GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage)));

                    /* Scalar multiply Vector3f by float scalar */
                    vp::util::Float3 *lhs = reinterpret_cast<vp::util::Float3*>(lhs_ptr);

                    *lhs = *lhs * *rhs_ptr;

                    break;
                }
                case ExbInstructionType::ScalarDivideVec3f:
                {
                    float *rhs_ptr = reinterpret_cast<float*>(GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage)));

                    /* Scalar divide Vector3f by float scalar */
                    vp::util::Float3 *lhs = reinterpret_cast<vp::util::Float3*>(lhs_ptr);

                    *lhs = *lhs / *rhs_ptr;

                    break;
                }
                case ExbInstructionType::LogicalShiftLeft:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Left shift s32 */
                    s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                    s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                    *lhs = *lhs << (*rhs & 0x1f);

                    break;
                }
                case ExbInstructionType::LogicalShiftRight:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Right shift s32 */
                    s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                    s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                    *lhs = *lhs >> (*rhs & 0x1f);

                    break;
                }
                case ExbInstructionType::LessThan:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));
                    bool *output  = (static_cast<ExbSourceType>(lhs_source) == ExbSourceType::ExternGlobal) ? reinterpret_cast<bool*>(GetOutputValuePointerUserMemory(ExbDataType::Bool, arg0)) : reinterpret_cast<bool*>(lhs_ptr); 

                    /* operator< s32 or float */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *output = *lhs < *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *output = *lhs < *rhs;
                    }

                    break;
                }
                case ExbInstructionType::LessThanEqual:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));
                    bool *output  = (static_cast<ExbSourceType>(lhs_source) == ExbSourceType::ExternGlobal) ? reinterpret_cast<bool*>(GetOutputValuePointerUserMemory(ExbDataType::Bool, arg0)) : reinterpret_cast<bool*>(lhs_ptr); 

                    /* operator<= s32 or float */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *output = *lhs <= *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *output = *lhs <= *rhs;
                    }

                    break;
                }
                case ExbInstructionType::GreaterThan:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));
                    bool *output  = (static_cast<ExbSourceType>(lhs_source) == ExbSourceType::ExternGlobal) ? reinterpret_cast<bool*>(GetOutputValuePointerUserMemory(ExbDataType::Bool, arg0)) : reinterpret_cast<bool*>(lhs_ptr); 

                    /* operator> s32 or float */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *output = *lhs > *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *output = *lhs > *rhs;
                    }

                    break;
                }
                case ExbInstructionType::GreaterThanEqual:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));
                    bool *output  = (static_cast<ExbSourceType>(lhs_source) == ExbSourceType::ExternGlobal) ? reinterpret_cast<bool*>(GetOutputValuePointerUserMemory(ExbDataType::Bool, arg0)) : reinterpret_cast<bool*>(lhs_ptr); 

                    /* operator>= s32 or float */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *output = *lhs >= *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *output = *lhs >= *rhs;
                    }

                    break;
                }
                case ExbInstructionType::EqualTo:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));
                    bool *output  = (static_cast<ExbSourceType>(lhs_source) == ExbSourceType::ExternGlobal) ? reinterpret_cast<bool*>(GetOutputValuePointerUserMemory(ExbDataType::Bool, arg0)) : reinterpret_cast<bool*>(lhs_ptr); 

                    /* operator== s32 or float */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *output = *lhs == *rhs;
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *output = *lhs == *rhs;
                    }

                    break;
                }
                case ExbInstructionType::NotEqualTo:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));
                    bool *output  = (static_cast<ExbSourceType>(lhs_source) == ExbSourceType::ExternGlobal) ? reinterpret_cast<bool*>(GetOutputValuePointerUserMemory(ExbDataType::Bool, arg0)) : reinterpret_cast<bool*>(lhs_ptr); 

                    /* operator!= s32 or float */
                    if (exb_data_type == ExbDataType::S32) {
                        s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                        s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                        *output = !(*lhs == *rhs);
                    } else if (exb_data_type == ExbDataType::Float) {
                        float *lhs = reinterpret_cast<float*>(lhs_ptr);
                        float *rhs = reinterpret_cast<float*>(rhs_ptr);
                        *output = !(*lhs == *rhs);
                    }

                    break;
                }
                case ExbInstructionType::And:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* And s32 */
                    s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                    s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                    *lhs = *lhs & *rhs;

                    break;
                }
                case ExbInstructionType::Xor:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Xor s32 */
                    s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                    s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                    *lhs = *lhs ^ *rhs;

                    break;
                }
                case ExbInstructionType::Or:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Or s32 */
                    s32 *lhs = reinterpret_cast<s32*>(lhs_ptr);
                    s32 *rhs = reinterpret_cast<s32*>(rhs_ptr);
                    *lhs = *lhs | *rhs;

                    break;
                }
                case ExbInstructionType::LogicalAnd:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Logical And bool */
                    bool *lhs = reinterpret_cast<bool*>(lhs_ptr);
                    bool *rhs = reinterpret_cast<bool*>(rhs_ptr);
                    *lhs = *lhs && *rhs;

                    break;
                }
                case ExbInstructionType::LogicalOr:
                {
                    void *rhs_ptr = GetInputValuePointerVoid(exb_data_type, rhs_source, arg1, std::addressof(immediate_storage));

                    /* Logical Or bool */
                    bool *lhs = reinterpret_cast<bool*>(lhs_ptr);
                    bool *rhs = reinterpret_cast<bool*>(rhs_ptr);
                    *lhs = *lhs || *rhs;

                    break;
                }
                case ExbInstructionType::UserFunction:
                {
                    /* Call a user function */
                    m_user_function_parameter->exb_data_type = exb_data_type;
                    const u16          local_index    = m_instruction_iter->function_id;
                    const u32          function_index = m_user_function_parameter->function_table->GetFunctionIndex(local_index);
                    const ExbFunction *exb_function   = vp::res::GetExbFunctionManager()->GetFunction(function_index);
                    VP_ASSERT(exb_function != nullptr);

                    if (exb_function->direct_function == nullptr) {
                        exb_function->function_object->Invoke(m_user_function_parameter);
                    } else if (exb_function->function_object) {
                        (*exb_function->direct_function)(m_user_function_parameter);
                    }

                    break;
                }
                case ExbInstructionType::JumpZero:
                {
                    /* Break on non-zero argument, else fallthrough */
                    bool *lhs = reinterpret_cast<bool*>(lhs_ptr);
                    if (*lhs != 0) { break; }
                    if (arg1 == 0) { break; }
                    [[fallthrough]];
                }
                case ExbInstructionType::Jump:
                    /* Jump to another command */
                    m_instruction_iter = base_entry + (arg1 - 1);
                    break;
                default:
                    VP_ASSERT(false);
                    break;
            }

            /* Advance command */
            m_instruction_iter = m_instruction_iter + 1;
        }

        return;
    }
}

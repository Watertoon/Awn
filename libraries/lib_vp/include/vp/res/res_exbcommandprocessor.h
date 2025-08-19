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

    class ExbUserMemoryCallback {
        private:
        public:
            constexpr ExbUserMemoryCallback() {/*...*/}
            virtual constexpr ~ExbUserMemoryCallback() {/*...*/}

            virtual s32              *GetUserMemoryConstS32(u16 arg);
            virtual float            *GetUserMemoryConstFloat(u16 arg);
            virtual bool             *GetUserMemoryConstBool(u16 arg);
            virtual char             *GetUserMemoryConstString(u16 arg);
            virtual vp::util::Float3 *GetUserMemoryConstVector3f(u16 arg);
    
            virtual s32              **GetUserMemoryGlobalS32(u16 arg);
            virtual float            **GetUserMemoryGlobalFloat(u16 arg);
            virtual bool             **GetUserMemoryGlobalBool(u16 arg);
            virtual char             **GetUserMemoryGlobalString(u16 arg);
            virtual vp::util::Float3 **GetUserMemoryGlobalVector3f(u16 arg);
    };

    struct ExbCommandProcessorInfo;

    struct ExbCommandProcessor {
        ResExbInstruction        *m_instruction_iter;
        void                     *m_input_memory;
        void                     *m_output_memory;
        void                     *m_local_memory_u32;
        void                     *m_local_memory_u64;
        void                     *m_global_memory;
        void                     *m_constant_memory;
        char                     *m_string_pool;
        ExbUserFunctionParameter *m_user_function_parameter;
        ExbUserMemoryCallback    *m_user_memory_callback;

        void *GetInputValuePointerVoid(ExbDataType data_type, u8 source_type, u16 in_arg, u64 *immediate_storage);
        void *GetOutputValuePointerVoid(ExbDataType data_type, u8 source_type, u16 in_arg);

        void *GetOutputValuePointerUserMemory(ExbDataType data_type,u16 in_arg);

        void ProcessCommands();
    };
}

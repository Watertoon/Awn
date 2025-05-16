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

            virtual s32              *GetUserMemoryInputS32(u16 arg);
            virtual float            *GetUserMemoryInputFloat(u16 arg);
            virtual bool             *GetUserMemoryInputBool(u16 arg);
            virtual char             *GetUserMemoryInputString(u16 arg);
            virtual vp::util::Float3 *GetUserMemoryInputVector3f(u16 arg);
    
            virtual s32              **GetUserMemoryOutputS32(u16 arg);
            virtual float            **GetUserMemoryOutputFloat(u16 arg);
            virtual bool             **GetUserMemoryOutputBool(u16 arg);
            virtual char             **GetUserMemoryOutputString(u16 arg);
            virtual vp::util::Float3 **GetUserMemoryOutputVector3f(u16 arg);
    };

    struct ExbCommandProcessorInfo;

    struct ExbCommandProcessor {
        ResExbCommand            *m_res_command_entry;
        void                     *m_input_memory;
        void                     *m_output_memory;
        void                     *m_scratch_u32_aligned;
        void                     *m_scratch_u64_aligned;
        void                     *m_static_memory;
        void                     *m_res_exb_parameter_region;
        char                     *m_res_exb_string_pool;
        ExbUserFunctionParameter *m_user_function_parameter;
        ExbUserMemoryCallback    *m_exb_callback;

        void *GetInputValuePointerVoid(ExbDataType data_type, u8 source_type, u16 in_arg, u64 *immediate_storage);
        void *GetOutputValuePointerVoid(ExbDataType data_type, u8 source_type, u16 in_arg);

        void *GetOutputValuePointerUserMemory(ExbDataType data_type,u16 in_arg);

        void ProcessCommands();
    };
}

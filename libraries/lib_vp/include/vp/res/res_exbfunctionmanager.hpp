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

    class ExbUserFunctionParameter;

    class ExbFunctionObject {
        public:
            constexpr ExbFunctionObject() {/*...*/}
            virtual ~ExbFunctionObject() {/*...*/}

            virtual void Invoke([[maybe_unused]] vp::res::ExbUserFunctionParameter *user_param) {/*...*/}
            virtual const char *GetSignature() const { return ""; }
    };
    using ExbDirectFunction = void (*) (void*);

    struct ExbFunction {
        const char        *function_signature;
        ExbFunctionObject *function_object;
        ExbDirectFunction  direct_function;
    };

	class ExbFunctionManager {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
		private:
			vp::util::PointerArrayAllocator<ExbFunction> m_function_array;
        public:
            constexpr  ExbFunctionManager() : m_function_array() {/*...*/}
            constexpr ~ExbFunctionManager() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, u32 function_count);
            void Finalize();

            void RegisterDirectFunction(ExbDirectFunction direct_function, const char *signature);

            void RegisterFunctionObject(ExbFunctionObject *function_object);

            constexpr const ExbFunction *GetFunction(u32 function_index) const {
                return m_function_array[function_index];
            }

            constexpr u32 GetFunctionIndexByName(const char *signature) const {

                for (u32 i = 0; i < m_function_array.GetUsedCount(); ++i) {
                    if (::strcmp(signature, m_function_array[i]->function_signature) == 0) { return i; }
                }

                return cInvalidEntryIndex;
            }
	};

    extern ExbFunctionManager *sExbFunctionManagerInstance;

    void SetExbFunctionManager(ExbFunctionManager *function_manager);
    ExbFunctionManager *GetExbFunctionManager();

    class ExbFunctionTable {
        private:
            vp::util::HeapArray<u32> m_function_index_array;
        public:
            constexpr  ExbFunctionTable() : m_function_index_array() {/*...*/}
            constexpr ~ExbFunctionTable() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, vp::res::ResExb *res_exb);
            void Finalize();

            constexpr u32 GetFunctionIndex(u32 local_index) const { return m_function_index_array[local_index]; }
    };

    struct ExbUserFunctionParameter {
        ExbDataType             exb_data_type;
        u8                      reserve0;
        u16                     reserve1;
        u32                     reserve2;
        ResExb                 *res_exb;
        ExbFunctionTable       *function_table;
    };
}

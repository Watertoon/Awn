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

	ExbFunctionManager *sExbFunctionManagerInstance = nullptr;

    void SetExbFunctionManager(ExbFunctionManager *function_manager) {
        sExbFunctionManagerInstance = function_manager;
    }
    ExbFunctionManager *GetExbFunctionManager() {
        return sExbFunctionManagerInstance;
    }
    
    void ExbFunctionManager::Initialize(vp::imem::IHeap *heap, u32 function_count) {
        m_function_array.Initialize(heap, function_count);
    }

    void ExbFunctionManager::Finalize() {
        m_function_array.Finalize();
    }

    void ExbFunctionManager::RegisterDirectFunction(ExbDirectFunction direct_function, const char *signature) {
        ExbFunction *new_function = m_function_array.Allocate();
        new_function->function_signature = signature;
        new_function->direct_function    = direct_function;
    }

    void ExbFunctionManager::RegisterFunctionObject(ExbFunctionObject *function_object) {
        ExbFunction *new_function = m_function_array.Allocate();
        new_function->function_signature = function_object->GetSignature();
        new_function->function_object    = function_object;
    }

    void ExbFunctionTable::Initialize(vp::imem::IHeap *heap, vp::res::ResExb *res_exb) {

        /* Initialize indice array */
        vp::res::ResExbSignatureTable *signature_table = res_exb->GetSignatureTable();
        m_function_index_array.Initialize(heap, signature_table->signature_count);

        /* Resolve function indices */
        ExbFunctionManager *function_manager = GetExbFunctionManager();
        VP_ASSERT(function_manager != nullptr);
        for (u32 i = 0; i < signature_table->signature_count; ++i) {
            m_function_index_array[i] = function_manager->GetFunctionIndexByName(res_exb->GetString(signature_table->signature_offset_array[i]));
        }

        return;
    }

    void ExbFunctionTable::Finalize() {
        m_function_index_array.Finalize();
    }
}

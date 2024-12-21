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
#include <awn.hpp>

namespace awn::gfx::impl {

    bool ShaderArchiveUnit::Initialize(const ShaderArchiveUnitInfo *shader_archive_unit_info) {

        /* Reset state */
        m_status          = Status::InRegister;
        m_state           = 0;
        m_reference_count = 1;
        m_priority        = shader_archive_unit_info->priority;
        m_bnsh_container  = nullptr;

        /* Set bnsh */
        m_bnsh = vp::res::ResBnsh::ResCast(shader_archive_unit_info->bnsh);
        if (m_bnsh == nullptr) {
            m_is_fail_invalid_file = true;
            m_status = Status::Error;
            return false;
        }
        m_bnsh_container = reinterpret_cast<vp::res::ResBnshShaderContainer*>(m_bnsh->GetFirstBlock());
        
        #ifdef VP_TARGET_GRAPHICS_API_vk
            const u16 target_api = static_cast<u16>(vp::res::BnshTargetApiType::VpExtVulkan);
        #elif VP_TARGET_GRAPHICS_API_nvn
            const u16 target_api = static_cast<u16>(vp::res::BnshTargetApiType::Nvn);
        #endif
        if (m_bnsh_container == nullptr || m_bnsh_container->magic != vp::res::ResBnshShaderContainer::cMagic || m_bnsh_container->target_api_type != target_api) {
            m_is_fail_invalid_file = true;
            m_status = Status::Error;
            return false;
        }

        return true;
    }

    void ShaderArchiveUnit::Finalize() {
        for (u32 i = 0; i < m_bnsh_container->variation_count; ++i) {
            vp::res::ResBnshShaderProgram *program = m_bnsh_container->variation_array[i].program_binary;
            Shader *shader = reinterpret_cast<Shader*>(program->runtime_shader);
            shader->Finalize();
        }
    }

    void ShaderArchiveUnit::RequestFinalize() {

        /* Cancel transfer if it's scheduled */
        if (m_transfer_task.IsPending()) {
            m_transfer_task.CancelTask();
        }

        /* Schedule free */
        AsyncGpuResourceManager *gpu_res_mgr = AsyncGpuResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = gpu_res_mgr->m_memory_thread,
            .task_function = std::addressof(gpu_res_mgr->m_task_function_finalize_shader_archive_unit),
            .user_data     = this,
            .priority      = 0,
        };
        m_transfer_task.PushTask(std::addressof(push_info));

        return;
    }

    bool ShaderArchiveUnit::RequestTransfer() {

        if (m_bnsh_container == nullptr) { return false; }
        if (m_bnsh_container->variation_count < cSmallVariationCount) { 
            this->Transfer();
            return false;
        }

        /* Push transfer task */
        AsyncGpuResourceManager *gpu_res_mgr = AsyncGpuResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = gpu_res_mgr->m_transfer_thread,
            .task_function = std::addressof(gpu_res_mgr->m_task_function_shader_archive_transfer),
            .user_data     = this,
            .priority      = m_priority,
        };
        m_transfer_task.PushTask(std::addressof(push_info));

        return true;
    }

    void ShaderArchiveUnit::Transfer() {

        /* Compile all shader variations */
        for (u32 i = 0; i < m_bnsh_container->variation_count; ++i) {
            vp::res::ResBnshShaderProgram *program = m_bnsh_container->variation_array[i].program_binary;
            Shader *shader = reinterpret_cast<Shader*>(program->runtime_shader);
            bool is_api_error = shader->Initialize(program);
            if (is_api_error) {
                this->Finalize();
                m_is_fail_create_api_resource = true;
                m_status = Status::Error;
                return;
            }
        }

        m_status = Status::Registered;

        return;
    }

    void ShaderArchiveUnit::WaitForTransfer() {
        m_transfer_task.Wait();
    }
}

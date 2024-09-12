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

#ifdef VP_TARGET_GRAPHICS_API_vk
    bool BufferUnit::InitializeForPlatform(const BufferUnitInfo *buffer_unit_info) {

        /* Initialize buffer */
        const bool result_api = m_buffer.InitializeForDedicatedAllocation(buffer_unit_info->buffer_info);
        if (!result_api) {
            m_is_fail_create_api_resource = true;
            m_status = Status::Error;
            return false;
        }

        /* Allocate dedicated memory */
        mem::Heap *meta_data_heap = AsyncGpuResourceManager::GetInstance()->GetMetaDataHeap();
        Context *context = Context::GetInstance();
        VkBuffer vk_buffer = m_buffer.GetVkBuffer();
        const VkMemoryDedicatedAllocateInfo dedicated_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
            .buffer = vk_buffer,
        };
        m_memory_handle = mem::AllocateGpuMemoryHandle(meta_data_heap, buffer_unit_info->memory_property_flags, m_memory_requirements.size, std::addressof(dedicated_info));
        if (!m_memory_handle.IsValid()) {
            this->Finalize();
            m_is_fail_memory_allocation = true;
            m_status = Status::Error;
            return false;
        }
        const VkResult is_bind = ::pfn_vkBindBufferMemory(context->GetVkDevice(), vk_buffer, m_memory_handle.vk_device_memory, 0);
        if (is_bind != VK_SUCCESS) {
            this->Finalize();
            m_is_fail_memory_allocation = true;
            m_status = Status::Error;
            return false;
        }

        /* Map memory */
        void *mapping = nullptr;
        const VkResult result_mapping = ::pfn_vkMapMemory(context->GetVkDevice(), m_memory_handle.vk_device_memory, 0, VK_WHOLE_SIZE, 0, std::addressof(mapping));
        if (result_mapping != VK_SUCCESS) {
            this->Finalize();
            m_is_fail_memory_mapping = true;
            m_status = Status::Error;
            return false;
        }
        m_buffer.SetAddressForDedicatedAllocation(mapping);

        return true;
    }
#elif VP_TARGET_GRAPHICS_API_nvn
    bool BufferUnit::InitializeForPlatform(const BufferUnitInfo *buffer_unit_info) {

        /* Allocate memory */
        mem::Heap *meta_data_heap = AsyncGpuResourceManager::GetInstance()->GetMetaDataHeap();
        m_memory_handle = mem::AllocateGpuMemoryHandle(meta_data_heap, buffer_unit_info->memory_property_flags, m_memory_requirements.size);
        if (!m_memory_handle.IsValid()) {
            this->Finalize();
            m_is_fail_memory_allocation = true;
            m_status = Status::Error;
            return false;
        }

        /* Create buffer */
        m_buffer.Initialize(m_memory_handle, buffer_unit_info->buffer_info);

        return true;
    }
#endif

    bool BufferUnit::Initialize(const BufferUnitInfo *buffer_unit_info) {

        /* Reset state */
        m_status          = Status::InRegister;
        m_state           = 0;
        m_reference_count = 1;
        m_import_memory   = nullptr;
        m_priority        = buffer_unit_info->priority;

        /* Set memory requirements */
        m_memory_requirements   = buffer_unit_info->memory_requirements;
        m_memory_property_flags = buffer_unit_info->memory_property_flags;

        /* Track memory usage */
        const bool is_in_limit = AsyncGpuResourceManager::GetInstance()->AddMemoryUsage(m_memory_requirements, buffer_unit_info->memory_property_flags);
        if (!is_in_limit) {
            this->Finalize();
            m_is_fail_out_of_memory = true;
            m_status = Status::Error;
            return false;
        }

        /* Initialize for platform (assume it sets the error) */
        const bool is_platform_init = this->InitializeForPlatform(buffer_unit_info);
        if (!is_platform_init) { return false; }

        /* Set import memory  */
        m_import_memory = buffer_unit_info->copy_memory;

        return true;
    }

    void BufferUnit::Finalize() {
        m_buffer.Finalize();
        mem::FreeGpuMemoryHandle(m_memory_handle);
        m_memory_handle = mem::cInvalidGpuMemoryHandle;
    }

    void BufferUnit::RequestFinalize() {

        /* Cancel transfer if it's scheduled */
        if (m_transfer_task.IsBusy()) {
            m_transfer_task.CancelTask();
        }

        /* Schedule free */
        AsyncGpuResourceManager *gpu_res_mgr = AsyncGpuResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = gpu_res_mgr->m_memory_thread,
            .task_function = std::addressof(gpu_res_mgr->m_task_function_finalize_buffer_unit),
            .user_data     = this,
            .priority      = 0,
        };
        m_transfer_task.PushTask(std::addressof(push_info));

        return;
    }

    bool BufferUnit::RequestTransfer() {

        /* No transfer if no host memory */
        if (m_import_memory == nullptr) { 
            m_status = Status::Registered;
            return false; 
        }

        /* Do an inline copy if our memory size is small */
        if (m_memory_requirements.size <= cSmallMemoryCopySize) {
            this->Transfer();
            return false;
        }

        /* Schedule transfer task */
        AsyncGpuResourceManager *gpu_res_mgr = AsyncGpuResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = gpu_res_mgr->m_transfer_thread,
            .task_function = std::addressof(gpu_res_mgr->m_task_function_buffer_transfer),
            .user_data     = this,
            .priority      = m_priority,
        };
        m_transfer_task.PushTask(std::addressof(push_info));

        return true;
    }

    void BufferUnit::Transfer() {
        ::memcpy(m_buffer.GetAddress(), m_import_memory, m_memory_requirements.size);
        m_status = Status::Registered;
    }

    void BufferUnit::WaitForTransfer() {
        m_transfer_task.Wait();
    }
}

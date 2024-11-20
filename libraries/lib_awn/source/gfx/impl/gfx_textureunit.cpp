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
    bool TextureUnit::InitializeForPlatform(const TextureUnitInfo *texture_unit_info) {

        /* Initialize texture */
        const bool is_init_api = m_texture.InitializeForDedicatedAllocation(texture_unit_info->texture_info);
        if (!is_init_api) {
            m_is_fail_create_api_resource = true;
            m_status = Status::Error;
            return false;
        }

        /* Allocate dedicated memory */
        mem::Heap *meta_data_heap = AsyncGpuResourceManager::GetInstance()->GetMetaDataHeap();
        Context *context = Context::GetInstance();
        VkImage vk_image = m_texture.GetVkImage();
        const VkMemoryDedicatedAllocateInfo dedicated_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
            .image = vk_image,
        };
        m_memory_handle = mem::AllocateGpuMemoryHandle(meta_data_heap, texture_unit_info->memory_property_flags, m_memory_requirements.size, std::addressof(dedicated_info));
        if (!m_memory_handle.IsValid()) {
            this->Finalize();
            m_is_fail_memory_allocation = true;
            m_status = Status::Error;
            return false;
        }
        const VkResult is_bind = ::pfn_vkBindImageMemory(context->GetVkDevice(), vk_image, m_memory_handle.vk_device_memory, 0);
        if (is_bind != VK_SUCCESS) {
            this->Finalize();
            m_is_fail_memory_allocation = true;
            m_status = Status::Error;
            return false;
        }

        return true;
    }
#elif VP_TARGET_GRAPHICS_API_nvn
    bool TextureUnit::InitializeForPlatform(const TextureUnitInfo *texture_unit_info) {

        /* Allocate dedicated memory */
        mem::Heap *meta_data_heap = AsyncGpuResourceManager::GetInstance()->GetMetaDataHeap();
        m_memory_handle = mem::AllocateGpuMemoryHandle(meta_data_heap, texture_unit_info->memory_property_flags, m_memory_requirements.size);
        if (!m_memory_handle.IsValid()) {
            this->Finalize();
            m_is_fail_memory_allocation = true;
            m_status = Status::Error;
            return false;
        }

        /* Initialize texture */
        m_texture.Initialize(m_memory_handle, texture_unit_info->texture_info);

        return true;
    }
#endif

    bool TextureUnit::Initialize(const TextureUnitInfo *texture_unit_info) {

        /* Reset state */
        m_status              = Status::InRegister;
        m_state               = 0;
        m_reference_count     = 1;
        m_import_mip_array[0] = nullptr;
        m_priority            = texture_unit_info->priority;

        /* Set memory requirements */
        m_memory_requirements   = texture_unit_info->memory_requirements;
        m_memory_property_flags = texture_unit_info->memory_property_flags;

        /* Track memory usage */
        const bool is_in_limit = AsyncGpuResourceManager::GetInstance()->AddMemoryUsage(m_memory_requirements, m_memory_property_flags);
        if (!is_in_limit) {
            this->Finalize();
            m_is_fail_out_of_memory = true;
            m_status = Status::Error;
            return false;
        }
        
        /* Initialize for platform (assume it sets the error) */
        const bool is_platform_init = this->InitializeForPlatform(texture_unit_info);
        if (!is_platform_init) { return false; }

        /* Set transfer options array */
        for (u32 i = 0; i < m_texture.GetMipLevelCount(); ++i) {
            m_import_mip_array[i] = texture_unit_info->import_mip_array[i];
        }

        return true;
    }

    void TextureUnit::Finalize() {
        m_texture.Finalize();
        mem::FreeGpuMemoryHandle(m_memory_handle);
        m_memory_handle = mem::cInvalidGpuMemoryHandle;
    }

    void TextureUnit::RequestFinalize() {

        /* Cancel transfer if it's scheduled */
        if (m_transfer_task.IsPending()) {
            m_transfer_task.CancelTask();
        }

        /* Schedule free */
        AsyncGpuResourceManager *gpu_res_mgr = AsyncGpuResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = gpu_res_mgr->m_memory_thread,
            .task_function = std::addressof(gpu_res_mgr->m_task_function_finalize_texture_unit),
            .user_data     = this,
            .priority      = 0,
        };
        m_transfer_task.PushTask(std::addressof(push_info));

        return;
    }

    bool TextureUnit::RequestTransfer() {

        /* No transfer if no host memory */
        if (m_import_mip_array[0] == nullptr) { 
            m_status = Status::Registered;
            return false; 
        }

        /* Transfer inline if small memory */
        if (m_memory_requirements.size <= cSmallMemorySize) {
            this->Transfer();
            return false;
        }

        /* Push transfer task */
        AsyncGpuResourceManager *gpu_res_mgr = AsyncGpuResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = gpu_res_mgr->m_transfer_thread,
            .task_function = std::addressof(gpu_res_mgr->m_task_function_texture_transfer),
            .user_data     = this,
            .priority      = m_priority,
        };
        m_transfer_task.PushTask(std::addressof(push_info));

        return true;
    }

#ifdef VP_TARGET_GRAPHICS_API_vk
    bool TextureUnit::TransferForPlatform() {

        /* Setup copy arguments */
        const u32 mip_count   = m_texture.GetMipLevelCount();
        const u32 layer_count = m_texture.GetArrayLayerCount();
        const u32 width       = m_texture.GetWidth();
        const u32 height      = m_texture.GetHeight();
        const u32 depth       = m_texture.GetDepth();
        const u32 aspect_mask = m_texture.GetVkAspectMask();
        VkMemoryToImageCopyEXT memory_to_image_regions[Context::cTargetMaxMipLevelCount] = {};
        for (u32 i = 0; i < mip_count; ++i) {
            memory_to_image_regions[i].sType                           = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY_EXT,
            memory_to_image_regions[i].pHostPointer                    = m_import_mip_array[i];
            memory_to_image_regions[i].imageSubresource.aspectMask     = aspect_mask;
            memory_to_image_regions[i].imageSubresource.mipLevel       = i;
            memory_to_image_regions[i].imageSubresource.baseArrayLayer = 0;
            memory_to_image_regions[i].imageSubresource.layerCount     = layer_count;
            memory_to_image_regions[i].imageExtent.width               = (width >> i);
            memory_to_image_regions[i].imageExtent.height              = (height >> i);
            memory_to_image_regions[i].imageExtent.depth               = depth;
        }

        /* Transfer memory */
        const VkHostImageCopyFlagBitsEXT convert_mode = (m_texture_conversion_mode == 1) ? static_cast<VkHostImageCopyFlagBitsEXT>(0) : VK_HOST_IMAGE_COPY_MEMCPY_EXT;
        const VkCopyMemoryToImageInfoEXT copy_info = {
            .sType          = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO_EXT,
            .flags          = convert_mode,
            .dstImage       = m_texture.GetVkImage(),
            .dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .regionCount    = mip_count,
            .pRegions       = memory_to_image_regions,
        };
        const VkResult result = ::pfn_vkCopyMemoryToImageEXT(Context::GetInstance()->GetVkDevice(), std::addressof(copy_info));
        if (result != VK_SUCCESS) {
            this->Finalize();
            m_is_fail_memory_copy = true;
            m_status = Status::Error;
            return false;
        }

        return true;
    }
#elif VP_TARGET_GRAPHICS_API_nvn
    bool TextureUnit::TransferForPlatform() {
        VP_ASSERT(false);
        return true;
    }
#endif

    void TextureUnit::Transfer() {

        /* Copy impl */
        const bool is_transferred = this->TransferForPlatform();
        if (!is_transferred) { return; }

        /* Complete */
        m_status = Status::Registered;

        return;
    }

    void TextureUnit::WaitForTransfer() {
        m_transfer_task.Wait();
    }
}

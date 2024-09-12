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

namespace awn::gfx {

    bool Buffer::Initialize(void *gpu_memory_address, const BufferInfo *buffer_info) {

        /* Create vk buffer  */
        m_vk_buffer = gfx::impl::CreateVkBuffer(gpu_memory_address, buffer_info);
        if (m_vk_buffer == VK_NULL_HANDLE) { return false; }

        /* Set gpu address */
        m_address = gpu_memory_address;

        /* Query VkDeviceAddress */
        m_gpu_buffer_address = gfx::impl::GetGpuBufferAddress(m_vk_buffer);

        return true;
    }

    void Buffer::Finalize() {
        if (m_vk_buffer != VK_NULL_HANDLE) {
            gfx::impl::DestroyVkBuffer(m_vk_buffer);
            m_vk_buffer         = VK_NULL_HANDLE;
            m_gpu_buffer_address = gfx::cInvalidGpuBufferAddress;
        }
    }

    GpuMemoryRequirements Buffer::GetMemoryRequirements(const BufferInfo *buffer_info) {

        /* Get memory requirements */
        VkMemoryRequirements2 memory_requirements_2 = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        };
        const VkBufferCreateInfo vk_buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = buffer_info->size,
            .usage = vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(buffer_info->gpu_access_flags)) | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        const VkDeviceBufferMemoryRequirements buffer_requirements = {
            .sType       = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
            .pCreateInfo = std::addressof(vk_buffer_info),
        };
        ::pfn_vkGetDeviceBufferMemoryRequirements(impl::Context::GetInstance()->GetVkDevice(), std::addressof(buffer_requirements), std::addressof(memory_requirements_2));

        return { memory_requirements_2.memoryRequirements.size, memory_requirements_2.memoryRequirements.alignment };
    }

    bool Buffer::InitializeForDedicatedAllocation(const BufferInfo *buffer_info) {

        /* Create VkBuffer */
        m_vk_buffer = gfx::impl::CreateVkBuffer(buffer_info);
        if (m_vk_buffer == VK_NULL_HANDLE) { return false; }

        return true;
    }

    void Buffer::SetAddressForDedicatedAllocation(void *gpu_memory_address) {

        /* Query VkDeviceAddress */
        m_gpu_buffer_address = gfx::impl::GetGpuBufferAddress(m_vk_buffer);

        /* Map memory */
        m_address = gpu_memory_address;

        return;
    }
}

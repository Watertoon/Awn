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

    Result Buffer::Initialize(MemoryPool *memory_pool, size_t memory_offset, const BufferInfo *buffer_info) {

        /* Create vk buffer  */
        RESULT_TRY(gfx::impl::CreateVkBuffer(std::addressof(m_vk_buffer), memory_pool, memory_offset, buffer_info));

        /* Set gpu address */
        m_memory_pool = memory_pool;
        m_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory_pool->GetAddress()) + memory_offset);
        m_memory_offset = memory_offset;

        /* Query VkDeviceAddress */
        m_gpu_address = gfx::impl::GetGpuAddress(m_vk_buffer);

        RESULT_RETURN_SUCCESS;
    }

    Result Buffer::Initialize(void *gpu_memory_address, const BufferInfo *buffer_info) {

        /* Create vk buffer  */
        size_t offset;
        MemoryPool *memory_pool = gfx::FindMemoryPool(std::addressof(offset), gpu_memory_address);
        RESULT_TRY(gfx::impl::CreateVkBuffer(std::addressof(m_vk_buffer), memory_pool, offset, buffer_info));

        /* Set gpu address */
        m_memory_pool = memory_pool;
        m_address = gpu_memory_address;
        m_memory_offset = offset;

        /* Query VkDeviceAddress */
        m_gpu_address = gfx::impl::GetGpuAddress(m_vk_buffer);

        RESULT_RETURN_SUCCESS;
    }

    void Buffer::Finalize() {
        if (m_vk_buffer != VK_NULL_HANDLE) {
            gfx::impl::DestroyVkBuffer(m_vk_buffer);
            m_vk_buffer = VK_NULL_HANDLE;
            m_gpu_address = gfx::cInvalidGpuAddress;
            m_address = nullptr;
            m_memory_pool = nullptr;
            m_memory_offset = 0;
        }
        return;
    }

    void Buffer::FlushMappedRange(size_t offset, size_t size) {
        if (!m_memory_pool->IsCached()) { return; }
        const size_t base_offset = reinterpret_cast<uintptr_t>(m_address) - reinterpret_cast<uintptr_t>(m_memory_pool->GetAddress());
        m_memory_pool->FlushMappedRange(base_offset + offset, size);
    }

    void Buffer::InvalidateMappedRange(size_t offset, size_t size) {
        if (!m_memory_pool->IsCached()) { return; }
        const size_t base_offset = reinterpret_cast<uintptr_t>(m_address) - reinterpret_cast<uintptr_t>(m_memory_pool->GetAddress());
        m_memory_pool->InvalidateMappedRange(base_offset + offset, size);
    }

    GpuMemoryRequirements Buffer::GetGpuMemoryRequirements(const BufferInfo *buffer_info) {

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

        return { memory_requirements_2.memoryRequirements.size, static_cast<s32>(memory_requirements_2.memoryRequirements.alignment), };
    }

    Result Buffer::InitializeForDedicatedAllocation(const BufferInfo *buffer_info) {

        /* Create VkBuffer */
        RESULT_TRY(gfx::impl::CreateVkBuffer(std::addressof(m_vk_buffer), buffer_info));

        RESULT_RETURN_SUCCESS;
    }

    void Buffer::SetAddressForDedicatedAllocation(void *gpu_memory_address) {

        /* Query VkDeviceAddress */
        m_gpu_address = gfx::impl::GetGpuAddress(m_vk_buffer);

        /* Map memory */
        m_address = gpu_memory_address;

        return;
    }
}

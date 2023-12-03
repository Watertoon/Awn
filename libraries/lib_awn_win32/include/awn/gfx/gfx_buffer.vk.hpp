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

namespace awn::gfx {

    struct GpuMemoryRequirements {
        size_t size;
        size_t alignment;
    };

	class Buffer {
		private:
            VkBuffer         m_vk_buffer;
            VkDeviceAddress  m_vk_device_address;
            void            *m_buffer_gpu_memory_address;
		public:
			constexpr ALWAYS_INLINE  Buffer() : m_vk_buffer(VK_NULL_HANDLE), m_vk_device_address(0), m_buffer_gpu_memory_address() {/*...*/}
            constexpr ALWAYS_INLINE ~Buffer() {/*...*/}

			void Initialize(void *gpu_memory_address, BufferInfo *buffer_info) {

                /* Set gpu address */
                m_buffer_gpu_memory_address = gpu_memory_address;

                /* Create vk buffer  */
                m_vk_buffer = gfx::CreateVkBuffer(gpu_memory_address, buffer_info);

                /* Query VkDeviceAddress */
                const VkBufferDeviceAddressInfo device_address_info = {
                    .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                    .buffer = m_vk_buffer
                    
                };
                m_vk_device_address = ::vkGetBufferDeviceAddress(Context::GetInstance()->GetVkDevice(), std::addressof(device_address_info));

                return;
			}

			void Initialize(VkDeviceMemory vk_device_memory, const BufferInfo *buffer_info) {

                /* Create VkBuffer */
                const VkBufferCreateInfo vk_buffer_info = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size  = buffer_info->size,
                    .usage = vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(buffer_info->gpu_access_flags)) | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                };
                const u32 result0 = ::pfn_vkCreateBuffer(Context::GetInstance()->GetVkDevice(), std::addressof(vk_buffer_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_buffer));
                VP_ASSERT(result0 == VK_SUCCESS);

                /* Bind device memory to buffer */
                const u32 result1 = ::pfn_vkBindBufferMemory(Context::GetInstance()->GetVkDevice(), m_vk_buffer, vk_device_memory, 0);
                VP_ASSERT(result1 == VK_SUCCESS);

                /* Query VkDeviceAddress */
                const VkBufferDeviceAddressInfo device_address_info = {
                    .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                    .buffer = m_vk_buffer
                    
                };
                m_vk_device_address = ::vkGetBufferDeviceAddress(Context::GetInstance()->GetVkDevice(), std::addressof(device_address_info));

                return;
			}

            void Finalize() {

                /* Delete VkBuffer */
                if (m_vk_buffer != VK_NULL_HANDLE) {
                    ::pfn_vkDestroyBuffer(Context::GetInstance()->GetVkDevice(), m_vk_buffer, Context::GetInstance()->GetVkAllocationCallbacks());
                }
                m_vk_buffer         = VK_NULL_HANDLE;
                m_vk_device_address = 0;
            }

            static GpuMemoryRequirements GetMemoryRequirements(const BufferInfo *buffer_info) {

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
                ::pfn_vkGetDeviceBufferMemoryRequirements(Context::GetInstance()->GetVkDevice(), std::addressof(buffer_requirements), std::addressof(memory_requirements_2));

                return { memory_requirements_2.memoryRequirements.size, memory_requirements_2.memoryRequirements.alignment };
            }

            constexpr ALWAYS_INLINE void *Map()                { return m_buffer_gpu_memory_address; }
            constexpr ALWAYS_INLINE void  Unmap(void *address) {/*...*/}

            constexpr ALWAYS_INLINE VkDeviceAddress       GetVkDeviceAddress()  const { return m_vk_device_address; } 
            constexpr ALWAYS_INLINE VkBuffer              GetVkBuffer()         const { return m_vk_buffer; } 
	};
    static_assert(sizeof(Buffer) <= cMaxGfxBufferSize);
}

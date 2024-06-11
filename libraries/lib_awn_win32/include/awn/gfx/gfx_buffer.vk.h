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

			bool Initialize(void *gpu_memory_address, const BufferInfo *buffer_info);

            void Finalize();

            static GpuMemoryRequirements GetMemoryRequirements(const BufferInfo *buffer_info);

            constexpr ALWAYS_INLINE void *GetAddress() const { return m_buffer_gpu_memory_address; }

            constexpr ALWAYS_INLINE VkDeviceAddress       GetVkDeviceAddress()  const { return m_vk_device_address; } 
            constexpr ALWAYS_INLINE VkBuffer              GetVkBuffer()         const { return m_vk_buffer; } 

			bool InitializeForDedicatedAllocation(const BufferInfo *buffer_info);
            void SetAddressForDedicatedAllocation(void *gpu_memory_address);
	};
    static_assert(sizeof(Buffer) <= cMaxGfxBufferSize);
}

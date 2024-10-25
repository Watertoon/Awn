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

    using GpuBufferAddress = impl::GpuBufferAddress;
    static constexpr GpuBufferAddress cInvalidGpuBufferAddress = impl::cInvalidGpuBufferAddress;

    static constexpr size_t cTargetVertexBufferAlignment = impl::Context::cTargetVertexBufferAlignment;
    static constexpr size_t cTargetIndexBufferAlignment = impl::Context::cTargetIndexBufferAlignment;
    static constexpr size_t cTargetUniformBufferAlignment = impl::Context::cTargetUniformBufferAlignment;
    static constexpr size_t cTargetStorageBufferAlignment = impl::Context::cTargetStorageBufferAlignment;
    static constexpr size_t cTargetDescriptorBufferAlignment = impl::Context::cTargetDescriptorBufferAlignment;

	class Buffer {
		private:
            VkBuffer          m_vk_buffer;
            GpuBufferAddress  m_gpu_buffer_address;
            void             *m_address;
		public:
			constexpr ALWAYS_INLINE  Buffer() : m_vk_buffer(), m_gpu_buffer_address(), m_address() {/*...*/}
            constexpr ALWAYS_INLINE ~Buffer() {/*...*/}

			bool Initialize(mem::GpuMemoryHandle memory_handle, size_t memory_offset, const BufferInfo *buffer_info);
			bool Initialize(void *gpu_memory_address, const BufferInfo *buffer_info);
            void Finalize();

            static GpuMemoryRequirements GetMemoryRequirements(const BufferInfo *buffer_info);

            constexpr ALWAYS_INLINE void *GetAddress() const { return m_address; }
            constexpr ALWAYS_INLINE GpuBufferAddress GetGpuBufferAddress(size_t offset = 0) const { return { m_gpu_buffer_address.vk_device_address + offset, m_gpu_buffer_address.vk_buffer }; } 

        public:
            constexpr ALWAYS_INLINE VkBuffer GetVkBuffer() const { return m_vk_buffer; } 
			bool InitializeForDedicatedAllocation(const BufferInfo *buffer_info);
            void SetAddressForDedicatedAllocation(void *gpu_memory_address);
	};
    static_assert(sizeof(Buffer) <= cMaxGfxBufferSize);
}

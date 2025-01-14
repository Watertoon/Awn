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

    using GpuAddress = impl::GpuAddress;
    static constexpr GpuAddress cInvalidGpuAddress = impl::cInvalidGpuAddress;

    class Buffer;

    struct BufferAddress {
        const Buffer *buffer;
        size_t offset;

        constexpr GpuAddress GetGpuAddress();

        constexpr VkBuffer GetVkBuffer();
    };

    static constexpr size_t cTargetVertexBufferAlignment = impl::Context::cTargetVertexBufferAlignment;
    static constexpr size_t cTargetIndexBufferAlignment = impl::Context::cTargetIndexBufferAlignment;
    static constexpr size_t cTargetUniformBufferAlignment = impl::Context::cTargetUniformBufferAlignment;
    static constexpr size_t cTargetStorageBufferAlignment = impl::Context::cTargetStorageBufferAlignment;
    static constexpr size_t cTargetDescriptorBufferAlignment = impl::Context::cTargetDescriptorBufferAlignment;

	class Buffer {
		private:
            VkBuffer    m_vk_buffer;
            GpuAddress  m_gpu_address;
            void       *m_address;
		public:
			constexpr ALWAYS_INLINE  Buffer() : m_vk_buffer(), m_gpu_address(), m_address() {/*...*/}
            constexpr ALWAYS_INLINE ~Buffer() {/*...*/}

			Result Initialize(mem::GpuMemoryHandle memory_handle, size_t memory_offset, const BufferInfo *buffer_info);
			Result Initialize(void *gpu_memory_address, const BufferInfo *buffer_info);
            void Finalize();

            constexpr ALWAYS_INLINE void *GetAddress() const { return m_address; }
            constexpr ALWAYS_INLINE GpuAddress GetGpuAddress() const { return m_gpu_address; }
            constexpr ALWAYS_INLINE BufferAddress GetBufferAddress(size_t offset = 0) const { return { this, offset, }; } 
		public:
            static GpuMemoryRequirements GetGpuMemoryRequirements(const BufferInfo *buffer_info);
        public:
            constexpr ALWAYS_INLINE VkBuffer GetVkBuffer() const { return m_vk_buffer; } 
			Result InitializeForDedicatedAllocation(const BufferInfo *buffer_info);
            void SetAddressForDedicatedAllocation(void *gpu_memory_address);
	};
    static_assert(sizeof(Buffer) <= cMaxGfxBufferSize);

    constexpr GpuAddress BufferAddress::GetGpuAddress() { return { buffer->GetGpuAddress().vk_device_address + this->offset, }; }
    constexpr VkBuffer BufferAddress::GetVkBuffer() { return buffer->GetVkBuffer(); }
}

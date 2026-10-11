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

    class Buffer;

    static constexpr size_t cMaxVertexBufferAlignment = impl::Context::cTargetVertexBufferAlignment;
    static constexpr size_t cMaxIndexBufferAlignment = impl::Context::cTargetIndexBufferAlignment;
    static constexpr size_t cMaxUniformBufferAlignment = impl::Context::cTargetUniformBufferAlignment;
    static constexpr size_t cMaxStorageBufferAlignment = impl::Context::cTargetStorageBufferAlignment;

	class Buffer {
		private:
            VkBuffer    m_vk_buffer;
            GpuAddress  m_gpu_address;
            void       *m_address;
            MemoryPool *m_memory_pool;
            size_t      m_memory_offset;
		public:
			constexpr ALWAYS_INLINE  Buffer() : m_vk_buffer(VK_NULL_HANDLE), m_gpu_address(), m_address(), m_memory_pool(), m_memory_offset() {/*...*/}
            constexpr ALWAYS_INLINE ~Buffer() {/*...*/}

			Result Initialize(MemoryPool *memory_pool, size_t memory_offset, const BufferInfo *buffer_info);
			//Result Initialize(void *gpu_memory_address, const BufferInfo *buffer_info);
            void Finalize();

            constexpr void *GetAddress() const { return m_address; }
            void *GetAddress(size_t offset) const { return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_address) + offset); }
            constexpr GpuAddress GetGpuAddress() const { return m_gpu_address; }
            constexpr GpuAddress GetGpuAddress(size_t offset) const { return m_gpu_address + offset; }

            void FlushMappedRange(size_t offset, size_t size);
            void InvalidateMappedRange(size_t offset, size_t size);

            constexpr bool IsValid() const {
                return m_vk_buffer != VK_NULL_HANDLE;
            }
            constexpr uintptr_t GetMemoryPoolOffset() const {
                return m_memory_offset;
            }
		public:
            static GpuMemoryRequirements GetGpuMemoryRequirements(const BufferInfo *buffer_info);
        public:
            constexpr ALWAYS_INLINE VkBuffer GetVkBuffer() const { return m_vk_buffer; } 
			Result InitializeForDedicatedAllocation(const BufferInfo *buffer_info);
            void SetAddressForDedicatedAllocation(void *gpu_memory_address);
	};
    static_assert(sizeof(Buffer) <= cMaxGfxBufferSize);
}

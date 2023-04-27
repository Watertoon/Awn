#pragma once

namespace awn::gfx {

	class Buffer {
		private:
            VkBuffer         m_vk_buffer;
            GpuMemoryAddress m_buffer_gpu_memory_address;
		public:
			constexpr ALWAYS_INLINE Buffer() : m_vk_buffer(VK_NULL_HANDLE), m_buffer_gpu_memory_address() {/*...*/}
            constexpr ALWAYS_INLINE ~Buffer() {/*...*/}

			void Initialize(GpuMemoryAddress gpu_memory_address, BufferInfo *buffer_info) {
                m_buffer_gpu_memory_address = gpu_memory_address;
                m_vk_buffer                 = gpu_memory_address.CreateBuffer(buffer_info);
			}

            void Finalize() {
                
                /* Delete VkBuffer */
                if (m_vk_buffer != VK_NULL_HANDLE) {
                    ::pfn_vkDestroyBuffer(Context::GetInstance()->GetVkDevice(), m_vk_buffer, Context::GetInstance()->GetVkAllocationCallbacks());
                }
            }
	};
}

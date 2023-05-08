#pragma once

namespace awn::gfx {

	class Buffer {
		private:
            VkBuffer         m_vk_buffer;
            VkDeviceAddress  m_vk_device_address;
            GpuMemoryAddress m_buffer_gpu_memory_address;
		public:
			constexpr ALWAYS_INLINE Buffer() : m_vk_buffer(VK_NULL_HANDLE), m_vk_device_address(0), m_buffer_gpu_memory_address() {/*...*/}
            constexpr ALWAYS_INLINE ~Buffer() {/*...*/}

			void Initialize(GpuMemoryAddress gpu_memory_address, BufferInfo *buffer_info) {

                /* Set gpu address */
                m_buffer_gpu_memory_address = gpu_memory_address;

                /* Create vk buffer  */
                m_vk_buffer                 = gpu_memory_address.CreateBuffer(buffer_info);

                /* Query VkDeviceAddress */
                const VkBufferDeviceAddressInfo device_address_info = {
                    .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                    .buffer = m_vk_buffer
                    
                };
                m_vk_device_address = ::vkGetBufferDeviceAddress(Context::GetInstance()->GetVkDevice(), std::addressof(device_address_info));
			}

            void Finalize() {

                /* Delete VkBuffer */
                if (m_vk_buffer != VK_NULL_HANDLE) {
                    ::pfn_vkDestroyBuffer(Context::GetInstance()->GetVkDevice(), m_vk_buffer, Context::GetInstance()->GetVkAllocationCallbacks());
                }
                m_vk_buffer         = VK_NULL_HANDLE;
                m_vk_device_address = 0;
            }

            constexpr ALWAYS_INLINE GpuMemoryAddress GetGpuAddress()      const { return m_buffer_gpu_memory_address; } 
            constexpr ALWAYS_INLINE VkDeviceAddress  GetVkDeviceAddress() const { return m_vk_device_address; } 
            constexpr ALWAYS_INLINE VkBuffer         GetVkBuffer()        const { return m_vk_buffer; } 
	};
}

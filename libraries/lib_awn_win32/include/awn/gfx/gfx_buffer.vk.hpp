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
			//void Initialize(VkDeviceMemory device_memory, const BufferInfo *buffer_info) {
            //
            //    /* Create VkBuffer */
            //    VkBuffer buffer = 0;
            //    const VkBufferCreateInfo vk_buffer_info = {
            //        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            //        .size  = size,
            //        .usage = static_cast<u32>(usage_flags) | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            //    };
            //    const u32 result0 = ::pfn_vkCreateBuffer(Context::GetInstance()->GetVkDevice(), std::addressof(vk_buffer_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_buffer));
            //    VP_ASSERT(result0 == VK_SUCCESS);
            //
            //    /* Bind gpu memory allocation to buffer from offset */
            //    DeviceOffset device_offset = GpuHeapManager::GetInstance()->GetDeviceOffset(m_gpu_address, m_parent_gpu_heap->GetMemoryPropertyFlags());
            //    const u32 result1 = ::pfn_vkBindBufferMemory(Context::GetInstance()->GetVkDevice(), m_vk_buffer, device_offset.vk_device_address, device_offset.offset);
            //    VP_ASSERT(result1 == VK_SUCCESS);
            //
            //    /* Query VkDeviceAddress */
            //    const VkBufferDeviceAddressInfo device_address_info = {
            //        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            //        .buffer = m_vk_buffer
            //        
            //    };
            //    m_vk_device_address = ::vkGetBufferDeviceAddress(Context::GetInstance()->GetVkDevice(), std::addressof(device_address_info));
			//}

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
    static_assert(sizeof(Buffer) <= cMaxGfxBufferSize);
}

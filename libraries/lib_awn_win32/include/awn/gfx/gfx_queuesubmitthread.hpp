#pragma once

namespace awn::gfx {

	class QueueSubmitThread : public sys::ServiceThread {
		private:
            VkQueue           m_vk_queue;
			VkCommandPool     m_vk_command_pool;
            bool              m_is_back_buffer;
            VkCommandBuffer   m_vk_command_buffer[2];
            VkFence           m_vk_finish_fence;
            VkSemaphore       m_vk_finish_semaphore;
            QueueType         m_queue_type;
			sys::ServiceEvent m_finish_event;
		public:
            QueueSubmitThread(QueueType queue_type) : ServiceThread() {

                /* Create command pool */
                
                /* Allocate 2 resetable command buffers */
                
                /* Create fence for queue completion */
                
                /* Create semaphore for queue linkage */
                
                /* Select QueueType from Context */
            }

			virtual void UserMain() {

				/* Submit queue */
                
                /* Wait for queue completion */
                
                /* Signal queue completion event */
                
			}

            void CallCommands(CommandList command_list) {

                /* Call secondary commands */
            }

            void SubmitQueue() {

                /* Wait for current submission completion to finish */
                
                /* Reset processed command buffer */

                /* Clear completion event */
                

                /* Send thread a submission message */
                

                /* Swap selected command buffer */
                
            }
            
            void WaitForQueueCompletion() {

                /* Wait for current submission completion to finish */
                
            }
            
            constexpr ALWAYS_INLINE VkFence     GetVkFence()     const { return m_vk_finish_fence; }
            constexpr ALWAYS_INLINE VkSemaphore GetVkSemaphore() const { return m_vk_finish_semaphore; }
	};
}

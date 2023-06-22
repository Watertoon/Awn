#pragma once

namespace awn::sys {

    class MessageQueue {
        private:
            size_t                           *m_message_buffer;
            s32                               m_max_messages;
            s32                               m_pending_messages;
            s32                               m_current_message;
            ukern::InternalCriticalSection    m_message_mutex;
            ukern::InternalConditionVariable  m_message_sent_cv;
            ukern::InternalConditionVariable  m_buffer_full_cv;
        private:
            ALWAYS_INLINE void WaitForMessage() {

                /* Wait on condition variable until we get a message */
                while(m_pending_messages == 0) {
                    m_message_sent_cv.Wait(std::addressof(m_message_mutex));
                }
            }

            ALWAYS_INLINE void WaitForMessageClear() {

                /* Wait on condition variable until we have space for our message */
                while(m_max_messages <= m_pending_messages) {
                    m_buffer_full_cv.Wait(std::addressof(m_message_mutex));
                }
            }

            ALWAYS_INLINE void SendMessageImpl(size_t message) {

                /* Calculate our messages buffer index */
                s32 index = m_pending_messages + m_current_message;
                if(m_max_messages <= index) {
                    index = index - m_max_messages;
                }

                /* Add message */
                m_message_buffer[index] = message;
                m_pending_messages += 1;
                m_message_sent_cv.Broadcast();
            }

            ALWAYS_INLINE void RecieveMessageImpl(size_t *out_message) {

                /* Pull our current message */
                *out_message = m_message_buffer[m_current_message];
                m_current_message += 1;
                m_pending_messages -= 1;
                const u32 offset = (m_max_messages <= m_current_message) ? m_max_messages : 0;
                m_current_message = m_current_message - offset;
                m_buffer_full_cv.Broadcast();
            }
        public:
            constexpr MessageQueue() : m_message_buffer(nullptr), m_max_messages(0), m_pending_messages(0), m_current_message(0), m_message_mutex(), m_message_sent_cv(), m_buffer_full_cv() {/*...*/}
            constexpr ~MessageQueue() {/*...*/}

            void Initialize(s32 max_message_count) {

                /* Allocate message buffer */
                m_message_buffer = new size_t[max_message_count];
                VP_ASSERT(m_message_buffer != nullptr);
                
                m_max_messages = max_message_count;
            }
            void Initialize(mem::Heap *heap, s32 max_message_count) {

                /* Allocate message buffer */
                m_message_buffer = new (heap, 8) size_t[max_message_count];
                VP_ASSERT(m_message_buffer != nullptr);
                
                m_max_messages = max_message_count;
            }

            void Finalize() {

                delete[] m_message_buffer;
                m_message_buffer = nullptr;
            }

            void ReceiveMessage(size_t *out_message) {
                std::scoped_lock l(m_message_mutex);

                this->WaitForMessage();

                this->RecieveMessageImpl(out_message);
            }

            bool TryReceiveMessage(size_t *out_message) {
                std::scoped_lock l(m_message_mutex);

                if (m_pending_messages == 0) { return false; }

                this->RecieveMessageImpl(out_message);

                return true;
            }

            void SendMessage(size_t message) {
                std::scoped_lock l(m_message_mutex);

                this->WaitForMessageClear();

                this->SendMessageImpl(message);
            }

            bool TrySendMessage(size_t message) {
                std::scoped_lock l(m_message_mutex);

                if (m_max_messages <= m_pending_messages) { return false; }

                this->SendMessageImpl(message);

                return true;
            }
    };
}

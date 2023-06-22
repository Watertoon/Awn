#pragma once

namespace awn::sys {

    class ServiceMessageQueue {
        private:
            size_t              *m_message_buffer;
            s32                  m_max_messages;
            s32                  m_pending_messages;
            s32                  m_current_message;
            vp::util::BusyMutex  m_message_busy_mutex;
            u32                  m_sent_wait_value;
            u32                  m_full_wait_value;
        private:
            ALWAYS_INLINE void WaitForMessage() {

                /* Wait on condition variable until we get a message */
                while(m_pending_messages == 0) {
                    m_message_busy_mutex.Leave();
                    if (m_sent_wait_value != 1) {
                        ::InterlockedExchange(std::addressof(m_sent_wait_value), 1);
                    }
                    if (::IsThreadAFiber() == false) {
                        u32 wait_value = 1;
                        ::WaitOnAddress(std::addressof(m_sent_wait_value), std::addressof(wait_value), sizeof(u32), INFINITE);
                    } else {
                        ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_sent_wait_value)), ukern::ArbitrationType_WaitIfEqual, 1, 0xffff'ffff'ffff'ffff);
                    }
                    m_message_busy_mutex.Enter();
                }

                return;
            }

            ALWAYS_INLINE void WaitForMessageClear() {

                /* Wait on condition variable until we have space for our message */
                while(m_max_messages <= m_pending_messages) {
                    if (m_full_wait_value != 1) {
                        ::InterlockedExchange(std::addressof(m_full_wait_value), 1);
                    }
                    m_message_busy_mutex.Leave();
                    if (::IsThreadAFiber() == false) {
                        u32 wait_value = 0;
                        ::WaitOnAddress(std::addressof(m_full_wait_value), std::addressof(wait_value), sizeof(u32), INFINITE);
                    } else {
                        ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_full_wait_value)), ukern::ArbitrationType_WaitIfEqual, 1, 0xffff'ffff'ffff'ffff);
                    }
                    m_message_busy_mutex.Enter();
                }

                return;
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

                /* Signal */
                ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_sent_wait_value)), ukern::SignalType_SignalAndIncrementIfEqual, 1, 0xffff'ffff);
                ::WakeByAddressAll(std::addressof(m_sent_wait_value));

                return;
            }

            ALWAYS_INLINE void RecieveMessageImpl(size_t *out_message) {

                /* Pull our current message */
                *out_message        = m_message_buffer[m_current_message];
                m_current_message  += 1;
                m_pending_messages -= 1;
                const u32 offset    = (m_max_messages <= m_current_message) ? m_max_messages : 0;
                m_current_message   = m_current_message - offset;

                /* Signal */
                ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_full_wait_value)), ukern::SignalType_SignalAndIncrementIfEqual, 1, 0xffff'ffff);
                ::WakeByAddressAll(std::addressof(m_full_wait_value));

                return;
            }
        public:
            constexpr  ServiceMessageQueue() : m_message_buffer(nullptr), m_max_messages(0), m_pending_messages(0), m_current_message(0), m_message_busy_mutex{}, m_sent_wait_value(0), m_full_wait_value(0) {/*...*/}
            constexpr ~ServiceMessageQueue() {/*...*/}

            void Initialize(s32 max_message_count) {

                /* Allocate message buffer */
                m_message_buffer = new size_t[max_message_count];
                VP_ASSERT(m_message_buffer != nullptr);

                m_max_messages = max_message_count;

                return;
            }
            void Initialize(mem::Heap *heap, s32 max_message_count) {

                /* Allocate message buffer */
                m_message_buffer = new (heap, 8) size_t[max_message_count];
                VP_ASSERT(m_message_buffer != nullptr);
    
                m_max_messages = max_message_count;

                return;
            }

            void Finalize() {

                /* Free and clear state */
                if (m_message_buffer != nullptr) {
                    delete [] m_message_buffer;
                }
                m_message_buffer   = nullptr;
                m_max_messages     = 0;
                m_pending_messages = 0;
                m_current_message  = 0;

                return;
            }

            void ReceiveMessage(size_t *out_message) {
                vp::util::ScopedBusyMutex l(std::addressof(m_message_busy_mutex));

                this->WaitForMessage();

                this->RecieveMessageImpl(out_message);

                return;
            }

            bool TryReceiveMessage(size_t *out_message) {
                vp::util::ScopedBusyMutex l(std::addressof(m_message_busy_mutex));

                if (m_pending_messages == 0) { return false; }

                this->RecieveMessageImpl(out_message);

                return true;
            }

            void SendMessage(size_t message) {
                vp::util::ScopedBusyMutex l(std::addressof(m_message_busy_mutex));

                this->WaitForMessageClear();

                this->SendMessageImpl(message);

                return;
            }

            bool TrySendMessage(size_t message) {
                vp::util::ScopedBusyMutex l(std::addressof(m_message_busy_mutex));

                if (m_max_messages <= m_pending_messages) { return false; }

                this->SendMessageImpl(message);

                return true;
            }
    };
}

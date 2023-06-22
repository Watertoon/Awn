#pragma once

namespace awn::sys {

    class Mutex {
        private:
            ukern::InternalCriticalSection m_cs;
            u32                            m_lock_count;
        public:
            constexpr ALWAYS_INLINE Mutex() : m_cs(), m_lock_count(0) {/*...*/}
            constexpr ~Mutex() {/*...*/}

            void Enter() {

                /* Defer to the internal cs if we are not the owner */
                if (m_lock_count == 0 && m_cs.IsLockedByCurrentThread() == false) {
                    m_cs.Enter();
                }

                /* Increment count as the owner */
                m_lock_count  = m_lock_count + 1;
            }
            bool TryEnter() {

                /* Defer to the internal cs if we are not the owner */
                if (m_lock_count == 0 || m_cs.IsLockedByCurrentThread() == false || m_cs.TryEnter() == false) {
                    return false;
                }

                /* Increment count as the owner */
                m_lock_count  = m_lock_count + 1;
                return true;
            }
            void Leave() {

                /* Decrement count */
                m_lock_count  = m_lock_count - 1;
                if (m_lock_count != 0) {
                    return;
                }

                /* Leave internal cs when count drops to 0 */
                m_cs.Leave();
            }

            void lock() {
                this->Enter();
            }
            void unlock() {
                this->Leave();
            }
            bool try_lock() {
                return this->TryEnter();
            }

            bool IsLockedByCurrentThread() {
                return m_cs.IsLockedByCurrentThread();
            }
    };
}

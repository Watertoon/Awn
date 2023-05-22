#pragma once

namespace awn::sys {

    class Event {
        private:
            u16                              m_signal_state;
            u16                              m_auto_reset_mode;
            u32                              m_wake_id;
            ukern::InternalCriticalSection   m_cs;
            ukern::InternalConditionVariable m_cv;
        public:
            constexpr ALWAYS_INLINE Event() : m_signal_state(), m_auto_reset_mode(), m_wake_id(), m_cs(), m_cv() {/*...*/}
            constexpr ALWAYS_INLINE Event(bool create_signaled, bool auto_reset) : m_signal_state(create_signaled), m_auto_reset_mode(auto_reset) {/*...*/}

            void Signal() {
                std::scoped_lock lock(m_cs);

                /* If we auto reset, we can only signal one waiter */
                if (m_auto_reset_mode == 1) {
                    m_cv.Signal();
                } else {
                    /* Increment wait id so all previous waiters wake even if the signal is reset */
                    ++m_wake_id;
                    m_cv.Broadcast();
                }
            }

            void Clear() {
                std::scoped_lock lock(m_cs);
                m_signal_state = 0;
            }

            void Wait() {
                std::scoped_lock lock(m_cs);

                /* Bail if already signaled */
                if (m_signal_state == 1) { return; }

                /* Wait until wake id changes or the event is signaled */
                const u32 wake_id = m_wake_id;
                do {
                    m_cv.Wait(std::addressof(m_cs));
                } while (wake_id == m_wake_id && m_signal_state == 0);

                /* Clear signal if auto reset is enabled */
                if (m_auto_reset_mode == 1) {
                    m_signal_state = 0;
                }

                return;
            }

            void TimedWait([[maybe_unused]] TimeSpan timeout) {
                VP_ASSERT(false);
            }
    };
}

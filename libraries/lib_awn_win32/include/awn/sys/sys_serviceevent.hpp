#pragma once

namespace awn::sys {

    /* This is a variant meant to synchronize ukern threads with win32 service threads */
    class ServiceEvent {
        private:
            u32    m_signal_state;
            HANDLE m_win32_event_handle;
        public:
            constexpr ServiceEvent() : m_signal_state(0), m_win32_event_handle(INVALID_HANDLE_VALUE) {/*...*/}
            constexpr ~ServiceEvent() {/*...*/}

            void Initialize() {
                m_win32_event_handle = ::CreateEvent(nullptr, true, false, nullptr);
            }

            void Finalize() {
                ::CloseHandle(m_win32_event_handle);
            }

            void Wait() {

                /* Check signal state */
                if (m_signal_state == 1) { return; }

                /* Respective waits */
                if (::IsThreadAFiber() == true) {
                    ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_signal_state)), ukern::ArbitrationType_WaitIfLessThan, 1, vp::TimeSpan::cMaxTime);
                } else {
                    ::WaitForSingleObject(m_win32_event_handle, INFINITE);
                }
            }

            void Signal() {
                m_signal_state = 1;
                ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_signal_state)), ukern::SignalType_Signal, 1, -1);
                ::SetEvent(m_win32_event_handle);
            }

            void Clear() {
                ::ResetEvent(m_win32_event_handle);
                m_signal_state = 0;
            }
    };
}

#pragma once

namespace awn::sys {

    class ServiceEvent {
        private:
            u32    m_signal_state;
            HANDLE m_win32_event_handle;
        public:
            constexpr ServiceEvent() : m_signal_state(0), m_win32_event_handle(INVALID_HANDLE_VALUE) {/*...*/}
            constexpr ~ServiceEvent() {/*...*/}

            void Initialize() {
                m_win32_event_handle = ::CreateEvent(nullptr, true, false, nullptr);
                VP_ASSERT(m_win32_event_handle != nullptr);
            }

            void Finalize() {
                ::CloseHandle(m_win32_event_handle);
                m_signal_state = 0;
            }

            void Wait() {

                /* Check signal state */
                if (m_signal_state == 1) { return; }

                /* Respective waits */
                if (::IsThreadAFiber() == true) {
                    RESULT_ABORT_UNLESS(ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_signal_state)), ukern::ArbitrationType_WaitIfLessThan, 1, vp::TimeSpan::cMaxTime));
                } else {
                    const u32 result = ::WaitForSingleObject(m_win32_event_handle, INFINITE);
                    VP_ASSERT(result == WAIT_OBJECT_0);
                }

                return;
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

            bool TimedWait(TimeSpan timeout) {

                /* Check signal state */
                if (m_signal_state == 1) { return true; }

                /* Respective waits */
                if (::IsThreadAFiber() == true) {
                    const Result result = ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_signal_state)), ukern::ArbitrationType_WaitIfLessThan, 1, timeout.GetNanoSeconds());
                    if (result == ukern::ResultTimeout) { return false; }
                    RESULT_ABORT_UNLESS(result);
                    return true;
                }

                const u32 result = ::WaitForSingleObject(m_win32_event_handle, timeout.GetMilliSeconds());
                VP_ASSERT(result == WAIT_TIMEOUT || result == WAIT_OBJECT_0);
                return result != WAIT_TIMEOUT;
            }
    };
}

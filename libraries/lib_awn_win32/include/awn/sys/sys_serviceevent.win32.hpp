/*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

namespace awn::sys {

    class ServiceEvent {
        private:
            u32    m_signal_state;
            u32    m_auto_reset_mode;
            HANDLE m_win32_event_handle;
        public:
            constexpr ServiceEvent() : m_signal_state(0), m_auto_reset_mode(0), m_win32_event_handle(INVALID_HANDLE_VALUE) {/*...*/}
            constexpr ~ServiceEvent() {/*...*/}

            void Initialize(SignalState signal_state, ResetMode reset_mode) {
                m_signal_state       = static_cast<bool>(signal_state);
                m_auto_reset_mode    = static_cast<bool>(reset_mode);
                m_win32_event_handle = ::CreateEvent(nullptr, static_cast<bool>(reset_mode) ^ 1, static_cast<bool>(signal_state), nullptr);
                VP_ASSERT(m_win32_event_handle != nullptr);
            }

            void Finalize() {
                ::CloseHandle(m_win32_event_handle);
                m_signal_state = 0;
            }

            void Wait() {

                /* Check signal state */
                if (m_signal_state == 1) { 
                    if (m_auto_reset_mode == false) {
                        return;
                    } else if (this->Clear() == true) { 
                        return; 
                    }
                }

                /* UKern impl */
                bool is_acquired_for_auto_reset = true;
                if (::IsThreadAFiber() == true) {
                    do {
                        const Result result = ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_signal_state)), ukern::ArbitrationType_WaitIfLessThan, 1, vp::TimeSpan::cMaxTime);
                        VP_ASSERT(result == ResultSuccess || result == ukern::ResultInvalidWaitAddressValue);
                        if (m_auto_reset_mode == true) {
                            is_acquired_for_auto_reset = this->Clear();
                        }
                    } while (is_acquired_for_auto_reset == false);

                    return;
                }

                /* Service thread impl */
                do {
                    const u32 result = ::WaitForSingleObject(m_win32_event_handle, INFINITE);
                    VP_ASSERT(result == WAIT_OBJECT_0);       
                    if (m_auto_reset_mode == true) {
                        u32 last_signal_state = 0;
                        is_acquired_for_auto_reset = vp::util::InterlockedCompareExchange(std::addressof(last_signal_state), std::addressof(m_signal_state), 0u, 1u); 
                    }
                } while (is_acquired_for_auto_reset == false);

                return;
            }

            bool Signal() {

                /* Attempt to set the signal */
                u32 last_signal_state = 0;
                const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(last_signal_state), std::addressof(m_signal_state), 1u, 0u);
                if (result == false) { return false; }

                /* If the signal is signaled, unblock waiters and set the OS event */
                ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_signal_state)), ukern::SignalType_Signal, 1, (m_auto_reset_mode == true) ? 1 : -1);
                ::SetEvent(m_win32_event_handle);

                return true;
            }

            bool Clear() {

                /* Attempt to clear the signal */
                u32 last_signal_state = 0;
                const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(last_signal_state), std::addressof(m_signal_state), 0u, 1u);
                if (result == false) { return false; }

                /* If the signal was cleared, clear the OS event */
                ::ResetEvent(m_win32_event_handle);

                return true;
            }

            bool TimedWait(TimeSpan timeout) {

                /* Auto reset is not currently supported for timed waits */
                VP_ASSERT(m_auto_reset_mode == false);

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

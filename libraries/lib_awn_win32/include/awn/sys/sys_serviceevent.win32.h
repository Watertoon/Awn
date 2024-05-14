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
            bool m_is_auto_reset;
            u32  m_state;
        private:
            ALWAYS_INLINE Result WaitImpl(u32 last) {

                if (last != 1) { return ukern::ResultInvalidWaitAddressValue; }

                if (::IsThreadAFiber() == false) {
                    u32 value = 1;
                    do {
                        ::WaitOnAddress(std::addressof(m_state), std::addressof(value), sizeof(u32), INFINITE);
                    } while (last == value);
                    return ukern::ResultInvalidWaitAddressValue;
                }

                return ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_state)), ukern::ArbitrationType_WaitIfEqual, 1, TimeSpan::cMaxTime);
            }
            ALWAYS_INLINE Result WaitTimeoutImpl(s64 timeout_tick, u32 last) {

                if (last != 1) { return ukern::ResultInvalidWaitAddressValue; }

                TimeSpan time_left = TimeSpan::GetTimeLeftOnTarget(timeout_tick);
                if (::IsThreadAFiber() == false) {
                    u32 state;
                    u32 value = 1;
                    do {
                        const bool result = ::WaitOnAddress(std::addressof(m_state), std::addressof(value), sizeof(u32), time_left.GetMilliSeconds());
                        if (result == false && ::GetLastError() == ERROR_TIMEOUT) { time_left = TimeSpan::GetTimeLeftOnTarget(timeout_tick); }
                    } while (m_state == value);
                    return ukern::ResultInvalidWaitAddressValue;
                }

                return ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_state)), ukern::ArbitrationType_WaitIfEqual, 1, time_left.GetNanoSeconds());
            }
        public:
            constexpr ServiceEvent() : m_is_auto_reset(), m_state() {/*...*/}
            constexpr ~ServiceEvent() {/*...*/}

            constexpr void Initialize(SignalState signal_state, ResetMode reset_mode) {
                m_is_auto_reset = static_cast<bool>(reset_mode);
                m_state         = (signal_state == SignalState::Signaled) ? 2 : 0;
            }

            constexpr void Finalize() {
                m_is_auto_reset = 0;
                m_state           = 0;
            }

            void Wait();
            void Signal();
            void Clear();

            bool TimedWait(TimeSpan timeout);
    };
}

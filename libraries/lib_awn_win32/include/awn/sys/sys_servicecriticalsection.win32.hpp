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

    class ThreadBase;

    sys::ThreadBase *GetCurrentThread();

    class ServiceCriticalSection {
        private:
            u32              m_wait_value;
            sys::ThreadBase *m_owner;
            SRWLOCK          m_win32_lock;
        public:
            constexpr ALWAYS_INLINE ServiceCriticalSection() : m_wait_value(0), m_owner(nullptr), m_win32_lock(0) {/*...*/}
            constexpr ALWAYS_INLINE ~ServiceCriticalSection() {/*...*/}

            void Enter() {

                /* Service thread impl */
                sys::ThreadBase *thread = sys::GetCurrentThread();
                if (::IsThreadAFiber() == false) {
                    ::AcquireSRWLockExclusive(std::addressof(m_win32_lock));
                    m_owner = thread;
                    return;
                }

                /* Ukern thread impl */
                for(;;) {

                    /* Try acquire ownership atomically */
                    if (m_owner == nullptr) {
                        ::AcquireSRWLockExclusive(std::addressof(m_win32_lock));
                        m_owner = thread;
                        return;
                    }

                    /* Fallback wait */
                    ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::ArbitrationType_WaitIfEqual, 0, -1);
                }
            }

            void Leave() {
                m_owner = nullptr;
                ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::SignalType_Signal, 0, 1);
                ::ReleaseSRWLockExclusive(std::addressof(m_win32_lock));
            }

            ALWAYS_INLINE void lock()   { this->Enter(); }
            ALWAYS_INLINE void unlock() { this->Leave(); }
    };

    class ServiceMutex {
        public:
            static constexpr u32 cInvalidLockCount = 0xffff'ffff;
        private:
            u32               m_wait_value;
            u32               m_lock_count;
            sys::ThreadBase  *m_owner;
            CRITICAL_SECTION  m_win32_lock;
        public:
            constexpr ALWAYS_INLINE  ServiceMutex() : m_wait_value(0), m_lock_count(-1), m_owner(nullptr), m_win32_lock(0) {/*...*/}
            constexpr ALWAYS_INLINE ~ServiceMutex() {/*...*/}

            void Initialize() {
                ::InitializeCriticalSection(std::addressof(m_win32_lock));
                m_lock_count = 0;
            }
            void Finalize() {
                if (m_lock_count == cInvalidLockCount) { return; }
                ::DeleteCriticalSection(std::addressof(m_win32_lock));
                m_lock_count = cInvalidLockCount;
            }

            void Enter() {

                /* Service thread impl */
                sys::ThreadBase *thread = sys::GetCurrentThread();
                if (::IsThreadAFiber() == false) {
                    ::EnterCriticalSection(std::addressof(m_win32_lock));
                    m_owner = thread;
                    ++m_lock_count;
                    return;
                }

                /* Ukern thread impl */
                for(;;) {

                    /* Try acquire ownership atomically */
                    sys::ThreadBase *last_thread = m_owner;

                    /* Acquire win32 lock */
                    if (last_thread == nullptr || last_thread == thread) {
                        ::EnterCriticalSection(std::addressof(m_win32_lock));
                        m_owner = thread;
                        ++m_lock_count;
                        return;
                    }

                    /* Fallback wait */
                    ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::ArbitrationType_WaitIfEqual, 0, -1);
                }
            }

            void Leave() {
                --m_lock_count;
                const u32 lock_count = m_lock_count;
                if (lock_count == 0) {
                    m_owner = nullptr;
                    ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::SignalType_Signal, 0, 1);
                }
                ::LeaveCriticalSection(std::addressof(m_win32_lock));
            }

            ALWAYS_INLINE void lock()   { this->Enter(); }
            ALWAYS_INLINE void unlock() { this->Leave(); }
    };
}

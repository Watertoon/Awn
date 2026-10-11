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

    class ConditionVariable;

    class Mutex {
        public:
            friend class ConditionVariable;
        private:
            SRWLOCK m_cs;
            u32 m_lock_count;
            u32 m_thread_id;
        public:
            constexpr Mutex() : m_cs(SRWLOCK_INIT), m_lock_count(), m_thread_id() {/*...*/}
            constexpr ~Mutex() = default;

            void Enter() {
                if (!this->IsLockedByCurrentThread()) {
                    ::AcquireSRWLockExclusive(std::addressof(m_cs));
                    m_thread_id = ::GetCurrentThreadId();
                    //m_cs.Enter();
                }
                const u32 count = m_lock_count;
                m_lock_count = count + 1;
                VP_ASSERT(count != 0xffff'ffff);
                return;
            }
            bool TryEnter() {
                if (!this->IsLockedByCurrentThread() && !::TryAcquireSRWLockExclusive(std::addressof(m_cs))) { return false; }
                m_thread_id = ::GetCurrentThreadId();
                const u32 count = m_lock_count;
                m_lock_count = count + 1;
                VP_ASSERT(count != 0xffff'ffff);
                return true;
            }
            void Leave() {
                const u32 n_count = m_lock_count - 1;
                m_lock_count = n_count;
                if (n_count == 0) {
                    m_thread_id = 0;
                    ::ReleaseSRWLockExclusive(std::addressof(m_cs));
                    //m_cs.Leave();
                }
                return;
            }

            ALWAYS_INLINE void lock() {
                this->Enter();
            }
            ALWAYS_INLINE void unlock() {
                this->Leave();
            }
            ALWAYS_INLINE bool try_lock() {
                return this->TryEnter();
            }

            bool IsLockedByCurrentThread() { return m_thread_id == ::GetCurrentThreadId(); }
    };

    class SlimMutex {
        public:
            friend class SlimConditionVariable;
        private:
            SRWLOCK m_cs;
            u32 m_thread_id;
        public:
            constexpr SlimMutex() : m_cs(SRWLOCK_INIT), m_thread_id() {/*...*/}
            constexpr ~SlimMutex() = default;

            void Enter() {
                VP_ABORT_UNLESS(!this->IsLockedByCurrentThread());
                //m_cs.Enter();
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                m_thread_id = ::GetCurrentThreadId();
                return;
            }
            bool TryEnter() {
                VP_ABORT_UNLESS(!this->IsLockedByCurrentThread());
                if (!::TryAcquireSRWLockExclusive(std::addressof(m_cs))) { return false; }
                m_thread_id = ::GetCurrentThreadId();
                return true;
            }
            void Leave() {
                //m_cs.Leave();
                m_thread_id = 0;
                ::ReleaseSRWLockExclusive(std::addressof(m_cs));
                return;
            }

            ALWAYS_INLINE void lock() {
                this->Enter();
            }
            ALWAYS_INLINE void unlock() {
                this->Leave();
            }
            ALWAYS_INLINE bool try_lock() {
                return this->TryEnter();
            }

            bool IsLockedByCurrentThread() { return m_thread_id == ::GetCurrentThreadId(); }
    };
}

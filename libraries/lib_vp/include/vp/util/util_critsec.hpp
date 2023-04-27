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

namespace vp::util {

    class CriticalSection {
        public:
            friend class ConditionVariable;
        private:
            SRWLOCK m_srwlock;
            size_t  m_locked_thread_id;
        private:
            size_t UnsetId() {
                const size_t id = ::InterlockedExchange64(reinterpret_cast<volatile LONG64*>(std::addressof(m_locked_thread_id)), 0);
                return id;
            }

            void SetId(size_t manual_id) {
                ::InterlockedExchange64(reinterpret_cast<volatile LONG64*>(std::addressof(m_locked_thread_id)), manual_id);
            }
        public:
            constexpr ALWAYS_INLINE CriticalSection() : m_srwlock{0}, m_locked_thread_id(0) {/*...*/}

            void lock() {
                DD_ASSERT(IsLockedByCurrentThread() == false);
                ::AcquireSRWLockExclusive(std::addressof(m_srwlock));
                ::InterlockedExchange64(reinterpret_cast<volatile LONG64*>(std::addressof(m_locked_thread_id)), static_cast<size_t>(::GetCurrentThreadId()));
            }

            void unlock() {
                DD_ASSERT(IsLockedByCurrentThread() == true);
                ::InterlockedExchange64(reinterpret_cast<volatile LONG64*>(std::addressof(m_locked_thread_id)), 0);
                ::ReleaseSRWLockExclusive(std::addressof(m_srwlock));
            }

            bool try_lock() {
                const bool result = ::TryAcquireSRWLockExclusive(std::addressof(m_srwlock));
                if (result == true) {
                    ::InterlockedExchange64(reinterpret_cast<volatile LONG64*>(std::addressof(m_locked_thread_id)), static_cast<size_t>(::GetCurrentThreadId()));
                }
                return result;
            }

            void Enter() {
                this->lock();
            }

            void Leave() {
                this->unlock();
            }

            bool TryEnter() {
                return this->try_lock();
            }

            bool IsLockedByCurrentThread() {
                const size_t thread_id = static_cast<size_t>(::GetCurrentThreadId());
                return thread_id == ::InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(std::addressof(m_locked_thread_id)), m_locked_thread_id, thread_id);
            }

            SRWLOCK *GetSRWLOCK() { return std::addressof(m_srwlock); }
    };
}

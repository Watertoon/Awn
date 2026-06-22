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
            impl::InternalCriticalSection m_cs;
            u32 m_lock_count;
        public:
            constexpr Mutex() : m_cs(), m_lock_count() {/*...*/}
            constexpr ~Mutex() {/*...*/}

            void Enter() {
                if (!m_cs.IsLockedByCurrentThread()) {
                    m_cs.Enter();
                }
                const u32 count = m_lock_count;
                m_lock_count = count + 1;
                VP_ASSERT(count != 0xffff'ffff);
                return;
            }
            bool TryEnter() {
                if (!m_cs.IsLockedByCurrentThread() && !m_cs.TryEnter()) { return false; }
                const u32 count = m_lock_count;
                m_lock_count = count + 1;
                VP_ASSERT(count != 0xffff'ffff);
                return true;
            }
            void Leave() {
                const u32 n_count = m_lock_count - 1;
                m_lock_count = n_count;
                if (n_count == 0) {
                    m_cs.Leave();
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

            bool IsLockedByCurrentThread() { return m_cs.IsLockedByCurrentThread(); }
    };
}

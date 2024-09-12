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

    class Mutex {
        private:
            SRWLOCK          m_cs;
            sys::ThreadBase *m_owner;
            u32              m_lock_count;
        public:
            constexpr ALWAYS_INLINE Mutex() : m_cs(SRWLOCK_INIT), m_owner(), m_lock_count() {/*...*/}
            constexpr ~Mutex() {/*...*/}

            void Enter();
            bool TryEnter();
            void Leave();

            void lock() {
                this->Enter();
            }
            void unlock() {
                this->Leave();
            }
            bool try_lock() {
                return this->TryEnter();
            }

            ALWAYS_INLINE bool IsLockedByCurrentThread() {
                sys::ThreadBase *thread = GetCurrentThread();
                return thread == m_owner;
            }
    };
}

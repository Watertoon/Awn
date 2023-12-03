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

    class Mutex {
        private:
            ukern::InternalCriticalSection m_cs;
            u32                            m_lock_count;
        public:
            constexpr ALWAYS_INLINE Mutex() : m_cs(), m_lock_count(0) {/*...*/}
            constexpr ~Mutex() {/*...*/}

            void Enter() {

                /* Defer to the internal cs if this thread is not the owner */
                if (m_cs.IsLockedByCurrentThread() == false) {
                    m_cs.Enter();
                }

                /* Increment count as the owner */
                m_lock_count  = m_lock_count + 1;

                return;
            }
            bool TryEnter() {

                /* Defer to the internal cs if this thread is not the owner */
                if (m_cs.IsLockedByCurrentThread() == false && m_cs.TryEnter() == false) { return false; }

                /* Increment count as the owner */
                m_lock_count  = m_lock_count + 1;

                return true;
            }
            void Leave() {

                /* Decrement count */
                m_lock_count  = m_lock_count - 1;
                if (m_lock_count != 0) { return; }

                /* Leave internal cs when count drops to 0 */
                m_cs.Leave();

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

            ALWAYS_INLINE bool IsLockedByCurrentThread() {
                return m_cs.IsLockedByCurrentThread();
            }
    };
}

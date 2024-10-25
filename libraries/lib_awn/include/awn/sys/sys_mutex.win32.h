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
            constexpr ALWAYS_INLINE Mutex() : m_cs(), m_lock_count() {/*...*/}
            constexpr ~Mutex() {/*...*/}

            void Enter();
            bool TryEnter();
            void Leave();

            ALWAYS_INLINE void lock() {
                this->Enter();
            }
            ALWAYS_INLINE void unlock() {
                this->Leave();
            }
            ALWAYS_INLINE bool try_lock() {
                return this->TryEnter();
            }

            bool IsLockedByCurrentThread();
        private:
            constexpr impl::InternalCriticalSection *GetInternalCriticalSection() { return std::addressof(m_cs); }
    };
}

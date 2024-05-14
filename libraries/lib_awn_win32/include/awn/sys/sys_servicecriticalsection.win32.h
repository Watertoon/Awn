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
        public:
            constexpr ALWAYS_INLINE ServiceCriticalSection() : m_wait_value(0), m_owner(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE ~ServiceCriticalSection() {/*...*/}

            void Enter();
            bool TryEnter();

            void Leave();

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
        public:
            constexpr ALWAYS_INLINE  ServiceMutex() : m_wait_value(0), m_lock_count(0), m_owner(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE ~ServiceMutex() {/*...*/}

            void Enter();
            bool TryEnter();

            void Leave();

            ALWAYS_INLINE void lock()   { this->Enter(); }
            ALWAYS_INLINE void unlock() { this->Leave(); }
    };
}

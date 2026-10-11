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

    template <typename T>
    class MessageQueue {
        private:
            using MessageRing = vp::util::RingBuffer<T>;
        private:
            MessageRing         m_ring;
            SRWLOCK             m_cs;
            CONDITION_VARIABLE  m_recv_cv;
            CONDITION_VARIABLE  m_send_cv;
        public:
            constexpr  MessageQueue() : m_ring(), m_cs(SRWLOCK_INIT), m_recv_cv(CONDITION_VARIABLE_INIT), m_send_cv(CONDITION_VARIABLE_INIT) {/*...*/}
            constexpr ~MessageQueue() {/*...*/}

            Result Initialize(mem::Heap *heap, s32 max_message_count) {
                RESULT_RETURN(m_ring.Initialize(heap,max_message_count));
            }
            void Finalize() {
                m_ring.Finalize();
            }

            constexpr void SetMemory(T *memory, s32 max_message_count) {
                m_ring.SetMemory(memory, max_message_count);
            }
        public:
            void ReceiveMessage(T *out_message) {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                ON_SCOPE_EXIT { ::ReleaseSRWLockExclusive(std::addressof(m_cs)); };
                while (m_ring.IsEmpty()) {
                    ::SleepConditionVariableSRW(std::addressof(m_send_cv), std::addressof(m_cs), INFINITE, 0);
                }
                *out_message = m_ring.RemoveFront();
                ::WakeAllConditionVariable(std::addressof(m_recv_cv));
                return;
            }
            bool TryReceiveMessage(T *out_message) {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                ON_SCOPE_EXIT { ::ReleaseSRWLockExclusive(std::addressof(m_cs)); };
                if (m_ring.IsEmpty()) { return false; }
                *out_message = m_ring.RemoveFront();
                ::WakeAllConditionVariable(std::addressof(m_recv_cv));
                return true;
            }
            bool TryPeekMessage(T *out_message) {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                ON_SCOPE_EXIT { ::ReleaseSRWLockExclusive(std::addressof(m_cs)); };
                if (m_ring.IsEmpty()) { return false; }
                *out_message = m_ring.PeekFront();
                return true;
            }

            void SendMessage(T message) {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                ON_SCOPE_EXIT { ::ReleaseSRWLockExclusive(std::addressof(m_cs)); };
                while (m_ring.IsFull()) {
                    ::SleepConditionVariableSRW(std::addressof(m_recv_cv), std::addressof(m_cs), INFINITE, 0);
                }
                m_ring.PushBack(message);
                ::WakeAllConditionVariable(std::addressof(m_send_cv));
                return;
            }
            bool TrySendMessage(T message) {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                ON_SCOPE_EXIT { ::ReleaseSRWLockExclusive(std::addressof(m_cs)); };
                if (m_ring.IsFull()) { return false; }
                m_ring.PushBack(message);
                ::WakeAllConditionVariable(std::addressof(m_send_cv));
                return true;
            }

            void JamMessage(T message) {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                ON_SCOPE_EXIT { ::ReleaseSRWLockExclusive(std::addressof(m_cs)); };
                while (m_ring.IsFull()) {
                    ::SleepConditionVariableSRW(std::addressof(m_recv_cv), std::addressof(m_cs), INFINITE, 0);
                }
                m_ring.PushFront(message);
                ::WakeAllConditionVariable(std::addressof(m_send_cv));
                return;
            }
            bool TryJamMessage(T message) {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                ON_SCOPE_EXIT { ::ReleaseSRWLockExclusive(std::addressof(m_cs)); };
                if (m_ring.IsFull()) { return false; }
                m_ring.PushBack(message);
                ::WakeAllConditionVariable(std::addressof(m_send_cv));
                return true;
            }
    };
}

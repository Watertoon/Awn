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

    class ServiceMessageQueue {
        private:
            size_t              *m_message_buffer;
            s32                  m_max;
            s32                  m_count;
            s32                  m_offset;
            vp::util::BusyMutex  m_busy_mutex;
            u32                  m_message_wait_value;
            u32                  m_clear_wait_value;
        private:
            ALWAYS_INLINE void WaitForMessage() {

                /* Ensure memory coherancy on exit */
                ON_SCOPE_EXIT { vp::util::MemoryBarrierReadWrite(); };

                /* Complete if we have a message, set value to waiting unless set */
                bool result;
                u32 value = m_message_wait_value;
                do {                    
                    if (value == 2) { return; }
                    if (value == 1) { break; }
                    VP_ASSERT(value == 0);
                    result = vp::util::InterlockedCompareExchange(std::addressof(value), std::addressof(m_message_wait_value), 1u, 0u);
                } while (result == false);

                /* Wait on address until we have message */
                if (::IsThreadAFiber() == false) {
                    u32 wait_value = 1;
                    do {
                        ::WaitOnAddress(std::addressof(m_message_wait_value), std::addressof(wait_value), sizeof(u32), INFINITE);
                    } while (m_message_wait_value == wait_value);
                } else {
                    const Result result = ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_message_wait_value)), ukern::ArbitrationType_WaitIfEqual, 1, TimeSpan::cMaxTime);
                    if (result != ukern::ResultInvalidWaitAddressValue) { RESULT_ABORT_UNLESS(result); }
                }

                return;
            }

            ALWAYS_INLINE void WaitForMessageClear() {

                /* Ensure memory coherancy on exit */
                ON_SCOPE_EXIT { vp::util::MemoryBarrierReadWrite(); };

                /* Complete if we are clear, set value to waiting unless set */
                bool result;
                u32 value = m_clear_wait_value;
                do {                    
                    if (value == 2) { return; }
                    if (value == 1) { break; }
                    VP_ASSERT(value == 0);
                    result = vp::util::InterlockedCompareExchange(std::addressof(value), std::addressof(m_clear_wait_value), 1u, 0u);
                } while (result == false);

                /* Wait on address until we have space for our message */
                if (::IsThreadAFiber() == false) {
                    u32 wait_value = 1;
                    do {
                        ::WaitOnAddress(std::addressof(m_clear_wait_value), std::addressof(wait_value), sizeof(u32), INFINITE);
                    } while (m_clear_wait_value == wait_value);
                } else {
                    const Result result = ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_clear_wait_value)), ukern::ArbitrationType_WaitIfEqual, 1, TimeSpan::cMaxTime);
                    if (result != ukern::ResultInvalidWaitAddressValue) { RESULT_ABORT_UNLESS(result); }
                }

                return;
            }

            ALWAYS_INLINE bool SendMessageImpl(size_t message) {

                u32 message_count, max_messages;
                {                    
                    vp::util::ScopedBusyMutex l(std::addressof(m_busy_mutex));

                    message_count = m_count;
                    max_messages  = m_max;
                    if (message_count < max_messages) {
                        
                        /* Calculate end message index*/
                        const s32 offset = message_count + m_offset;
                        const s32 index  = (m_max <= offset) ? offset - m_max : offset;

                        /* Insert message */
                        m_message_buffer[index] = message;
                        m_count                 = message_count + 1;

                    } else {
                        u32 last;
                        vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_clear_wait_value), 0u, 2u);
                    }
                }

                if (message_count < max_messages) {

                    Result result;
                    u32    last;
                    do {

                        /* Set we have sent a message */
                        const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_message_wait_value), 2u, 0u);
                        if (cmp_result == true) { return message_count < max_messages; }

                        /* Complete if already set to sent */
                        if (last != 1) {
                            VP_ASSERT(last == 2);
                            return message_count < max_messages;
                        }

                        /* Wake waiters */
                        result = ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_message_wait_value)), ukern::SignalType_SignalAndIncrementIfEqual, 1, 0xffff'ffff);
                        ::WakeByAddressAll(std::addressof(m_message_wait_value));
                    } while (result == ukern::ResultInvalidWaitAddressValue);
                    RESULT_ABORT_UNLESS(result);
                }

                return message_count < max_messages;
            }

            ALWAYS_INLINE bool RecieveMessageImpl(size_t *out_message) {

                u32 message_count, max_messages;
                {                    
                    vp::util::ScopedBusyMutex l(std::addressof(m_busy_mutex));

                    message_count = m_count;
                    if (message_count == 0) {
                        u32 last;
                        vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_message_wait_value), 0u, 2u);
                    } else {

                        /* Get message and calculate next offset */
                        const s32    offset      = m_offset;
                        const size_t message     = m_message_buffer[offset];
                        const s32    next_offset = offset + 1;
                        const s32    max         = m_max;
                        const s32    adjust      = (m_max <= next_offset) ? max : 0;

                        /* Commit outputs */
                        m_count      = message_count - 1;
                        m_offset     = next_offset - adjust;
                        *out_message = message;

                    }
                }

                if (message_count != 0) {
                    
                    Result result;
                    u32    last;
                    do {

                        /* Set we have cleared a message */
                        const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_clear_wait_value), 2u, 0u);
                        if (cmp_result == true) { return message_count != 0; }

                        /* Complete if already set to cleared */
                        if (last != 1) {
                            VP_ASSERT(last == 2);
                            return message_count != 0;
                        }

                        /* Wake waiters */
                        result = ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_clear_wait_value)), ukern::SignalType_SignalAndIncrementIfEqual, 1, 0xffff'ffff);
                        ::WakeByAddressAll(std::addressof(m_clear_wait_value));
                    } while (result == ukern::ResultInvalidWaitAddressValue);
                    RESULT_ABORT_UNLESS(result);
                }

                return message_count != 0;
            }

            ALWAYS_INLINE bool JamMessageImpl(size_t message) {

                u32 message_count, max_messages;
                {                    
                    vp::util::ScopedBusyMutex l(std::addressof(m_busy_mutex));

                    message_count = m_count;
                    max_messages  = m_max;
                    if (message_count < max_messages) {
                        
                        /* Calculate our front message buffer index */
                        const s32 offset = m_offset;
                        const s32 adjust = (offset < 1) ? max_messages : 0;
                        const s32 index  = offset + adjust - 1;

                        /* Add message */
                        m_message_buffer[index] = message;
                        m_count      = message_count + 1;
                        m_offset       = index;

                    } else {
                        u32 last;
                        vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_clear_wait_value), 0u, 2u);
                    }
                }

                if (message_count < max_messages) {

                    Result result;
                    u32    last;
                    do {

                        /* Set we have sent a message */
                        const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_message_wait_value), 2u, 0u);
                        if (cmp_result == true) { return message_count < max_messages; }

                        /* Complete if already set to sent */
                        if (last != 1) {
                            VP_ASSERT(last == 2);
                            return message_count < max_messages;
                        }

                        /* Wake waiters */
                        result = ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_message_wait_value)), ukern::SignalType_SignalAndIncrementIfEqual, 1, 0xffff'ffff);
                        ::WakeByAddressAll(std::addressof(m_message_wait_value));
                    } while (result == ukern::ResultInvalidWaitAddressValue);
                    RESULT_ABORT_UNLESS(result);
                }

                return message_count < max_messages;
            }
        public:
            constexpr  ServiceMessageQueue() : m_message_buffer(nullptr), m_max(0), m_count(0), m_offset(0), m_busy_mutex{}, m_message_wait_value(0), m_clear_wait_value(0) {/*...*/}
            constexpr ~ServiceMessageQueue() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, s32 max_message_count) {

                /* Allocate message buffer */
                m_message_buffer = new (heap, alignof(size_t)) size_t[max_message_count];
                VP_ASSERT(m_message_buffer != nullptr);

                m_max = max_message_count;

                return;
            }
            void Initialize(mem::Heap *heap, s32 max_message_count) {

                /* Allocate message buffer */
                m_message_buffer = new (heap, alignof(size_t)) size_t[max_message_count];
                VP_ASSERT(m_message_buffer != nullptr);
    
                m_max = max_message_count;

                return;
            }

            void Finalize() {

                /* Free and clear state */
                if (m_message_buffer != nullptr) {
                    delete [] m_message_buffer;
                }
                m_message_buffer   = nullptr;
                m_max     = 0;
                m_count = 0;
                m_offset  = 0;

                return;
            }

            void ReceiveMessage(size_t *out_message) {

                bool result = this->RecieveMessageImpl(out_message);
                while (result == false) {
                    this->WaitForMessage();
                    result = this->RecieveMessageImpl(out_message);
                }

                return;
            }

            bool TryReceiveMessage(size_t *out_message) {

                u32 message_count, max_messages;
                {                    
                    vp::util::ScopedBusyMutex l(std::addressof(m_busy_mutex));

                    message_count = m_count;
                    if (message_count != 0) {

                        /* Get message and calculate next offset */
                        const s32    offset      = m_offset;
                        const size_t message     = m_message_buffer[offset];
                        const s32    next_offset = offset + 1;
                        const s32    max         = m_max;
                        const s32    adjust      = (m_max <= next_offset) ? max : 0;

                        /* Commit outputs */
                        m_count      = message_count - 1;
                        m_offset     = next_offset - adjust;
                        *out_message = message;
                    }
                }

                if (message_count != 0) {
                    
                    Result result;
                    u32    last;
                    do {

                        /* Set we have cleared a message */
                        const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_clear_wait_value), 2u, 0u);
                        if (cmp_result == true) { return message_count != 0; }

                        /* Complete if already set to cleared */
                        if (last != 1) {
                            VP_ASSERT(last == 2);
                            return message_count != 0;
                        }

                        /* Wake waiters */
                        result = ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_clear_wait_value)), ukern::SignalType_SignalAndIncrementIfEqual, 1, 0xffff'ffff);
                        ::WakeByAddressAll(std::addressof(m_clear_wait_value));
                    } while (result == ukern::ResultInvalidWaitAddressValue);
                    RESULT_ABORT_UNLESS(result);
                }

                return message_count != 0;
            }

            bool TryPeekMessage(size_t *out_message) {
                vp::util::ScopedBusyMutex l(std::addressof(m_busy_mutex));

                const u32 pending_messages = m_count;
                if (pending_messages != 0) { return false; }

                *out_message = m_message_buffer[m_offset];

                return true;
            }

            void SendMessage(size_t message) {

                bool result = this->SendMessageImpl(message);
                while (result == false) {
                    this->WaitForMessageClear();
                    result = this->SendMessageImpl(message);
                }

                return;
            }

            bool TrySendMessage(size_t message) {
                
                u32 message_count, max_messages;
                {                    
                    vp::util::ScopedBusyMutex l(std::addressof(m_busy_mutex));

                    message_count = m_count;
                    max_messages  = m_max;
                    if (message_count < max_messages) {
                        
                        /* Calculate end message index*/
                        const s32 offset = message_count + m_offset;
                        const s32 index  = (m_max <= offset) ? offset - m_max : offset;

                        /* Insert message */
                        m_message_buffer[index] = message;
                        m_count      = message_count + 1;

                    }
                }

                if (message_count < max_messages) {

                    Result result;
                    u32    last;
                    do {

                        /* Set we have sent a message */
                        const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_message_wait_value), 2u, 0u);
                        if (cmp_result == true) { return message_count < max_messages; }

                        /* Complete if already set to sent */
                        if (last != 1) {
                            VP_ASSERT(last == 2);
                            return message_count < max_messages;
                        }

                        /* Wake waiters */
                        result = ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_message_wait_value)), ukern::SignalType_SignalAndIncrementIfEqual, 1, 0xffff'ffff);
                        ::WakeByAddressAll(std::addressof(m_message_wait_value));
                    } while (result == ukern::ResultInvalidWaitAddressValue);
                    RESULT_ABORT_UNLESS(result);
                }

                return message_count < max_messages;
            }

            void JamMessage(size_t message) {

                bool result = this->JamMessageImpl(message);
                while (result == false) {
                    this->WaitForMessageClear();
                    result = this->JamMessageImpl(message);
                }

                return;
            }

            bool TryJamMessage(size_t message) {
                
                u32 message_count, max_messages;
                {                    
                    vp::util::ScopedBusyMutex l(std::addressof(m_busy_mutex));

                    message_count = m_count;
                    max_messages  = m_max;
                    if (message_count < max_messages) {
                        
                        /* Calculate our front message buffer index */
                        const s32 offset = m_offset;
                        const s32 adjust = (offset < 1) ? max_messages : 0;
                        const s32 index  = offset + adjust - 1;

                        /* Add message */
                        m_message_buffer[index] = message;
                        m_count      = message_count + 1;
                        m_offset       = index;

                    }
                }

                if (message_count < max_messages) {

                    Result result;
                    u32    last;
                    do {

                        /* Set we have sent a message */
                        const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_message_wait_value), 2u, 0u);
                        if (cmp_result == true) { return message_count < max_messages; }

                        /* Complete if already set to sent */
                        if (last != 1) {
                            VP_ASSERT(last == 2);
                            return message_count < max_messages;
                        }

                        /* Wake waiters */
                        result = ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_message_wait_value)), ukern::SignalType_SignalAndIncrementIfEqual, 1, 0xffff'ffff);
                        ::WakeByAddressAll(std::addressof(m_message_wait_value));
                    } while (result == ukern::ResultInvalidWaitAddressValue);
                    RESULT_ABORT_UNLESS(result);
                }

                return message_count < max_messages;
            }
    };
}

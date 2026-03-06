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

    class MessageQueue {
        private:
            u64                *m_message_buffer;
            s32                  m_max;
            s32                  m_count;
            s32                  m_offset;
            vp::util::BusyMutex  m_busy_mutex;
            u32                  m_message_wait_value;
            u32                  m_clear_wait_value;
        private:
            void WaitForMessage();
            void WaitForMessageClear();
            bool SendMessageImpl(u64 message);
            bool RecieveMessageImpl(u64 *out_message);
            bool JamMessageImpl(u64 message);
        public:
            constexpr  MessageQueue() : m_message_buffer(), m_max(), m_count(), m_offset(), m_busy_mutex{}, m_message_wait_value(), m_clear_wait_value() {/*...*/}
            constexpr ~MessageQueue() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, s32 max_message_count);
            void Initialize(mem::Heap *heap, s32 max_message_count);
            constexpr void InitializeByReference(u64 *memory, s32 max_message_count);
            void Finalize();

            void ReceiveMessage(u64 *out_message);
            bool TryReceiveMessage(u64 *out_message);
            bool TryPeekMessage(u64 *out_message);

            void SendMessage(u64 message);
            bool TrySendMessage(u64 message);

            void JamMessage(u64 message);
            bool TryJamMessage(u64 message);
    };

    /* Constexpr impl */
    constexpr void MessageQueue::InitializeByReference(u64 *memory, s32 max_message_count) {

        /* Allocate message buffer */
        m_message_buffer = memory;
        VP_ASSERT(m_message_buffer != nullptr);

        m_max = max_message_count;
        m_count  = 0;
        m_offset = 0;

        return;
    }
}

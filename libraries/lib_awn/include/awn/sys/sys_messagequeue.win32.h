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
            size_t              *m_message_buffer;
            s32                  m_max;
            s32                  m_count;
            s32                  m_offset;
            vp::util::BusyMutex  m_busy_mutex;
            u32                  m_message_wait_value;
            u32                  m_clear_wait_value;
        private:
            void WaitForMessage();
            void WaitForMessageClear();
            bool SendMessageImpl(size_t message);
            bool RecieveMessageImpl(size_t *out_message);
            bool JamMessageImpl(size_t message);
        public:
            constexpr  MessageQueue() : m_message_buffer(), m_max(), m_count(), m_offset(), m_busy_mutex{}, m_message_wait_value(), m_clear_wait_value() {/*...*/}
            constexpr ~MessageQueue() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, s32 max_message_count);
            void Initialize(mem::Heap *heap, s32 max_message_count);
            constexpr void InitializeByReference(size_t *memory, s32 max_message_count);
            void Finalize();

            void ReceiveMessage(size_t *out_message);
            bool TryReceiveMessage(size_t *out_message);
            bool TryPeekMessage(size_t *out_message);

            void SendMessage(size_t message);
            bool TrySendMessage(size_t message);

            void JamMessage(size_t message);
            bool TryJamMessage(size_t message);
    };

    /* Constexpr impl */
    constexpr void MessageQueue::InitializeByReference(size_t *memory, s32 max_message_count) {

        /* Allocate message buffer */
        m_message_buffer = memory;
        VP_ASSERT(m_message_buffer != nullptr);

        m_max = max_message_count;
        m_count  = 0;
        m_offset = 0;

        return;
    }
}

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
            size_t                           *m_message_buffer;
            s32                               m_max_messages;
            s32                               m_pending_messages;
            s32                               m_current_message;
            ukern::InternalCriticalSection    m_message_mutex;
            ukern::InternalConditionVariable  m_message_sent_cv;
            ukern::InternalConditionVariable  m_buffer_full_cv;
        private:
            void WaitForMessage();
            void WaitForMessageClear();
            void SendMessageImpl(size_t message);
            void RecieveMessageImpl(size_t *out_message);
            void JamMessageImpl(size_t message);
        public:
            constexpr MessageQueue() : m_message_buffer(nullptr), m_max_messages(0), m_pending_messages(0), m_current_message(0), m_message_mutex(), m_message_sent_cv(), m_buffer_full_cv() {/*...*/}
            constexpr ~MessageQueue() {/*...*/}

            void Initialize(s32 max_message_count);
            void Initialize(mem::Heap *heap, s32 max_message_count);
            void Finalize();

            void ReceiveMessage(size_t *out_message);
            bool TryReceiveMessage(size_t *out_message);
            bool TryPeekMessage(size_t *out_message);

            void SendMessage(size_t message);
            bool TrySendMessage(size_t message);

            void JamMessage(size_t message);
            bool TryJamMessage(size_t message);
    };
}

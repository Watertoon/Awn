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
            void WaitForMessage();
            void WaitForMessageClear();
            bool SendMessageImpl(size_t message);
            bool RecieveMessageImpl(size_t *out_message);
            bool JamMessageImpl(size_t message);
        public:
            constexpr  ServiceMessageQueue() : m_message_buffer(nullptr), m_max(0), m_count(0), m_offset(0), m_busy_mutex{}, m_message_wait_value(0), m_clear_wait_value(0) {/*...*/}
            constexpr ~ServiceMessageQueue() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, s32 max_message_count);
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

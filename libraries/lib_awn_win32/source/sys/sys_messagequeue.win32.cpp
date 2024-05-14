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
#include <awn.hpp>

namespace awn::sys {

    void MessageQueue::WaitForMessage() {

        /* Wait on condition variable until we get a message */
        while(m_pending_messages == 0) {
            m_message_sent_cv.Wait(std::addressof(m_message_mutex));
        }
    }

    void MessageQueue::WaitForMessageClear() {

        /* Wait on condition variable until we have space for our message */
        while(m_max_messages <= m_pending_messages) {
            m_buffer_full_cv.Wait(std::addressof(m_message_mutex));
        }
    }

    void MessageQueue::SendMessageImpl(size_t message) {

        /* Calculate our messages buffer index */
        s32 index = m_pending_messages + m_current_message;
        if(m_max_messages <= index) {
            index = index - m_max_messages;
        }

        /* Add message */
        m_message_buffer[index] = message;
        m_pending_messages += 1;
        m_message_sent_cv.Broadcast();
    }

    void MessageQueue::RecieveMessageImpl(size_t *out_message) {

        /* Pull our current message */
        *out_message = m_message_buffer[m_current_message];
        m_current_message += 1;
        m_pending_messages -= 1;
        const u32 offset = (m_max_messages <= m_current_message) ? m_max_messages : 0;
        m_current_message = m_current_message - offset;
        m_buffer_full_cv.Broadcast();
    }

    void MessageQueue::JamMessageImpl(size_t message) {

        /* Calculate our front message buffer index */
        const s32 offset = m_current_message;
        const s32 adjust = (offset < 1) ? m_max_messages : 0;
        const s32 index  = offset + adjust - 1;

        /* Add message */
        m_message_buffer[index]  = message;
        m_pending_messages      += 1;
        m_current_message        = index;

        /* Signal */
        m_message_sent_cv.Broadcast();

        return;
    }

    void MessageQueue::Initialize(s32 max_message_count) {

        /* Allocate message buffer */
        m_message_buffer = new size_t[max_message_count];
        VP_ASSERT(m_message_buffer != nullptr);
        
        m_max_messages     = max_message_count;
        m_pending_messages = 0;
        m_current_message  = 0;
    }

    void MessageQueue::Initialize(mem::Heap *heap, s32 max_message_count) {

        /* Allocate message buffer */
        m_message_buffer = new (heap, 8) size_t[max_message_count];
        VP_ASSERT(m_message_buffer != nullptr);
        
        m_max_messages     = max_message_count;
        m_pending_messages = 0;
        m_current_message  = 0;
    }

    void MessageQueue::Finalize() {

        if (m_message_buffer != nullptr) {            
            delete [] m_message_buffer;
            m_message_buffer = nullptr;
            m_max_messages   = 0;
        }
        m_pending_messages = 0;
        m_current_message  = 0;

        return;
    }

    void MessageQueue::ReceiveMessage(size_t *out_message) {
        std::scoped_lock l(m_message_mutex);

        this->WaitForMessage();

        this->RecieveMessageImpl(out_message);
    }

    bool MessageQueue::TryReceiveMessage(size_t *out_message) {
        std::scoped_lock l(m_message_mutex);

        if (m_pending_messages == 0) { return false; }

        this->RecieveMessageImpl(out_message);

        return true;
    }

    bool MessageQueue::TryPeekMessage(size_t *out_message) {
        std::scoped_lock l(m_message_mutex);

        const u32 pending_messages = m_pending_messages;
        if (pending_messages == 0) { return false; }

        *out_message = m_message_buffer[m_current_message];

        return true;
    }

    void MessageQueue::SendMessage(size_t message) {
        std::scoped_lock l(m_message_mutex);

        this->WaitForMessageClear();

        this->SendMessageImpl(message);
    }

    bool MessageQueue::TrySendMessage(size_t message) {
        std::scoped_lock l(m_message_mutex);

        if (m_max_messages <= m_pending_messages) { return false; }

        this->SendMessageImpl(message);

        return true;
    }

    void MessageQueue::JamMessage(size_t message) {
        std::scoped_lock l(m_message_mutex);

        this->WaitForMessageClear();

        this->JamMessageImpl(message);

        return;
    }

    bool MessageQueue::TryJamMessage(size_t message) {
        std::scoped_lock l(m_message_mutex);

        if (m_max_messages <= m_pending_messages) { return false; }

        this->JamMessageImpl(message);

        return true;
    }
}

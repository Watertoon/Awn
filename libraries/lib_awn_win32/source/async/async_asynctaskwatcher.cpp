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

namespace awn::async {

    void AsyncTaskWatcher::Reference() {
        std::scoped_lock l(m_queue->m_queue_mutex);
        ++m_reference_count;
    }

    void AsyncTaskWatcher::ReleaseReference() {
        std::scoped_lock l(m_queue->m_queue_mutex);

        const u32 new_count = m_reference_count - 1;
        if ((m_reference_count < 1 || (m_reference_count = new_count) == 0) && m_async_task != nullptr) {
            m_async_task->FreeToAllocator();
            m_async_task = nullptr;
        }
    }

    void AsyncTaskWatcher::CancelTask() {

        /* Nothing to do if no task or queue */
        if (m_async_task == nullptr || m_queue == nullptr) { return; }

        /* Reference watcher */
        this->Reference();

        /* Wait for task to finish (if not out of reference) */
        if (m_async_task != nullptr && m_async_task->m_queue != nullptr) {
            m_async_task->m_queue->CancelTask(m_async_task);                    
        }

        /* Unreference watcher */
        this->ReleaseReference();

        return;
    }

    void AsyncTaskWatcher::WaitForCompletion() {

        /* Nothing to do if no task or queue */
        if (m_async_task == nullptr || m_queue == nullptr) { return; }

        /* Reference watcher */
        this->Reference();

        /* Wait for task to finish (if not out of reference) */
        if (m_async_task != nullptr) {
            m_async_task->m_finish_event.Wait();                    
        }

        /* Unreference watcher */
        this->ReleaseReference();
        
        return;
    }
}

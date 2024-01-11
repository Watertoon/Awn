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

    bool AsyncTask::TryInvokeSync() {

        /* Find exe queue thread */
        sys::ThreadBase *thread = sys::GetCurrentThread();
        for (u32 i = 0; i < m_queue->m_task_thread_array.GetUsedCount(); ++i) {

            sys::ThreadBase *task_thread = m_queue->m_task_thread_array[i];
            if (task_thread != thread) { continue; }

            this->Invoke(reinterpret_cast<AsyncQueueThread*>(task_thread));

            return true;
        }

        return false;
    }

    void AsyncTask::Invoke(AsyncQueueThread *thread) {

        /* Set thread */
        m_queue_thread = thread;
        m_status       = static_cast<u32>(Status::Queued); 

        /* Execute */
        this->Execute();

        m_status = static_cast<u32>(Status::PostExecute); 
        this->PostExecute();

        /* Result */
        Result result = 0;
        if (m_result_delegate != nullptr) {
            const TaskResultInvokeInfo invoke_info = {
                .task      = this,
                .user_data = m_user_data,
            };
            result = m_result_delegate->Invoke(std::addressof(invoke_info));
        }

        /* Free if necessary */
        if (result != ResultRescheduled) {
            m_status = static_cast<u32>(Status::FreeExecute); 
            this->FreeExecute();
            m_status = static_cast<u32>(Status::Complete);
            m_finish_event.Signal();
        }

        return;
    }

    void AsyncTask::Cancel() {

        /* Clear state */
        m_queue_thread    = nullptr;
        m_result_delegate = nullptr;

        /* Free cancel */
        this->FreeCancel();

        /* Set status to cancelled */
        m_status = static_cast<u32>(Status::Cancelled);

        return;
    }

    Result AsyncTask::PushTask(AsyncTaskPushInfo *push_info) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(m_queue_list_node.IsLinked() == false, ResultAlreadyQueued);

        /* Get queue */
        AsyncQueue *queue = push_info->GetQueue();
        RESULT_RETURN_UNLESS(push_info->priority <= queue->m_priority_level_array.GetCount(), ResultInvalidPriority);

        /* Setup task */
        m_queue           = queue;
        m_task_delegate   = push_info->task_delegate;
        m_result_delegate = push_info->result_delegate;
        m_user_data       = push_info->user_data;
        this->FormatPushInfo(push_info);

        /* Try invoke sync */
        if (push_info->is_sync == true) {
            std::scoped_lock l(queue->m_queue_mutex);
            if (this->TryInvokeSync() == true) { RESULT_RETURN_SUCCESS; }
        }

        {
            std::scoped_lock l(queue->m_queue_mutex);

            /* Find lowest populated priority level */
            const u32 priority = push_info->priority;
            u32 priority_iter  = priority;
            do {
                priority_iter = priority_iter - 1;
            } while(priority_iter != cInvalidPriorityLevel && queue->m_priority_level_array[priority_iter].async_task_head == nullptr);

            /* Insert into list at respective priority level */
            if (priority_iter == cInvalidPriorityLevel) {

                /* Insert as head since there are no other tasks */
                queue->m_task_list.PushBack(*this);

                /* Clear priorty level event */
                queue->m_priority_level_array[priority].priority_cleared_event.Clear();
            } else {

                /* Link next to back of current priority */
                queue->m_priority_level_array[priority_iter].async_task_head->m_queue_list_node.LinkNext(std::addressof(m_queue_list_node));
            }

            /* Increment task list count */
            ++queue->m_task_count;

            /* Set priority level head */
            if (queue->m_priority_level_array[priority].async_task_head == nullptr) {
                queue->m_priority_level_array[priority].async_task_head = this;
            }
            m_status = static_cast<u32>(Status::Queued);

            /* Clear event if first job */
            if (queue->m_task_count == 1) {
                queue->m_all_task_complete_event.Clear();
            }
        }

        /* Resume threads */
        sys::ThreadBase *current_thread = sys::GetCurrentThread();
        for (;;) {
            u32 is_failed_to_signal_thread = 1;
            {
                std::scoped_lock l(queue->m_queue_mutex);

                size_t thread_mask = 0;
                for (u32 i = 0; i < queue->m_task_thread_array.GetUsedCount(); ++i) {
                    sys::ThreadBase *q_thread = queue->m_task_thread_array[i];
                    
                    /* Check if thread needs to be signaled */
                    if (current_thread == q_thread)                                              { continue; }
    
                    size_t     message = 0;
                    q_thread->TryPeekMessage(std::addressof(message));
                    if ((message - 1) <= static_cast<size_t>(AsyncQueueThread::Message::Resume)) { continue; }
                    if (message == q_thread->GetExitMessage())                                   { continue; }

                    const size_t thread_mask_index = (1 << (i & 0x3f));
                    if ((thread_mask & thread_mask_index) != 0)                                  { continue; }

                    /* Signal thread */
                    const bool is_sent_message  = q_thread->TrySendMessage(static_cast<size_t>(AsyncQueueThread::Message::Start));

                    /* Adjust thread mask */
                    is_failed_to_signal_thread &= is_sent_message;
                    if (is_sent_message == true) {
                        thread_mask |= thread_mask_index;
                    }
                }
            }
            if (is_failed_to_signal_thread == true) { break; }
            sys::SleepThread(0);
        }

        /* Wait for job completion if sync */
        if (push_info->is_sync == true) {
            m_finish_event.Wait();
        }

        RESULT_RETURN_SUCCESS;
    }
}

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

    void AsyncTask::FormatPushInfo(AsyncTaskPushInfo *push_info) { VP_UNUSED(push_info); }

    void AsyncTask::Execute() {
        if (m_task_function == nullptr) { return; }
        m_task_function->Invoke(m_user_data);
    }

    void AsyncTask::PostExecute() {/*...*/}

    void AsyncTask::FreeExecute() {/*...*/}

    void AsyncTask::FreeCancel() {/*...*/}

    bool AsyncTask::TryInvokeSync() {

        /* Find exe queue thread */
        sys::ThreadBase *thread = sys::GetCurrentThread();
        for (u32 i = 0; i < m_queue->m_task_thread_array.GetUsedCount(); ++i) {

            sys::ThreadBase *task_thread = m_queue->m_task_thread_array[i];
            if (task_thread != thread) { continue; }

            this->InvokeSync(reinterpret_cast<AsyncQueueThread*>(task_thread));

            return true;
        }

        return false;
    }

    void AsyncTask::InvokeSync(AsyncQueueThread *thread) {

        /* Set thread */
        {
            std::scoped_lock l(m_queue->m_queue_mutex);
            m_queue_thread = thread;
            m_status = static_cast<u32>(Status::Queued); 
        }

        /* Execute */
        this->Execute();

        /* Post execute */
        m_status = static_cast<u32>(Status::PostExecute); 
        {
            std::scoped_lock l(m_queue->m_queue_mutex);
            this->PostExecute();
        }

        /* Result execute */
        m_cancel_function               = nullptr;
        Result          result          = ResultSuccess;
        ResultFunction *result_function = m_result_function;
        if (result_function != nullptr) {
            m_result_function = nullptr;

            TaskResultInvokeInfo invoke_info = {
                .task         = this,
                .user_data    = m_user_data,
                .is_cancelled = static_cast<bool>(m_is_cancelled),
            };
            result = result_function->Invoke(std::addressof(invoke_info));

            m_queue_thread = nullptr;
        }

        /* Free if necessary */
        if (result != ResultRescheduled) {
            std::scoped_lock l(m_queue->m_queue_mutex);
            this->InvokeFreeExecute();
        }

        return;
    }

    Result AsyncTask::Invoke() {

        /* Execute */
        this->Execute();

        /* Post execute */
        m_status = static_cast<u32>(Status::PostExecute); 
        {
            std::scoped_lock l(m_queue->m_queue_mutex);
            this->PostExecute();
            m_queue_thread->m_current_task = nullptr;
        }

        /* Result execute */
        m_cancel_function               = nullptr;
        Result          result          = ResultSuccess;
        ResultFunction *result_function = m_result_function;
        if (result_function != nullptr) {
            m_result_function = nullptr;

            TaskResultInvokeInfo invoke_info = {
                .task         = this,
                .user_data    = m_user_data,
                .is_cancelled = static_cast<bool>(m_is_cancelled),
            };
            result = result_function->Invoke(std::addressof(invoke_info));

            m_queue_thread = nullptr;
        }
        vp::util::MemoryBarrierInnerWrite();

        return result;
    }

    void AsyncTask::InvokeFreeExecute() {

        /* Free execute */
        m_status = static_cast<u32>(Status::FreeExecute); 
        this->FreeExecute();
        m_status = static_cast<u32>(Status::Complete);
        if (m_is_signal_finish_event) {
            m_is_cancelled = false;
            m_finish_event.Signal();
        }

        /* Set as free for allocator */
        m_is_free_for_allocator = true;
    }

    void AsyncTask::Finalize() {
        if (m_status == static_cast<u32>(Status::ForceCancelled)) { return; }
        this->CancelTask();
        m_queue           = nullptr;
        m_user_data       = nullptr;
        m_result_function = nullptr;
        m_cancel_function = nullptr;
        m_status          = static_cast<u32>(Status::ForceCancelled);
    }

    void AsyncTask::CancelTask() {
        if (m_queue != nullptr) { m_queue->CancelTask(this, true); }
    }

    Result AsyncTask::TryCancelTask() {
        if (m_queue == nullptr) { RESULT_RETURN_SUCCESS; }
        return m_queue->CancelTask(this, false);
    }

    void AsyncTask::Cancel() {

        /* Clear thread and result function */
        m_queue_thread    = nullptr;
        m_result_function = nullptr;

        /* Invoke cancel function */
        CancelFunction *cancel_function = m_cancel_function;
        if (cancel_function != nullptr) {
            m_cancel_function = nullptr;

            TaskCancelInvokeInfo invoke_info = {
                .task      = this,
                .user_data = m_user_data,
            };
            cancel_function->Invoke(std::addressof(invoke_info));
        }

        /* Lock queue */
        std::scoped_lock l(m_queue->m_queue_mutex);

        /* Free cancel */
        this->FreeCancel();

        /* Set status to cancelled */
        m_status = static_cast<u32>(Status::Cancelled);

        /* Signal finish event */
        if (m_is_signal_finish_event) {
            m_is_cancelled = false;
            m_finish_event.Signal();
        }

        /* Set as free for allocator */
        vp::util::MemoryBarrierInnerWrite();
        m_is_free_for_allocator = true;

        return;
    }

    void AsyncTask::CancelWhileActive() {
        m_finish_event.Clear();
        m_is_signal_finish_event = true;
        m_is_cancelled = true;
    }

    Result AsyncTask::PushTask(AsyncTaskPushInfo *push_info) {

        /* Get queue */
        AsyncQueue *queue = push_info->GetQueue();
        VP_ASSERT(queue != nullptr);
        /* Ensure we aren't already scheduled */
        RESULT_RETURN_IF(this->IsPending() && !this->IsExecutingOnCurrentThread(), ResultAlreadyQueued);

        /* Setup event flags */
        const bool is_signal_event = (push_info->is_sync) | (!push_info->is_skip_finish_event);
        m_is_signal_finish_event = is_signal_event;
        m_is_free_for_allocator = false;
        m_is_thread_boost = push_info->is_thread_boost;

        /* Ensure queued and valid priority */
        RESULT_RETURN_UNLESS(!m_queue_list_node.IsLinked(), ResultAlreadyQueued);
        RESULT_RETURN_UNLESS(push_info->priority <= queue->m_priority_level_array.GetCount(), ResultInvalidPriority);

        /* Clear job completion event */
        if (is_signal_event) { m_finish_event.Clear(); }

        /* Set queue, functions, and user data */
        m_queue           = queue;
        m_task_function   = push_info->task_function;
        m_result_function = push_info->result_function;
        m_cancel_function = push_info->cancel_function;
        m_user_data       = push_info->user_data;

        /* Format push info for derived task */
        this->FormatPushInfo(push_info);

        /* Try invoke sync */
        if (push_info->is_sync) {
            const bool is_invoke_sync = this->TryInvokeSync();
            if (is_invoke_sync) { RESULT_RETURN_SUCCESS; }
        }

        /* Queue task */
        queue->EnqueueTask(this, push_info->priority);

        /* Wait for job completion if sync */
        if (push_info->is_sync) {
            m_finish_event.Wait();
        }

        RESULT_RETURN_SUCCESS;
    }
    
    void AsyncTask::AdjustPriority(u32 new_priority) {

        /* Get queue and ensure it exists */
        async::AsyncQueue *queue = m_queue;
        if (queue == nullptr) { return; }
        queue->AdjustPriority(this, new_priority);

        return;
    }

    void AsyncTask::Wait() {
        m_finish_event.Wait();
    }

    bool AsyncTask::IsExecutingOnCurrentThread() const {
        if (m_queue_thread == nullptr) { return false; }
        const Status status = static_cast<Status>(m_status);
        if (status != Status::PostExecute) { return false; }
        return reinterpret_cast<sys::ThreadBase*>(m_queue_thread) == sys::GetCurrentThread();
    }

    bool AsyncTask::IsSchedulable() const {
        const bool is_pending = this->IsPending();
        if (!is_pending) { return true; }
        return this->IsExecutingOnCurrentThread();
    }
}

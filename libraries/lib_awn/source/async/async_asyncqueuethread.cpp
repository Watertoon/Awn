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

    void AsyncQueueThread::ThreadMain(size_t message) {

        /* Handle suspension */
        if (!m_state.GetBit(State_IsSuspended)) {

            if (message == static_cast<size_t>(Message::Suspend)) {

                m_state.SetBit(State_IsSuspended);
                m_suspend_event.Signal();
                return;
            }

        } else {
            if (message != static_cast<size_t>(Message::Resume)) { return; }

            m_state.ClearBit(State_IsSuspended);
            m_suspend_event.Signal();
        }

        /* Task main loop */
        u32 yield_iter = m_requests_per_yield;
        m_state.SetBit(State_IsRunning);
        m_state.SetBit(State_IsNotScheduling);
        bool is_change_set = false;
        u32 boost_id = AsyncQueue::cInvalidThreadLink;
        for (;;) {

            /* Peek latest message for exit or suspension */
            const bool result1 = this->TryPeekMessage(std::addressof(message));
            if (!result1) { message = 0; }
            if (message == m_exit_message || message == static_cast<size_t>(Message::Suspend)) { break; }

            /* Task scheduler */
            AsyncTask *task = nullptr;
            u32 current_set;
            {
                std::scoped_lock l(m_queue->m_queue_mutex);
                m_state.ClearBit(State_IsNotScheduling);
                vp::util::MemoryBarrierInnerReadWrite();

                /* Acquire next task */
                task = m_queue->AcquireNextTask(this);

                /* Schedule thread set */
                is_change_set = m_queue->UpdateThreadSetForAcquireTask(std::addressof(boost_id), task, this);
                current_set = m_queue->m_current_thread_set;

                /* If no task update completion and exit */
                if (task == nullptr) {
                    const u32 state = m_state.GetBitMask(0) & ~(State_IsRunning | State_IsNotScheduling);
                    m_state.SetBitMask(0, state);
                    m_queue->UpdateCompletion();
                    return;
                }

                /* Set state to we have a task */
                const u32 state = m_state.GetBitMask(0) | (State_IsTask | State_IsNotScheduling);
                m_state.SetBitMask(0, state);
            }

            /* Change or boost thread set, preempting ourselves */
            if (is_change_set) {
                is_change_set = false;
                boost_id = AsyncQueue::cInvalidThreadLink;
                m_queue->WakeThreadSet(current_set, this);
            } else if (boost_id != AsyncQueue::cInvalidThreadLink) {
                m_queue->BoostThread(boost_id, this);
                boost_id = AsyncQueue::cInvalidThreadLink;
            }

            /* Execute task */
            const Result result = task->Invoke();

            u32 task_count;
            {
                std::scoped_lock l(m_queue->m_queue_mutex);

                /* Update thread for task completion */
                is_change_set = m_queue->UpdateThreadSetForCompleteTask(task->m_priority, this);
                current_set = m_queue->m_current_thread_set;

                /* Free if necessary */
                if (result != ResultRescheduled) {
                    task->InvokeFreeExecute();
                }

                /* Signal execute event */
                m_execute_event.Signal();

                /* Signal finished tasks */
                m_state.ClearBit(State_IsTask);
                m_queue->UpdateCompletion();
                m_state.ClearBit(State_IsNotScheduling);
                vp::util::MemoryBarrierInnerReadWrite();

                /* Update state for task count */
                task_count = m_queue->m_task_count;
                const u32 next_state = (task_count < 1) ? m_state.GetBitMask(0) & ~(State_IsNotScheduling | State_IsRunning) : m_state.GetBitMask(0) | State_IsNotScheduling;
                m_state.SetBitMask(0, next_state);
            }

            /* Change thread set */
            if (is_change_set) {
                is_change_set = false;
                m_queue->WakeThreadSet(current_set, this);
                current_set = AsyncQueue::cInvalidThreadLink;
            }

            /* Complete if out of tasks */
            if (task_count == 0) { break; }

            /* Yield thread if request count is reached */
            if (0 < m_requests_per_yield) {
                yield_iter = yield_iter - 1;
                if (yield_iter == 0) {
                    yield_iter = m_requests_per_yield;
                    sys::SleepThread(0);
                }
            }
        }

        /* Update state for not running */
        const u32 state = m_state.GetBitMask(0) & ~(State_IsRunning | State_IsNotScheduling);
        m_state.SetBitMask(0, state);

        return;
    }

    void AsyncQueueThread::Suspend() {

        /* Ensure not current thread */
        if (this == sys::GetCurrentThread()) { return; }

        /* Change status */
        Status last_status;
        const bool result = vp::util::InterlockedCompareExchange(std::addressof(last_status), std::addressof(m_status), Status::Suspended, Status::Active);
        if (result == false) { return; }

        /* Set message */
        m_suspend_event.Wait();
        m_suspend_event.Clear();
        this->JamMessage(static_cast<uintptr_t>(Message::Suspend));
        m_suspend_event.Wait();

        return;
    }

    void AsyncQueueThread::Resume() {

        /* Ensure not current thread */
        if (this == sys::GetCurrentThread()) { return; }

        /* Change status */
        Status last_status;
        const bool result = vp::util::InterlockedCompareExchange(std::addressof(last_status), std::addressof(m_status), Status::Active, Status::Suspended);
        if (result == false) { return; }

        /* Set message */
        m_suspend_event.Wait();
        m_suspend_event.Clear();
        this->JamMessage(static_cast<uintptr_t>(Message::Resume));
        m_suspend_event.Wait();

        return;
    }

    void AsyncQueueThread::CancelCurrentTaskIfPriority(u32 priority) {
        std::scoped_lock l(m_queue->m_queue_mutex);
        if (m_current_task == nullptr || m_current_task->m_priority != priority) { return; }
        m_current_task->CancelWhileActive();
    }
}

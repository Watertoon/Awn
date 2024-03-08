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

namespace awn::async {

    class AsyncQueueThread : public sys::ServiceThread {
        public:
            friend struct AsyncTaskPushInfo;
            friend class AsyncTask;
            friend class AsyncQueue;
        public:
            enum class Message : size_t {
                Exit    = 0,
                Resume  = 1,
                Suspend = 2,
            };
            enum class Status : u32 {
                None      = 0,
                Finished  = 1,
                Suspended = 2,
                Active    = 3,
            };
        private:
            Status             m_status;
            u32                m_is_finished;
            u32                m_requests_per_yield;
            AsyncTask         *m_current_task;
            AsyncQueue        *m_queue;
            sys::ServiceEvent  m_execute_event;
            sys::ServiceEvent  m_suspend_event;
        public:
            VP_RTTI_BASE(AsyncQueueThread);
        public:
             AsyncQueueThread(AsyncQueue *async_queue, const char *name, mem::Heap *thread_heap, u32 stack_size, s32 priority) : ServiceThread(name, thread_heap, sys::ThreadRunMode::WaitForMessage, 0x7fff'ffff, 8, stack_size, priority), m_status(), m_is_finished(true), m_requests_per_yield(), m_current_task(), m_queue(async_queue), m_execute_event() { 

                async_queue->m_task_thread_array.PushPointer(this);

                m_execute_event.Initialize(sys::SignalState::Signaled, sys::ResetMode::Manual);
            }
            ~AsyncQueueThread() { m_execute_event.Finalize(); }

            virtual void ThreadMain(size_t message) override {

                /* Handle suspension */
                if (m_status != Status::Suspended) {

                    if (message == static_cast<size_t>(Message::Suspend)) {

                        m_status = Status::Suspended;
                        m_suspend_event.Signal();
                        return;
                    }

                } else {
                    if (message != static_cast<size_t>(Message::Resume)) { return; }

                    m_status = Status::Active;
                    m_suspend_event.Signal();
                }

                /* Task main loop */
                u32 yield_iter = m_requests_per_yield;
                m_is_finished = false;
                do {

                    /* Schedule next task */
                    AsyncTask *task = m_queue->AcquireNextTask(this);
                    if (task == nullptr) { break; }

                    /* Execute task */
                    task->Invoke();

                    /* Signal execute event */
                    m_execute_event.Signal();

                    /* Signal finished tasks */
                    m_queue->UpdateCompletion();

                    /* Memory barrier */
                    vp::util::MemoryBarrierReadWrite();

                    /* Handle yield */
                    if (0 < m_requests_per_yield) {
                        yield_iter = yield_iter - 1;
                        if (yield_iter == 0) {
                            yield_iter = m_requests_per_yield;
                            sys::SleepThread(0);
                        }
                    }

                    /* Peek next message */
                    this->TryPeekMessage(std::addressof(message));

                } while (message != m_exit_message);

                m_is_finished = true;

                return;
            }

            void Suspend() {

                /* Ensure not current thread */
                if (this == sys::GetCurrentThread()) { return; }

                /* Change status */
                Status     last_status;
                const bool result      = vp::util::InterlockedCompareExchange(std::addressof(last_status), std::addressof(m_status), Status::Suspended, Status::Active);
                m_suspend_event.Wait();
                if (result == false) { return; }

                /* Set message */
                m_suspend_event.Clear();
                this->JamMessage(static_cast<uintptr_t>(Message::Suspend));

                return;
            }

            void Resume() {

                /* Ensure not current thread */
                if (this == sys::GetCurrentThread()) { return; }

                /* Change status */
                Status     last_status;
                const bool result      = vp::util::InterlockedCompareExchange(std::addressof(last_status), std::addressof(m_status), Status::Active, Status::Suspended);
                m_suspend_event.Wait();
                if (result == false) { return; }

                /* Set message */
                m_suspend_event.Clear();
                this->JamMessage(static_cast<uintptr_t>(Message::Resume));

                return;
            }

            ALWAYS_INLINE void WaitForExecute() {
                m_execute_event.Wait();
            }

            ALWAYS_INLINE void WaitForSuspension() {
                m_suspend_event.Wait();
            }

            ALWAYS_INLINE void WaitForPriorityLevel(u32 priority_level) {
                m_queue->WaitForPriorityLevel(priority_level);
            }
            ALWAYS_INLINE void CancelPriorityLevel(u32 priority_level) {
                m_queue->CancelPriorityLevel(priority_level);
            }

            void CancelCurrentTaskIfPriority(u32 priority) {
                std::scoped_lock l(m_queue->m_queue_mutex);
                if (m_current_task == nullptr || m_current_task->m_priority != priority) { return; }
                m_current_task->CancelWhileActive();
            }

            constexpr bool IsActive()    const { return m_status == Status::Active; }
            constexpr bool IsSuspended() const { return m_status == Status::Suspended; }

            constexpr AsyncQueue *GetQueue() const { return m_queue; }
    };
    
    constexpr AsyncQueue *AsyncTaskPushInfo::GetQueue() {
        VP_ASSERT(queue != nullptr || queue_thread != nullptr);
        AsyncQueue *out_queue = (queue != nullptr) ? queue : queue_thread->GetQueue();
        VP_ASSERT(out_queue != nullptr);
        return out_queue;
    }
}

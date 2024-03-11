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
            union {
                u32 m_state;
                struct {                    
                    u32 m_is_finished  : 1;  
                    u32 m_is_suspended : 1;  
                    u32 m_reserve      : 30;
                };
            };
            u32                m_requests_per_yield;
            AsyncTask         *m_current_task;
            AsyncQueue        *m_queue;
            sys::ServiceEvent  m_execute_event;
            sys::ServiceEvent  m_suspend_event;
        public:
            VP_RTTI_BASE(AsyncQueueThread);
        public:
             AsyncQueueThread(AsyncQueue *async_queue, const char *name, mem::Heap *thread_heap, u32 stack_size, s32 priority);
            virtual ~AsyncQueueThread() override;

            virtual void ThreadMain(size_t message) override;

            void Suspend();
            void Resume();

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

            void CancelCurrentTaskIfPriority(u32 priority);

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

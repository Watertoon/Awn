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

	class AsyncTask;
	class AsyncTaskWatcher;
    class AsyncQueue;
    class AsyncQueueThread;

    struct TaskResultInvokeInfo {
        AsyncTask *task;
        void      *user_data;
    };

    using TaskDelegate   = vp::util::IDelegate<void*>;
    using ResultDelegate = vp::util::IDelegateReturn<Result, const TaskResultInvokeInfo*>;

    struct AsyncTaskPushInfo {
        AsyncQueue       *queue;
        AsyncQueueThread *queue_thread;
        TaskDelegate     *task_delegate;
        ResultDelegate   *result_delegate;
        void             *user_data;
        u32               priority;
        bool              is_sync;
        
        constexpr AsyncQueue *GetQueue();
    };

    class AsyncTask {
        public:
            friend class AsyncQueue;
            friend class AsyncQueueThread;
            friend class AsyncTaskWatcher;
        public:
            enum class Status : u32 {
                Uninitialized = 0,
                Cancelled     = 1,
                Queued        = 2,
                Acquired      = 3,
                FreeExecute   = 4,
                PostExecute   = 5,
                Complete      = 6,
            };
        public:
            static constexpr u32 cInvalidPriorityLevel = 0xffff'ffff;
        protected:
            u16                          m_priority;
            union {
                u16 m_state;
                struct {
                    u16 m_is_free_for_allocator : 1;
                    u16 m_reserve0 : 15;
                };
            };                          
            u32                          m_status;
            AsyncQueue                  *m_queue;
            AsyncQueueThread            *m_queue_thread;
            TaskDelegate                *m_task_delegate;
            ResultDelegate              *m_result_delegate;
            void                        *m_user_data;
            sys::ServiceEvent            m_finish_event;
            vp::util::IntrusiveListNode  m_queue_list_node;
        public:
            VP_RTTI_BASE(AsyncTask);
        protected:
            virtual void Execute() {
                if (m_task_delegate == nullptr) { return; }
                m_task_delegate->Invoke(m_user_data);
            }

            virtual void PostExecute() {/*...*/}

            virtual void FreeExecute() {/*...*/}

            virtual void FormatPushInfo([[maybe_unused]] AsyncTaskPushInfo *push_info) {/*...*/}

            virtual void FreeCancel() {/*...*/}
        protected:
            bool TryInvokeSync();
        
            void Invoke(AsyncQueueThread *thread);

            void Cancel();
        public:
            AsyncTask() : m_priority(), m_state(), m_status(), m_queue(), m_queue_thread(), m_task_delegate(), m_result_delegate(), m_user_data(), m_finish_event(), m_queue_list_node() { m_finish_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Manual); }
            virtual ~AsyncTask() { m_finish_event.Finalize(); }

            Result PushTask(AsyncTaskPushInfo *push_info);

            void ChangePriority(u32 new_priority);

            constexpr ALWAYS_INLINE u32 GetPriority() const { return m_priority; }

            void Wait() {
                m_finish_event.Wait();
            }
    };
}

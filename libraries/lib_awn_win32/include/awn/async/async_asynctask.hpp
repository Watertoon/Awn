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
        bool       is_cancelled;
    };
    struct TaskCancelInvokeInfo {
        AsyncTask *task;
        void      *user_data;
    };

    using TaskFunction   = vp::util::IFunction<void(void*)>;
    using ResultFunction = vp::util::IFunction<Result(TaskResultInvokeInfo*)>;
    using CancelFunction = vp::util::IFunction<void(TaskCancelInvokeInfo*)>;

    using StaticTaskFunction   = vp::util::StaticFunction<void(void*)>;
    using StaticResultFunction = vp::util::StaticFunction<Result(TaskResultInvokeInfo*)>;
    using StaticCancelFunction = vp::util::StaticFunction<void(TaskCancelInvokeInfo*)>;

    struct AsyncTaskPushInfo {
        AsyncQueue       *queue;
        AsyncQueueThread *queue_thread;
        TaskFunction     *task_function;
        ResultFunction   *result_function;
        CancelFunction   *cancel_function;
        void             *user_data;
        u32               priority;
        bool              is_sync;
        bool              is_skip_finish_event;

        constexpr AsyncQueue *GetQueue();
    };

    class AsyncTask {
        public:
            friend class AsyncQueue;
            friend class AsyncQueueThread;
            friend class AsyncTaskWatcher;
        public:
            enum class Status : u16 {
                Uninitialized   = 0,
                Cancelled       = 1,
                ForceCancelled  = 2,
                Queued          = 3,
                Acquired        = 4,
                FreeExecute     = 5,
                PostExecute     = 6,
                Complete        = 7,
            };
        public:
            static constexpr u32 cInvalidPriorityLevel = 0xffff'ffff;
        protected:
            u32     m_priority;
            union {
                u16 m_state;
                struct {
                    u16 m_is_signal_finish_event : 1;
                    u16 m_is_cancelled           : 1;
                    u16 m_is_free_for_allocator  : 1;
                    u16 m_reserve0 : 13;
                };
            };                          
            u16                          m_status;
            AsyncQueue                  *m_queue;
            AsyncQueueThread            *m_queue_thread;
            TaskFunction                *m_task_function;
            ResultFunction              *m_result_function;
            CancelFunction              *m_cancel_function;
            void                        *m_user_data;
            sys::ServiceEvent            m_finish_event;
            vp::util::IntrusiveListNode  m_queue_list_node;
        public:
            VP_RTTI_BASE(AsyncTask);
        protected:
            virtual void Execute() {
                if (m_task_function == nullptr) { return; }
                m_task_function->Invoke(m_user_data);
            }

            virtual void PostExecute() {/*...*/}

            virtual void FreeExecute() {/*...*/}

            virtual void FormatPushInfo([[maybe_unused]] AsyncTaskPushInfo *push_info) {/*...*/}

            virtual void FreeCancel() {/*...*/}
        protected:
            bool TryInvokeSync();
            void InvokeSync(AsyncQueueThread *thread);

            void Invoke();

            void Cancel();

            void CancelWhileActive();
        public:
            AsyncTask() : m_priority(), m_state(), m_status(), m_queue(), m_queue_thread(), m_task_function(), m_result_function(), m_cancel_function(), m_user_data(), m_finish_event(), m_queue_list_node() { m_finish_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Manual); }
            virtual ~AsyncTask() { this->Finalize(); m_finish_event.Finalize(); }

            Result PushTask(AsyncTaskPushInfo *push_info);
            void Finalize();

            void CancelTask();

            void ChangePriority(u32 new_priority);

            constexpr ALWAYS_INLINE u32 GetPriority() const { return m_priority; }

            void Wait() {
                m_finish_event.Wait();
            }
            
            constexpr Status GetStatus() const { return static_cast<Status>(m_status); }
    };
}

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

    class DependencyJobThreadManager;

    struct DependencyJobThreadInfo {
        const char *name;
        DependencyJobThreadManager *thread_manager;
        u64 stack_size;
        u32 priority;
    };

    class DependencyJobThread : public sys::Thread {
        public:
            friend class DependencyJobThreadManager;
        public:
            static constexpr size_t cExitCode = 0;
            static constexpr size_t cMessageCount = 8;
        private:
            DependencyJobThreadControl m_thread_control;
            sys::Event m_finish_event;
            DependencyJobQueue *m_queue;
            DependencyJobThreadManager *m_thread_manager;
            u64 m_message_array[cMessageCount];
        public:
            VP_RTTI_DERIVED(DependencyJobThread, sys::Thread);
        public:
            constexpr DependencyJobThread() : Thread(), m_thread_control(), m_finish_event(), m_queue(), m_thread_manager(), m_message_array{} {/*...*/}
            virtual ~DependencyJobThread() override {/*...*/}

            Result Initialize(mem::Heap *heap, const DependencyJobThreadInfo *info) {
                sys::ThreadInfo thread_info = {
                    .name = info->name,
                    .exit_code = cExitCode,
                    .run_mode = sys::ThreadRunMode::WaitForMessage,
                    .message_count = cMessageCount,
                    .message_memory = m_message_array,
                    .stack_size = info->stack_size,
                    .priority = info->priority,
                };
                RESULT_TRY(this->Thread::Initialize(heap, std::addressof(thread_info)));
                m_thread_manager = info->thread_manager;
                RESULT_RETURN_SUCCESS;
            }

            virtual void ThreadMain(u64 message) override;

            constexpr void SetJobQueue(DependencyJobQueue *queue) { m_queue = queue; }
            constexpr DependencyJobThreadManager *GetThreadManager() { return m_thread_manager; }
            constexpr bool IsActive(sys::CoreMask active_mask) { return (active_mask & this->GetCoreMask()) != 0; }
    };

    struct DependencyJobThreadManagerInfo {
        sys::CoreMask per_core_worker_core_mask = 0x7;
        const char *name = "DepWorker";
        u64 worker_stack_size = 0x20000;
        u32 worker_priority = sys::cPriorityNormal;
    };

    class DependencyJobThreadManager {
        public:
            using DependencyJobThreadArray = vp::util::HeapArray<DependencyJobThread>;
        private:
            DependencyJobThreadArray m_thread_array;
            DependencyJobThreadControl m_main_thread_control;
            sys::CoreMask m_all_core_mask;
            sys::CoreMask m_active_core_mask;
            union {
                u32 m_core_flags;
                struct {
                    u32 m_is_process_in_mainthread : 1;
                    u32 m_reserve0                 : 31;
                };
            };
            DependencyJobQueue *m_queue;
        private:
            void BuildThreadControl();
            void StartThreads();
            void Dispatch();
        public:
            constexpr  DependencyJobThreadManager() : m_thread_array(), m_main_thread_control(), m_all_core_mask(), m_active_core_mask(), m_core_flags(), m_queue()  {/*...*/}
            constexpr ~DependencyJobThreadManager() {/*...*/}

            Result Initialize(mem::Heap *heap, const DependencyJobThreadManagerInfo *info);
            void Finalize();

            void SubmitGraph(DependencyJobQueue *job_queue, DependencyJobGraph *graph);
            void FinishRun();
    };
}

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

    class DependencyJobThread : public sys::Thread {
        public:
            friend class DependencyJobThreadManager;
        private:
            DependencyJobThreadControl  m_thread_control;
            sys::Event                  m_finish_event;
            DependencyJobQueue         *m_queue;
        public:
             DependencyJobThread(const char *name, mem::Heap *heap, u32 stack_size, u32 priority) : Thread(name, heap, sys::ThreadRunMode::WaitForMessage, 0, 8, stack_size, priority), m_thread_control(), m_finish_event(), m_queue() {/*...*/}
            ~DependencyJobThread() {/*...*/}

            virtual void ThreadMain(size_t message) override;

            constexpr void SetJobQueue(DependencyJobQueue *queue) { m_queue = queue; }
    };

    class DependencyJobThreadManager {
        public:
            struct WorkerContainer {
                vp::util::FixedString<0x10>  thread_name;
                DependencyJobThread         *thread;
            };
        public:
            using DependencyJobThreadArray = vp::util::HeapArray<WorkerContainer>;
        private:
            DependencyJobThreadArray    m_thread_array;
            DependencyJobThreadControl  m_main_thread_control;
            u64                         m_all_core_mask;
            u64                         m_active_core_mask;
            union {
                u32 m_core_flags;
                struct {
                    u32 m_is_process_in_mainthread : 1;
                    u32 m_reserve0                 : 31;
                };
            };
            DependencyJobQueue         *m_queue;
        private:
            void BuildThreadControl();

            void StartThreads();
        
            void Dispatch();
        public:
            constexpr  DependencyJobThreadManager() : m_thread_array(), m_main_thread_control(), m_all_core_mask(), m_active_core_mask(), m_core_flags(), m_queue()  {/*...*/}
            constexpr ~DependencyJobThreadManager() {/*...*/}

            void Initialize(mem::Heap *heap);
            void Finalize();

            void SubmitGraph(DependencyJobQueue *job_queue, DependencyJobGraph *graph);
            void FinishRun();
    };
}

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

    struct AsyncTaskScheduleInfo {
        AsyncTaskForAllocator *task_for_allocator;
        AsyncTaskPushInfo     *push_info;
        AsyncTaskWatcher      *watcher;
    };

    using AsyncTaskCreateFunction = vp::util::IFunction<AsyncTaskForAllocator*(AsyncTaskAllocator*, mem::Heap*)>;

    class AsyncTaskAllocator {
        public:
            friend class AsyncTaskForAllocator;
        public:
            using AsyncTaskList  = vp::util::IntrusiveListTraits<AsyncTaskForAllocator, &AsyncTaskForAllocator::m_queue_list_node>::List;
            using AsyncTaskArray = vp::util::PointerArray<AsyncTaskForAllocator>;
        private:
            sys::Event m_acquire_event;
            sys::Event m_available_event;
            sys::Mutex m_acquire_cs;
            sys::Mutex m_free_cs;
            u32 m_list_index;
            AsyncTaskList m_task_list[2];
            AsyncTaskArray m_task_array;
        private:
            AsyncTaskForAllocator *AcquireTaskImpl();
            AsyncTaskForAllocator *AcquireTask();
        protected:
            void FreeTask(AsyncTaskForAllocator *task);
        public:
            constexpr  AsyncTaskAllocator() : m_acquire_event(), m_available_event(), m_acquire_cs(), m_free_cs(), m_list_index(0), m_task_list{}, m_task_array()  {/*...*/}
            constexpr ~AsyncTaskAllocator() {/*...*/}

            void Initialize(mem::Heap *heap, AsyncTaskCreateFunction *create_function, u32 count);
            void Finalize();

            void ScheduleTask(AsyncTaskScheduleInfo *schedule_info);
    };
}

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

    class AsyncTaskForAllocator;

    class AsyncTaskWatcher {
        public:
            friend class AsyncTaskForAllocator;
        public:
            enum class State : u32 {
                Uninitialized = 0,
                Pending       = 1,
                Complete      = 2,
                Cancelled     = 3,
            };
        private:
            u32                    m_state;
            u32                    m_reference_count;
            AsyncTaskForAllocator *m_async_task;
            AsyncQueue            *m_queue;
        public:
            constexpr  AsyncTaskWatcher() : m_state(), m_reference_count(), m_async_task(), m_queue() {/*...*/}
            constexpr ~AsyncTaskWatcher() {/*...*/}

            void Reference();

            void ReleaseReference();

            void CancelTask();

            void WaitForCompletion();
    };
}

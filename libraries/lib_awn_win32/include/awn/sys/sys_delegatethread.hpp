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

namespace awn::sys {

    using ThreadIDelegate = vp::util::IDelegate<size_t>;

    class DelegateThread : public Thread {
        private:
            ThreadIDelegate *m_delegate;
        private:

            virtual void ThreadMain(size_t message) override {
                m_delegate->Invoke(message);
            }
        public:
            explicit DelegateThread(ThreadIDelegate *delegate, const char *name, mem::Heap *heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, u32 priority) : Thread(name, heap, run_mode, exit_code, max_messages, stack_size, priority) {
                VP_ASSERT(delegate != nullptr);

                m_delegate = delegate;
            }
            virtual ~DelegateThread() override {/*...*/}
    };

    template <typename T>
    using ThreadDelegate = vp::util::Delegate<T, size_t>;

    using ThreadDelegateFunction = vp::util::DelegateFunction<size_t>;

    class DelegateServiceThread : public ServiceThread {
        private:
            ThreadIDelegate *m_delegate;
        private:

            virtual void ThreadMain(size_t message) override {
                m_delegate->Invoke(message);
            }
        public:
            explicit DelegateServiceThread(ThreadIDelegate *delegate, const char *name, mem::Heap *heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, u32 priority) : ServiceThread(name, heap, run_mode, exit_code, max_messages, stack_size, priority) {
                VP_ASSERT(delegate != nullptr);

                m_delegate = delegate;
            }
            virtual ~DelegateServiceThread() override {/*...*/}
    };
}

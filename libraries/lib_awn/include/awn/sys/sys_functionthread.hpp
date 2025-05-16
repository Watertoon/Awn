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

    using ThreadFunction = vp::util::IFunction<void(size_t)>;

    class FunctionThread : public Thread {
        private:
            ThreadFunction *m_function;
        public:
            VP_RTTI_DERIVED(FunctionThread, Thread);
        private:
            virtual void ThreadMain(size_t message) override {
                m_function->Invoke(message);
            }
        public:
            constexpr FunctionThread() : Thread(), m_function() {}
            virtual ~FunctionThread() override {/*...*/}

            Result Initialize(mem::Heap *heap, const ThreadInfo *thread_info, ThreadFunction *function) {
                m_function = function;
                RESULT_TRY(this->Thread::Initialize(heap, thread_info));
                RESULT_RETURN_SUCCESS;
            }
    };

    template <typename T>
    using ThreadMemberFunction = vp::util::MemberFunction<T, void(size_t)>;

    using ThreadStaticFunction = vp::util::StaticFunction<void(size_t)>;
}

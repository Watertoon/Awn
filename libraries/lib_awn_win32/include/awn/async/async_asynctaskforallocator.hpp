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

    class AsyncTaskAllocator;

    class AsyncTaskForAllocator : public AsyncTask {
        public:
            friend class AsyncTaskAllocator;
            friend class AsyncTaskWatcher;
        private:
            AsyncTaskAllocator *m_task_allocator;
            AsyncTaskWatcher   *m_watcher;
        public:
            VP_RTTI_DERIVED(AsyncTaskForAllocator, AsyncTask);
        protected:
            void SetWatcher(AsyncTaskWatcher *watcher, AsyncQueue *queue);

            void FreeToAllocator();

            constexpr void SetAllocator(AsyncTaskAllocator *allocator) {                
                m_is_free_for_allocator = false;
                m_task_allocator        = allocator;
                m_queue_list_node.Unlink();
            }
        private:
            void ReleaseWatcher(AsyncTaskWatcher::State state);
        protected:
            virtual void Execute() override {
                this->AsyncTask::Execute();
                this->OnFinishExecute();
            }

            virtual void PostExecute() override {/*...*/}

            virtual void FreeExecute() override {
                this->ReleaseWatcher(AsyncTaskWatcher::State::Complete);
            }

            virtual void FormatPushInfo([[maybe_unused]] AsyncTaskPushInfo *push_info) override {/*...*/}

            virtual void FreeCancel() override {
                this->OnCancel();
                this->ReleaseWatcher(AsyncTaskWatcher::State::Cancelled);
            }

            virtual void OnCancel() {/*...*/}
            virtual void OnFinishExecute() {/*...*/}
        public:
            AsyncTaskForAllocator() : AsyncTask(), m_task_allocator(), m_watcher() { m_is_free_for_allocator = true; }
            virtual ~AsyncTaskForAllocator() override {/*...*/}
    };
}

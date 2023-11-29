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
            ~AsyncTaskForAllocator() {/*...*/}
    };
}

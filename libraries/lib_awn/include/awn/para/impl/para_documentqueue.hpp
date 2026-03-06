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

namespace awn::para::impl {

    struct DocumentTaskPushInfo : public async::AsyncTaskPushInfo {
        DocumentBinder      *document_binder;
        res::ResourceBinder  archive_binder;
        const char          *file_path;
        u8                   is_allow_dummy;
        u8                   is_reference_local_archive;
    };

    class DocumentTask : public async::AsyncTaskForAllocator {
        public:
            struct UserData {
                DocumentBinder       *document_binder;
                res::MaxPathString    file_path;
                res::ResourceBinder   bgyml_binder;
                res::ResourceBinder   archive_binder;
                u8                    is_allow_dummy;
                u8                    is_reference_local_archive;
            };
        private:
            UserData m_user_data;
        public:
            DocumentTask() : AsyncTaskForAllocator(), m_user_data() {/*...*/}
            virtual ~DocumentTask() override {/*...*/}

            virtual void FormatPushInfo(async::AsyncTaskPushInfo *push_info) override;
    };

    class DocumentWatcher;

    class DocumentQueue {
        private:
            async::AsyncQueueThread m_task_thread;
            async::AsyncQueue m_queue;
            async::AsyncTaskAllocator m_task_allocator;
            async::MemberTaskFunction<DocumentQueue> m_exe_function;
        public:
            AWN_SINGLETON_TRAITS(DocumentQueue);
        private:
            void Exe(void *user_data_base);
        public:
            constexpr DocumentQueue() : m_task_thread(), m_queue(), m_task_allocator(), m_exe_function(this, &DocumentQueue::Exe) {/*...*/}
            ~DocumentQueue();

            Result Initialize(mem::Heap *heap, u32 task_count);
            void Finalize();

            Result LoadDocumentAsync(DocumentBinderAsync *binder, const char *file_path, bool is_allow_dummy);
    };
}

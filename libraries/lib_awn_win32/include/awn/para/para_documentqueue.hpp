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

namespace awn::para {

    struct DocumentTaskPushInfo : public async::AsyncTaskPushInfo {
        DocumentBinder      *document_binder;
        res::ResourceBinder  archive_binder;
        const char          *file_path;
        u8                   is_allow_dummy;
        u8                   is_reference_local_archive;
    };

    class DocumentTask : public async::AsyncTask {
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
            DocumentTask() : AsyncTask(), m_user_data() {/*...*/}
            virtual ~DocumentTask() override {/*...*/}

            virtual void FormatPushInfo(async::AsyncTaskPushInfo *push_info) override {

                /* Cast push info */
                DocumentTaskPushInfo *document_push_info = reinterpret_cast<DocumentTaskPushInfo*>(push_info);

                /* Set document binder */
                m_user_data.document_binder = document_push_info->document_binder;

                /* Set file path */
                const char *path_no_work = AdvanceNoWorkExtension(document_push_info->file_path);
                m_user_data.file_path    = path_no_work;

                /* Find file extension */
                const u32 index = m_user_data.file_path.FindCharFromBack(0, '.');
                
                /* Attempt to load file */
                if (index != res::MaxPathString::cInvalidIndex) {

                    /* Clobber previous extension with bgyml */
                    m_user_data.file_path.Insert(index + 1, "bgyml");

                    /* Try to load bgyml async */
                    res::AsyncResourceLoadInfo load_info = {
                        .resource_factory = DocumentManager::GetInstance()->GetResourceFactory(),
                    };
                    m_user_data.bgyml_binder.TryLoadAsync(m_user_data.file_path.GetString(), std::addressof(load_info));
                }

                /* Reference archive binder */
                if (document_push_info->archive_binder.IsResourceInitialized() == true) {
                    m_user_data.archive_binder.ReferenceBinderSync(std::addressof(document_push_info->archive_binder));
                }

                /* Set push info user data */
                push_info->user_data = std::addressof(m_user_data);

                return;
            }
    };

    class DocumentQueue {
        private:
            async::AsyncQueueThread                  *m_task_thread;
            async::AsyncQueue                         m_queue;
            async::AsyncTaskAllocator                 m_task_allocator;
            async::MemberTaskFunction<DocumentQueue>  m_exe_function;
        private:
            void Exe(void *user_data_base) {

                /* Cast to our Task user data*/
                DocumentTask::UserData *user_data = reinterpret_cast<DocumentTask::UserData*>(user_data_base);
                
                /* Set local archive of bgyml if initialized */
                res::ScopedThreadLocalArchive local_archive_scope;
                if (user_data->archive_binder.IsResourceInitialized() == true) { local_archive_scope.SetArchive(std::addressof(user_data->archive_binder)); }
                
                /* Reference local archive if requested */
                if (user_data->is_reference_local_archive == true) { res::ReferenceThreadLocalArchive(); }

                /* Release thread local archive reference on exit */
                ON_SCOPE_EXIT {
                    user_data->archive_binder.Finalize();
                    user_data->bgyml_binder.Finalize();
                    if (user_data->is_reference_local_archive == true) { res::ReleaseThreadLocalArchive(); } 
                };

                /* Parse out file name and */
                const u32 extension_offset = user_data->file_path.FindCharFromBack(0, '.');
                const u32 class_offset     = user_data->file_path.FindCharFromBack(extension_offset + 1, '.');

                /* Split file path string to separate file and class names */
                char *file_path = user_data->file_path.m_string_array;
                file_path[class_offset]     = '\0';
                file_path[extension_offset] = '\0';

                /* Get file name and class name */
                const char *file_name  = file_path;
                const char *class_name = file_path + class_offset + 1;

                /* Wait for document to load */
                user_data->bgyml_binder.WaitForLoad();

                /* Try initialize resource */
                if (user_data->bgyml_binder.IsLoaded() == false) {
                    ResourceDocument::CreateInfo document_create_info(file_name, class_name);
                    const Result result = user_data->bgyml_binder.Complete(std::addressof(document_create_info));
                    RESULT_ABORT_UNLESS(result);
                }

                /* Load document */
                {
                    res::ScopedThreadLocalArchiveReference local_reference;
                    user_data->document_binder->LoadDocument(file_name, class_name, user_data->is_allow_dummy);
                }

                return;
            }
        public:
            constexpr DocumentQueue() : m_task_thread(), m_queue(), m_task_allocator(), m_exe_function(this, Exe) {/*...*/}
            ~DocumentQueue() {/*...*/}

            void Initialize();
            void Finalize();

            void LoadDocument();
    };
}

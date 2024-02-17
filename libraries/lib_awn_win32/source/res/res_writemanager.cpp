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
#include <awn.hpp>

namespace awn::res {

    void AsyncSaveManager::Read() {

        /* Clear requests on exit */
        ON_SCOPE_EXIT {
            vp::util::InterlockedAnd(std::addressof(m_request_flags), (1u << 5));
        };

        /* Handle pause */
        if (m_pause == true) { return; }

        /* Load file */
        FileDeviceBase *file_device = (m_user_file_device == nullptr) ? std::addressof(m_save_file_device) : m_user_file_device;
        FileLoadContext load_context = {
            .file_buffer = m_output,
            .file_size   = m_output_size,
        };
        RESULT_ABORT_UNLESS(file_device->LoadFile(m_save_path.GetString(), std::addressof(load_context)));

        return;
    }

    void AsyncSaveManager::Size() {

        /* Clear requests on exit */
        ON_SCOPE_EXIT {
            vp::util::InterlockedAnd(std::addressof(m_request_flags), (1u << 5));
        };

        /* Handle pause */
        if (m_pause == true) { return; }

        /* Get file size */
        FileDeviceBase *file_device = (m_user_file_device == nullptr) ? std::addressof(m_save_file_device) : m_user_file_device;
        file_device->GetFileSize(std::addressof(m_output_size), m_save_path.GetString());

        return;
    }

    void AsyncSaveManager::Save() {

        /* Clear requests on exit */
        ON_SCOPE_EXIT {
            vp::util::InterlockedAnd(std::addressof(m_request_flags), (1u << 5));
        };

        /* Handle pause */
        if (m_pause == true) { return; }

        /* Get file size */
        FileDeviceBase *file_device = (m_user_file_device == nullptr) ? std::addressof(m_save_file_device) : m_user_file_device;
        FileSaveInfo    save_file_info {
            .file        = m_output,
            .file_size   = m_output_size,
        };
        file_device->SaveFile(m_save_path.GetString(), std::addressof(save_file_info));

        return;
    }

    void AsyncSaveManager::Commit() {

        /* Clear requests on exit */
        ON_SCOPE_EXIT {
            vp::util::InterlockedAnd(std::addressof(m_request_flags), (1u << 5));
        };

        /* Handle pause */
        if (m_pause == true) { return; }

        /* Commit writes */
        FileDeviceBase *file_device = (m_user_file_device == nullptr) ? std::addressof(m_save_file_device) : m_user_file_device;
        file_device->Commit();

        return;
    }

    void AsyncSaveManager::Copy() {

        /* Clear requests on exit */
        ON_SCOPE_EXIT {
            vp::util::InterlockedAnd(std::addressof(m_request_flags), (1u << 5));
        };

        /* Handle pause */
        if (m_pause == true) { return; }

        /* Copy file */
        FileDeviceBase *file_device = (m_user_file_device == nullptr) ? std::addressof(m_save_file_device) : m_user_file_device;
        RESULT_ABORT_UNLESS(file_device->CopyFile(m_save_path.GetString(), m_copy_path.GetString(), m_output, m_output_size));

        return;
    }

    void AsyncSaveManager::ThreadMain(size_t message) {

        switch (static_cast<Message>(message)) {
            case Message::Read:
                this->Read();
                break;
            case Message::Size:
                this->Size();
                break;
            case Message::Save:
                this->Save();
                break;
            case Message::Commit:
                this->Commit();
                break;
            case Message::Copy:
                this->Copy();
                break;
            default:
                break;
        }

        m_user_file_device = nullptr;

        return;
    }

    void AsyncSaveManager::Initialize(mem::Heap *heap, const char *thread_name, u32 priority, sys::CoreMask core_mask) {

        /* Initialize thread */
        m_save_thread = new (heap, alignof(sys::DelegateThread)) sys::DelegateThread(std::addressof(m_delegate), thread_name, heap, sys::ThreadRunMode::WaitForMessage, 0, 4, 0x4000, priority);
        m_save_thread->SetCoreMask(core_mask);
        m_save_thread->StartThread();

        return;
    }
    void AsyncSaveManager::Finalize() {
        delete m_save_thread;
    }

    bool AsyncSaveManager::RequestRead(void *file_buffer, size_t buffer_size, const char *path, FileDeviceBase *user_file_device) {

        /* Set request flag and fail if another request was set */
        const u32 last_flags = vp::util::InterlockedFetchOr(std::addressof(m_request_flags), (1u << 0));
        if (last_flags != 0) { return false; }

        /* Call thread */
        m_output           = file_buffer;
        m_output_size      = buffer_size;
        m_save_path        = path;
        m_user_file_device = user_file_device;
        m_save_thread->SendMessage(static_cast<size_t>(Message::Read));

        return true;
    }
    bool AsyncSaveManager::RequestSize(const char *path, FileDeviceBase *user_file_device) {

        /* Set request flag and fail if another request was set */
        const u32 last_flags = vp::util::InterlockedFetchOr(std::addressof(m_request_flags), (1u << 1));
        if (last_flags != 0) { return false; }

        /* Call thread */
        m_output           = nullptr;
        m_output_size      = 0;
        m_save_path        = path;
        m_user_file_device = user_file_device;
        m_save_thread->SendMessage(static_cast<size_t>(Message::Size));

        return true;
    }
    bool AsyncSaveManager::RequestSave(void *save_data, size_t save_size, const char *path, FileDeviceBase *user_file_device) {

        /* Set request flag and fail if another request was set */
        const u32 last_flags = vp::util::InterlockedFetchOr(std::addressof(m_request_flags), (1u << 2));
        if (last_flags != 0) { return false; }

        /* Call thread */
        m_output           = save_data;
        m_output_size      = save_size;
        m_save_path        = path;
        m_user_file_device = user_file_device;
        m_save_thread->SendMessage(static_cast<size_t>(Message::Save));

        return true;
    }
    bool AsyncSaveManager::RequestCommit(FileDeviceBase *user_file_device) {

        /* Set request flag and fail if another request was set */
        const u32 last_flags = vp::util::InterlockedFetchOr(std::addressof(m_request_flags), (1u << 3));
        if (last_flags != 0) { return false; }

        /* Call thread */
        m_output      = nullptr;
        m_output_size = 0;
        m_save_path.Clear();
        m_user_file_device = user_file_device;
        m_save_thread->SendMessage(static_cast<size_t>(Message::Commit));

        return true;
    }
    bool AsyncSaveManager::RequestCopy(const char *dst_path, const char *src_path, void *copy_buffer, size_t copy_buffer_size, FileDeviceBase *user_file_device) {

        /* Set request flag and fail if another request was set */
        const u32 last_flags = vp::util::InterlockedFetchOr(std::addressof(m_request_flags), (1u << 4));
        if (last_flags != 0) { return false; }

        /* Call thread */
        m_output           = copy_buffer;
        m_output_size      = copy_buffer_size;
        m_save_path        = dst_path;
        m_copy_path        = src_path;
        m_user_file_device = user_file_device;
        m_save_thread->SendMessage(static_cast<size_t>(Message::Copy));

        return true;
    }
}

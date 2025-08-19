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

namespace awn::res::impl {

    AWN_SINGLETON_TRAITS_IMPL(FileDeviceManager);

    void FileDeviceManager::Initialize(mem::Heap *heap) {

#ifdef VP_TARGET_PLATFORM_win32
        const u32 cur_len = ::GetCurrentDirectoryA(m_current_directory.GetLength() - cCurrentDirectoryReserved, m_current_directory.GetString());
        VP_ASSERT(cur_len != 0);
        m_current_directory.SetLength(cur_len);
        m_current_directory.AssureTermination();
#endif

        /* Allocate and initialize file devices for content, cache, and save */
        ContentFileDevice *content_device = new (heap, alignof(ContentFileDevice)) ContentFileDevice();
        SaveFileDevice *save_device = new (heap, alignof(SaveFileDevice)) SaveFileDevice();
        CacheFileDevice *cache_device = new (heap, alignof(CacheFileDevice)) CacheFileDevice();
        m_content_file_device = content_device;
        m_save_file_device = save_device;
        m_cache_file_device = cache_device;

        this->RegisterFileDevice(content_device);
        this->RegisterFileDevice(save_device);
        this->RegisterFileDevice(cache_device);

        return;
    }

    void FileDeviceManager::Finalize() {
        if (m_content_file_device != nullptr) {
            delete m_content_file_device;
            m_content_file_device = nullptr;
        }
        if (m_save_file_device != nullptr) {
            delete m_save_file_device;
            m_save_file_device = nullptr;
        }
        if (m_cache_file_device != nullptr) {
            delete m_cache_file_device;
            m_cache_file_device = nullptr;
        }
    }

    void FileDeviceManager::RegisterFileDevice(FileDeviceBase *file_device) {
        VP_ASSERT(file_device != nullptr);
        if (file_device->m_manager_tree_node.IsLinked()) { return; }
        m_device_tree_rwlock.AcquireWriteLock();
        ON_SCOPE_EXIT { m_device_tree_rwlock.ReleaseWriteLock(); };

        m_mounted_file_device_tree.Insert(file_device);
        return;
    }

    void FileDeviceManager::RemoveFileDevice(FileDeviceBase *file_device) {
        VP_ASSERT(file_device != nullptr);
        if (!file_device->m_manager_tree_node.IsLinked()) { return; }
        m_device_tree_rwlock.AcquireWriteLock();
        ON_SCOPE_EXIT { m_device_tree_rwlock.ReleaseWriteLock(); };

        m_mounted_file_device_tree.Remove(file_device);
        return;
    }

    FileDeviceBase *FileDeviceManager::GetFileDeviceByDrive(const char *drive) {
        const u32 hash = vp::util::HashCrc32b(drive);
        m_device_tree_rwlock.AcquireReadLock();
        ON_SCOPE_EXIT { m_device_tree_rwlock.ReleaseReadLock(); };
        FileDeviceBase *device = m_mounted_file_device_tree.Find(hash);
        return device;
    }

    FileDeviceBase *FileDeviceManager::GetFileDeviceByPath(const char *path) {

        /* Get drive */
        MaxDriveString drive;
        vp::util::GetDrive(std::addressof(drive), path);

        return this->GetFileDeviceByDrive(drive.GetString());
    }

    Result FileDeviceManager::LoadFile(const char *path, FileLoadContext *file_load_context) {

        /* Find file device */
        FileDeviceBase *device = this->GetFileDeviceByPath(path);

        /* Get path without drive */
        MaxPathString path_no_drive;
        vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

        /* Load */
        return device->LoadFile(path_no_drive.GetString(), file_load_context);
    }

    Result FileDeviceManager::LoadFileWithDecompressor(const char *path, FileLoadContext *file_load_context, IDecompressor *decompressor) {

        /* Find file device */
        FileDeviceBase *device = this->GetFileDeviceByPath(path);

        /* Get path without drive */
        MaxPathString path_no_drive;
        vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

        /* Load */
        return decompressor->LoadDecompressFile(nullptr, nullptr, path_no_drive.GetString(), file_load_context, device);
    }

    Result FileDeviceManager::SaveFile(const char *path, FileSaveInfo *save_info) {

        /* Find file device */
        FileDeviceBase *device = this->GetFileDeviceByPath(path);

        /* Get path without drive */
        MaxPathString path_no_drive;
        vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

        /* Load */
        return device->SaveFile(path_no_drive.GetString(), save_info);
    }

    Result FileDeviceManager::SaveFileWithCompressor(const char *path, FileSaveInfo *save_info, ICompressor *compressor) {

        /* Find file device */
        FileDeviceBase *device = this->GetFileDeviceByPath(path);

        /* Get path without drive */
        MaxPathString path_no_drive;
        vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

        /* Load */
        return compressor->SaveCompressFile(path_no_drive.GetString(), save_info, device);
    }
}

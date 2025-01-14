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

        /* Allocate and initialize ContentFileDevice */
        ContentFileDevice *content_device = new (heap, alignof(ContentFileDevice)) ContentFileDevice();
        m_main_file_device = content_device;

        this->RegisterFileDevice(content_device);

        return;
    }

    void FileDeviceManager::Finalize() {
        if (m_main_file_device != nullptr) {
            delete m_main_file_device;
            m_main_file_device = nullptr;
        }
    }

    void FileDeviceManager::RegisterFileDevice(FileDeviceBase *file_device) {
        VP_ASSERT(file_device != nullptr);
        std::scoped_lock l(m_device_tree_cs);
        VP_ASSERT(!file_device->m_manager_tree_node.IsLinked());
        m_mounted_file_device_tree.Insert(file_device);
    }

    void FileDeviceManager::RemoveFileDevice(FileDeviceBase *file_device) {
        VP_ASSERT(file_device != nullptr);
        std::scoped_lock l(m_device_tree_cs);
        if (!file_device->m_manager_tree_node.IsLinked()) { return; }
        m_mounted_file_device_tree.Remove(file_device);
    }

    FileDeviceBase *FileDeviceManager::GetFileDeviceByDrive(const char *drive) {
        const u32 hash = vp::util::HashCrc32b(drive);
        std::scoped_lock l(m_device_tree_cs);
        return m_mounted_file_device_tree.Find(hash);
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
        vp::util::FixedString<vp::util::cMaxPath> path_no_drive;
        vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

        /* Load */
        return device->LoadFile(path_no_drive.GetString(), file_load_context);
    }

    Result FileDeviceManager::LoadFileWithDecompressor(const char *path, FileLoadContext *file_load_context, IDecompressor *decompressor) {

        /* Find file device */
        FileDeviceBase *device = this->GetFileDeviceByPath(path);

        /* Get path without drive */
        vp::util::FixedString<vp::util::cMaxPath> path_no_drive;
        vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

        /* Load */
        return decompressor->LoadDecompressFile(nullptr, nullptr, path_no_drive.GetString(), file_load_context, device);
    }
}

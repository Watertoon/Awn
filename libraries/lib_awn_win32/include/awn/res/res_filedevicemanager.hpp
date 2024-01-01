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

namespace awn::res {

    class FileDeviceManager {
        private:
            using FileDeviceTree = vp::util::IntrusiveRedBlackTreeTraits<FileDeviceBase, &FileDeviceBase::m_manager_tree_node>::Tree;
        private:
            sys::ServiceCriticalSection  m_device_tree_cs;
            FileDeviceTree               m_mounted_file_device_tree;
            ContentFileDevice           *m_main_file_device;
        public:
            AWN_SINGLETON_TRAITS(FileDeviceManager);
        public:
            constexpr ALWAYS_INLINE  FileDeviceManager() : m_device_tree_cs(), m_mounted_file_device_tree(), m_main_file_device() {/*...*/}
            constexpr ALWAYS_INLINE ~FileDeviceManager() {/*...*/}

            void Initialize(mem::Heap *heap) {

                /* Allocate and initialize ContentFileDevice */
                ContentFileDevice *content_device = new (heap, alignof(ContentFileDevice)) ContentFileDevice();
                VP_ASSERT(content_device != nullptr);

                std::construct_at(content_device);

                m_main_file_device = content_device;

                this->AddFileDevice(content_device);
            }

            void Finalize() {
                if (m_main_file_device != nullptr) {
                    delete m_main_file_device;
                }
                m_main_file_device = nullptr;
            }

            void AddFileDevice(FileDeviceBase *file_device) {
                VP_ASSERT(file_device != nullptr);
                std::scoped_lock l(m_device_tree_cs);
                m_mounted_file_device_tree.Insert(file_device);
            }

            FileDeviceBase *GetFileDeviceByName(const char *device_name) {
                const u32 hash = vp::util::HashCrc32b(device_name);
                std::scoped_lock l(m_device_tree_cs);
                return m_mounted_file_device_tree.Find(hash);
            }

            Result LoadFile(const char *path, FileLoadContext *file_load_context) {

                /* Get drive */
                MaxDriveString drive;
                vp::util::GetDrive(std::addressof(drive), path);

                /* Find file device */
                FileDeviceBase *device = this->GetFileDeviceByName(drive.GetString());

                /* Get path without drive */
                vp::util::FixedString<vp::util::cMaxPath> path_no_drive;
                vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

                /* Load */
                return device->LoadFile(path_no_drive.GetString(), file_load_context);
            }
    };
}

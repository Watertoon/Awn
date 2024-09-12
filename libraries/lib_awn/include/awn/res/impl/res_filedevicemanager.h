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

namespace awn::res::impl {

    class FileDeviceManager {
        private:
            using FileDeviceTree = vp::util::IntrusiveRedBlackTreeTraits<FileDeviceBase, &FileDeviceBase::m_manager_tree_node>::Tree;
        private:
            sys::Mutex m_device_tree_cs;
            FileDeviceTree m_mounted_file_device_tree;
            ContentFileDevice *m_main_file_device;
        public:
            AWN_SINGLETON_TRAITS(FileDeviceManager);
        public:
            constexpr ALWAYS_INLINE  FileDeviceManager() : m_device_tree_cs(), m_mounted_file_device_tree(), m_main_file_device() {/*...*/}
            constexpr ALWAYS_INLINE ~FileDeviceManager() {/*...*/}

            void Initialize(mem::Heap *heap);
            void Finalize();

            void AddFileDevice(FileDeviceBase *file_device);
    
            Result LoadFile(const char *path, FileLoadContext *file_load_context);

            FileDeviceBase *GetFileDeviceByDrive(const char *drive);
            FileDeviceBase *GetFileDeviceByPath(const char *path);

            constexpr FileDeviceBase *GetDefaultFileDevice() { return m_main_file_device; }
    };
}

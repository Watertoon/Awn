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
            static constexpr u32 cCurrentDirectoryReserved = 110;
        private:
            using FileDeviceTree = vp::util::IntrusiveRedBlackTreeTraits<FileDeviceBase, &FileDeviceBase::m_manager_tree_node>::Tree;
        private:
            sys::ReaderWriterLock m_device_tree_rwlock;
            FileDeviceTree m_mounted_file_device_tree;
            ContentFileDevice *m_content_file_device;
            SaveFileDevice *m_save_file_device;
            CacheFileDevice *m_cache_file_device;
            MaxPathString m_current_directory;
        public:
            AWN_SINGLETON_TRAITS(FileDeviceManager);
        public:
            constexpr ALWAYS_INLINE  FileDeviceManager() : m_device_tree_rwlock(), m_mounted_file_device_tree(), m_content_file_device(), m_save_file_device(), m_cache_file_device(), m_current_directory()
            {/*...*/}
            constexpr ALWAYS_INLINE ~FileDeviceManager() {/*...*/}

            void Initialize(mem::Heap *heap);
            void Finalize();

            void RegisterFileDevice(FileDeviceBase *file_device);
            void RemoveFileDevice(FileDeviceBase *file_device);
    
            Result LoadFile(const char *path, FileLoadContext *file_load_context);
            Result LoadFileWithDecompressor(const char *path, FileLoadContext *file_load_context, IDecompressor *decompressor);
    
            Result SaveFile(const char *path, FileSaveInfo *save_info);
            Result SaveFileWithCompressor(const char *path, FileSaveInfo *save_info, ICompressor *compressor);

            FileDeviceBase *GetFileDeviceByDrive(const char *drive);
            FileDeviceBase *GetFileDeviceByPath(const char *path);

            constexpr FileDeviceBase *GetDefaultFileDevice() { return m_content_file_device; }
            constexpr MaxPathString *GetCurrentDirectory() { return std::addressof(m_current_directory); }
    };
}

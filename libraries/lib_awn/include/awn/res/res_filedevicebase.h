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

    namespace impl {
        class FileDeviceManager;
    }

    using MaxPathString      = vp::util::MaxPathString;
    using MaxDriveString     = vp::util::MaxDriveString;

    enum class OpenMode : u32 {
        Read            = (1 << 0),
        Write           = (1 << 1),
        ReadWrite       = (1 << 1) | (1 << 0),
        Append          = (1 << 2),
        ReadWriteAppend = (1 << 2) | (1 << 1) | (1 << 0),
    };
    VP_ENUM_FLAG_TRAITS(OpenMode);

#ifdef VP_TARGET_PLATFORM_win32
    using NativeFileHandle = HANDLE;
    using NativeDirectoryHandle = HANDLE;

    constexpr u32 AwnOpenModeToWin32AccessRights(OpenMode open_mode) {
        u32 access_mask = 0;
        if ((open_mode & OpenMode::Read) == OpenMode::Read) {
            access_mask |= GENERIC_READ;
        }
        if ((open_mode & OpenMode::Write) == OpenMode::Write) {
            access_mask |= GENERIC_WRITE;
        }
        return access_mask;
    }
#elifdef VP_TARGET_PLATFORM_nx
    using NativeFileHandle = nn::fs::FileHandle;
    using NativeDirectoryHandle = nn::fs::DirectoryHandle;

    constexpr s32 AwnOpenModeToNnFsOpenMode(OpenMode open_mode) {
        return static_cast<s32>(open_mode);
    }
#elifdef VP_TARGET_PLATFORM_linux
    using NativeFileHandle = int;
    using NativeDirectoryHandle = DIR*;
#endif

    struct DirectoryEntry {
        u32           archive_entry_index;
        size_t        file_size;
        MaxPathString file_path;
    };

    struct DirectoryHandle {
        static constexpr inline u32 cMaxDirectoryDepth = 8;

        u32 entry_index;
        u32 directory_depth;
        NativeDirectoryHandle search_handle_array[cMaxDirectoryDepth];
        MaxPathString directory_path_array[cMaxDirectoryDepth];
    };

    constexpr inline const u32 cInvalidEntryIndex = 0xffff'ffff;
    static_assert(cInvalidEntryIndex == vp::res::SarcExtractor::cInvalidEntryIndex);
    static_assert(cInvalidEntryIndex == vp::res::ResNintendoWareDictionary::cInvalidEntryIndex);

    class FileDeviceBase;

    struct FileHandle {
        union {
            NativeFileHandle handle;
            void *archive_file;
        };
        FileDeviceBase *file_device;
        u32             archive_entry_index;
        u32             open_mode;
        size_t          file_offset;
        size_t          file_size;

        constexpr void SetDefaults() {
            handle              = 0;
            file_device         = nullptr;
            archive_entry_index = cInvalidEntryIndex;
            file_offset         = 0;
            file_size           = 0;
            open_mode           = static_cast<u32>(OpenMode::Read);
        }

        Result Close();
    };

    struct FileLoadContext {
        mem::Heap  *heap;
        u32         read_div;
        bool        out_is_file_memory_allocated;
        void       *file;
        size_t      file_size;
        s32         file_alignment;
    };

    struct FileSaveInfo {
        void   *file;
        size_t  file_size;
    };

    class FileDeviceBase {
        private:
            friend class impl::FileDeviceManager;
        public:
            static constexpr s32    cMinimumFileAlignment = 0x20;
            static constexpr size_t cMaxAutoReadSize      = 0x20;
        protected:
            vp::util::IntrusiveRedBlackTreeNode<u32> m_manager_tree_node;
            MaxDriveString m_drive;
        public:
            VP_RTTI_BASE(FileDeviceBase);
        protected:
            virtual Result LoadFileImpl(const char *path, FileLoadContext *file_load_context);
            virtual Result SaveFileImpl(const char *path, FileSaveInfo *file_save_info);
            virtual Result CreateFileImpl(FileHandle *out_file_handle, const char *path, size_t file_size) { VP_ASSERT(false); VP_UNUSED(out_file_handle, path, file_size); RESULT_RETURN_SUCCESS; }
            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode)                                             { VP_ASSERT(false); VP_UNUSED(out_file_handle, path, open_mode); RESULT_RETURN_SUCCESS; }
            virtual Result CloseFileImpl(FileHandle *file_handle)                                                                                      { VP_ASSERT(false); VP_UNUSED(file_handle); RESULT_RETURN_SUCCESS; }
            virtual Result ReadFileImpl(void *read_buffer, size_t *out_read_size, FileHandle *file_handle, size_t read_size, size_t file_offset)       { VP_ASSERT(false); VP_UNUSED(read_buffer, out_read_size, file_handle, read_size, file_offset); RESULT_RETURN_SUCCESS; }
            virtual Result WriteFileImpl(size_t *out_written_size, FileHandle *file_handle, void *write_buffer, size_t write_size, size_t file_offset) { VP_ASSERT(false); VP_UNUSED(out_written_size, file_handle, write_buffer, write_size, file_offset); RESULT_RETURN_SUCCESS; }
            virtual Result CopyFileImpl(const char *dest_path, const char *source_path, void *copy_buffer, size_t copy_size);
            virtual Result CommitImpl()                                                                                                                { VP_ASSERT(false); RESULT_RETURN_SUCCESS; }
            virtual Result FlushFileImpl(FileHandle *file_handle)                                                                                      { VP_ASSERT(false); VP_UNUSED(file_handle); RESULT_RETURN_SUCCESS; }

            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) { VP_ASSERT(false); VP_UNUSED(out_size, file_handle); RESULT_RETURN_SUCCESS; }
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path)        { VP_ASSERT(false); VP_UNUSED(out_size, path); RESULT_RETURN_SUCCESS; }

            virtual Result CheckFileExistsImpl(const char *path) { VP_ASSERT(false); VP_UNUSED(path); RESULT_RETURN_SUCCESS; }

            virtual Result CreateDirectoryImpl(const char *path) { VP_ASSERT(false);  VP_UNUSED(path); RESULT_RETURN_SUCCESS; }

            virtual Result OpenDirectoryImpl(DirectoryHandle *out_directory_handle, const char *path)                         { VP_ASSERT(false); VP_UNUSED(out_directory_handle, path); RESULT_RETURN_SUCCESS; }
            virtual Result CloseDirectoryImpl(DirectoryHandle *directory_handle)                                              { VP_ASSERT(false); VP_UNUSED(directory_handle); RESULT_RETURN_SUCCESS; }
            virtual Result ReadDirectoryImpl(u32 *out_read_count, DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) { VP_ASSERT(false); VP_UNUSED(out_read_count, directory_handle, entry_array, entry_count); RESULT_RETURN_SUCCESS; }

            virtual Result CheckDirectoryExistsImpl(const char *path) { VP_ASSERT(false); VP_UNUSED(path); RESULT_RETURN_SUCCESS; }

            virtual Result FormatPath(MaxPathString *out_formatted_path, const char *path) { 
                out_formatted_path->SetString(path);
                RESULT_RETURN_SUCCESS;
            }

            virtual u32 GetDriveSize() const { return 0; }
        public:
            constexpr FileDeviceBase() : m_manager_tree_node(), m_drive() {/*...*/}
            constexpr FileDeviceBase(const char *drive_name) : m_manager_tree_node(), m_drive(drive_name) {
                const u32 hash = vp::util::HashCrc32b(drive_name);
                m_manager_tree_node.SetKey(hash);
            }
            virtual ~FileDeviceBase();

            Result LoadFile(const char *path, FileLoadContext *file_load_context)                                                          { return this->LoadFileImpl(path, file_load_context); }
            Result SaveFile(const char *path, FileSaveInfo *file_save_info)                                                                { return this->SaveFileImpl(path, file_save_info); }
            Result CreateFile(FileHandle *out_file_handle, const char *path, size_t size)                                                  { return this->CreateFileImpl(out_file_handle, path, size); }
            Result OpenFile(FileHandle *out_file_handle, const char *path, OpenMode open_mode)                                             { return this->OpenFileImpl(out_file_handle, path, open_mode); }
            Result CloseFile(FileHandle *file_handle)                                                                                      { return this->CloseFileImpl(file_handle); }
            Result ReadFile(void *read_buffer, size_t *out_read_size, FileHandle *file_handle, size_t read_size, size_t file_offset)       { return this->ReadFileImpl(read_buffer, out_read_size, file_handle, read_size, file_offset); }
            Result WriteFile(size_t *out_written_size, FileHandle *file_handle, void *write_buffer, size_t write_size, size_t file_offset) { return this->WriteFileImpl(out_written_size, file_handle, write_buffer, write_size, file_offset); }
            Result CopyFile(const char *dest_path, const char *source_path, void *copy_buffer, size_t copy_size)                           { return this->CopyFileImpl(dest_path, source_path, copy_buffer, copy_size); }
            Result FlushFile(FileHandle *file_handle)                                                                                      { return this->FlushFileImpl(file_handle); }

            Result Commit() { return this->CommitImpl(); }
            Result GetFileSize(size_t *out_size, FileHandle *file_handle) { return this->GetFileSizeImpl(out_size, file_handle); }
            Result GetFileSize(size_t *out_size, const char *path)        { return this->GetFileSizeImpl(out_size, path); }

            Result CheckFileExists(const char *path) { return CheckFileExistsImpl(path); }
        public:
            Result CreateDirectory(const char *path) { return this->CreateDirectoryImpl(path); }
            Result OpenDirectory(DirectoryHandle *out_directory_handle, const char *path)                         { return this->OpenDirectoryImpl(out_directory_handle, path); }
            Result CloseDirectory(DirectoryHandle *directory_handle)                                              { return this->CloseDirectoryImpl(directory_handle); }
            Result ReadDirectory(u32 *out_read_count, DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) { return this->ReadDirectoryImpl(out_read_count, directory_handle, entry_array, entry_count); }

            Result CheckDirectoryExists(const char *path) { return this->CheckDirectoryExistsImpl(path); }
        public:
            constexpr const char *GetDeviceName() const { return m_drive.GetString(); }
    };
}

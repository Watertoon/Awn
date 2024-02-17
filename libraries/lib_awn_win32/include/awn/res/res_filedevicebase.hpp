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

    using MaxPathString      = vp::util::MaxPathString;
    using MaxDriveString     = vp::util::MaxDriveString;
    using MaxExtensionString = vp::util::MaxExtensionString;

    enum class CompressionType : u16 {
        None        = (1 << 0),
        Auto        = (1 << 1),
        Szs         = (1 << 2),
        Zstandard   = (1 << 3),
    };
    VP_ENUM_FLAG_TRAITS(CompressionType);

    constexpr CompressionType GetCompressionType(MaxExtensionString *extension) {
        const MaxExtensionString zs_ext(".zs");
        if (*extension == zs_ext) {
            return CompressionType::Zstandard;
        }
        const MaxExtensionString szs_ext(".szs");
        if (*extension == szs_ext) {
            return CompressionType::Szs;
        }
        return CompressionType::None;
    }

    enum class OpenMode : u32 {
        Read            = GENERIC_READ,
        Write           = GENERIC_WRITE,
        ReadWrite       = GENERIC_READ | GENERIC_WRITE,
        ReadWriteAppend = static_cast<u32>(OpenMode::ReadWrite) | 0x1,
    };
    VP_ENUM_FLAG_TRAITS(OpenMode);

    struct DirectoryEntry {
        u32           archive_entry_index;
        size_t        file_size;
        MaxPathString file_path;
    };

    struct DirectoryHandle {
        static constexpr inline u32 cMaxDirectoryDepth = 8;

        u32           entry_index;
        u32           directory_depth;
        HANDLE        search_handle_array[cMaxDirectoryDepth];
        MaxPathString directory_path_array[cMaxDirectoryDepth];
    };

    constexpr inline const u32 cInvalidEntryIndex = 0xffff'ffff;
    static_assert(cInvalidEntryIndex == vp::res::SarcExtractor::cInvalidEntryIndex);
    static_assert(cInvalidEntryIndex == vp::res::ResNintendoWareDictionary::cInvalidEntryIndex);

    class FileDeviceBase;

    struct FileHandle {
        void           *handle;
        FileDeviceBase *file_device;
        u32             archive_entry_index;
        u32             open_mode;
        size_t          file_offset;
        size_t          file_size;

        constexpr void SetDefaults() {
            handle              = INVALID_HANDLE_VALUE;
            file_device         = nullptr;
            archive_entry_index = cInvalidEntryIndex;
            file_offset         = 0;
            file_size           = 0;
            open_mode           = static_cast<u32>(OpenMode::Read);
        }

        ALWAYS_INLINE Result Close();
    };

    struct FileLoadContext {
        mem::Heap  *file_heap;
        u32         read_div;
        bool        out_is_file_memory_allocated;
        void       *file_buffer;
        size_t      file_size;
        s32         file_alignment;
    };

    struct FileSaveInfo {
        void   *file;
        size_t  file_size;
    };

    class FileDeviceBase {
        private:
            friend class FileDeviceManager;
        public:
            static constexpr s32    cMinimumFileAlignment = 0x20;
            static constexpr size_t cMaxAutoReadSize      = 0x20;
        protected:
            vp::util::IntrusiveRedBlackTreeNode<u32> m_manager_tree_node;
            MaxDriveString                           m_device_name;
        public:
            VP_RTTI_BASE(FileDeviceBase);
        protected:
            virtual Result LoadFileImpl(const char *path, FileLoadContext *file_load_context);
            virtual Result SaveFileImpl(const char *path, FileSaveInfo *file_save_info);
            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode)                                             { VP_ASSERT(false); VP_UNUSED(out_file_handle, path, open_mode); }
            virtual Result CloseFileImpl(FileHandle *file_handle)                                                                                      { VP_ASSERT(false); VP_UNUSED(file_handle); }
            virtual Result ReadFileImpl(void *read_buffer, size_t *out_read_size, FileHandle *file_handle, size_t read_size, size_t file_offset)       { VP_ASSERT(false); VP_UNUSED(read_buffer, out_read_size, file_handle, read_size, file_offset); }
            virtual Result WriteFileImpl(size_t *out_written_size, FileHandle *file_handle, void *write_buffer, size_t write_size, size_t file_offset) { VP_ASSERT(false); VP_UNUSED(out_written_size, file_handle, write_buffer, write_size, file_offset); }
            virtual Result CopyFileImpl(const char *dest_path, const char *source_path, void *copy_buffer, size_t copy_size);
            virtual Result CommitImpl()                                                                                                                { VP_ASSERT(false); }
            virtual Result FlushFileImpl(FileHandle *file_handle)                                                                                      { VP_ASSERT(false); VP_UNUSED(file_handle); }

            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) { VP_ASSERT(false); VP_UNUSED(out_size, file_handle); }
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path)        { VP_ASSERT(false); VP_UNUSED(out_size, path); }

            virtual Result CheckFileExistsImpl(const char *path) { VP_ASSERT(false); VP_UNUSED(path); }

            virtual Result CreateDirectoryImpl(const char *path) { VP_ASSERT(false);  VP_UNUSED(path); }

            virtual Result OpenDirectoryImpl(DirectoryHandle *out_directory_handle, const char *path)                         { VP_ASSERT(false); VP_UNUSED(out_directory_handle, path); }
            virtual Result CloseDirectoryImpl(DirectoryHandle *directory_handle)                                              { VP_ASSERT(false); VP_UNUSED(directory_handle); }
            virtual Result ReadDirectoryImpl(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) { VP_ASSERT(false); VP_UNUSED(directory_handle, entry_array, entry_count); }

            virtual Result CheckDirectoryExistsImpl(const char *path) { VP_ASSERT(false); VP_UNUSED(path); }

            virtual Result FormatPath(vp::util::FixedString<vp::util::cMaxPath> *out_formatted_path, const char *path) { VP_ASSERT(false); VP_UNUSED(out_formatted_path, path); }

            virtual u32 GetDriveSize() const { return 0; }
        public:
            explicit constexpr ALWAYS_INLINE FileDeviceBase() : m_manager_tree_node(), m_device_name() {/*...*/}
            explicit constexpr ALWAYS_INLINE FileDeviceBase(const char *device_name) : m_manager_tree_node(), m_device_name(device_name) {
                const u32 hash = vp::util::HashCrc32b(device_name);
                m_manager_tree_node.SetKey(hash);
            }
            constexpr virtual ALWAYS_INLINE ~FileDeviceBase() {/*...*/}

            ALWAYS_INLINE Result LoadFile(const char *path, FileLoadContext *file_load_context)                                                          { return this->LoadFileImpl(path, file_load_context); }
            ALWAYS_INLINE Result SaveFile(const char *path, FileSaveInfo *file_save_info)                                                                { return this->SaveFileImpl(path, file_save_info); }
            ALWAYS_INLINE Result OpenFile(FileHandle *out_file_handle, const char *path, OpenMode open_mode)                                             { return this->OpenFileImpl(out_file_handle, path, open_mode); }
            ALWAYS_INLINE Result CloseFile(FileHandle *file_handle)                                                                                      { return this->CloseFileImpl(file_handle); }
            ALWAYS_INLINE Result ReadFile(void *read_buffer, size_t *out_read_size, FileHandle *file_handle, size_t read_size, size_t file_offset)       { return this->ReadFileImpl(read_buffer, out_read_size, file_handle, read_size, file_offset); }
            ALWAYS_INLINE Result WriteFile(size_t *out_written_size, FileHandle *file_handle, void *write_buffer, size_t write_size, size_t file_offset) { return this->WriteFileImpl(out_written_size, file_handle, write_buffer, write_size, file_offset); }
            ALWAYS_INLINE Result CopyFile(const char *dest_path, const char *source_path, void *copy_buffer, size_t copy_size)                           { return this->CopyFileImpl(dest_path, source_path, copy_buffer, copy_size); }
            ALWAYS_INLINE Result FlushFile(FileHandle *file_handle)                                                                                      { return this->FlushFileImpl(file_handle); }

            ALWAYS_INLINE Result Commit() { return this->CommitImpl(); }
    
            ALWAYS_INLINE Result GetFileSize(size_t *out_size, FileHandle *file_handle) { return this->GetFileSizeImpl(out_size, file_handle); }
            ALWAYS_INLINE Result GetFileSize(size_t *out_size, const char *path)        { return this->GetFileSizeImpl(out_size, path); }

            ALWAYS_INLINE Result CheckFileExists(const char *path) { return CheckFileExistsImpl(path); }

            ALWAYS_INLINE Result CreateDirectory(const char *path) { 

                /* Nothing to do if directory already exists */
                const Result check_dir_result = this->CheckDirectoryExists(path);
                if (check_dir_result != ResultDirectoryNotFound) { return check_dir_result; }

                return this->CreateDirectoryImpl(path); 
            }
            ALWAYS_INLINE Result OpenDirectory(DirectoryHandle *out_directory_handle, const char *path)                         { return this->OpenDirectoryImpl(out_directory_handle, path); }
            ALWAYS_INLINE Result CloseDirectory(DirectoryHandle *directory_handle)                                              { return this->CloseDirectoryImpl(directory_handle); }
            ALWAYS_INLINE Result ReadDirectory(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) { return this->ReadDirectoryImpl(directory_handle, entry_array, entry_count); }

            ALWAYS_INLINE Result CheckDirectoryExists(const char *path) { return this->CheckDirectoryExistsImpl(path); }

            constexpr ALWAYS_INLINE const char *GetDeviceName() const { return m_device_name.GetString(); }
    };

    ALWAYS_INLINE Result FileHandle::Close() {
        if (file_device == nullptr) { RESULT_RETURN_SUCCESS; }
        const Result result = file_device->CloseFile(this);
        file_device         = nullptr;
        return result;
    }
}

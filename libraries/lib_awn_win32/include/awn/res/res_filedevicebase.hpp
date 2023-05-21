#pragma once

namespace awn::res {

    enum OpenMode : u32 {
        OpenMode_Read            = GENERIC_READ,
        OpenMode_Write           = GENERIC_WRITE,
        OpenMode_ReadWrite       = GENERIC_READ | GENERIC_WRITE,
        OpenMode_ReadWriteAppend = OpenMode_ReadWrite | 0x1,
    };

    struct DirectoryEntry {
        const char *file_path;
    };

    class DirectoryEntryArray {
        private:
            vp::util::HeapArray<DirectoryEntry> m_directory_entries;
        public:
            constexpr DirectoryEntryArray() {/*...*/}
    };

    struct DirectoryHandle {
        HANDLE search_handle;
    };

    struct FileHandle {
        HANDLE   win32_handle;
        size_t   file_offset;
        size_t   file_size;
        OpenMode open_mode;
    };

    class FileDeviceBase;

    struct FileLoadContext {
        void       *out_file;
        size_t      out_file_size;
        bool        out_is_heap_allocated;
        mem::Heap  *heap;
        const char *file_path;
        u32         read_div;
        s32         alignment;
    };

    class FileDeviceBase {
        private:
            friend class FileDeviceManager;
        protected:
            vp::util::IntrusiveListNode             m_manager_list_node;
            vp::util::FixedString<vp::util::cMaxDrive> m_device_name;
        protected:
            virtual Result LoadFileImpl(FileLoadContext *file_load_context);
            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode);
            virtual Result CloseFileImpl(FileHandle *file_handle);
            virtual Result ReadFileImpl(void *out_read_buffer, FileHandle *file_handle, u32 read_size);
            virtual Result WriteFileImpl(FileHandle *file_handle, void *write_buffer, u32 write_size);
            virtual Result FlushFileImpl(FileHandle *file_handle);
            
            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle);
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path);
            
            virtual Result CheckFileExistsImpl(const char *path) { return CheckFileExistsImpl(path); }
            
            virtual Result FormatPath(vp::util::FixedString<vp::util::cMaxPath> *out_formatted_path, const char *path);
        public:
            explicit constexpr ALWAYS_INLINE FileDeviceBase() {/*...*/}
            explicit constexpr ALWAYS_INLINE FileDeviceBase(const char *device_name) : m_device_name(device_name) {/*...*/}
            
            ALWAYS_INLINE Result TryLoadFile(FileLoadContext *file_load_context)                                { return this->LoadFileImpl(file_load_context); }
            ALWAYS_INLINE Result TryOpenFile(FileHandle *out_file_handle, const char *path, OpenMode open_mode) { return this->OpenFileImpl(out_file_handle, path, open_mode); }
            ALWAYS_INLINE Result TryCloseFile(FileHandle *file_handle)                                          { return this->CloseFileImpl(file_handle); }
            ALWAYS_INLINE Result TryReadFile(void *out_read_buffer, FileHandle *file_handle, u32 read_size)     { return this->ReadFileImpl(out_read_buffer, file_handle, read_size); }
            ALWAYS_INLINE Result TryWriteFile(FileHandle *file_handle, void *write_buffer, u32 write_size)      { return this->WriteFileImpl(file_handle, write_buffer, write_size); }
            ALWAYS_INLINE Result TryFlushFile(FileHandle *file_handle)                                          { return this->FlushFileImpl(file_handle); }

            ALWAYS_INLINE Result GetFileSize(size_t *out_size, FileHandle *file_handle) { return this->GetFileSizeImpl(out_size, file_handle); }
            ALWAYS_INLINE Result GetFileSize(size_t *out_size, const char *path)        { return this->GetFileSizeImpl(out_size, path); }

            ALWAYS_INLINE Result CheckFileExists(const char *path) { return CheckFileExistsImpl(path); }

            //ALWAYS_INLINE Result OpenDirectory(DirectoryHandle *out_directory_handle, const char *path)               { return this->OpenDirectoryImpl(out_directory_handle, path); }
            //ALWAYS_INLINE Result CloseDirectory(DirectoryHandle *directory_handle)                                    { return this->CloseDirectoryImpl(directory_handle); }
            //ALWAYS_INLINE Result ReadDirectory(DirectoryHandle *directory_handle, DirectoryEntryArray *entry_holder)  { return this->ReadDirectoryImpl(directory_handle, entry_holder); }

            //ALWAYS_INLINE Result CheckDirectoryExists(const char *path) { return this->CheckDirectoryExistsImpl(path); }

            constexpr ALWAYS_INLINE const char *GetDeviceName() const { return m_device_name.GetString(); }
    };
    
    //class SarcFileDevice : public FileDeviceBase {
    //    private:
    //        SarcExtractor m_sarc_extractor;
    //    public:
    //        explicit constexpr ALWAYS_INLINE SarcFileDevice() {/*...*/}
    //        explicit constexpr ALWAYS_INLINE SarcFileDevice(const char *device_name) : m_device_name(device_name) {/*...*/}
    //};
}

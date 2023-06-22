#pragma once

namespace awn::res {

    using MaxPathString      = vp::util::FixedString<vp::util::cMaxPath>;
    using MaxExtensionString = vp::util::FixedString<vp::util::cMaxExtension>;

    enum OpenMode : u32 {
        OpenMode_Read            = GENERIC_READ,
        OpenMode_Write           = GENERIC_WRITE,
        OpenMode_ReadWrite       = GENERIC_READ | GENERIC_WRITE,
        OpenMode_ReadWriteAppend = OpenMode_ReadWrite | 0x1,
    };

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

    struct FileHandle {
        void     *handle;
        u32       archive_entry_index;
        size_t    file_offset;
        size_t    file_size;
        OpenMode  open_mode;

        constexpr void SetDefaults() {
            handle              = INVALID_HANDLE_VALUE;
            archive_entry_index = cInvalidEntryIndex;
            file_offset         = 0;
            file_size           = 0;
            open_mode           = OpenMode_Read;
        }
    };

    class FileDeviceBase;

    struct FileLoadContext {
        void            *out_file;
        size_t           out_file_size;
        size_t           out_compressed_file_size;
        CompressionType  out_compression_type;
        bool             out_is_heap_allocated;
        mem::Heap       *heap;
        const char      *file_path;
        u32              read_div;
        s32              alignment;
    };

    class FileDeviceBase {
        private:
            friend class FileDeviceManager;
        protected:
            vp::util::IntrusiveListNode                m_manager_list_node;
            vp::util::FixedString<vp::util::cMaxDrive> m_device_name;
        public:
            VP_RTTI_BASE(FileDeviceBase);
        protected:
            virtual Result LoadFileImpl(FileLoadContext *file_load_context) {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_load_context->out_file != nullptr && file_load_context->out_file_size == 0, ResultInvalidFileBufferSize);
                RESULT_RETURN_UNLESS((file_load_context->read_div & 0x1f) != 0, ResultInvalidReadDivAlignment);

                /* Open file handle */
                FileHandle handle = {};
                const Result open_result = this->TryOpenFile(std::addressof(handle), file_load_context->file_path, OpenMode_Read);
                RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);

                /* Get file size */
                size_t file_size = 0;
                const Result size_result = this->GetFileSize(std::addressof(file_size), std::addressof(handle));
                RESULT_RETURN_UNLESS(size_result == ResultSuccess, size_result);

                /* Handle file buffer */
                if (file_load_context->out_file == nullptr) {
                    file_load_context->out_file              = ::operator new(file_size, file_load_context->heap, file_load_context->alignment);
                    file_load_context->out_file_size         = file_size;
                    file_load_context->out_is_heap_allocated = true;
                } else {
                    RESULT_RETURN_UNLESS(file_size <= file_load_context->out_file_size, ResultInvalidFileBufferSize);
                }

                /* Read loop */
                const u32 div_size  = file_load_context->read_div;
                size_t    remaining = vp::util::AlignDown(file_size, div_size);
                size_t    offset    = 0;
                while (offset != remaining) {
                    this->TryReadFile(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(file_load_context->out_file)) + offset), std::addressof(handle), div_size);
                    offset = offset + div_size;
                }
                this->TryReadFile(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(file_load_context->out_file)) + offset), std::addressof(handle), file_size - remaining);

                /* Close file handle */
                const Result close_result = this->TryCloseFile(std::addressof(handle));
                RESULT_RETURN_UNLESS(close_result == ResultSuccess, close_result);

                RESULT_RETURN_SUCCESS;
            }
            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode) { VP_ASSERT(false); VP_UNUSED(out_file_handle, path, open_mode); }
            virtual Result CloseFileImpl(FileHandle *file_handle) { VP_ASSERT(false); VP_UNUSED(file_handle); }
            virtual Result ReadFileImpl(void *out_read_buffer, FileHandle *file_handle, u32 read_size) { VP_ASSERT(false); VP_UNUSED(out_read_buffer, file_handle, read_size); }
            virtual Result WriteFileImpl(FileHandle *file_handle, void *write_buffer, u32 write_size) { VP_ASSERT(false); VP_UNUSED(file_handle, write_buffer, write_size); }
            virtual Result FlushFileImpl(FileHandle *file_handle) { VP_ASSERT(false); VP_UNUSED(file_handle); }

            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) { VP_ASSERT(false); VP_UNUSED(out_size, file_handle); }
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path) { VP_ASSERT(false); VP_UNUSED(out_size, path); }

            virtual Result CheckFileExistsImpl(const char *path) { VP_ASSERT(false); VP_UNUSED(path); }
            
            virtual Result OpenDirectoryImpl(DirectoryHandle *out_directory_handle, const char *path)                         { VP_ASSERT(false); VP_UNUSED(out_directory_handle, path); }
            virtual Result CloseDirectoryImpl(DirectoryHandle *directory_handle)                                              { VP_ASSERT(false); VP_UNUSED(directory_handle); }
            virtual Result ReadDirectoryImpl(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) { VP_ASSERT(false); VP_UNUSED(directory_handle, entry_array, entry_count); }

            virtual bool CheckDirectoryExistsImpl(const char *path) { VP_UNUSED(path); return false; }

            virtual Result FormatPath(vp::util::FixedString<vp::util::cMaxPath> *out_formatted_path, const char *path) { VP_ASSERT(false); VP_UNUSED(out_formatted_path, path); }
        public:
            explicit constexpr ALWAYS_INLINE FileDeviceBase() {/*...*/}
            explicit constexpr ALWAYS_INLINE FileDeviceBase(const char *device_name) : m_device_name(device_name) {/*...*/}
            constexpr virtual ALWAYS_INLINE ~FileDeviceBase() {/*...*/}

            ALWAYS_INLINE Result TryLoadFile(FileLoadContext *file_load_context)                                { return this->LoadFileImpl(file_load_context); }
            ALWAYS_INLINE Result TryOpenFile(FileHandle *out_file_handle, const char *path, OpenMode open_mode) { return this->OpenFileImpl(out_file_handle, path, open_mode); }
            ALWAYS_INLINE Result TryCloseFile(FileHandle *file_handle)                                          { return this->CloseFileImpl(file_handle); }
            ALWAYS_INLINE Result TryReadFile(void *out_read_buffer, FileHandle *file_handle, u32 read_size)     { return this->ReadFileImpl(out_read_buffer, file_handle, read_size); }
            ALWAYS_INLINE Result TryWriteFile(FileHandle *file_handle, void *write_buffer, u32 write_size)      { return this->WriteFileImpl(file_handle, write_buffer, write_size); }
            ALWAYS_INLINE Result TryFlushFile(FileHandle *file_handle)                                          { return this->FlushFileImpl(file_handle); }

            ALWAYS_INLINE Result GetFileSize(size_t *out_size, FileHandle *file_handle) { return this->GetFileSizeImpl(out_size, file_handle); }
            ALWAYS_INLINE Result GetFileSize(size_t *out_size, const char *path)        { return this->GetFileSizeImpl(out_size, path); }

            ALWAYS_INLINE Result CheckFileExists(const char *path) { return CheckFileExistsImpl(path); }

            ALWAYS_INLINE Result OpenDirectory(DirectoryHandle *out_directory_handle, const char *path)                         { return this->OpenDirectoryImpl(out_directory_handle, path); }
            ALWAYS_INLINE Result CloseDirectory(DirectoryHandle *directory_handle)                                              { return this->CloseDirectoryImpl(directory_handle); }
            ALWAYS_INLINE Result ReadDirectory(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) { return this->ReadDirectoryImpl(directory_handle, entry_array, entry_count); }

            ALWAYS_INLINE Result CheckDirectoryExists(const char *path) { return this->CheckDirectoryExistsImpl(path); }

            constexpr ALWAYS_INLINE const char *GetDeviceName() const { return m_device_name.GetString(); }
    };
}

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
        MeshCodec   = (1 << 4),
        Zlib        = (1 << 5),
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
        const MaxExtensionString mc_ext(".mc");
        if (*extension == mc_ext) {
            return CompressionType::MeshCodec;
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

    struct FileHandle {
        void     *handle;
        u32       archive_entry_index;
        u32       open_mode;
        size_t    file_offset;
        size_t    file_size;

        constexpr void SetDefaults() {
            handle              = INVALID_HANDLE_VALUE;
            archive_entry_index = cInvalidEntryIndex;
            file_offset         = 0;
            file_size           = 0;
            open_mode           = static_cast<u32>(OpenMode::Read);
        }
    };

    class FileDeviceBase;

    struct FileLoadContext {
        void            *out_file;
        size_t           out_file_size;
        s32              out_file_alignment;
        size_t           compressed_file_size;
        CompressionType  compression_type;
        bool             is_heap_allocated;
        mem::Heap       *heap;
        const char      *file_path;
        u32              read_div;
        s32              alignment;
    };

    class FileDeviceBase {
        private:
            friend class FileDeviceManager;
        public:
            static constexpr size_t cMaxAutoReadSize = 0x20;
        protected:
            vp::util::IntrusiveRedBlackTreeNode<u32> m_manager_tree_node;
            MaxDriveString                           m_device_name;
        public:
            VP_RTTI_BASE(FileDeviceBase);
        protected:
            virtual Result LoadFileImpl([[maybe_unused]] FileLoadContext *file_load_context) {

           //     /* Integrity checks */
           //     RESULT_RETURN_UNLESS(file_load_context->out_file != nullptr && file_load_context->out_file_size == 0, ResultInvalidFileBufferSize);
           //     RESULT_RETURN_UNLESS((file_load_context->read_div & 0x1f) != 0, ResultInvalidReadDivAlignment);
           //
           //     /* Open file handle */
           //     FileHandle handle = {};
           //     const Result open_result = this->TryOpenFile(std::addressof(handle), file_load_context->file_path, OpenMode::Read);
           //     RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);
           //
           //     /* Get file size */
           //     size_t       file_size   = 0;
           //     const Result size_result = this->GetFileSize(std::addressof(file_size), std::addressof(handle));
           //     RESULT_RETURN_UNLESS(size_result == ResultSuccess, size_result);
           //
           //
           //     /* Check for compression detection */
           //     size_t output_size         = file_load_context->out_file_size;
           //     size_t output_alignment    = vp::util::AlignUp(file_load_context->out_file_alignment, alignof(u64));
           //     DecompressorBase *decompressor = nullptr;
           //     if ((file_load_context->compression_type != CompressionType::None && file_load_context->out_file == nullptr) || file_load_context->compression_type == CompressionType::Auto) {
           //
           //         /* Load auto buffer */
           //         char auto_read[cMaxAutoReadSize] = {};
           //
           //         /* Read header */
           //         const u32 auto_size = (cMaxAutoReadSize < file_size) ? cMaxAutoReadSize : file_size;
           //         this->TryReadFile(auto_read, std::addressof(handle), auto_size);
           //
           //         /* Auto detect compression */
           //         if (file_load_context->compression_type == CompressionType::Auto) {
           //             file_load_context->compression_type = DecompressorManager::GetInstance()->AutoDetectCompressionType(auto_read, auto_size);
           //         }
           //
           //         /* Get decompressor */
           //         decompressor = DecompressorManager::GetInstance()->GetDecompressor(file_load_context->compression_type);
           //         RESULT_RETURN_IF(decompressor == nullptr, ResultInvalidDecompressor);
           //
           //         DecompSize decomp_size = decompressor->GetDecompressedSize(auto_read, auto_size);
           //         output_alignment       = (output_alignment == alignof(u64)) ? decomp_size->alignment : output_alignment;
           //         if (file_load_context->out_file == nullptr) {
           //             output_size = decomp_size->size;
           //
           //             /* Allocate output buffer */
           //             file_load_context->out_file              = ::operator new(output_size, file_load_context->heap, output_alignment);
           //             file_load_context->out_file_size         = output_size;
           //             file_load_context->out_file_alignment    = output_alignment;
           //             file_load_context->out_is_heap_allocated = true;
           //         }
           //
           //         /* Copy auto header */
           //         file_size = file_size - auto_size;
           //         ::memcpy(file_load_context->out_file, auto_read, auto_size);
           //
           //     } else {
           //         output_size = file_size;
           //
           //         /* Allocate output file buffers */
           //         if (file_load_context->out_file == nullptr) {
           //             file_load_context->out_file              = ::operator new(output_size, file_load_context->heap, file_load_context->out_file_alignment);
           //             file_load_context->out_file_size         = output_size;
           //             file_load_context->out_file_alignment    = output_alignment;
           //             file_load_context->out_is_heap_allocated = true;
           //         }
           //     }
           //
           //     /* Calculate read div */
           //     const u32    div_size          = (file_load_context->read_div == 0) : file_size ? file_load_context->read_div;
           //     const size_t div_total_size    = vp::util::AlignDown(file_size, div_size);
           //     const u32    stream_read_count = file_size / div_size;
           //
           //     /* Allocate decompressor */
           //     DecompThread *decomp_thread = nullptr;
           //     StreamState  *stream_state  = nullptr;
           //     if (file_load_context->compression_type != CompressionType::None && 1 < stream_read_count) {
           //         stream_state  = decompressor->AllocateStreamState();
           //         decomp_thread = decompressor->GetDecompThread();
           //     }
           //
           //     RESULT_RETURN_UNLESS(file_size <= file_load_context->out_file_size, ResultInvalidFileBufferSize);
           //
           //     /* First read */
           //     this->TryReadFile(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(file_load_context->out_file)) + offset), std::addressof(handle), div_size);
           //
           //     /* Stream read */
           //     size_t offset = div_size;
           //     while (offset != div_total_size) {
           //
           //         /* Signal decompressor thread */
           //         if (stream_state != nullptr) {
           //             stream_state->read_loop_event.Clear();
           //             decompressor->SignalDecompThread();
           //         }
           //
           //         /* Read next chunk */
           //         this->TryReadFile(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(file_load_context->out_file)) + offset), std::addressof(handle), div_size);
           //
           //         /* Wait for previous chunk */
           //         if (stream_state != nullptr) {
           //             stream_state->read_loop_event.Wait();
           //         }
           //
           //         offset = offset + div_size;
           //     }
           //
           //     if (stream_state != nullptr) {
           //         decompressor->FreeStreamState(stream_state);
           //     }
           //
           //     /* Close file handle */
           //     const Result close_result = this->TryCloseFile(std::addressof(handle));
           //     RESULT_RETURN_UNLESS(close_result == ResultSuccess, close_result);

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
            explicit constexpr ALWAYS_INLINE FileDeviceBase() : m_manager_tree_node(), m_device_name() {/*...*/}
            explicit constexpr ALWAYS_INLINE FileDeviceBase(const char *device_name) : m_manager_tree_node(), m_device_name(device_name) {
                const u32 hash = vp::util::HashCrc32b(device_name);
                m_manager_tree_node.SetKey(hash);
            }
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

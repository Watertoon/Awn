#pragma once

namespace awn::res {
    
    namespace {

        constexpr ALWAYS_INLINE CompressionType BeaCompressionTypeToAwnCompressionType(vp::res::BeaCompressionType compression_type) {
            switch (compression_type) {
                case vp::res::BeaCompressionType::None:
                    return CompressionType::None;
                case vp::res::BeaCompressionType::Zlib:
                    return CompressionType::Zlib;
                case vp::res::BeaCompressionType::Zstandard:
                    return CompressionType::Zstandard;
                default:
                    break;
            }
            return CompressionType::None;
        }
    }

    class BeaArchiveResource : public ArchiveResource {
        private:
            vp::res::ResBea *m_bea;
        public:
            constexpr BeaArchiveResource() {/*...*/}

            virtual bool Initialize(mem::Heap *heap, void *file, u32 file_size) override {
                m_bea = vp::res::ResBea::ResCast(file);
                return (m_bea != nullptr);
            }

            virtual u32 TryGetEntryIndex(const char *path) const override {
                return m_bea->file_dictionary->FindEntryIndex(path);
            }

            virtual bool TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 entry_index) override {
                vp::res::ResBeaFileEntry *file_entry = m_bea->TryGetFileEntryByIndex(entry_index);
                if (file_entry == nullptr) { return false; }
                out_file_return->file                = m_bea->GetFileByEntry(file_entry);
                out_file_return->file_size           = file_entry->compressed_size;
                out_file_return->decompressed_size   = file_entry->uncompressed_size;
                out_file_return->alignment           = file_entry->alignment;
                out_file_return->compression_type    = BeaCompressionTypeToAwnCompressionType(vp::res::BeaCompressionType{file_entry->compression_type});
                return true;
            }
            virtual bool TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) override {
                vp::res::ResBeaFileEntry *file_entry = m_bea->TryGetFileEntryByPath(path);
                if (file_entry == nullptr) { return false; }
                out_file_return->file              = m_bea->GetFileByEntry(file_entry);
                out_file_return->file_size         = file_entry->compressed_size;
                out_file_return->decompressed_size = file_entry->uncompressed_size;
                out_file_return->alignment         = file_entry->alignment;
                out_file_return->compression_type  = BeaCompressionTypeToAwnCompressionType(vp::res::BeaCompressionType{file_entry->compression_type});
                return true;
            }
    };
}

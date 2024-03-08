#pragma once

namespace awn::res {
    
    class AsyncSaveManager {
        public:
            using ThreadDelegate = vp::util::Delegate<AsyncSaveManager, size_t>;
        public:
            enum class Message : size_t {
                Read   = vp::util::TCharCode32("Read"),
                Size   = vp::util::TCharCode32("Size"),
                Save   = vp::util::TCharCode32("Save"),
                Commit = vp::util::TCharCode32("Cmit"),
                Copy   = vp::util::TCharCode32("Copy"),
            };
        private:
            SaveFileDevice      *m_save_file_device;
            FileDeviceBase      *m_user_file_device;
            sys::DelegateThread *m_save_thread;
            ThreadDelegate       m_delegate;
            void                *m_output;
            size_t               m_output_size;
            size_t               m_last_size;
            MaxPathString        m_save_path;
            MaxPathString        m_copy_path;
            union {
                u32 m_request_flags;
                struct {
                    u32 m_request_read   : 1;
                    u32 m_request_size   : 1;
                    u32 m_request_save   : 1;
                    u32 m_request_cmit   : 1;
                    u32 m_request_copy   : 1;
                    u32 m_pause          : 1;
                    u32 m_reserve0       : 26;
                };
            };
        private:
            void Read();
            void Size();
            void Save();
            void Commit();
            void Copy();

            void ThreadMain(size_t message);
        public:
            constexpr AsyncSaveManager() : m_save_file_device(), m_user_file_device(), m_save_thread(), m_delegate(this, ThreadMain), m_output(), m_output_size(), m_save_path(), m_copy_path(), m_request_flags() {/*...*/}
            constexpr ~AsyncSaveManager() {/*...*/}

            void Initialize(mem::Heap *heap, const char *thread_name, u32 priority, sys::CoreMask core_mask);
            void Finalize();

            bool RequestRead(void *file_buffer, size_t buffer_size, const char *path, FileDeviceBase *user_file_device);
            bool RequestSize(const char *path, FileDeviceBase *user_file_device);
            bool RequestSave(void *save_data, size_t save_size, const char *path, FileDeviceBase *user_file_device);
            bool RequestCommit(FileDeviceBase *user_file_device);
            bool RequestCopy(const char *dst_path, const char *src_path, void *copy_buffer, size_t copy_buffer_size, FileDeviceBase *user_file_device);

            constexpr bool GetSize(size_t *out_size, const char *path) const {
                if (m_request_size == true)            { return false; }
                if (m_save_path.IsSame(path) == false) { return false; }
                *out_size = m_last_size;
                return true;
            }

            constexpr bool IsRequestPending() const { return m_request_flags != 0; }

            constexpr void Pause()   { vp::util::InterlockedOr(std::addressof(m_request_flags), (1u << 5)); }
            constexpr void Unpause() { vp::util::InterlockedAnd(std::addressof(m_request_flags), ~(1u << 5)); }
    };
}

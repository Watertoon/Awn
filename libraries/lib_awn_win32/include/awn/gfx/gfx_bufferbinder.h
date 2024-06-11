#pragma once

namespace awn::gfx {

    struct BufferRegisterInfo {
        const BufferInfo     *buffer_info;
        const void           *copy_memory;
        MemoryPropertyFlags   memory_property_flags;
        u32                   priority;
    };

    class AsyncGpuResourceManager;
    class BufferUnit;

	class BufferBinder {
        public:
            friend class AsyncGpuResourceManager;
            friend class BufferUnit;
        public:
            enum class Status {
                Uninitialized          = 0,
                InRegister             = 1,
                Registered             = 2,
                CreateApiResourceError = 3,
                MemoryAllocationError  = 4,
                OutOfMemoryError       = 5,
                MemoryMappingError     = 6,
                Cancelled              = 7,
            };
		private:
            union {
                u32 m_state;
                struct {
                    u32 m_register_guard : 1;
                    u32 m_complete_guard : 1;
                    u32 m_release_guard  : 1;
                    u32 m_reserve        : 29;
                };
            };
            Status                   m_status;
            BufferUnit              *m_buffer_unit;
			async::AsyncTaskWatcher  m_watcher;
        public:
            constexpr BufferBinder() : m_state(), m_status(), m_buffer_unit(), m_watcher() {/*...*/}
            ~BufferBinder() { this->Release(); }

            Result RegisterBufferAsync(const BufferRegisterInfo *register_info);
            Result RegisterBufferSync(const BufferRegisterInfo *register_info);
            Result ReferenceBinderSync(BufferBinder *reference_binder);

            void Release();

            void WaitForRegister();

            constexpr bool Complete();
            constexpr Buffer *GetBuffer();

            constexpr bool IsInRegister() const;
            constexpr bool IsRegistered() const;
            constexpr bool IsFailed() const;
            constexpr bool IsErrorStatus() const;
    };
}

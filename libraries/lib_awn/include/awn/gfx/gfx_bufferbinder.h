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

namespace awn::gfx {

    struct BufferRegisterInfo {
        const BufferInfo     *buffer_info;
        const void           *copy_memory;
        MemoryPropertyFlags   memory_property_flags;
        u32                   priority;
    };

    namespace impl {        
        class AsyncGpuResourceManager;
        class BufferUnit;
    }

	class BufferBinder {
        public:
            friend class impl::AsyncGpuResourceManager;
            friend class impl::BufferUnit;
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
            impl::BufferUnit        *m_buffer_unit;
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

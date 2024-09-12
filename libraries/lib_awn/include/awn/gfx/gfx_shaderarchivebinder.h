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

    struct ShaderArchiveRegisterInfo {
        void *bnsh;
        u32   priority;
    };

    namespace impl {        
        class AsyncGpuResourceManager;
        class ShaderArchiveUnit;
    }

	class ShaderArchiveBinder {
        public:
            friend class impl::AsyncGpuResourceManager;
            friend class impl::ShaderArchiveUnit;
        public:
            enum class Status {
                Uninitialized          = 0,
                InRegister             = 1,
                Registered             = 2,
                CreateApiResourceError = 3,
                InvalidFileError       = 4,
                Cancelled              = 5,
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
            impl::ShaderArchiveUnit *m_shader_archive_unit;
			async::AsyncTaskWatcher  m_watcher;
        public:
            constexpr ShaderArchiveBinder() : m_state(), m_status(), m_shader_archive_unit(), m_watcher() {/*...*/}
            ~ShaderArchiveBinder() { this->Release(); }

            Result RegisterShaderArchiveAsync(const ShaderArchiveRegisterInfo *register_info);
            Result RegisterShaderArchiveSync(const ShaderArchiveRegisterInfo *register_info);
            Result ReferenceBinderSync(ShaderArchiveBinder *reference_shader_archive_binder);

            void Release();

            void WaitForRegister();

            /* Defined in gfx_shaderarchiveunit.h */
            constexpr bool Complete();
            constexpr vp::res::ResBnshShaderContainer *GetShaderContainer();

            constexpr bool IsInRegister() const;
            constexpr bool IsRegistered() const;
            constexpr bool IsFailed() const;
            constexpr bool IsErrorStatus() const;
    };
}

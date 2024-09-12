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

    struct TextureRegisterInfo {
        const TextureInfo     *texture_info;
        u32                    copy_mip_level_count;
        const void           **copy_mip_level_array;
        MemoryPropertyFlags    memory_property_flags;
        u32                    priority;
    };

    namespace impl {        
        class AsyncGpuResourceManager;
        class TextureUnit;
    }

	class TextureBinder {
        public:
            friend class impl::AsyncGpuResourceManager;
            friend class impl::TextureUnit;
        public:
            enum class Status {
                Uninitialized          = 0,
                InRegister             = 1,
                Registered             = 2,
                CreateApiResourceError = 3,
                MemoryAllocationError  = 4,
                OutOfMemoryError       = 5,
                CopyMemoryError        = 6,
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
            impl::TextureUnit       *m_texture_unit;
			async::AsyncTaskWatcher  m_watcher;
        public:
            constexpr TextureBinder() : m_state(), m_status(), m_texture_unit(), m_watcher() {/*...*/}
            ~TextureBinder() { this->Release(); }

            void Release();

            Result RegisterTextureSync(const TextureRegisterInfo *register_info);
            Result ReferenceBinderSync(TextureBinder *reference_texture_binder);

            Result RegisterTextureAsync(const TextureRegisterInfo *register_info);
            void WaitForRegister();
            constexpr bool Complete();

            /* Defined in gfx_textureunit.h */
            constexpr Texture *GetTexture();

            constexpr bool IsInRegister() const;
            constexpr bool IsRegistered() const;
            constexpr bool IsFailed() const;
            constexpr bool IsErrorStatus() const;
	};
}

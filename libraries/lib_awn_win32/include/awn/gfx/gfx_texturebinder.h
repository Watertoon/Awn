#pragma once

namespace awn::gfx {

    struct TextureRegisterInfo {
        const TextureInfo     *texture_info;
        u32                    copy_mip_level_count;
        const void           **copy_mip_level_array;
        MemoryPropertyFlags    memory_property_flags;
        u32                    priority;
    };

    class AsyncGpuResourceManager;
    class TextureUnit;

	class TextureBinder {
        public:
            friend class AsyncGpuResourceManager;
            friend class TextureUnit;
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
            TextureUnit             *m_texture_unit;
			async::AsyncTaskWatcher  m_watcher;
        public:
            constexpr TextureBinder() : m_state(), m_status(), m_texture_unit(), m_watcher() {/*...*/}
            ~TextureBinder() { this->Release(); }

            Result RegisterTextureAsync(const TextureRegisterInfo *register_info);
            Result RegisterTextureSync(const TextureRegisterInfo *register_info);
            Result ReferenceBinderSync(TextureBinder *reference_texture_binder);

            void Release();

            void WaitForRegister();

            /* Defined in gfx_textureunit.h */
            constexpr bool Complete();
            constexpr Texture *GetTexture();

            constexpr bool IsInRegister() const;
            constexpr bool IsRegistered() const;
            constexpr bool IsFailed() const;
            constexpr bool IsErrorStatus() const;
	};
}

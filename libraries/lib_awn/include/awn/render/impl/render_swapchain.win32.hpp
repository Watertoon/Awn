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

namespace awn::render {
    struct SwapchainInfo;
}

namespace awn::render::impl {

    using NativeWindowHandle = HWND;

	static constexpr gfx::ImageFormat cDefaultSwapchainImageFormat = gfx::ImageFormat::R8G8B8A8_Unorm;

    static constexpr u32 cTargetMinSwapchainImageCount = 2;
    static constexpr u32 cTargetMaxSwapchainImageCount = 3;
    static constexpr u32 cDefaultSwapchainImageCount = 2;

    static constexpr u32 cTargetMinSwapchainViewCount = 1;
    static constexpr u32 cTargetMaxSwapchainViewCount = 2;
    static constexpr u32 cDefaultSwapchainViewCount = 1;

    static constexpr u32 cTargetMinSwapchainPresentInterval = 0;
    static constexpr u32 cTargetMaxSwapchainPresentInterval = 4;
    static constexpr u32 cDefaultSwapchainPresentInterval = 1;

    constexpr DXGI_FORMAT GfxImageFormatToDxgiFormat(gfx::ImageFormat format) {
        switch(format) {
            case gfx::ImageFormat::B8G8R8A8_Unorm:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            case gfx::ImageFormat::R8G8B8A8_Unorm:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case gfx::ImageFormat::R16G16B16A16_Float:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;
            default:
                break;
        }
        VP_ASSERT(false);
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    class Swapchain {
        public:
            static constexpr u64 cInvalidPresentId = 0xffff'ffff'ffff'ffff;
            static constexpr size_t cMessageExitCode = 0x7fff'ffff'ffff'ffff;
        public:
            using PresentThread = vp::util::TypedStorage<sys::DelegateThread>;
            using PresentThreadFunc = sys::ThreadDelegate<Swapchain>;
        public:
            struct SwapchainSettings {
                vp::util::Int2   size;
                gfx::ImageFormat image_format;
                u32              image_count;
                u32              view_count;
            };
        private:
            NativeWindowHandle      m_window_handle;
            IDXGISwapChain4        *m_dxgi_swapchain;
            ID3D12Resource2        *m_d3d12_resource_array[cTargetMaxSwapchainImageCount];
            ID3D12Fence            *m_d3d12_fence;
            VkDeviceMemory          m_vk_device_memory_array[cTargetMaxSwapchainImageCount];
            VkImage                 m_vk_image_array[cTargetMaxSwapchainImageCount];
            gfx::RenderTargetColor  m_render_target_color_array[cTargetMaxSwapchainImageCount];
            HANDLE                  m_image_handle_array[cTargetMaxSwapchainImageCount];
            HANDLE                  m_sync_handle;
            gfx::Sync               m_submit_sync;
            gfx::CommandList        m_command_list;
            PresentThread           m_present_thread;
            PresentThreadFunc       m_present_thread_function;
            sys::Event              m_present_event;
            sys::Mutex              m_present_cs;
            HANDLE                  m_wait_handle;
            u8                      m_present_stage;
            bool                    m_is_drop_request_for_resize;
            u32                     m_current_index;
            u32                     m_last_index;
            u32                     m_present_interval;
            union {
                u32 m_request_flags;
                struct {
                    u32 m_is_request_resize : 1;
                    u32 m_is_request_recreate : 1;
                    u32 m_reserve0 : 30;
                };
            };
            mutable sys::Mutex      m_settings_cs;
            SwapchainSettings       m_settings;
            SwapchainSettings       m_update_settings;
        protected:
            void PresentThreadMain(size_t message);
            void Present(u64 present_id);
        protected:
            void CreateSwapchain();
            void DestroySwapchain();
            void CreateImages();
            void DestroyImages();

            void IdlePresent();
        public:
            constexpr Swapchain() : m_window_handle(), m_dxgi_swapchain(), m_d3d12_resource_array(), m_d3d12_fence(), m_vk_device_memory_array(), m_vk_image_array(), m_render_target_color_array(), m_image_handle_array(), m_sync_handle(), m_submit_sync(), m_command_list(), 
                m_present_thread(), m_present_thread_function(this, PresentThreadMain), m_present_event(), m_present_cs(), 
                m_wait_handle(), m_present_stage(), m_is_drop_request_for_resize(), m_current_index(), m_last_index(),
                m_present_interval(), m_request_flags(), m_settings_cs(), m_settings(), m_update_settings() {/*...*/}
            constexpr ~Swapchain() {/*...*/}

            void Initialize(const SwapchainInfo *swapchain_info, NativeWindowHandle handle);
            void Finalize();

            void RequestPresent(gfx::CommandList command_list);
            void WaitForPresent();

            constexpr u32 GetAcquireIndex() { return m_current_index; }
            constexpr gfx::RenderTargetColor *GetRenderTargetColor() { return std::addressof(m_render_target_color_array[m_current_index]); }
    
            constexpr IDXGISwapChain4 *GetDxgiSwapchain() { return m_dxgi_swapchain; }
        public:
            bool ApplyUpdates();
            void RequestSetSize(vp::util::Int2 size);
            void RequestSetImageFormat(gfx::ImageFormat image_format);
            void RequestSetImageCount(u32 image_count);
            void RequestSetViewCount(u32 view_count);

            constexpr void SetPresentInterval(u32 present_interval) {
                VP_ASSERT(cTargetMinSwapchainPresentInterval <= present_interval && present_interval <= cTargetMaxSwapchainPresentInterval)
                m_present_interval = present_interval;
            }

            constexpr vp::util::Int2 GetSize() const {
                return m_settings.size;
            }
            constexpr gfx::ImageFormat GetImageFormat() const {
                return m_settings.image_format;
            }
            constexpr u32 GetImageCount() const {
                return m_settings.image_count;
            }
            constexpr u32 GetViewCount() const {
                return m_settings.view_count;
            }
            constexpr u32 GetPresentInterval() const {
                return m_present_interval;
            }

            vp::util::Int2 GetUpdateSize() const {
                if (m_is_request_resize) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.size;
                }
                return m_settings.size;
            }

            gfx::ImageFormat GetUpdateImageFormat() const {
                if (m_is_request_resize) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.image_format;
                }
                return m_settings.image_format;
            }

            u32 GetUpdateImageCount() const {
                if (m_is_request_resize) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.image_count;
                }
                return m_settings.image_count;
            }

            u32 GetUpdateViewCount() const {
                if (m_is_request_resize) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.view_count;
                }
                return m_settings.view_count;
            }

            u32 GetUpdatePresentInterval() const {
                return m_present_interval;
            }
    };
}

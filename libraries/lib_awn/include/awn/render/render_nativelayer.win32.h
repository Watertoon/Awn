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

    namespace impl {        
        class NativeVideoManager;
    }
    class NativeDisplay;
    class NativeLayer;

    using DisplayResizeCallback = vp::util::IFunction<void(NativeDisplay*,NativeLayer*)>;

    struct SwapchainInfo {
        vp::util::Int2     size;
        gfx::ImageFormat   image_format = impl::cDefaultSwapchainImageFormat;
        u32                image_count = impl::cDefaultSwapchainImageCount;
        u32                view_count = impl::cDefaultSwapchainViewCount;
        u32                present_interval = impl::cDefaultSwapchainPresentInterval;
    };

	struct NativeLayerInfo {
        const char                  *name;
        NativeDisplay               *parent_display;
        vp::util::Float2             position;
        float                        alpha = 1.0f;
        bool                         is_visible = true;
        SwapchainInfo                swapchain_info;
        const DisplayResizeCallback *display_resize_function;
    };

    class NativeLayer {
        public:
            friend class impl::NativeVideoManager;
            friend class NativeDisplay;
        public:
            static constexpr u32 cWin32AwnMessage = WM_USER;
            static constexpr size_t cMessageExitCode = 0;
            static constexpr u32 cMessageStart = 1;
            static constexpr u32 cMessageQuit = 2;
        public:
            using NativeLayerName = vp::util::FixedString<0x40>;
            using DisplayResizeFunction = vp::util::AnyFunction<4, void(NativeDisplay*,NativeLayer*)>;
        public:
            struct NativeLayerSettings {
                NativeDisplay    *parent_display;
                vp::util::Float2  position;
                float             alpha = 1.0f;
                u32               z;
                bool              is_visible;
            };
        private:
            vp::util::IntrusiveListNode  m_mgr_node;
            vp::util::IntrusiveListNode  m_display_node;
            DisplayResizeFunction        m_resize_function;
            NativeLayerName              m_name;
            IDCompositionVisual3        *m_dcmp_visual;
            IDCompositionEffectGroup    *m_dcmp_effect_group;
            impl::Swapchain              m_swapchain;
            mutable sys::Mutex           m_settings_cs;
            bool                         m_is_opacity_effect;
            bool                         m_is_force_invisible_for_zero_alpha;
            union {
                u32 m_flags;
                struct {
                    u32 m_is_update_swapchain : 1;
                    u32 m_is_update_parent : 1;
                    u32 m_is_update_is_visible : 1;
                    u32 m_is_update_alpha : 1;
                    u32 m_is_update_position : 1;
                    u32 m_reserve0 : 26;
                };
            };
            u32                 m_dpi;
            NativeLayerSettings m_settings;
            NativeLayerSettings m_update_settings;
        protected:
            void ApplyUpdates();
            void InvokeDisplayResize(NativeDisplay *parent_display) {
                m_resize_function.Invoke(parent_display, this);
            }
        public:
            constexpr NativeLayer() : m_mgr_node(), m_display_node(), m_resize_function(), m_name(), m_dcmp_visual(), m_dcmp_effect_group(), m_swapchain(), m_settings_cs(), m_is_opacity_effect(), m_is_force_invisible_for_zero_alpha(), m_flags(), m_dpi(), m_settings(), m_update_settings() {/*...*/}
            constexpr ~NativeLayer() {/*...*/}

            Result Initialize(const NativeLayerInfo *layer_info);
            void Finalize();

            gfx::RenderTargetColor *GetRenderTargetColor() {
                return m_swapchain.GetRenderTargetColor();
            }
        public:
            constexpr IDCompositionVisual3 *GetDcmpVisual() {
                return m_dcmp_visual;
            }
            constexpr impl::Swapchain *GetSwapchain() {
                return std::addressof(m_swapchain);
            }
        public:
            void RequestSetPosition(vp::util::Float2 pos);
            void RequestSetZ(u32 z);
            void RequestSetSize(vp::util::Int2 size);
            void RequestSetIsVisible(bool is_visible);
            void RequestSetAlpha(float alpha);
            void RequestSetDisplay(NativeDisplay *native_display);
            void RequestSetImageFormat(gfx::ImageFormat image_format) {
                std::scoped_lock l(m_settings_cs);
                m_swapchain.RequestSetImageFormat(image_format);
                m_is_update_swapchain = true;
                return;
            }
            void RequestGetImageCount(u32 image_count) {
                std::scoped_lock l(m_settings_cs);
                m_swapchain.RequestSetImageCount(image_count);
                m_is_update_swapchain = true;
                return;
            }
            void RequestGetViewCount(u32 view_count) {
                std::scoped_lock l(m_settings_cs);
                m_swapchain.RequestSetViewCount(view_count);
                m_is_update_swapchain = true;
                return;
            }

            void SetDisplayResizeCallback(DisplayResizeCallback *callback) {
                std::scoped_lock l(m_settings_cs);
                m_resize_function.SetFunction(*callback);
            }
            void SetPresentInterval(u32 present_interval) {
                m_swapchain.SetPresentInterval(present_interval);
            }

            constexpr vp::util::Float2 GetPosition() const {
                return m_settings.position;
            }
            constexpr vp::util::Int2 GetSize() const {
                return m_swapchain.GetSize();
            }
            constexpr float GetAlpha() const {
                return m_settings.alpha;
            }
            constexpr u32 GetZ() const {
                return m_settings.z;
            }
            constexpr bool GetIsVisible() const {
                return m_settings.is_visible;
            }
            constexpr u32 GetDpi() const {
                return m_dpi;
            }
            constexpr u32 GetPresentInterval() const {
                return m_swapchain.GetPresentInterval();
            }
            constexpr gfx::ImageFormat GetImageFormat() const {
                return m_swapchain.GetImageFormat();
            }
            constexpr u32 GetImageCount() const {
                return m_swapchain.GetImageCount();
            }
            constexpr u32 GetViewCount() const {
                return m_swapchain.GetViewCount();
            }

            vp::util::Float2 GetUpdatePosition() const {
                if (m_is_update_position) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.position;
                }
                return m_settings.position;
            }
            u32 GetUpdateZ() const {
                if (m_is_update_parent) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.z;
                }
                return m_settings.z;
            }
            vp::util::Int2 GetUpdateSize() const {
                return m_swapchain.GetUpdateSize();
            }
            float GetUpdateAlpha() const {
                if (m_is_update_alpha) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.alpha;
                }
                return m_settings.alpha;
            }
            bool GetUpdateIsVisible() const {
                if (m_is_update_is_visible) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.is_visible;
                }
                return m_settings.is_visible;
            }
    };
}

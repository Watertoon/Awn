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
    using LayerResizeCallback = vp::util::IFunction<void(NativeLayer*)>;

	struct NativeLayerInfo {
        const char                  *name = "DefaultLayer";
        NativeDisplay               *parent_display;
        vp::util::MatrixRowMajor4x3f transform = vp::util::MatrixRowMajor4x3f::cIdentity;
        float                        alpha = 1.0f;
        u32                          z_order = 0x80;
        u32                          present_interval = 1;
        bool                         is_visible = true;
        vp::util::Int2               size = { 640, 480, };
        gfx::ImageFormat             image_format = gfx::ImageFormat::R8G8B8A8_Unorm;
        u32                          max_image_count = 2;
        const DisplayResizeCallback *display_resize_function;
        const LayerResizeCallback   *layer_resize_function;
    };

    class NativeLayer {
        public:
            friend class impl::NativeVideoManager;
            friend class NativeDisplay;
        public:
            struct NativeLayerSettings {
                NativeDisplay *parent_display;
                vp::util::MatrixRowMajor4x3f transform;
                float alpha;
                u32 z_order;
                bool is_visible;
                vp::util::Int2 size;
                gfx::ImageFormat image_format;
            };
            struct SwapchainTarget {
                bool                   is_valid;
                ID3D11Texture2D       *d3d11_scanout_texture;
                IDCompositionTexture  *dcomp_texture;
                ID3D11Texture2D       *d3d11_texture;
                mem::GpuMemoryHandle   memory_handle;
                gfx::Texture           texture;
                gfx::RenderTargetColor render_target_color;

                void CreateTarget(gfx::ImageFormat format, vp::util::Int2 size);
                void DestroyTarget();
                void BarrierTarget(HANDLE event_handle);
            };
        public:
            using NativeLayerName = vp::util::FixedString<0x40>;
            using DisplayResizeFunction = vp::util::AnyFunction<4, void(NativeDisplay*,NativeLayer*)>;
            using LayerResizeFunction = vp::util::AnyFunction<4, void(NativeLayer*)>;
            using SwapchainTargetArray = vp::util::HeapArray<SwapchainTarget>;
        private:
            vp::util::IntrusiveListNode  m_mgr_node;
            vp::util::IntrusiveListNode  m_display_node;
            DisplayResizeFunction        m_display_resize_function;
            LayerResizeFunction          m_layer_resize_function;
            NativeLayerName              m_name;

            IDCompositionVisual3        *m_dcmp_visual;
            HANDLE                       m_target_event;
            SwapchainTargetArray         m_target_array;
            u32                          m_target_index;
            u32                          m_image_count;

            u32                          m_present_interval;
            u32                          m_present_counter;
            bool                         m_is_request_present;

            bool                         m_is_opacity_effect;
            bool                         m_is_force_invisible_for_zero_alpha;
            mutable sys::Mutex           m_settings_cs;
            union {
                u32 m_flags;
                struct {
                    u32 m_is_update_parent : 1;
                    u32 m_is_update_is_visible : 1;
                    u32 m_is_update_alpha : 1;
                    u32 m_is_update_transform : 1;
                    u32 m_reserve0 : 26;
                };
            };
            u32                 m_dpi;
            NativeLayerSettings m_settings;
            NativeLayerSettings m_update_settings;
        protected:
            void CopyScanOut();
            void CommitVisual();
            void AcquireNextTarget();
            void CommitTarget();
            void InvokeDisplayResize(NativeDisplay *parent_display) {
                m_display_resize_function.Invoke(parent_display, this);
            }
            void InvokeLayerResize() {
                m_layer_resize_function.Invoke(this);
            }
        public:
            constexpr NativeLayer() : m_mgr_node(), m_display_node(), m_display_resize_function(), m_layer_resize_function(), m_name(), 
            m_dcmp_visual(), m_target_event(), m_target_array(), m_target_index(), m_present_interval(), m_present_counter(), m_is_request_present(),
            m_is_opacity_effect(), m_is_force_invisible_for_zero_alpha(), m_settings_cs(), m_flags(), m_dpi(), m_settings(), m_update_settings() {/*...*/}
            ~NativeLayer() {/*...*/}

            Result Initialize(mem::Heap *heap, const NativeLayerInfo *layer_info);
            void Finalize();

            constexpr void RequestPresent() {
                m_is_request_present = true;
            }

            constexpr gfx::RenderTargetColor *GetRenderTargetColor() {
                return std::addressof(m_target_array[m_target_index].render_target_color);
            }
        public:
            void RequestSetTransform(const vp::util::MatrixRowMajor4x3f transform);
            void RequestSetZ(u32 z);
            void RequestSetSize(vp::util::Int2 size);
            void RequestSetIsVisible(bool is_visible);
            void RequestSetAlpha(float alpha);
            void RequestSetDisplay(NativeDisplay *native_display);
            void RequestSetImageFormat(gfx::ImageFormat image_format) {
                std::scoped_lock l(m_settings_cs);
                m_update_settings.image_format = image_format;
                return;
            }
            void RequestGetImageCount(u32 image_count) {
                std::scoped_lock l(m_settings_cs);
                m_image_count = image_count;
                return;
            }

            void SetDisplayResizeCallback(DisplayResizeCallback *callback) {
                std::scoped_lock l(m_settings_cs);
                m_display_resize_function.SetFunction(*callback);
            }
            void SetPresentInterval(u32 present_interval) {
                std::scoped_lock l(m_settings_cs);
                m_present_interval = present_interval;
            }

            constexpr vp::util::MatrixRowMajor4x3f GetTransform() const {
                return m_settings.transform;
            }
            constexpr vp::util::Int2 GetSize() const {
                return m_settings.size;
            }
            constexpr float GetAlpha() const {
                return m_settings.alpha;
            }
            constexpr u32 GetZOrder() const {
                return m_settings.z_order;
            }
            constexpr bool GetIsVisible() const {
                return m_settings.is_visible;
            }
            constexpr gfx::ImageFormat GetImageFormat() const {
                return m_settings.image_format;
            }
            constexpr u32 GetDpi() const {
                return m_dpi;
            }
            constexpr u32 GetPresentInterval() const {
                return m_present_interval;
            }
            constexpr u32 GetImageCount() const {
                return m_image_count;
            }

            vp::util::MatrixRowMajor4x3f GetUpdateTransform() const {
                if (m_is_update_transform) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.transform;
                }
                return m_settings.transform;
            }
            u32 GetUpdateZOrder() const {
                if (m_is_update_parent) {
                    std::scoped_lock l(m_settings_cs);
                    return m_update_settings.z_order;
                }
                return m_settings.z_order;
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
        public:
            constexpr IDCompositionVisual3 *GetDcmpVisual() {
                return m_dcmp_visual;
            }
    };
}

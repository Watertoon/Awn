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

namespace awn::grr {

    struct DisplayInfo {
        NativeWindow *native_window;
        gfx::ImageFormat image_format = gfx::ImageFormat::R8G8B8A8_Unorm;
        u32 present_interval = 1;
        u32 target_count = 2;
        u32 active_target_count = 2;
        bool is_framebuffer_alpha = true;
    };

    namespace impl {
        class DisplayManager;
    }

    class Display {
        public:
            friend class impl::DisplayManager;
        public:
            static constexpr u32 cPresentIntervalCount = 6;
        public:
            static constexpr u32 cFramerateArray[cPresentIntervalCount] = {
                120,
                60,
                40,
                30,
                24,
                20,
            };
            static constexpr u64 cPresentTimeArray[cPresentIntervalCount] = {
                static_cast<u64>((1.0 / static_cast<double>(cFramerateArray[0])) * 10000000.0),
                static_cast<u64>((1.0 / static_cast<double>(cFramerateArray[1])) * 10000000.0),
                static_cast<u64>((1.0 / static_cast<double>(cFramerateArray[2])) * 10000000.0),
                static_cast<u64>((1.0 / static_cast<double>(cFramerateArray[3])) * 10000000.0),
                static_cast<u64>((1.0 / static_cast<double>(cFramerateArray[4])) * 10000000.0),
                static_cast<u64>((1.0 / static_cast<double>(cFramerateArray[5])) * 10000000.0),
            };
        public:
            struct SwapchainTarget {
                bool                   create_guard;
                ID3D11Texture2D       *d3d11_scanout_texture;
                ID3D11Texture2D       *d3d11_render_texture;
                IPresentationManager  *parent_manager;
                IPresentationBuffer   *presentation_buffer;
                gfx::MemoryPool        memory_pool;
                gfx::Texture           texture;
                gfx::RenderTargetColor render_target_color;
                u64                    present_id;
                u64                    present_time;

                void CreateTarget(IPresentationManager *presentation_mgr, gfx::ImageFormat format, vp::util::Int2 size);
                void AddToPresentManager(IPresentationManager *presentation_mgr);
                void DestroyTarget();
                void BarrierTarget();
            };
            struct Settings {
                gfx::ImageFormat image_format;
                u32 present_interval;
                u32 active_target_count;
                bool is_framebuffer_alpha;
            };
        public:
            using SwapchainTargetArray = vp::util::HeapArray<SwapchainTarget>;
        private:
            vp::util::IntrusiveListNode m_list_node;
            NativeWindow         *m_native_window;
            NativeWindowHandle    m_handle;
            IDCompositionTarget  *m_dcmp_target;
            IDCompositionVisual  *m_dcmp_visual;
            IPresentationManager *m_presentation_manager;
            HANDLE                m_surface_handle;
            IPresentationSurface *m_presentation_surface;
            IUnknown             *m_dcmp_surface;
            SwapchainTargetArray  m_target_array;
            u64                   m_timer;
            sys::Mutex            m_settings_cs;
            Settings              m_settings;
            Settings              m_update_settings;
            u32                   m_update_present_interval;
            u32                   m_image_id;
            bool                  m_is_request_present;
        public:
            constexpr Display() : m_list_node(), m_native_window(), m_handle(), m_dcmp_target(), m_dcmp_visual(), m_presentation_manager(), m_surface_handle(), m_presentation_surface(), m_dcmp_surface(), m_target_array(), m_timer(), m_settings_cs(), m_settings(), m_update_settings(), m_image_id(), m_is_request_present() {/*...*/}
            constexpr ~Display() {/*...*/}

            void Initialize(mem::Heap *heap, const DisplayInfo *info);
            void Finalize();
        public:
            constexpr gfx::RenderTargetColor *GetRenderTargetColor() { return std::addressof(m_target_array[m_image_id].render_target_color); }
            constexpr void RequestPresent() { m_is_request_present = true; }
        public:
            void SetPresentInterval(u32 present_interval) {
                std::scoped_lock l(m_settings_cs);
                m_update_settings.present_interval = vp::util::Min(present_interval, cPresentIntervalCount); 
            }
            void SetActiveTargetCount(u32 count) {
                std::scoped_lock l(m_settings_cs);
                const u32 min_count = vp::util::Max(count, 2u);
                m_update_settings.active_target_count = vp::util::Min(min_count, m_target_array.GetCount()); 
            }
            void SetImageFormat(gfx::ImageFormat format) {
                std::scoped_lock l(m_settings_cs);
                m_update_settings.image_format = format; 
            }

            constexpr u32 GetPresentInterval() { return m_settings.present_interval; }
            constexpr u32 GetActiveTargetCount() { return m_settings.active_target_count; }
            constexpr gfx::ImageFormat GetImageFormat() { return m_settings.image_format; }
        private:
            void CreatePresentationManager();
            void DestroyPresentationManager();
            void CopyScanOut();
            void Present();
            void AcquireNext();
            void CreateTarget();
    };
}

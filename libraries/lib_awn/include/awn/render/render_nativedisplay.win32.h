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

    using NativeWindowHandle = HWND;

    struct NativeDisplayInfo {
        const char     *name;
        vp::util::Int2  position;
        vp::util::Int2  size;
        bool            is_border = true;
        bool            is_free_movable = true;
        bool            is_free_scalable = true;
    };

    class NativeDisplay {
        public:
            friend class impl::NativeVideoManager;
        public:
            static constexpr u32 cWin32AwnMessage = WM_USER;
            static constexpr u32 cMessageExitCode = 0;
            static constexpr u32 cMessageStart = 1;
            static constexpr u32 cMessageQuit = 2;
        public:
            using NativeLayerList = vp::util::IntrusiveListTraits<NativeLayer, &NativeLayer::m_display_node>::List;
        public:
            struct NativeDisplaySettings {
                vp::util::Int2 position;
                vp::util::Int2 size;
            };
        public:
            using NativeDisplayName = vp::util::FixedString<0x40>;
            using WindowThread = vp::util::TypedStorage<sys::DelegateThread>;
            using ThreadDelegate = sys::ThreadDelegate<NativeDisplay>;
        private:
            vp::util::IntrusiveListNode  m_mgr_node;
            NativeDisplayName            m_name;
            sys::Mutex                   m_layer_cs;
            NativeLayerList              m_layer_list;
            u32                          m_layer_count;
            HWND                         m_hwnd;
            IDCompositionTarget         *m_dcmp_target;
            IDCompositionVisual         *m_dcmp_visual_main;
            ThreadDelegate               m_thread_delegate;
            WindowThread                 m_window_thread;
            sys::Event                   m_window_event;
            bool                         m_is_closed;
            bool                         m_is_visible;
            u32                          m_dpi;
            union {
                u32 m_flags;
                struct {
                    u32 m_is_update_size : 1;
                    u32 m_is_update_position : 1;
                    u32 m_is_update_style : 1;
                    u32 m_update_is_border : 1;
                    u32 m_update_free_scalable : 1;
                    u32 m_update_free_movable : 1;
                    u32 m_is_border : 1;
                    u32 m_is_free_scalable : 1;
                    u32 m_is_free_movable : 1;
                    u32 m_is_resize : 2;
                    u32 m_reserve0 : 24;
                };
            };
            sys::Mutex                  m_settings_cs;
            NativeDisplaySettings       m_settings;
            NativeDisplaySettings       m_update_settings;
        protected:
            static long long int WindowProcedure(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam);

            void WindowThreadMain(size_t message);
            void SendMessage(size_t message);

            void CreateWindow();
        protected:
            void ApplyTransform(HDWP hdwp);

            void SetPositionForFreeMoveable(vp::util::Int2 position);
            void SetSizeImpl(vp::util::Int2 size);
        public:
            constexpr NativeDisplay() : m_mgr_node(), m_name(), m_layer_cs(), m_layer_list(), m_layer_count(), m_hwnd(), m_dcmp_target(), m_dcmp_visual_main(), m_thread_delegate(this, WindowThreadMain), m_window_thread(), m_window_event(), m_is_closed(), m_is_visible(), m_dpi(), m_flags(), m_settings_cs(), m_settings(), m_update_settings() {/*...*/}
            constexpr ~NativeDisplay() {/*...*/}

            void Initialize(const NativeDisplayInfo *display_info);
            void Finalize();

            void Calculate();

            void FinishResize();

            void AddLayer(NativeLayer *native_layer);
            void RemoveLayer(NativeLayer *native_layer);

            constexpr NativeWindowHandle GetNativeHandle() const {
                return m_hwnd;
            }
        public:
            void RequestSetIsBorder(bool is_border);
            void RequestSetIsFreeMovable(bool is_free_moveable);
            void RequestSetIsFreeScalable(bool is_free_scalable);
            void RequestSetPosition(vp::util::Int2 position);
            void RequestSetSize(vp::util::Int2 size);

            constexpr vp::util::Int2 GetPosition() const {
                return m_settings.position;
            }
            constexpr vp::util::Int2 GetSize() const {
                return m_settings.size;
            }
            constexpr bool IsClosed() const {
                return m_is_closed;
            }
            constexpr bool IsVisible() const {
                return m_is_closed | m_is_visible;
            }

            constexpr vp::util::Int2 GetUpdatePosition() const {
                const NativeDisplaySettings *settings = (m_is_update_position) ? std::addressof(m_update_settings) : std::addressof(m_settings);
                return settings->position;
            }
            constexpr vp::util::Int2 GetUpdateSize() const {
                const NativeDisplaySettings *settings = (m_is_update_size) ? std::addressof(m_update_settings) : std::addressof(m_settings);
                return settings->size;
            }
    };
}
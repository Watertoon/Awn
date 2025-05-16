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

    using NativeWindowHandle = HWND;

    struct NativeWindowInfo {
        const char     *name = "Default";
        vp::util::Int2  position = {};
        vp::util::Int2  size = { 640, 480, };
    };

    namespace impl {        
        class DisplayManager;
        class DropTargetHandler;
    }

    class NativeWindow {
        public:
            friend class impl::DisplayManager;
            friend class impl::DropTargetHandler;
        public:
            static constexpr u32 cWin32AwnMessage = WM_USER;
            static constexpr u32 cMessageExitCode = 0;
            static constexpr u32 cMessageOpen = 1;
            static constexpr u32 cMessageClose = 2;
        public:
            enum EventTrigger {
                EventTrigger_Move,
                EventTrigger_Resize,
            };
            enum ProgramTrigger {
                ProgramTrigger_DeferProc,
                ProgramTrigger_Style,
                ProgramTrigger_IsFocus,
                ProgramTrigger_DragDrop,
                ProgramTrigger_IsMove,
                ProgramTrigger_IsResize,
            };
            enum class FreeMoveEvent : u8 {
                None,
                Moving,
                Moved,
            };
            enum class FreeResizeEvent : u8 {
                None,
                Resizing,
                Resized,
            };
            enum class ResizeType : u8 {
                None,
                Minimized,
                Maximized,
                Restored,
            };
            enum class DragDropEvent : u8 {
                None,
                Dragging,
                Dropped,
            };
        public:
            struct EventSettings {
                vp::util::Int2 position;
                vp::util::Int2 size;
                bool is_close_event;
                bool is_moved;
                bool is_resized;

                FreeMoveEvent free_move_event;
                FreeResizeEvent free_resize_event;
                bool is_fullscreen;
                DragDropEvent drag_drop_event;
                bool is_dpi_change;
                u32 dpi;
            };
            struct ProgramSettings {
                vp::util::Int2 position;
                vp::util::Int2 size;
                bool is_visible;
                bool is_free_closeable;
                bool is_free_movable;
                bool is_free_resizable;
            };
            struct CommitSettings {
                vp::util::Int2 position;
                vp::util::Int2 size;
                bool is_closed;
                bool is_moved;
                bool is_resized;
                bool is_visible;

                bool is_free_closeable;
                bool is_free_movable;
                bool is_free_resizable;
                FreeMoveEvent free_move_event;
                FreeResizeEvent free_resize_event;
                bool is_fullscreen;
                DragDropEvent drag_drop_event;
                bool is_dpi_change;
                u32 dpi;

                TimeSpan present_time;
                bool is_programmatic_control;
                bool is_defer_pos;
                bool is_update_window_pos;
                POINT cursor;
                vp::util::Int2 c_pos;
            };
        public:
            using NativeWindowName = vp::util::FixedString16<0x40>;
            using WindowThread = sys::FunctionThread;
            using ThreadDelegate = sys::ThreadMemberFunction<NativeWindow>;
            using EventTriggers = vp::util::FixedConnection<u32, 0x20>;
            using ProgramTriggers = vp::util::FixedConnection<u32, 0x20>;
        private:
            vp::util::IntrusiveListNode m_list_node;
            NativeWindowName   m_name;
            size_t             m_message_array[4];
            ThreadDelegate     m_thread_delegate;
            WindowThread       m_window_thread;
            sys::Event         m_start_event;
            sys::Event         m_close_event;
            HWND               m_hwnd;

            CommitSettings     m_commit_settings;
            sys::Mutex         m_program_cs;
            ProgramSettings    m_program_settings;
            ProgramTriggers    m_program_triggers;
            sys::Mutex         m_event_cs;
            EventSettings      m_event_settings;
            EventTriggers      m_event_triggers;

            impl::DropTargetHandler *m_drop_handler;
            impl::DropData *m_drop_array;
            u32 m_drop_count;
            bool m_is_drop_target_enabled;
        private:
            constexpr u32 GetStyle() const {
                //const u32 is_minimizable = (m_program_settings.is_minimizable) ? WS_MINIMIZEBOX;
                //const u32 is_maximizable = (m_program_settings.is_maximizable) ? WS_MAXIMIZEBOX;
                const u32 is_caption = (m_program_settings.is_free_movable && !m_name.IsEmpty()) ? WS_CAPTION : 0;
                const u32 is_scaleable = (m_program_settings.is_free_resizable) ? WS_SIZEBOX : 0;
                const u32 is_moveable = (m_program_settings.is_free_movable) ? WS_BORDER : 0;
                const u32 style = is_caption | is_scaleable | is_moveable;
                return style;
            }
            constexpr u32 GetStyleEx() const {
                const u32 is_no_redir_map = WS_EX_NOREDIRECTIONBITMAP;
                const u32 ex_style = is_no_redir_map;
                return ex_style;
            }
        protected:
            static long long int WindowProcedure(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam);
            void WindowThreadMain(size_t message);
            void CreateWindow();
            void ApplyWindowPos(TimeSpan present_time);
        public:
            constexpr NativeWindow() : m_list_node(), m_name(), m_message_array(), m_thread_delegate(this, &NativeWindow::WindowThreadMain), m_window_thread(), m_start_event(), m_close_event(), m_hwnd(), m_commit_settings(), m_program_cs(), m_program_settings(), m_program_triggers(), m_event_cs(), m_event_settings(), m_event_triggers(), m_drop_handler(), m_drop_array(), m_drop_count(), m_is_drop_target_enabled() {/*...*/}
            constexpr ~NativeWindow() {/*...*/}

            void Initialize(mem::Heap *heap, const NativeWindowInfo *info);
            void Finalize();

            void Calculate();
            void RequestWindowPos(bool is_sync);
            void SetPresentTime(TimeSpan present_time);

            constexpr NativeWindowHandle GetNativeHandle() const { return m_hwnd; }
        public:
            void Open();
            void Close();
        public:
            constexpr bool IsClosed() const { return m_commit_settings.is_closed; }
            constexpr bool IsMoved() const { return m_commit_settings.is_moved; }
            constexpr bool IsResized() const { return m_commit_settings.is_resized; }
            constexpr bool IsVisible() const { return m_commit_settings.is_visible && 0 < m_commit_settings.size[0] && 0 < m_commit_settings.size[1]; }
        public:
            void SetPosition(vp::util::Int2 position);
            void SetSize(vp::util::Int2 size);
            void SetIsVisible(bool is_visible);
        public:
            constexpr vp::util::Int2 GetPosition() const { return m_commit_settings.position; }
            constexpr vp::util::Int2 GetSize() const { return m_commit_settings.size; }
        public:
            void SetFullscreen(bool is_enable);

            constexpr bool IsFullscreen() const { return m_commit_settings.is_fullscreen; }
        public:
            void SetIsFreeCloseable(bool is_enable);
            void SetIsFreeMoveable(bool is_enable);
            void SetIsFreeResizable(bool is_enable);

            constexpr FreeMoveEvent GetFreeMoveEvent() const { return m_commit_settings.free_move_event; }
            constexpr FreeResizeEvent GetFreeResizeEvent() const { return m_commit_settings.free_resize_event; }
        public:
            void SetIsEnableDragDrop(bool is_enable);
            vp::util::Int2 GetDragDropPosition();

            constexpr DragDropEvent GetDragDropEvent() const { return m_commit_settings.drag_drop_event; }
            constexpr u32 GetDragDropCount() const { return m_drop_count; }
            constexpr impl::DropData *GetDragDropArray() const { return m_drop_array; }
        public:
            constexpr bool IsDpiChange() const { return m_commit_settings.is_dpi_change; }
            constexpr u32 GetDpi() const { return m_commit_settings.dpi; }
        public:
            void OpenFileSelect();
            void OpenErrorWindow(const char *error_message);
    };
}

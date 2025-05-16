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
#include <awn.hpp>

namespace awn::grr {

    long long int NativeWindow::WindowProcedure(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam) {
        switch (msg) {
            case WM_CREATE:
            {
                /* Set NativeWindow to user data for other messages */
                const CREATESTRUCT *create_struct = reinterpret_cast<CREATESTRUCT*>(lparam);
                ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
                break;
            }
            case WM_CLOSE:
            {
                /* TODO; Close callback to prevent closing */
                NativeWindow *n_window = reinterpret_cast<NativeWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                std::scoped_lock l(n_window->m_event_cs);
                const bool result = ::DestroyWindow(n_window->m_hwnd);
                VP_ASSERT(result);
                break;
            }
            case WM_DESTROY:
            {
                /* Tell application we closed */
                NativeWindow *n_window = reinterpret_cast<NativeWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                std::scoped_lock l(n_window->m_event_cs);
                n_window->m_event_settings.is_close_event = true;
                ::PostQuitMessage(0);
                break;
            }
            case WM_ERASEBKGND:
            {
                return 1;
            }
            case WM_WINDOWPOSCHANGING:
            {
                return 0;
            }
            case WM_WINDOWPOSCHANGED:
            {
                /* Directly commit size for programmatic change */
                NativeWindow *n_window = reinterpret_cast<NativeWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                std::scoped_lock l(n_window->m_event_cs);
                //::puts("Def event");
                WINDOWPOS *pos = reinterpret_cast<WINDOWPOS*>(lparam);
                if (!n_window->m_commit_settings.is_programmatic_control) {
                    if ((pos->flags & SWP_NOMOVE) == 0) {
                        const vp::util::Int2 new_pos = { pos->x, pos->y, };
                        n_window->m_event_settings.position = new_pos;
                        n_window->m_event_settings.is_moved = (n_window->m_commit_settings.position != new_pos);
                    }
                    if ((pos->flags & SWP_NOSIZE) == 0) {
                        const vp::util::Int2 new_size = { pos->cx, pos->cy, };
                        n_window->m_event_settings.size = new_size;
                        n_window->m_event_settings.is_resized = (n_window->m_commit_settings.size != new_size);
                    }
                }
                if ((pos->flags & SWP_SHOWWINDOW) != 0) {
                    n_window->m_commit_settings.is_visible = true;
                }
                if ((pos->flags & SWP_HIDEWINDOW) != 0) {
                    n_window->m_commit_settings.is_visible = false;
                }
                break;
            }
            case WM_ENTERSIZEMOVE:
            {
                /* Tell application we are resizing */
                NativeWindow *n_window = reinterpret_cast<NativeWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                std::scoped_lock l(n_window->m_event_cs);
                //::printf("Enter sizemove\n");
                if (wparam == SC_SIZE) {
                    n_window->m_event_settings.free_move_event = FreeMoveEvent::Moving;
                } else if (wparam == SC_SIZE) {
                    n_window->m_event_settings.free_resize_event = FreeResizeEvent::Resizing;
                }
                std::scoped_lock l0(n_window->m_event_cs);
                ::GetCursorPos(std::addressof(n_window->m_commit_settings.cursor));
                n_window->m_commit_settings.c_pos = n_window->m_commit_settings.position;
                break;
            }
            case WM_EXITSIZEMOVE:
            {
                break;
            }
            case WM_NCCALCSIZE:
            {
                break;
            }
            case WM_MOVING:
            {
                NativeWindow *n_window = reinterpret_cast<NativeWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                if (n_window == nullptr) { break; }
                std::scoped_lock l0(n_window->m_event_cs);
                if (n_window->m_commit_settings.is_programmatic_control) {
                    //::printf(" move\n");
                    break;
                }
                RECT *rect = reinterpret_cast<RECT*>(lparam);
                POINT curse = {};
                ::GetCursorPos(std::addressof(curse));
                const vp::util::Int2 displacement = { curse.x - n_window->m_commit_settings.cursor.x, curse.y - n_window->m_commit_settings.cursor.y, };
                const vp::util::Int2 new_pos = n_window->m_commit_settings.c_pos + displacement;
                //::printf("Move event: (%i, %i) -> (%i, %i)\n", n_window->m_commit_settings.position[0], n_window->m_commit_settings.position[1], new_pos[0], new_pos[1]);
                if (n_window->m_commit_settings.position == new_pos) { break; }
                n_window->m_event_settings.position = new_pos;
                n_window->m_event_settings.is_moved = (n_window->m_commit_settings.position != new_pos);
                rect->left = n_window->m_commit_settings.position[0];
                rect->top = n_window->m_commit_settings.position[1];
                rect->right = n_window->m_commit_settings.position[0] + n_window->m_commit_settings.size[0];
                rect->bottom = n_window->m_commit_settings.position[1] + n_window->m_commit_settings.size[1];
                break;
            }
            case WM_SIZING:
            {
                NativeWindow *n_window = reinterpret_cast<NativeWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                if (n_window == nullptr) { break; }
                std::scoped_lock l0(n_window->m_event_cs);
                if (n_window->m_commit_settings.is_programmatic_control) {
                    //::printf(" size\n");
                    break;
                }
                RECT *rect = reinterpret_cast<RECT*>(lparam);
                const vp::util::Int2 new_pos = { rect->left, rect->top, };
                const vp::util::Int2 new_size = { rect->right - rect->left, rect->bottom - rect->top, };
                if (n_window->m_commit_settings.size == new_size) { break; }
                //::printf("Size event: (%i, %i) -> (%i, %i)\n", n_window->m_commit_settings.size[0], n_window->m_commit_settings.size[1], new_size[0], new_size[1]);
                n_window->m_event_settings.position = new_pos;
                n_window->m_event_settings.is_moved = (n_window->m_commit_settings.position != new_pos);
                n_window->m_event_settings.size = new_size;
                n_window->m_event_settings.is_resized = true;
                rect->left = n_window->m_commit_settings.position[0];
                rect->top = n_window->m_commit_settings.position[1];
                rect->right = n_window->m_commit_settings.position[0] + n_window->m_commit_settings.size[0];
                rect->bottom = n_window->m_commit_settings.position[1] + n_window->m_commit_settings.size[1];
                break;
            }
            case WM_GETDPISCALEDSIZE:
            {
                break;
            }
            case WM_DPICHANGED:
            {
                const RECT *rect = reinterpret_cast<const RECT*>(lparam);
                const u32 new_dpi = vp::util::Max(LOWORD(lparam), HIWORD(lparam));
                const vp::util::Int2 new_pos = { .x = rect->left, .y = rect->top, };
                const vp::util::Int2 new_size = { .x = rect->right - rect->left, .y = rect->bottom - rect->top, };

                /* Set event dpi changes */
                NativeWindow *n_window = reinterpret_cast<NativeWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                std::scoped_lock l(n_window->m_event_cs);
                n_window->m_event_settings.position = new_pos;
                n_window->m_event_settings.is_moved = (n_window->m_commit_settings.position != new_pos);
                n_window->m_event_settings.size = new_size;
                n_window->m_event_settings.is_resized = (n_window->m_commit_settings.size != new_size);
                n_window->m_event_settings.dpi = new_dpi;
                n_window->m_event_settings.is_dpi_change = (n_window->m_commit_settings.dpi != new_dpi);
                break;
            }
            case WM_USER:
            {
                /* Wait for programmatic move/resize timing */
                NativeWindow *n_window = reinterpret_cast<NativeWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                const TimeSpan present_time = TimeSpan::FromNanoSeconds(static_cast<s64>(lparam));
                n_window->ApplyWindowPos(present_time);
                break;
            }
            default:
                break;
        };
        return ::DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    void NativeWindow::RequestWindowPos(bool is_sync) {

        /* Signal window event for any extraneous close/exit */
        if (!is_sync && m_commit_settings.is_defer_pos) { return;}
        s64 time;
        {
            std::scoped_lock l0(m_program_cs);
            if (!m_commit_settings.is_update_window_pos) { return;}
            m_commit_settings.is_update_window_pos = false;
            time = m_commit_settings.present_time.GetNanoSeconds();
        }
        if (is_sync) {
            ::SendMessageA(m_hwnd, WM_USER, 0, time);
        } else {
            ::SendNotifyMessageA(m_hwnd, WM_USER, 0, time);
        }

        return;
    }

    void NativeWindow::WindowThreadMain(size_t message) {

        /* Signal window event for any extraneous close/exit */
        if (message != cMessageOpen) {
            m_close_event.Signal();
            return;
        }

        /* Create window */
        this->CreateWindow();
        m_start_event.Signal();

        /* Message loop */
        MSG msg = {};
        while (::GetMessage(std::addressof(msg), nullptr, 0, 0)) {
            ::TranslateMessage(std::addressof(msg));
            ::DispatchMessage(std::addressof(msg));
            /* Handle user message */
            size_t awn_message = 0;
            const bool is_message = m_window_thread.TryPeekMessage(std::addressof(awn_message));
            if (is_message) {
                if (awn_message == cMessageClose || awn_message == cMessageExitCode) { ::PostQuitMessage(0); }
            }
        }

        m_close_event.Signal();

        return;
    }

    void NativeWindow::CreateWindow() {

        std::scoped_lock l0(m_event_cs);
        /* Initialize ole */
        const HRESULT hresult0 = ::OleInitialize(nullptr);
        VP_ASSERT(hresult0 == S_OK);

        /* Create window */
        const u32 style = this->GetStyle();
        const u32 ex_style = this->GetStyleEx();
        HWND hwnd = ::CreateWindowExW(ex_style, impl::cNativeWindowClassName, reinterpret_cast<wchar_t*>(m_name.GetString()), style, m_commit_settings.position[0], m_commit_settings.position[1], 0, 0, nullptr, nullptr, ::GetModuleHandle(nullptr), this);
        VP_ASSERT(hwnd != nullptr);
        m_hwnd = hwnd;

        /* Get dpi */
        const u32 dpi = ::GetDpiForWindow(hwnd);
        m_commit_settings.dpi = dpi;

        /* Adjust base size for dpi */
        RECT adj_rect = {
            .right = m_commit_settings.size[0],
            .bottom = m_commit_settings.size[1],
        };
        const bool result0 = ::AdjustWindowRectExForDpi(reinterpret_cast<RECT*>(std::addressof(adj_rect)), style, false, ex_style, dpi);
        VP_ASSERT(result0);
        const vp::util::Int2 pos = { adj_rect.left, adj_rect.top };
        const vp::util::Int2 size = { adj_rect.right - adj_rect.left, adj_rect.bottom - adj_rect.top };
        ::SetWindowPos(hwnd, nullptr, pos[0], pos[1], size[0], size[1], SWP_NOZORDER);
        m_commit_settings.size = size;

        /* Register drag drop */
        //if (m_drag_drop_settings.is_enable) {
        //    const HRESULT hresult2 = ::RegisterDragDrop(m_hwnd, m_drop_target_handler);
        //    VP_ASSERT(hresult2 == S_OK);
        // }

        m_commit_settings.is_closed = false;
        m_event_settings.is_close_event = false;

        return;
    }

    void NativeWindow::Initialize(mem::Heap *heap, const NativeWindowInfo *info) {

        /* Set name */
        m_name = info->name;

        /* Set settings */
        m_commit_settings.position = info->position;
        m_commit_settings.size = info->size;
        m_commit_settings.is_closed = true;

        /* Create thread */
        sys::ThreadInfo thread_info = {
            .name = info->name,
            .exit_code = cMessageExitCode,
            .run_mode = sys::ThreadRunMode::WaitForMessage,
            .message_count = 4,
            .message_memory = m_message_array,
            .stack_size = vp::util::c16KB,
            .priority = sys::cPriorityNormal - 3,
        };
        RESULT_ABORT_UNLESS(m_window_thread.Initialize(heap, std::addressof(thread_info), std::addressof(m_thread_delegate)));
        m_window_thread.StartThread();

        impl::DisplayManager *display_manager = impl::DisplayManager::GetInstance();
        if (display_manager != nullptr) {
            display_manager->AddNativeWindow(this);
        }

        return;
    }

    void NativeWindow::Finalize() {
        impl::DisplayManager *display_manager = impl::DisplayManager::GetInstance();
        if (display_manager != nullptr) {
            display_manager->RemoveNativeWindow(this);
        }
        this->Close();
        m_window_thread.ExitThread(sys::ExitType::Jam);
        return;
    }

    void NativeWindow::ApplyWindowPos(TimeSpan present_time) {

        /* Wait until the vblank before next presentation */
        VP_UNUSED(present_time);
        impl::ContextDxgi11 *ctx_dxgi = impl::ContextDxgi11::GetInstance();
        IDCompositionDevice4 *dcmp_device = ctx_dxgi->GetDcmpDevice4();
        DCOMPOSITION_FRAME_STATISTICS stats;       
        std::scoped_lock l0(m_program_cs);
        for (;;) {
            const HRESULT hresult0 = dcmp_device->GetFrameStatistics(std::addressof(stats));
            VP_ASSERT(hresult0 == S_OK);
            const s64 next_time = TimeSpan::FromTick(stats.nextEstimatedFrameTime.QuadPart).GetNanoSeconds();
            const s64 target_time = m_commit_settings.present_time.GetNanoSeconds();
            if (target_time <= next_time) { break; }
            ::DCompositionWaitForCompositorClock(0, nullptr, INFINITE);
        }

        /* Apply updated window pos */
        std::scoped_lock l1(m_event_cs);
        u32 is_frame_change = 0;
        if (m_program_triggers.GetBit(ProgramTrigger_Style)) {
            is_frame_change = SWP_FRAMECHANGED;

            const u32 style = this->GetStyle();
            ::SetWindowLongPtrA(m_hwnd, GWL_STYLE, style);
            const u32 ex_style = this->GetStyleEx();
            ::SetWindowLongPtrA(m_hwnd, GWL_EXSTYLE, ex_style);

            m_commit_settings.is_free_closeable = m_program_settings.is_free_closeable;
            m_commit_settings.is_free_movable = m_program_settings.is_free_movable;
            m_commit_settings.is_free_resizable = m_program_settings.is_free_resizable;
        }

        const u32 is_move = (m_program_triggers.GetBit(ProgramTrigger_IsMove)) ? 0 : SWP_NOMOVE;
        const u32 is_resize = (m_program_triggers.GetBit(ProgramTrigger_IsResize)) ? 0 : SWP_NOSIZE;
        const u32 is_visible = (m_program_settings.is_visible) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
        const u32 is_keep_z = SWP_NOZORDER;
        const u32 is_no_activate = (m_program_triggers.GetBit(ProgramTrigger_IsFocus)) ? 0 : SWP_NOACTIVATE;
        const u32 flags = is_move | is_resize | is_visible | is_frame_change | is_keep_z | is_no_activate;

        m_commit_settings.is_programmatic_control = true;
        ::SetWindowPos(m_hwnd, nullptr, m_commit_settings.position[0], m_commit_settings.position[1], m_commit_settings.size[0], m_commit_settings.size[1], flags);
        m_commit_settings.is_programmatic_control = false;

        m_program_triggers.Clear();

        return;
    }

    void NativeWindow::SetPresentTime(TimeSpan present_time) {
        std::scoped_lock l(m_program_cs);
        m_commit_settings.present_time = present_time;
    }

    void NativeWindow::Calculate() {

        std::scoped_lock l1(m_program_cs);
        std::scoped_lock l0(m_event_cs);

        /* Nothing to do if closed */
        if (m_commit_settings.is_closed) { return; }

        /* Apply next drag drop */
        if (m_drop_array != nullptr) {
            ::free(m_drop_array);
            m_drop_array = nullptr;
            m_drop_count = 0;
        }
        if (m_program_triggers.GetBit(ProgramTrigger_DragDrop)) {
            m_drop_array = m_drop_handler->m_data_array;
            m_drop_count = m_drop_handler->m_data_count;
            m_drop_handler->m_data_array = nullptr;
            m_drop_handler->m_data_count = 0;
        }
        m_program_triggers.ClearBit(ProgramTrigger_DragDrop);

        /* Apply last frame events */
        bool is_wndpos = (m_program_triggers.GetBitMask(0) != 0);            
        if (m_event_settings.is_moved || m_program_triggers.GetBit(ProgramTrigger_IsMove)) {
            const vp::util::Int2 pos = (m_program_triggers.GetBit(ProgramTrigger_IsMove)) ? m_program_settings.position : m_event_settings.position;
            m_commit_settings.position = pos;
            is_wndpos = true;
            m_commit_settings.is_moved = true;
            m_event_settings.is_moved = false;
        } else {
            m_commit_settings.is_moved = false;
        }
        if (m_event_settings.is_resized || m_program_triggers.GetBit(ProgramTrigger_IsResize)) {
            const vp::util::Int2 size = (m_program_triggers.GetBit(ProgramTrigger_IsResize)) ? m_program_settings.size : m_event_settings.size;
            m_commit_settings.size = size;
            is_wndpos = true;
            m_commit_settings.is_resized = true;
            m_event_settings.is_resized = false;
        } else {
            m_commit_settings.is_resized = false;
        }
        m_commit_settings.is_update_window_pos |= is_wndpos;

        if (m_event_settings.is_close_event) {
            m_commit_settings.is_closed = true;
            m_event_settings.is_close_event = false;
        }
        if (m_event_settings.is_dpi_change) {
            m_commit_settings.dpi = m_event_settings.dpi;
            m_commit_settings.is_dpi_change = true;
            m_event_settings.is_dpi_change = false;
        } else {
            m_commit_settings.is_dpi_change = false;
        }
        m_commit_settings.free_move_event = m_event_settings.free_move_event;
        m_commit_settings.free_resize_event = m_event_settings.free_resize_event;

        return;
    }

    void NativeWindow::Open() {
        /* Send open message */
        if (!m_commit_settings.is_closed) { return; }
        m_start_event.Clear();
        m_window_thread.SendMessage(cMessageOpen);
        m_start_event.Wait();
        return;
    }

    void NativeWindow::Close() {
        /* Send exit message */
        if (m_commit_settings.is_closed) { return; }
        m_close_event.Clear();
        m_window_thread.SendMessage(cMessageClose);
        m_close_event.Wait();
        return;
    }

    void NativeWindow::SetPosition(vp::util::Int2 position) {
        std::scoped_lock l(m_program_cs);
        m_program_triggers.SetBit(ProgramTrigger_DeferProc);
        m_program_triggers.SetBit(ProgramTrigger_IsMove);
        m_program_settings.position = position;
    }

    void NativeWindow::SetSize(vp::util::Int2 size) {
        std::scoped_lock l(m_program_cs);
        m_program_triggers.SetBit(ProgramTrigger_DeferProc);
        m_program_triggers.SetBit(ProgramTrigger_IsResize);
        m_program_settings.size = size;
    }

    void NativeWindow::SetIsVisible(bool is_visible) {
        std::scoped_lock l(m_program_cs);
        m_program_triggers.SetBit(ProgramTrigger_DeferProc);
        m_program_settings.is_visible = is_visible;
    }

    void NativeWindow::SetIsFreeMoveable(bool is_enable) {
        std::scoped_lock l(m_program_cs);
        m_program_triggers.SetBit(ProgramTrigger_DeferProc);
        m_program_triggers.SetBit(ProgramTrigger_Style);
        m_program_settings.is_free_movable = is_enable;
    }

    void NativeWindow::SetIsFreeResizable(bool is_enable) {
        std::scoped_lock l(m_program_cs);
        m_program_triggers.SetBit(ProgramTrigger_DeferProc);
        m_program_triggers.SetBit(ProgramTrigger_Style);
        m_program_settings.is_free_resizable = is_enable;
    }

    void NativeWindow::SetIsFreeCloseable(bool is_enable) {
        std::scoped_lock l(m_program_cs);
        m_program_triggers.SetBit(ProgramTrigger_DeferProc);
        m_program_triggers.SetBit(ProgramTrigger_Style);
        m_program_settings.is_free_closeable = is_enable;
    }
}

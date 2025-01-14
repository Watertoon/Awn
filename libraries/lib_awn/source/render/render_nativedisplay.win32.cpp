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

namespace awn::render {

    long long int NativeDisplay::WindowProcedure(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam) {
        switch (msg) {
            case WM_CREATE:
            {
                const CREATESTRUCT *create_struct = reinterpret_cast<CREATESTRUCT*>(lparam);
                ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
                break;
            }
            case WM_CLOSE:
            {
                ::PostQuitMessage(0);
                break;
            }
            case WM_ERASEBKGND:
            {
                return 1;
            }
            case WM_NCCALCSIZE:
            {
                /* TODO; investigate moving resize to reduce jitter */
                RECT *client_rect = reinterpret_cast<RECT*>(lparam);
                NativeDisplay *native_display = reinterpret_cast<NativeDisplay*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                if (native_display == nullptr) { break; }
                //::DCompositionWaitForCompositorClock(0, nullptr, INFINITE);
                const vp::util::Int2 new_size = { .x = client_rect->right - client_rect->left, .y = client_rect->bottom - client_rect->top, };
                native_display->SetSizeImpl(new_size);
                ::DefWindowProcW(hwnd, msg, wparam, lparam);
                return WVR_REDRAW;
            }
            case WM_SIZE:
            {
                if (wparam == SIZE_MINIMIZED) {
                    NativeDisplay *native_display = reinterpret_cast<NativeDisplay*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                    native_display->m_is_visible = false;
                } else if (wparam == SIZE_MAXIMIZED) {
                    NativeDisplay *native_display = reinterpret_cast<NativeDisplay*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                    native_display->m_is_visible = (native_display->m_update_settings.size.x > 0 && native_display->m_update_settings.size.y > 0) && !native_display->m_update_settings.is_hidden;
                }
                break;
            }
            case WM_MOVE:
            {
                NativeDisplay *native_display = reinterpret_cast<NativeDisplay*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                if (!native_display->m_is_border) { break; }
                const vp::util::Int2 new_pos = { .x = LOWORD(lparam), .y = HIWORD(lparam), };
                native_display->SetPositionForFreeMoveable(new_pos);
                break;
            }
            default:
                break;
        };
        return ::DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    void NativeDisplay::SetSizeImpl(vp::util::Int2 size) {
        {
            std::scoped_lock l(m_settings_cs);
            if (m_update_settings.size == size) { return; }
            m_update_settings.size = size;
            m_is_update_size = true;
            for (NativeLayer &layer : m_layer_list) {
                layer.InvokeDisplayResize(this);
            }
            m_is_update_size = false;
            m_is_visible = (size.x > 0 && size.y > 0) && !m_update_settings.is_hidden;
            m_settings.size = size;
            VP_ASSERT(m_is_resize == 0);
            m_is_resize = 1;
            m_window_event.Clear();
        }
        m_window_event.Wait();
    }

    void NativeDisplay::SetPositionForFreeMoveable(vp::util::Int2 position) {
        std::scoped_lock l(m_settings_cs);
        if (m_update_settings.position == position) { return; }
        m_update_settings.position = position;
        m_settings.position = position;
    }

    void NativeDisplay::WindowThreadMain(size_t message) {

        if (message != cMessageStart) { return; }

        /* Create window */
        this->CreateWindow();

        m_is_closed = false;
        m_window_event.Signal();

        /* Message loop */
        MSG msg = {};
        while (::GetMessage(std::addressof(msg), nullptr, 0, 0)) {
            ::TranslateMessage(std::addressof(msg));
            /* Handle user message */
            if (LOWORD(msg.message) == cWin32AwnMessage) {
                size_t awn_message = 0;
                const bool is_message = vp::util::GetReference(m_window_thread).TryPeekMessage(std::addressof(awn_message));
                if (!is_message) { continue; }
                if (awn_message == cMessageQuit || awn_message == cMessageExitCode) {
                    ::PostQuitMessage(0);
                    break;
                }
                continue;
            }
            ::DispatchMessage(std::addressof(msg));
        }

        m_is_closed = true;

        return;
    }

    void NativeDisplay::SendMessage(size_t message) {
        vp::util::GetReference(m_window_thread).SendMessage(message);
        const bool result = ::PostMessageW(m_hwnd, cWin32AwnMessage, 0, 0);
        VP_ASSERT(m_is_closed || result);
    }

    void NativeDisplay::CreateWindow() {

        /* Determine style */
        const u32 ex_style = WS_EX_NOREDIRECTIONBITMAP;
        const u32 border_style = (m_update_is_border) ? WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU : 0;
        const u32 scale_style = (m_update_is_border && m_update_free_scalable) ?  WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX : 0;
        const u32 style = border_style | scale_style;
        m_is_border = m_update_is_border;
        m_is_free_scalable = m_update_free_scalable;
        m_is_free_movable = m_update_free_movable;

        /* Adjust size of window with respect to dpi */
        vp::util::Rectanglei rect_helper = {};
        rect_helper.SetSize(m_update_settings.size);
        //vp::util::Float3 position = vp::util::GetPosition(m_update_settings.transform);
        //rect_helper.SetPosition();
        RECT monitor_rect = {
            .left = rect_helper.left,
            .top = rect_helper.top,
            .right = rect_helper.right,
            .bottom = rect_helper.bottom,
        };
        HMONITOR hmonitor = ::MonitorFromRect(std::addressof(monitor_rect), MONITOR_DEFAULTTONEAREST);
        VP_ASSERT(hmonitor != nullptr);
        u32 dpi_x = 0;
        u32 dpi_y = 0;
        const HRESULT hresult0 = ::GetDpiForMonitor(hmonitor, MDT_EFFECTIVE_DPI, std::addressof(dpi_x), std::addressof(dpi_y));
        VP_ASSERT(hresult0 == S_OK);
        const u32 dpi = vp::util::Min(dpi_x, dpi_y);
        RECT adj_rect = {
            .right = m_update_settings.size.x,
            .bottom = m_update_settings.size.y,
        };
        const bool result0 = ::AdjustWindowRectExForDpi(reinterpret_cast<RECT*>(std::addressof(adj_rect)), style, false, ex_style, dpi);
        VP_ASSERT(result0);
        vp::util::Int2 size = { adj_rect.right - adj_rect.left, adj_rect.bottom - adj_rect.top };

        /* Create window */
        HWND window_handle = ::CreateWindowExW(ex_style, impl::cNativeDisplayWindowClassName, reinterpret_cast<wchar_t*>(m_name.GetString()), style, m_update_settings.position.x, m_update_settings.position.y, size.x, size.y, nullptr, nullptr, ::GetModuleHandle(nullptr), this);
        VP_ASSERT(window_handle != nullptr);

        /* Get window dpi */
        const u32 wnd_dpi = ::GetDpiForWindow(window_handle);
        VP_ASSERT(wnd_dpi != 0);
        m_dpi = wnd_dpi;

        /* Create direct composition target */
        impl::ContextDxgi11 *context = impl::ContextDxgi11::GetInstance();
        IDCompositionDesktopDevice *device_desktop = context->GetDcmpDevice();
        IDCompositionTarget *target = nullptr;
        const HRESULT result1 = device_desktop->CreateTargetForHwnd(window_handle, false, std::addressof(target));
        VP_ASSERT(result1 == S_OK);

        /* Create direct composition root visual */
        IDCompositionVisual2 *visual_2 = nullptr;
        const HRESULT result2 = device_desktop->CreateVisual(std::addressof(visual_2));
        VP_ASSERT(result2 == S_OK);
        IDCompositionVisual3 *visual_main = nullptr;
        const HRESULT result3 = visual_2->QueryInterface(std::addressof(visual_main));
        VP_ASSERT(result3 == S_OK);
        visual_2->Release();

        /* Set root visual to target */
        const HRESULT result4 = target->SetRoot(visual_main);
        VP_ASSERT(result4 == S_OK);

        /* Set params */
        m_hwnd = window_handle;
        m_dcmp_target = target;
        m_dcmp_visual_main = visual_main;
        m_settings.position = m_update_settings.position;
        m_settings.size = m_update_settings.size;

        return;
    }

    void NativeDisplay::ApplyTransform(HDWP hdwp) {

        std::scoped_lock l(m_settings_cs);

        /* Apply window style */
        if (m_is_update_style) {
            const u32 border_style = (m_update_is_border) ? WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU : 0;
            const u32 scale_style = (m_update_is_border && m_update_free_scalable) ?  WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX : 0;
            const u32 style = border_style | scale_style;
            ::SetWindowLongPtrW(m_hwnd, GWL_STYLE, style);
            m_is_update_style = false;
            m_is_border = m_update_is_border;
            m_is_free_scalable = m_update_free_scalable;
        }
        m_is_free_movable = m_update_free_movable;

        /* Apply transform */
        if (m_is_update_size || m_is_update_position || m_is_update_defer) {

            /* Prevent programmatically setting position and size if the user has control of it */
            const vp::util::Int2 pos = m_update_settings.position;
            const vp::util::Int2 size = m_update_settings.size;
            m_is_visible = (size.x > 0 && size.y > 0) && !m_update_settings.is_hidden;
            const u32 hidden_flags = (m_update_settings.is_hidden || !m_is_visible) ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;
            const u32 scale_flags = (m_is_free_scalable) ? SWP_NOSIZE : 0;
            const u32 move_flags = (m_is_free_movable) ? SWP_NOMOVE : 0;
            const u32 flags = hidden_flags | move_flags | scale_flags | SWP_NOZORDER | SWP_NOCOPYBITS;
            ::DeferWindowPos(hdwp, m_hwnd, nullptr, pos.x, pos.y, size.x, size.y, flags);

            if (size != m_settings.size) {
                for (NativeLayer &layer : m_layer_list) {
                    layer.InvokeDisplayResize(this);
                }
                m_is_resize = 1;
            }

            m_is_update_position = false;
            m_is_update_size = false;
            m_is_update_defer = false;
            m_settings.position = pos;
            m_settings.size = size;
        }

        return;
    }

    void NativeDisplay::FinishResize() {
        std::scoped_lock l(m_settings_cs);
        if (m_is_resize == 0) { return; }
        //if (m_is_resize == 1) { m_is_resize = 2; return; }
        m_is_resize = 0;
        m_window_event.Signal();
    }

    void NativeDisplay::AddLayerVisual(NativeLayer *native_layer) {
        auto iter = m_layer_list.IteratorTo(*native_layer);
        ++iter;
        auto end = m_layer_list.end();
        IDCompositionVisual3 *next_visual = nullptr;
        while (iter != end) {
            NativeLayer *layer = std::addressof(*iter);
            if (!layer->m_settings.is_visible) { continue; }
            next_visual = layer->GetDcmpVisual();
            break;
        }
        const HRESULT hresult0 = m_dcmp_visual_main->AddVisual(native_layer->GetDcmpVisual(), false, next_visual);
        VP_ASSERT(hresult0 == S_OK);
    }

    void NativeDisplay::RemoveLayerVisual(NativeLayer *native_layer) {
        const HRESULT hresult0 = m_dcmp_visual_main->RemoveVisual(native_layer->GetDcmpVisual());
        VP_ASSERT(hresult0 == S_OK);
    }

    void NativeDisplay::Initialize(const NativeDisplayInfo *display_info) {

        /* Set parameters */
        m_name = display_info->name;
        m_update_settings.position = display_info->position;
        m_update_settings.size = display_info->size;
        m_update_settings.is_hidden = false;
        m_settings.is_hidden = true;
        m_is_update_defer = true;
        m_update_is_border = display_info->is_border;
        m_update_free_scalable = display_info->is_free_scalable;
        m_update_free_movable = display_info->is_free_movable;
        m_is_visible = (display_info->size.x > 0 && display_info->size.y > 0) && !m_update_settings.is_hidden;

        /* Create window thread and barrier window initialization */
        vp::util::ConstructAt(m_window_thread, std::addressof(m_thread_delegate), "awn::render::NativeDisplayThread", nullptr, sys::ThreadRunMode::WaitForMessage, cMessageExitCode, 2, vp::util::c16KB, sys::cPriorityNormal + 3);
        vp::util::GetReference(m_window_thread).StartThread();
        vp::util::GetReference(m_window_thread).SendMessage(cMessageStart);
        m_window_event.Wait();
        //::ShowWindow(m_hwnd, true);

        /* Add display to video manager */
        impl::NativeVideoManager *video_mgr = impl::NativeVideoManager::GetInstance();
        VP_ASSERT(video_mgr != nullptr);
        video_mgr->AddDisplay(this);

        return;
    }

    void NativeDisplay::Finalize() {

        /* Remove display from video manager */
        impl::NativeVideoManager *video_mgr = impl::NativeVideoManager::GetInstance();
        video_mgr->RemoveDisplay(this);

        /* Destroy window */
        this->SendMessage(cMessageQuit);
        vp::util::GetReference(m_window_thread).ExitThread(sys::ExitType::Jam);
    }

    void NativeDisplay::AddLayer(NativeLayer *native_layer) {
        std::scoped_lock l(m_layer_cs);
        VP_ASSERT(!native_layer->m_display_node.IsLinked());
        NativeLayer *next_layer = nullptr;

        /* Insert on Z order */
        const u32 z = native_layer->GetZOrder();
        for (NativeLayer &layer : m_layer_list) {
            if (layer.GetZOrder() < z) { continue; }
            next_layer = std::addressof(layer);
            break;
        }
        if (next_layer != nullptr) {
            next_layer->m_display_node.LinkPrev(std::addressof(native_layer->m_display_node));
        } else {
            m_layer_list.PushBack(*native_layer);
        }

        ++m_layer_count;
    }

    void NativeDisplay::RemoveLayer(NativeLayer *native_layer){
        std::scoped_lock l(m_layer_cs);
        if (!native_layer->m_display_node.IsLinked()) { return; }
        m_layer_list.Remove(*native_layer);
        --m_layer_count;
    }

    void NativeDisplay::RequestSetIsHidden(bool is_hidden) {
        std::scoped_lock l(m_settings_cs);
        m_update_settings.is_hidden = is_hidden;
        m_is_update_defer = true;
    }

    void NativeDisplay::RequestSetIsBorder(bool is_border) {
        std::scoped_lock l(m_settings_cs);
        m_update_is_border = is_border;
        m_is_update_style = true;
    }

    void NativeDisplay::RequestSetIsFreeMovable(bool is_free_moveable) {
        std::scoped_lock l(m_settings_cs);
        m_update_free_movable = is_free_moveable;
    }

    void NativeDisplay::RequestSetIsFreeScalable(bool is_free_scalable) {
        std::scoped_lock l(m_settings_cs);
        m_update_free_scalable = is_free_scalable;
        m_is_update_style = true;
    }

    void NativeDisplay::RequestSetPosition(vp::util::Int2 position) {
        std::scoped_lock l(m_settings_cs);
        if (m_update_settings.position == position) { return; }
        m_update_settings.position = position;
        m_is_update_position = true;
    }

    void NativeDisplay::RequestSetSize(vp::util::Int2 size) {
        std::scoped_lock l(m_settings_cs);
        if (m_update_settings.size == size) { return; }
        m_window_event.Clear();
        m_update_settings.size = size;
        m_is_update_size = true;
    }
}

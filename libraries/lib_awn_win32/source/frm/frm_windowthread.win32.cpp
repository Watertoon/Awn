#include <awn.hpp>

namespace awn::frm {

    long long int FrameworkWindowFunction(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam) {

        /* Get thread */
        WindowThread *window_thread = reinterpret_cast<WindowThread*>(::GetWindowLongPtrA(hwnd, GWLP_USERDATA));

        switch (msg) {
            case WM_CREATE:
                break;
            case WM_DESTROY:
                ::PostQuitMessage(0);
                return 0;
            case WM_PAINT:
                return 0;
            case WM_SIZE:
            {
                /* Parse new window size */
                const s32 width  = LOWORD(lparam);
                const s32 height = HIWORD(lparam);

                /* Lock window */
                std::scoped_lock lock(window_thread->m_window_cs);
                
                /* Get previous size */
                const s32 prev_width  = window_thread->m_window_info.width;
                const s32 prev_height = window_thread->m_window_info.height;

                /* Check window size changed */
                if (window_thread->m_window_info.width == width && window_thread->m_window_info.height == height) { return ::DefWindowProc(hwnd, msg, wparam, lparam); }

                /* Set resize state */
                window_thread->m_window_info.width         = width;
                window_thread->m_window_info.height        = height;
                window_thread->m_require_swapchain_refresh = true;

                /* Handle minimization */
                if (width == 0 || height == 0) {
                    
                    /* Skip draw if minimized/surfaceless window */
                    window_thread->m_skip_draw = true;
                } else if (prev_width == 0 || prev_height == 0) {

                    /* Create a fresh swapchain on window unminimize */
                    window_thread->m_skip_draw = false;
                    window_thread->RecreateSwapchainIfNecessary();
                    window_thread->AcquireNextImage();
                }
            }
                break;
            case WM_DROPFILES:
            {
                /* Drop the input if we are still pending from the last drag drop */
                if (window_thread->m_drag_drop_status == DragDropStatus::Pending) { return ::DefWindowProc(hwnd, msg, wparam, lparam); }

                /* Iterate drag drop files */
                HDROP hdrop = reinterpret_cast<HDROP>(wparam);
                const u32 file_count = ::DragQueryFileA(hdrop, 0xffff'ffff, nullptr, 0);
                for (u32 i = 0; i < file_count; ++i) {

                    /* Verify path length */
                    const u32 message_size = ::DragQueryFileA(hdrop, i, nullptr, 0);
                    if (vp::util::cMaxPath <= message_size) { window_thread->m_drag_drop_status = DragDropStatus::TooLongPathError; return 0; }

                    /* Copy string */
                    const u32 result = ::DragQueryFileA(hdrop, i, window_thread->m_drag_drop_array[i]->GetString(), vp::util::cMaxPath);
                    if (result == 0) { window_thread->m_drag_drop_status = DragDropStatus::PathCopyError; return ::DefWindowProc(hwnd, msg, wparam, lparam); }
                }

                /* Set drag drop state */
                window_thread->m_drag_drop_status = DragDropStatus::Pending;
                window_thread->m_drag_drop_count  = file_count;
            }
                break;
            default:
                break;
        };
        return ::DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

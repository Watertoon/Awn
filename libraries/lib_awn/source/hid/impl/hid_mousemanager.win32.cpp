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

namespace awn::hid::impl {

    void Mouse::RegisterDevice(HANDLE device) {
        m_device = device;
        m_current_state.SetDefaults();
    }

    void Mouse::Disconnect() {
        m_device = nullptr;
        /* Todo; report disconnect in packet? */
    }

    void Mouse::ProcessInput(RAWMOUSE *input, bool is_unfocus, bool is_all_mouse) {

        /* Ignore if unfocus */
        if (m_is_ignore_no_focus && is_unfocus) { return; }

        /* Parse pos and delta */
        if ((input->usFlags & 1) == MOUSE_MOVE_RELATIVE) {

            /* Windows only natively supports 1 mouse, so if we are a relative mouse, positional data is not reported, unless we are the all mouse, in which we pull the system pos */
            if (!is_all_mouse) {
                m_current_state.position = cInvalidMousePosition;
            } else {
                const u32 msg_pos = static_cast<u32>(::GetMessagePos());
                const vp::util::Int2 pos = { static_cast<s16>(msg_pos & 0xffff), static_cast<s16>((msg_pos >> 0x10) & 0xffff), };
                m_current_state.position = pos;
            }

            /* Set delta direct */
            m_current_state.delta.x += input->lLastX;
            m_current_state.delta.y += input->lLastY;

        } else if ((input->usFlags & 1) == MOUSE_MOVE_ABSOLUTE) {

            /* Get abs virtual screen position */
            vp::util::Rectanglei rect;
            if ((input->usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP) {
                rect.left = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
                rect.top = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
                rect.right = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
                rect.bottom = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
            } else {
                rect.left = 0;
                rect.top = 0;
                rect.right = ::GetSystemMetrics(SM_CXSCREEN);
                rect.bottom = ::GetSystemMetrics(SM_CXSCREEN);
            }
            const vp::util::Int2 pos = {
                ::MulDiv(input->lLastX, rect.right, USHRT_MAX) + rect.left,
                ::MulDiv(input->lLastY, rect.left, USHRT_MAX) + rect.top,
            };
            const vp::util::Int2 prev = m_current_state.position;
            /* TODO; how does an absolute all mouse fair? Presumably the prev WM_MOUSEMOVE pos will be identical to the prev abs pos */
            if (!is_all_mouse) {
                m_current_state.position = pos;
            }

            /* Calc delta */
            m_current_state.delta.x += (pos.x - prev.x);
            m_current_state.delta.y += (pos.y - prev.y);
        }

        /* Unpack button mask */
        const bool is_left_down    = input->usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN;
        const bool is_left_up      = input->usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP;
        const bool is_right_down   = input->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN;
        const bool is_right_up     = input->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP;
        const bool is_middle_down  = input->usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN;
        const bool is_middle_up    = input->usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP;
        const bool is_button4_down = input->usButtonFlags & RI_MOUSE_BUTTON_4_DOWN;
        const bool is_button4_up   = input->usButtonFlags & RI_MOUSE_BUTTON_4_UP;
        const bool is_button5_down = input->usButtonFlags & RI_MOUSE_BUTTON_5_DOWN;
        const bool is_button5_up   = input->usButtonFlags & RI_MOUSE_BUTTON_5_UP;
        if (is_left_up) {
            m_current_state.button_set.ClearBit(static_cast<u32>(MouseButton::Left));
        }
        if (is_left_down) {
            m_current_state.button_set.SetBit(static_cast<u32>(MouseButton::Left));
        }
        if (is_right_up) {
            m_current_state.button_set.ClearBit(static_cast<u32>(MouseButton::Right));
        }
        if (is_right_down) {
            m_current_state.button_set.SetBit(static_cast<u32>(MouseButton::Right));
        }
        if (is_middle_up) {
            m_current_state.button_set.ClearBit(static_cast<u32>(MouseButton::Middle));
        }
        if (is_middle_down) {
            m_current_state.button_set.SetBit(static_cast<u32>(MouseButton::Middle));
        }
        if (is_button4_up) {
            m_current_state.button_set.ClearBit(static_cast<u32>(MouseButton::Forward));
        }
        if (is_button4_down) {
            m_current_state.button_set.SetBit(static_cast<u32>(MouseButton::Forward));
        }
        if (is_button5_up) {
            m_current_state.button_set.ClearBit(static_cast<u32>(MouseButton::Backward));
        }
        if (is_button5_down) {
            m_current_state.button_set.SetBit(static_cast<u32>(MouseButton::Backward));
        }
        /* Unpack wheel */
        const bool is_wheel_v_down = input->usButtonFlags & RI_MOUSE_WHEEL;
        const bool is_wheel_h_down = input->usButtonFlags & RI_MOUSE_HWHEEL;
        m_current_state.delta_wheel.y += (is_wheel_v_down) ? static_cast<s32>(static_cast<s16>(input->usButtonData)) : 0;
        m_current_state.delta_wheel.x += (is_wheel_h_down) ? static_cast<s32>(static_cast<s16>(input->usButtonData)) : 0;

        m_is_dirty = true;
        return;
    }

    void Mouse::ReportState() {
        if (!m_is_dirty) { return; }
        m_is_dirty = false;
        m_ring.PushState(std::addressof(m_current_state));
        m_current_state.delta = vp::util::Int2::cZero;
        m_current_state.delta_wheel = vp::util::Int2::cZero;
    }

    AWN_SINGLETON_TRAITS_IMPL(MouseManager);

    long long int MouseManager::MouseWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
        //::printf("%d\n", message);
        switch (message) {
            case WM_CREATE:
            {
                CREATESTRUCTA *create_struct = reinterpret_cast<CREATESTRUCTA*>(lparam);
                ::SetWindowLongPtrA(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
                break;
            }
            case WM_INPUT:
            {
                /* Get raw mouse data */
                //const bool is_sink = GET_RAWINPUT_CODE_WPARAM(wparam);
                HRAWINPUT hrawinput = reinterpret_cast<HRAWINPUT>(lparam);
                u32 data_size = 0;
                ::GetRawInputData(hrawinput, RID_INPUT, nullptr, std::addressof(data_size), sizeof(RAWINPUTHEADER));
                RAWINPUT *raw_input = reinterpret_cast<RAWINPUT*>(::malloc(data_size));
                VP_ASSERT(raw_input != nullptr);
                ON_SCOPE_EXIT { ::free(raw_input); };
                const u32 result = ::GetRawInputData(hrawinput, RID_INPUT, raw_input, std::addressof(data_size), sizeof(RAWINPUTHEADER));
                VP_ASSERT(result != 0xffff'ffff);

                /* Ignore non-mouse input */
                if (raw_input->header.dwType != RIM_TYPEMOUSE) { break; }

                /* Process for combined */
                MouseManager *mgr = reinterpret_cast<MouseManager*>(::GetWindowLongPtrA(window, GWLP_USERDATA));
                const bool is_unfocus = GET_RAWINPUT_CODE_WPARAM(wparam);
                mgr->m_all_mouse.ProcessInput(std::addressof(raw_input->data.mouse), is_unfocus, true);

                /* Process for slot */
                Mouse *mouse = mgr->GetMouseByDevice(raw_input->header.hDevice);
                if (mouse == nullptr) {
                    mouse = mgr->AddMouse(raw_input->header.hDevice);
                    if (mouse == nullptr) { break; }
                }
                mouse->ProcessInput(std::addressof(raw_input->data.mouse), is_unfocus, false);

                break;
            }
            case WM_INPUT_DEVICE_CHANGE:
            {
                /* Remove mouse device */
                MouseManager *mgr = reinterpret_cast<MouseManager*>(::GetWindowLongPtrA(window, GWLP_USERDATA));
                HANDLE change_device = reinterpret_cast<HANDLE>(lparam);
                if (wparam == GIDC_REMOVAL) {
                    mgr->RemoveMouse(change_device);
                }
                break;
            }
            default:
                break;
        };
        return ::DefWindowProc(window, message, wparam, lparam);
    }

    void MouseManager::ThreadMain(size_t message) {
        VP_UNUSED(message);

        this->InitializeRawInput();

        /* Win32 window message pump */
        MSG msg = {};
        HANDLE report_timer = ::CreateWaitableTimerExA(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        const LARGE_INTEGER due_time = { .QuadPart = -(TimeSpan::FromHertz(cReportRateHz).GetNanoSeconds() / 100), };
        const bool is_set = ::SetWaitableTimer(report_timer, std::addressof(due_time), TimeSpan::FromHertz(cReportRateHz).GetMilliSeconds(), nullptr, nullptr, false);
        VP_ASSERT(is_set);
        ON_SCOPE_EXIT {
            ::CloseHandle(report_timer);
            this->FinalizeRawInput();
        };
        for (;;) {
            /* Process win32 messages */
            const u32 wait_result = ::MsgWaitForMultipleObjects(1, std::addressof(report_timer), false, INFINITE, QS_ALLINPUT);
            if (wait_result == (WAIT_OBJECT_0 + 1)) {                        
                while (::PeekMessageA(std::addressof(msg), nullptr, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) { break; }
                    ::TranslateMessage(std::addressof(msg));
                    ::DispatchMessage(std::addressof(msg));
                }
            } else if (wait_result == WAIT_OBJECT_0) {
                this->ReportStates();
            } else {
                VP_UNEXPECTED_DEFAULT();
            }
            /* Process awn messages */
            size_t message;
            while (m_thread.TryPeekMessage(std::addressof(message))) {
                if (message == Message_ExitCode) { return; }
                m_thread.ReceiveMessage(std::addressof(message));
            }
        }

        return;
    }

    void MouseManager::InitializeRawInput() {

        /* Register mouse window thread */
        const HINSTANCE process_handle = ::GetModuleHandle(nullptr);
        const WNDCLASS wc = {
            .style = 0,
            .lpfnWndProc = MouseWindowProc,
            .hInstance = process_handle,
            .hbrBackground = (HBRUSH)(COLOR_BACKGROUND),
            .lpszClassName = cClassName,
        };
        const u32 result0 = ::RegisterClass(std::addressof(wc));
        VP_ASSERT(result0 != 0);

        /* Create Window thread */
        HWND window_handle = ::CreateWindowEx(0 , cClassName, "Mouse_RawInput", 0, 0, 0, 32, 32, HWND_MESSAGE, 0, process_handle, this);
        VP_ASSERT(window_handle != INVALID_HANDLE_VALUE);
        m_hwnd = window_handle;

        /* Register for mouse input */
        const RAWINPUTDEVICE raw_input_dev_array[] = {
            {
                .usUsagePage = 1,
                .usUsage = 2,
                .dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY,
                .hwndTarget = window_handle,
            },
        };
        const bool result1 = ::RegisterRawInputDevices(raw_input_dev_array, sizeof(raw_input_dev_array) / sizeof(RAWINPUTDEVICE), sizeof(RAWINPUTDEVICE));
        VP_ASSERT(result1);

        return;
    }

    void MouseManager::FinalizeRawInput() {
        if (m_hwnd != nullptr) {
            ::DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        ::UnregisterClassA(cClassName, ::GetModuleHandle(nullptr));
        return;
    }

    void MouseManager::ReportStates() {
        m_all_mouse.ReportState();
        for (Mouse &mouse : m_mouse_array) {
            if (mouse.m_device == nullptr) { continue; }
            mouse.ReportState();
        }
        return;
    }

    Mouse *MouseManager::AddMouse(HANDLE device) {

        /* Check info */
        UINT data_size = 0;
        const UINT result0 = ::GetRawInputDeviceInfoA(device, RIDI_DEVICEINFO, nullptr, std::addressof(data_size));
        if (result0 != 0) { return nullptr; }
        RID_DEVICE_INFO *data = reinterpret_cast<RID_DEVICE_INFO*>(::malloc(data_size));
        VP_ASSERT(data != nullptr);
        ON_SCOPE_EXIT { ::free(data); };
        data->cbSize = sizeof(RID_DEVICE_INFO);
        const UINT result1 = ::GetRawInputDeviceInfoA(device, RIDI_DEVICEINFO, data, std::addressof(data_size));
        if (result1 == 0xffff'ffff) { return nullptr; }
        if (data->dwType != RIM_TYPEMOUSE) { return nullptr; }

        /* Insert mouse to first empty device slot */
        std::scoped_lock l(m_used_cs);
        for (Mouse *&mouse : m_used_mouse_array) {
            if (mouse->m_device != nullptr) { continue; }
            mouse->RegisterDevice(device);
            return mouse;
        }

        return nullptr;
    }

    void MouseManager::RemoveMouse(HANDLE device) {

        /* Remove keyboard by device */
        for (Mouse &mouse : m_mouse_array) {
            if (mouse.m_device != device) { continue; }
            mouse.Disconnect();
        }

        return;
    }

    Mouse *MouseManager::GetMouseByDevice(HANDLE device) {
        for (Mouse &mouse : m_mouse_array) {
            if (mouse.m_device != device) { continue; }
            return std::addressof(mouse);
        }
        return nullptr;
    }

    Result MouseManager::Initialize(mem::Heap *heap, u32 max_mouse_count) {

        /* Initialize arrays */
        if (0 < max_mouse_count) {                    
            m_mouse_array.Initialize(heap, max_mouse_count);
            m_used_mouse_array.Initialize(heap, max_mouse_count);
            for (Mouse &mouse : m_mouse_array) {
                m_used_mouse_array.PushPointer(std::addressof(mouse));
            }
        }

        /* Start mouse thread */
        sys::ThreadInfo info = {
            .name = "awn::hid::MouseManager",
            .exit_code = Message_ExitCode,
            .run_mode = sys::ThreadRunMode::Looping,
            .message_count = cMaxMessageCount,
            .message_memory = m_message_memory,
            .stack_size = vp::util::c16KB,
            .priority = sys::cPriorityHigh,
        };
        RESULT_TRY(m_thread.Initialize(heap, std::addressof(info), std::addressof(m_function)));
        m_thread.StartThread();

        RESULT_RETURN_SUCCESS;
    }

    void MouseManager::Finalize() {
        m_thread.SendMessage(Message_ExitCode);
        const bool is_send = ::PostThreadMessageA(m_thread.GetWin32ThreadId(), WM_QUIT, 0, 0);
        VP_ASSERT(is_send);
        m_thread.ExitThread(sys::ExitType::Send);
        m_thread.Finalize();
        return;
    }

    u32 MouseManager::GetMouseStates(MouseState *out_state_array, u32 state_count, u32 mouse_id) {
        Mouse *mouse;
        if (mouse_id == m_mouse_array.GetCount()) {
            mouse = std::addressof(m_all_mouse);
        } else {
            mouse = m_used_mouse_array[mouse_id];
        }
        u32 out_count = 0;
        mouse->m_ring.ReadStates(std::addressof(out_count), out_state_array, state_count);
        return out_count;
    }

    void MouseManager::SwapMouseAssignment(u32 id0, u32 id1) {
        std::scoped_lock l(m_used_cs);
        m_used_mouse_array.Swap(id0, id1);
        return;
    }

    void MouseManager::DisconnectMouse(u32 mouse_id) {
        {
            std::scoped_lock l(m_used_cs);
            if (m_used_mouse_array[mouse_id]->m_device == nullptr) { return; }
        }
        m_thread.SendMessage(Message_Disconnect | mouse_id);
        return;
    }
}

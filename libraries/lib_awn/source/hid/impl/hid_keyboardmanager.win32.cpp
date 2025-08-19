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

    void Keyboard::RegisterDevice(HANDLE device) {
        m_device = device;
        m_current_state.SetDefaults();
    }

    void Keyboard::Disconnect() {
        m_device = nullptr;
        /* Todo; report disconnect in packet? */
    }

    void Keyboard::ProcessInput(RAWKEYBOARD *input, bool is_unfocus) {

        /* Ignore invalid input */
        if (input->MakeCode == KEYBOARD_OVERRUN_MAKE_CODE || UCHAR_MAX <= input->VKey) { return; }

        /* Compose key */
        u32 make_code;
        if (input->MakeCode != 0) {
            const u32 make_code0 = input->MakeCode & 0x7f;
            const u32 make_code1 = (input->Flags & RI_KEY_E0) ? 0xe0 : ((input->Flags & RI_KEY_E1) ? 0xe1 : 0x00);
            make_code = make_code0 | (make_code1 << 0x8);
        } else {
            make_code = LOWORD(::MapVirtualKey(input->VKey , MAPVK_VK_TO_VSC_EX));
        }
        const KeyboardKey key_location = ConvertScanCodeToKeyboardKey(make_code);
        if (key_location == KeyboardKey::Invalid) { return; }
        const KeyboardModifier key_modifier_location = ConvertKeyboardKeyToKeyboardModifier(key_location);
        const bool is_toggle_modifier = IsToggleModifier(key_modifier_location);

        /* Set key if focus, always clear key if clear event */
        const bool is_no_down = is_unfocus && m_is_ignore_no_focus;
        if ((input->Flags & RI_KEY_BREAK) == RI_KEY_MAKE) {
            if (!is_no_down) {                        
                m_current_state.key_set.SetBit(static_cast<u32>(key_location));
            }
            if (key_modifier_location != KeyboardModifier::Invalid) {
                if (!is_toggle_modifier) {
                    if (!is_no_down) {
                        m_current_state.modifier_set.SetBit(static_cast<u32>(key_modifier_location));
                    }
                } else {
                    if (!is_no_down) {
                        m_current_state.modifier_set.XorBit(static_cast<u32>(key_modifier_location));
                    } else {
                        m_current_state.modifier_set.ClearBit(static_cast<u32>(key_modifier_location));
                    }
                }
            }
        } else if ((input->Flags & RI_KEY_BREAK) == RI_KEY_BREAK) {
            m_current_state.key_set.ClearBit(static_cast<u32>(key_location));
            if (key_modifier_location != KeyboardModifier::Invalid && !is_toggle_modifier) {
                m_current_state.modifier_set.ClearBit(static_cast<u32>(key_modifier_location));
            }
        }

        m_is_dirty = true;
        return;
    }

    void Keyboard::ReportState() {
        if (!m_is_dirty) { return; }
        m_is_dirty = false;
        m_ring.PushState(std::addressof(m_current_state));
    }

    AWN_SINGLETON_TRAITS_IMPL(KeyboardManager);

    long long int KeyboardManager::KeyboardWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
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
                /* Get raw keyboard data */
                //const bool is_sink = GET_RAWINPUT_CODE_WPARAM(wparam);
                HRAWINPUT hrawinput = reinterpret_cast<HRAWINPUT>(lparam);
                u32 data_size = 0;
                ::GetRawInputData(hrawinput, RID_INPUT, nullptr, std::addressof(data_size), sizeof(RAWINPUTHEADER));
                RAWINPUT *raw_input = reinterpret_cast<RAWINPUT*>(::malloc(data_size));
                VP_ASSERT(raw_input != nullptr);
                ON_SCOPE_EXIT { ::free(raw_input); };
                const u32 result = ::GetRawInputData(hrawinput, RID_INPUT, raw_input, std::addressof(data_size), sizeof(RAWINPUTHEADER));
                VP_ASSERT(result != 0xffff'ffff);

                /* Ignore non-keyboard input */
                if (raw_input->header.dwType != RIM_TYPEKEYBOARD) { break; }

                /* Process for combined */
                KeyboardManager *mgr = reinterpret_cast<KeyboardManager*>(::GetWindowLongPtrA(window, GWLP_USERDATA));
                const bool is_unfocus = GET_RAWINPUT_CODE_WPARAM(wparam);
                mgr->m_all_keyboard.ProcessInput(std::addressof(raw_input->data.keyboard), is_unfocus);

                /* Process for slot */
                Keyboard *keyboard = mgr->GetKeyboardByDevice(raw_input->header.hDevice);
                if (keyboard == nullptr) {
                    keyboard = mgr->AddKeyboard(raw_input->header.hDevice);
                    if (keyboard == nullptr) { break; }
                }
                keyboard->ProcessInput(std::addressof(raw_input->data.keyboard), is_unfocus);

                break;
            }
            case WM_INPUT_DEVICE_CHANGE:
            {
                /* Remove keyboard device */
                KeyboardManager *mgr = reinterpret_cast<KeyboardManager*>(::GetWindowLongPtrA(window, GWLP_USERDATA));
                HANDLE change_device = reinterpret_cast<HANDLE>(lparam);
                if (wparam == GIDC_REMOVAL) {
                    mgr->RemoveKeyboard(change_device);
                }
                break;
            }
            default:
                break;
        };
        return ::DefWindowProc(window, message, wparam, lparam);
    }

    void KeyboardManager::ThreadMain(size_t message) {
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
                const size_t mess = (message & 0xffff'ffff'0000'0000);
                const size_t arg = (message & 0xffff'ffff);
                if (mess == Message_Disconnect) {
                    m_used_keyboard_array[arg]->Disconnect();
                }
            }
        }

        return;
    }

    void KeyboardManager::InitializeRawInput() {

        /* Register keyboard window thread */
        const HINSTANCE process_handle = ::GetModuleHandle(nullptr);
        const WNDCLASS wc = {
            .style = 0,
            .lpfnWndProc = KeyboardWindowProc,
            .hInstance = process_handle,
            .hbrBackground = (HBRUSH)(COLOR_BACKGROUND),
            .lpszClassName = cClassName,
        };
        const u32 result0 = ::RegisterClass(std::addressof(wc));
        VP_ASSERT(result0 != 0);

        /* Create Window thread */
        HWND window_handle = ::CreateWindowEx(0 , cClassName, "Keyboard_RawInput", 0, 0, 0, 32, 32, HWND_MESSAGE, 0, process_handle, this);
        VP_ASSERT(window_handle != INVALID_HANDLE_VALUE);
        m_hwnd = window_handle;

        /* Register for keyboard input */
        const RAWINPUTDEVICE raw_input_dev_array[] = {
            {
                .usUsagePage = 1,
                .usUsage = 6,
                .dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY,
                .hwndTarget = window_handle,
            },
        };
        const bool result1 = ::RegisterRawInputDevices(raw_input_dev_array, sizeof(raw_input_dev_array) / sizeof(RAWINPUTDEVICE), sizeof(RAWINPUTDEVICE));
        VP_ASSERT(result1);

        return;
    }

    void KeyboardManager::FinalizeRawInput() {
        if (m_hwnd != nullptr) {
            ::DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        ::UnregisterClassA(cClassName, ::GetModuleHandle(nullptr));
        return;
    }

    void KeyboardManager::ReportStates() {
        m_all_keyboard.ReportState();
        for (Keyboard &keyboard : m_keyboard_array) {
            if (keyboard.m_device == nullptr) { continue; }
            keyboard.ReportState();
        }
        return;
    }

    Keyboard *KeyboardManager::AddKeyboard(HANDLE device) {

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
        if (data->dwType != RIM_TYPEKEYBOARD) { return nullptr; }
        //::printf("Keyboard:\n");
        //::printf("  dwType: %ld\n", data->keyboard.dwType);
        //::printf("  dwSubType: %ld\n", data->keyboard.dwSubType);
        //::printf("  dwKeyboardMode: %ld\n", data->keyboard.dwKeyboardMode);
        //::printf("  dwNumberOfFunctionKeys: %ld\n", data->keyboard.dwNumberOfFunctionKeys);
        //::printf("  dwNumberOfIndicators: %ld\n", data->keyboard.dwNumberOfIndicators);
        //::printf("  dwNumberOfKeysTotal: %ld\n", data->keyboard.dwNumberOfKeysTotal);

        /* Insert keyboard to first empty device slot */
        std::scoped_lock l(m_used_cs);
        for (Keyboard *&keyboard : m_used_keyboard_array) {
            if (keyboard->m_device != nullptr) { continue; }
            keyboard->RegisterDevice(device);
            return keyboard;
        }

        return nullptr;
    }

    void KeyboardManager::RemoveKeyboard(HANDLE device) {

        /* Remove keyboard by device */
        for (Keyboard &keyboard : m_keyboard_array) {
            if (keyboard.m_device != device) { continue; }
            keyboard.Disconnect();
        }

        return;
    }

    Keyboard *KeyboardManager::GetKeyboardByDevice(HANDLE device) {
        for (Keyboard &keyboard : m_keyboard_array) {
            if (keyboard.m_device != device) { continue; }
            return std::addressof(keyboard);
        }
        return nullptr;
    }

    Result KeyboardManager::Initialize(mem::Heap *heap, u32 max_keyboard_count) {

        /* Initialize arrays */
        if (0 < max_keyboard_count) {                    
            m_keyboard_array.Initialize(heap, max_keyboard_count);
            m_used_keyboard_array.Initialize(heap, max_keyboard_count);
            for (Keyboard &board : m_keyboard_array) {
                m_used_keyboard_array.PushPointer(std::addressof(board));
            }
        }

        /* Start keyboard thread */
        sys::ThreadInfo info = {
            .name = "awn::hid::KeyboardManager",
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

    void KeyboardManager::Finalize() {
        m_thread.SendMessage(Message_ExitCode);
        const bool is_send = ::PostThreadMessageA(m_thread.GetWin32ThreadId(), WM_QUIT, 0, 0);
        VP_ASSERT(is_send);
        m_thread.ExitThread(sys::ExitType::Send);
        m_thread.Finalize();
        m_used_keyboard_array.Finalize();
        m_keyboard_array.Finalize();
        return;
    }

    u32 KeyboardManager::GetKeyboardStates(KeyboardState *out_state_array, u32 state_count, u32 keyboard_id) {
        Keyboard *keyboard;
        if (keyboard_id == m_keyboard_array.GetCount()) {
            keyboard = std::addressof(m_all_keyboard);
        } else {
            keyboard = m_used_keyboard_array[keyboard_id];
        }
        u32 out_count = 0;
        keyboard->m_ring.ReadStates(std::addressof(out_count), out_state_array, state_count);
        return out_count;
    }

    void KeyboardManager::SwapKeyboardAssignment(u32 id0, u32 id1) {
        std::scoped_lock l(m_used_cs);
        m_used_keyboard_array.Swap(id0, id1);
        return;
    }

    void KeyboardManager::DisconnectKeyboard(u32 keyboard_id) {
        {
            std::scoped_lock l(m_used_cs);
            if (m_used_keyboard_array[keyboard_id]->m_device == nullptr) { return; }
        }
        m_thread.SendMessage(Message_Disconnect | keyboard_id);
        return;
    }

}

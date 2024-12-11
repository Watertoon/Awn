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

namespace awn::hid {

    class KeyboardThread : public sys::Thread {
        public:
            static constexpr u32 cMaxReportQueueCount = 0x10;
        public:
            class KeyboardDevice {
                public:
                    using KeyboardStateRing = vp::util::FixedRingBuffer<KeyboardState*, cMaxReportQueueCount>;
                public:
                    HANDLE            m_device_handle;
                    bool              m_is_ignore_input_on_unfocus;
                    s64               m_sampling_iter;
                    KeyboardStateRing m_keyboard_ring;
                    KeyboardState     m_keyboard_state_array[cMaxReportQueueCount];
                public:
                    constexpr KeyboardDevice() : m_device_handle(INVALID_HANDLE_VALUE), m_is_ignore_input_on_unfocus(), m_sampling_iter(), m_keyboard_ring(), m_keyboard_state_array() {
                        for (u32 i = 0; i < cMaxReportQueueCount; ++i) {
                            m_keyboard_ring.PushFront(std::addressof(m_keyboard_state_array[i]));
                        }
                    }
                    constexpr ~KeyboardDevice() {/*...*/}

                    void GetKeyboardStates(KeyboardState *out_keyboard_states, u32 keyboard_state_count) {
                        KeyboardStateRing::iterator iter = m_keyboard_ring.begin();
                        for (u32 i = 0; i < keyboard_state_count && iter != m_keyboard_ring.end(); ++i) {
                            KeyboardState *state = *iter;
                            out_keyboard_states[i] = *state;
                            ++iter;
                        }
                        return;
                    }

                    void SetRawKeyboard(RAWKEYBOARD *raw_keyboard, bool is_sink) {

                        /* Discard input if we only want input while the process is in focus */
                        if (m_is_ignore_input_on_unfocus && is_sink) { return; }

                        /* Remove oldest sampling, advance from previous sampling */
                        KeyboardState *last_keyboard_state = m_keyboard_ring.PeekBack();
                        KeyboardState *keyboard_state = m_keyboard_ring.RemoveFront();
                        const s64 next_iter = m_sampling_iter + 1;
                        keyboard_state->sampling_count = next_iter;
                        keyboard_state->modifier_set = last_keyboard_state->modifier_set;
                        keyboard_state->key_set = last_keyboard_state->key_set;
                        m_sampling_iter = next_iter;
                        m_keyboard_ring.PushBack(keyboard_state);

                        const u32 make_code0 = raw_keyboard->MakeCode & 0x7f;
                        const u32 make_code1 = (raw_keyboard->Flags & RI_KEY_E0) ? 0xe0 : ((raw_keyboard->Flags & RI_KEY_E1) ? 0xe1 : 0x00);
                        const u32 make_code = make_code0 | (make_code1 << 0x8);
                        const KeyboardKey key_location = ConvertScanCodeToKeyboardKey(make_code);
                        if (key_location == KeyboardKey::Invalid) { return; }

                        const KeyboardModifier key_modifier_location = ConvertKeyboardKeyToKeyboardModifier(key_location);
                        const bool is_modifier_toggle = IsToggleModifier(key_modifier_location);
                        if ((raw_keyboard->Flags & RI_KEY_BREAK) == RI_KEY_MAKE) {
                            keyboard_state->key_set.SetBit(static_cast<u32>(key_location));
                            if (key_modifier_location != KeyboardModifier::Invalid) {
                                if (!is_modifier_toggle) {
                                    keyboard_state->modifier_set.SetBit(static_cast<u32>(key_modifier_location));
                                } else {
                                    keyboard_state->modifier_set.XorBit(static_cast<u32>(key_modifier_location));
                                }
                            }
                        } else if ((raw_keyboard->Flags & RI_KEY_BREAK) == RI_KEY_BREAK) {
                            keyboard_state->key_set.ClearBit(static_cast<u32>(key_location));
                            if (key_modifier_location != KeyboardModifier::Invalid && !is_modifier_toggle) {
                                keyboard_state->modifier_set.ClearBit(static_cast<u32>(key_modifier_location));
                            }
                        }

                        return;
                    }

                    void SetDisconnected() {
                        KeyboardState *keyboard_state = m_keyboard_ring.RemoveFront();
                        const s64 next_iter = m_sampling_iter + 1;
                        keyboard_state->sampling_count = next_iter;
                        m_sampling_iter = next_iter;
                        keyboard_state->key_set.Clear();
                        keyboard_state->modifier_set.Clear();
                        m_keyboard_ring.PushBack(keyboard_state);
                    }

                    constexpr KeyboardState *GetCurrentKeyboardState() { return m_keyboard_ring.PeekBack(); }
                    constexpr bool IsDevice(HANDLE device_handle) const { return device_handle == m_device_handle; }
            };
        public:
            using KeyboardDeviceArray = vp::util::HeapArray<KeyboardDevice>;
        private:
            HWND                m_window_handle;
            KeyboardDeviceArray m_keyboard_device_array;
            sys::Mutex          m_device_cs;
            bool                m_is_exceed_keyboard_limit_warning;
        private:
            static long long int KeyboardWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
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
                        const bool is_sink = GET_RAWINPUT_CODE_WPARAM(wparam);
                        HRAWINPUT hrawinput = reinterpret_cast<HRAWINPUT>(lparam);
                        u32 data_size = 0;
                        ::GetRawInputData(hrawinput, RID_INPUT, nullptr, std::addressof(data_size), sizeof(RAWINPUTHEADER));
                        RAWINPUT *raw_input = reinterpret_cast<RAWINPUT*>(::malloc(data_size));
                        VP_ASSERT(raw_input != nullptr);
                        ON_SCOPE_EXIT { ::free(raw_input); };
                        const u32 result = ::GetRawInputData(hrawinput, RID_INPUT, raw_input, std::addressof(data_size), sizeof(RAWINPUTHEADER));
                        VP_ASSERT(result != 0xffff'ffff);

                        /* Nothing to do if it's not keyboard input */
                        if (raw_input->header.dwType != RIM_TYPEKEYBOARD) { break; }

                        /* Find corresponding mouse device */
                        KeyboardThread *keyboard_thread = reinterpret_cast<KeyboardThread*>(::GetWindowLongPtrA(window, GWLP_USERDATA));
                        std::scoped_lock l(keyboard_thread->m_device_cs);
                        KeyboardDevice *keyboard_device = nullptr;
                        for (KeyboardDevice &device_iter : keyboard_thread->m_keyboard_device_array) {
                            if (device_iter.m_device_handle != raw_input->header.hDevice) { continue; }
                            keyboard_device = std::addressof(device_iter);
                            break;
                        }
                        if (keyboard_device == nullptr) {
                            keyboard_device = keyboard_thread->TryRegisterKeyboard(raw_input->header.hDevice);
                            if (keyboard_device == nullptr) { keyboard_thread->m_is_exceed_keyboard_limit_warning = true; break; }
                        }
                        keyboard_device->SetRawKeyboard(std::addressof(raw_input->data.keyboard), is_sink);

                        break;
                    }
                    case WM_INPUT_DEVICE_CHANGE:
                    {
                        /* Check whether we added of removed a mouse */
                        KeyboardThread *keyboard_thread = reinterpret_cast<KeyboardThread*>(::GetWindowLongPtrA(window, GWLP_USERDATA));
                        std::scoped_lock l(keyboard_thread->m_device_cs);
                        HANDLE change_device = reinterpret_cast<HANDLE>(lparam);
                        if (wparam == GIDC_ARRIVAL) {
                            keyboard_thread->TryRegisterKeyboard(change_device);
                        } else if (wparam == GIDC_REMOVAL) {
                            for (KeyboardDevice &keyboard_device : keyboard_thread->m_keyboard_device_array) {
                                if (keyboard_device.m_device_handle != change_device) { continue; }
                                keyboard_device.m_device_handle = INVALID_HANDLE_VALUE;
                                keyboard_device.SetDisconnected();
                                break;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                };
                return ::DefWindowProc(window, message, wparam, lparam);
            }
        private:
            void InitializeWindow() {

                /* Register mouse window thread */
                const HINSTANCE process_handle = ::GetModuleHandle(nullptr);
                const WNDCLASS wc = {
                    .style = 0,
                    .lpfnWndProc = KeyboardWindowProc,
                    .hInstance = process_handle,
                    .hbrBackground = (HBRUSH)(COLOR_BACKGROUND),
                    .lpszClassName = "awn::hid::KeyboardWindow",
                };
                const u32 result0 = ::RegisterClass(std::addressof(wc));
                VP_ASSERT(result0 != 0);

                /* Create Window thread */
                HWND window_handle = ::CreateWindowEx(0 ,"awn::hid::KeyboardWindow", "KeyboardWindow", 0, 0, 0, 32, 32, HWND_MESSAGE, 0, process_handle, this);
                VP_ASSERT(window_handle != INVALID_HANDLE_VALUE);

                /* Register for mouse input */
                const RAWINPUTDEVICE raw_input_dev_array[] = {
                    {
                        .usUsagePage = 1,
                        .usUsage = 6,
                        .dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY | RIDEV_NOLEGACY,
                        .hwndTarget = window_handle,
                    },
                };
                const bool result1 = ::RegisterRawInputDevices(raw_input_dev_array, sizeof(raw_input_dev_array) / sizeof(RAWINPUTDEVICE), sizeof(RAWINPUTDEVICE));
                VP_ASSERT(result1 == true);

                return;
            }

            void FinalizeWindow() {
                if (m_window_handle != nullptr) {
                    ::DestroyWindow(m_window_handle);
                    m_window_handle = nullptr;
                }
                ::UnregisterClassA("awn::hid::KeyboardWindow", ::GetModuleHandle(nullptr));
            }

            KeyboardDevice *TryRegisterKeyboard(HANDLE device_handle) {
                for (KeyboardDevice &device_iter : m_keyboard_device_array) {
                    if (device_iter.m_device_handle != INVALID_HANDLE_VALUE) { continue; }
                    device_iter.m_device_handle = device_handle;
                    device_iter.m_sampling_iter = 0;
                    for (KeyboardState &keyboard_state : device_iter.m_keyboard_state_array) {
                        keyboard_state.sampling_count = 0;
                        keyboard_state.modifier_set.Clear();
                        keyboard_state.key_set.Clear();
                    }
                    return std::addressof(device_iter);
                }
                return nullptr;
            }
        public:
            KeyboardThread(mem::Heap *heap, u32 max_mice) : Thread("awn::hid::KeyboardThread", heap, sys::ThreadRunMode::Looping, 0, 1, 0x1000, sys::cPriorityNormal + 5), m_window_handle(), m_keyboard_device_array(), m_device_cs(), m_is_exceed_keyboard_limit_warning() {
                m_keyboard_device_array.Initialize(heap, max_mice);
            }
            virtual ~KeyboardThread() override {
                ::PostThreadMessage(this->GetWin32ThreadId(), WM_QUIT, 0, 0);
                {
                    std::scoped_lock l(m_device_cs);
                    m_keyboard_device_array.Finalize();
                }
            }

            virtual void ThreadMain(size_t message) override {
                VP_UNUSED(message);

                this->InitializeWindow();

                /* Win32 window message pump */
                MSG msg = {};
                while (::GetMessage(std::addressof(msg), nullptr, 0, 0) != 0) {
                    ::TranslateMessage(std::addressof(msg));
                    ::DispatchMessage(std::addressof(msg));
                }

                this->FinalizeWindow();

                return;
            }

            void GetKeyboardStates(KeyboardState *out_keyboard_states, u32 state_count, u32 device_index) {

                /* Thread lock */
                std::scoped_lock l(m_device_cs);

                /* Read last inputs */
                m_keyboard_device_array[device_index].GetKeyboardStates(out_keyboard_states, state_count);

                return;
            }
    };

	u32 sKeyboardRefCount = 0;
	vp::util::TypedStorage<KeyboardThread> sKeyboardThread;

	void InitializeKeyboard(mem::Heap *heap, u32 max_keyboard_count) {

        /* Increment ref count */
        const u32 inc = vp::util::InterlockedIncrement(std::addressof(sKeyboardRefCount));
        if (inc != 1) { return; }

        /* Create mouse processing thread */
        vp::util::ConstructAt(sKeyboardThread, heap, max_keyboard_count);
        vp::util::GetReference(sKeyboardThread).StartThread();

        return;
    }

    void FinalizeKeyboard() {

        /* Decrement ref count */
        const u32 inc = vp::util::InterlockedDecrement(std::addressof(sKeyboardRefCount));
        if (inc != 0) { return; }

        /* Destroy mouse processing thread */
        vp::util::DestructAt(sKeyboardThread);

        return;
    }

    void GetKeyboardStates(KeyboardState *out_keyboard_states, u32 state_count, u32 device_index) {
        VP_ASSERT(sKeyboardRefCount != 0);
        vp::util::GetReference(sKeyboardThread).GetKeyboardStates(out_keyboard_states, state_count, device_index);
    }
}

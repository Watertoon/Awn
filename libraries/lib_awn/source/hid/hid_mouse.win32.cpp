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

    class MouseThread : public sys::Thread {
        public:
            static constexpr u32 cMaxReportQueueCount = 0x10;
        public:
            class MouseDevice {
                public:
                    using MouseStateRing = vp::util::FixedRingBuffer<MouseState*, cMaxReportQueueCount>;
                public:
                    HANDLE         m_device_handle;
                    bool           m_is_ignore_input_on_unfocus;
                    s64            m_sampling_iter;
                    MouseStateRing m_mouse_ring;
                    MouseState     m_mouse_state_array[cMaxReportQueueCount];
                public:
                    constexpr MouseDevice() : m_device_handle(INVALID_HANDLE_VALUE), m_is_ignore_input_on_unfocus(), m_sampling_iter(), m_mouse_ring(), m_mouse_state_array() {
                        for (u32 i = 0; i < cMaxReportQueueCount; ++i) {
                            m_mouse_ring.PushFront(std::addressof(m_mouse_state_array[i]));
                        }
                    }
                    constexpr ~MouseDevice() {/*...*/}

                    void GetMouseStates(MouseState *out_mouse_states, u32 mouse_state_count) {
                        MouseStateRing::iterator iter = m_mouse_ring.begin();
                        for (u32 i = 0; i < mouse_state_count && iter != m_mouse_ring.end(); ++i) {
                            MouseState *state = *iter;
                            out_mouse_states[i] = *state;
                            ++iter;
                        }
                        return;
                    }

                    void SetRawMouse(RAWMOUSE *raw_mouse, bool is_sink) {

                        /* Discard input if we only want input while the process is in focus */
                        if (m_is_ignore_input_on_unfocus && is_sink) { return; }

                        /* Advance sampling */
                        MouseState *last_mouse_state = m_mouse_ring.PeekBack();
                        MouseState *mouse_state = m_mouse_ring.RemoveFront();
                        const s64 next_iter = m_sampling_iter + 1;
                        mouse_state->sampling_count = next_iter;
                        mouse_state->button_set = last_mouse_state->button_set;
                        m_sampling_iter = next_iter;
                        m_mouse_ring.PushBack(mouse_state);

                        if ((raw_mouse->usFlags & 1) == MOUSE_MOVE_RELATIVE) {

                            /* Windows only natively support 1 mouse, so use a win32 fallback for cursor pos */
                            const u32 msg_pos = static_cast<u32>(::GetMessagePos());
                            const vp::util::Vector2i pos = { static_cast<s16>(msg_pos & 0xffff), static_cast<s16>((msg_pos >> 0x10) & 0xffff), };
                            //const vp::util::Rectanglei virt_rect = sys::GetVirtualScreenRectangle();
                            //const vp::util::Float2 norm_pos = virt_rect.NormalizePointFloat(std::addressof(pos));
                            //mouse_state->x = static_cast<s32>(static_cast<float>(vp::util::cMaxU16) * norm_pos.x);
                            //mouse_state->y = static_cast<s32>(static_cast<float>(vp::util::cMaxU16) * norm_pos.y);
                            mouse_state->position.x = pos[0];
                            mouse_state->position.y = pos[1];

                            /* Set delta direct */
                            mouse_state->delta.x = raw_mouse->lLastX;
                            mouse_state->delta.y = raw_mouse->lLastY;

                        } else if ((raw_mouse->usFlags & 1) == MOUSE_MOVE_ABSOLUTE) {

                            /* Set abs position */
                            mouse_state->position.x = raw_mouse->lLastX;
                            mouse_state->position.y = raw_mouse->lLastY;

                            /* Calculate delta */
                            mouse_state->delta.x = (raw_mouse->lLastX - last_mouse_state->delta.x);
                            mouse_state->delta.y = (raw_mouse->lLastY - last_mouse_state->delta.y);
                        }

                        /* Unpack button mask */
                        const bool is_left_down    = raw_mouse->usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN;
                        const bool is_left_up      = raw_mouse->usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP;
                        const bool is_right_down   = raw_mouse->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN;
                        const bool is_right_up     = raw_mouse->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP;
                        const bool is_middle_down  = raw_mouse->usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN;
                        const bool is_middle_up    = raw_mouse->usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP;
                        const bool is_button4_down = raw_mouse->usButtonFlags & RI_MOUSE_BUTTON_4_DOWN;
                        const bool is_button4_up   = raw_mouse->usButtonFlags & RI_MOUSE_BUTTON_4_UP;
                        const bool is_button5_down = raw_mouse->usButtonFlags & RI_MOUSE_BUTTON_5_DOWN;
                        const bool is_button5_up   = raw_mouse->usButtonFlags & RI_MOUSE_BUTTON_5_UP;
                        const bool is_wheel_v_down = raw_mouse->usButtonFlags & RI_MOUSE_WHEEL;
                        const bool is_wheel_h_down = raw_mouse->usButtonFlags & RI_MOUSE_HWHEEL;
                        if (is_left_up) {
                            mouse_state->button_set.ClearBit(static_cast<u32>(MouseButton::Left));
                        }
                        if (is_left_down) {
                            mouse_state->button_set.SetBit(static_cast<u32>(MouseButton::Left));
                        }
                        if (is_right_up) {
                            mouse_state->button_set.ClearBit(static_cast<u32>(MouseButton::Right));
                        }
                        if (is_right_down) {
                            mouse_state->button_set.SetBit(static_cast<u32>(MouseButton::Right));
                        }
                        if (is_middle_up) {
                            mouse_state->button_set.ClearBit(static_cast<u32>(MouseButton::Middle));
                        }
                        if (is_middle_down) {
                            mouse_state->button_set.SetBit(static_cast<u32>(MouseButton::Middle));
                        }
                        if (is_button4_up) {
                            mouse_state->button_set.ClearBit(static_cast<u32>(MouseButton::Forward));
                        }
                        if (is_button4_down) {
                            mouse_state->button_set.SetBit(static_cast<u32>(MouseButton::Forward));
                        }
                        if (is_button5_up) {
                            mouse_state->button_set.ClearBit(static_cast<u32>(MouseButton::Backward));
                        }
                        if (is_button5_down) {
                            mouse_state->button_set.SetBit(static_cast<u32>(MouseButton::Backward));
                        }
                        mouse_state->delta_wheel.y = (is_wheel_v_down) ? static_cast<s32>(static_cast<s16>(raw_mouse->usButtonData)) : 0;
                        mouse_state->delta_wheel.x = (is_wheel_h_down) ? static_cast<s32>(static_cast<s16>(raw_mouse->usButtonData)) : 0;

                        mouse_state->attribute_set.SetBit(0);

                        return;
                    }

                    void SetDisconnected() {
                        MouseState *mouse_state = m_mouse_ring.RemoveFront();
                        const s64 next_iter = m_sampling_iter + 1;
                        mouse_state->sampling_count = next_iter;
                        mouse_state->attribute_set.ClearBit(0);
                        m_sampling_iter = next_iter;
                        m_mouse_ring.PushBack(mouse_state);
                    }

                    constexpr MouseState *GetCurrentMouseState() { return m_mouse_ring.PeekBack(); }
                    constexpr bool IsDevice(HANDLE device_handle) const { return device_handle == m_device_handle; }
            };
        public:
            using MouseDeviceArray = vp::util::HeapArray<MouseDevice>;
        private:
            HWND             m_window_handle;
            MouseDeviceArray m_mouse_device_array;
            sys::Mutex       m_device_cs;
            bool             m_is_exceed_mouse_limit_warning;
        private:
            static long long int MouseWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
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
                        const bool is_sink = GET_RAWINPUT_CODE_WPARAM(wparam);
                        HRAWINPUT hrawinput = reinterpret_cast<HRAWINPUT>(lparam);
                        u32 data_size = 0;
                        ::GetRawInputData(hrawinput, RID_INPUT, nullptr, std::addressof(data_size), sizeof(RAWINPUTHEADER));
                        RAWINPUT *raw_input = reinterpret_cast<RAWINPUT*>(::malloc(data_size));
                        VP_ASSERT(raw_input != nullptr);
                        ON_SCOPE_EXIT { ::free(raw_input); };
                        const u32 result = ::GetRawInputData(hrawinput, RID_INPUT, raw_input, std::addressof(data_size), sizeof(RAWINPUTHEADER));
                        VP_ASSERT(result != 0xffff'ffff);

                        /* Nothing to do if it's not mouse input */
                        if (raw_input->header.dwType != RIM_TYPEMOUSE) { break; }

                        /* Find corresponding mouse device */
                        MouseThread *mouse_thread = reinterpret_cast<MouseThread*>(::GetWindowLongPtrA(window, GWLP_USERDATA));
                        std::scoped_lock l(mouse_thread->m_device_cs);
                        MouseDevice *mouse_device = nullptr;
                        for (MouseDevice &device_iter : mouse_thread->m_mouse_device_array) {
                            if (device_iter.m_device_handle != raw_input->header.hDevice) { continue; }
                            mouse_device = std::addressof(device_iter);
                            break;
                        }
                        if (mouse_device == nullptr) {
                            mouse_device = mouse_thread->TryRegisterMouse(raw_input->header.hDevice); 
                            if (mouse_device == nullptr) { mouse_thread->m_is_exceed_mouse_limit_warning = true; break; }   
                        }
                        mouse_device->SetRawMouse(std::addressof(raw_input->data.mouse), is_sink);

                        break;
                    }
                    case WM_INPUT_DEVICE_CHANGE:
                    {
                        /* Check whether we added of removed a mouse */
                        MouseThread *mouse_thread = reinterpret_cast<MouseThread*>(::GetWindowLongPtrA(window, GWLP_USERDATA));
                        std::scoped_lock l(mouse_thread->m_device_cs);
                        HANDLE change_device = reinterpret_cast<HANDLE>(lparam);
                        if (wparam == GIDC_ARRIVAL) {
                            mouse_thread->TryRegisterMouse(change_device);
                        } else if (wparam == GIDC_REMOVAL) {
                            for (MouseDevice &mouse_device : mouse_thread->m_mouse_device_array) {
                                if (mouse_device.m_device_handle != change_device) { continue; }
                                mouse_device.m_device_handle = INVALID_HANDLE_VALUE;
                                mouse_device.SetDisconnected();
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
                    .lpfnWndProc = MouseWindowProc,
                    .hInstance = process_handle,
                    .hbrBackground = (HBRUSH)(COLOR_BACKGROUND),
                    .lpszClassName = "awn::hid::MouseWindow",
                };
                const u32 result0 = ::RegisterClass(std::addressof(wc));
                VP_ASSERT(result0 != 0);

                /* Create Window thread */
                HWND window_handle = ::CreateWindowEx(0 ,"awn::hid::MouseWindow", "MouseWindow", 0, 0, 0, 32, 32, HWND_MESSAGE, 0, process_handle, this);
                VP_ASSERT(window_handle != INVALID_HANDLE_VALUE);

                /* Register for mouse input */
                const RAWINPUTDEVICE raw_input_dev_array[] = {
                    {
                        .usUsagePage = 1,
                        .usUsage = 2,
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
                ::UnregisterClassA("awn::hid::MouseWindow", ::GetModuleHandle(nullptr));
            }

            MouseDevice *TryRegisterMouse(HANDLE device_handle) {
                for (MouseDevice &device_iter : m_mouse_device_array) {
                    if (device_iter.m_device_handle != INVALID_HANDLE_VALUE) { continue; }
                    device_iter.m_device_handle = device_handle;
                    device_iter.m_sampling_iter = 0;
                    for (MouseState &mouse_state : device_iter.m_mouse_state_array) {
                        mouse_state.sampling_count = 0;
                        mouse_state.position.x = 0;
                        mouse_state.position.y = 0;
                        mouse_state.delta.x = 0;
                        mouse_state.delta.y = 0;
                        mouse_state.delta_wheel.x = 0;
                        mouse_state.delta_wheel.y = 0;
                        mouse_state.button_set.Clear();
                        mouse_state.attribute_set.Clear();
                    }
                    return std::addressof(device_iter);
                }
                return nullptr;
            }
        public:
            MouseThread(mem::Heap *heap, u32 max_mice) : Thread("awn::hid::MouseThread", heap, sys::ThreadRunMode::Looping, 0, 1, 0x1000, sys::cPriorityNormal + 5), m_window_handle(), m_mouse_device_array(), m_device_cs(), m_is_exceed_mouse_limit_warning() {
                m_mouse_device_array.Initialize(heap, max_mice);
            }
            virtual ~MouseThread() override {
                ::PostThreadMessage(this->GetWin32ThreadId(), WM_QUIT, 0, 0);
                {
                    std::scoped_lock l(m_device_cs);
                    m_mouse_device_array.Finalize();
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

            void GetMouseStates(MouseState *out_mouse_state, u32 state_count, u32 device_index) {

                /* Thread lock */
                std::scoped_lock l(m_device_cs);

                /* Read last inputs */
                m_mouse_device_array[device_index].GetMouseStates(out_mouse_state, state_count);

                return;
            }
    };

	u32 sMouseRefCount = 0;
	vp::util::TypedStorage<MouseThread> sMouseThread;

	void InitializeMouse(mem::Heap *heap, u32 max_mice) {

        /* Increment ref count */
        const u32 inc = vp::util::InterlockedIncrement(std::addressof(sMouseRefCount));
        if (inc != 1) { return; }

        /* Create mouse processing thread */
        vp::util::ConstructAt(sMouseThread, heap, max_mice);
        vp::util::GetReference(sMouseThread).StartThread();

        return;
    }

    void FinalizeMouse() {

        /* Decrement ref count */
        const u32 inc = vp::util::InterlockedDecrement(std::addressof(sMouseRefCount));
        if (inc != 0) { return; }

        /* Destroy mouse processing thread */
        vp::util::DestructAt(sMouseThread);

        return;
    }

    void GetMouseStates(MouseState *out_mouse_states, u32 state_count, u32 device_index) {
        VP_ASSERT(sMouseRefCount != 0);
        vp::util::GetReference(sMouseThread).GetMouseStates(out_mouse_states, state_count, device_index);
    }
}

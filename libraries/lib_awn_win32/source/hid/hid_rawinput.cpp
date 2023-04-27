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

    namespace {

        union MouseData {
            struct {
                RAWINPUT raw_input;
            };
            char _buf[0x100];
        };

        union KeyboardData {
            struct {
                RAWINPUT raw_input;
            };
            char _buf[0x100];
        };
        MouseData                   raw_mouse = {};
        MouseData                   last_raw_mouse = {};
        MouseState                  interim_mouse_state = {};
        MouseState                  frame_mouse_state = {};

        sys::ServiceCriticalSection mouse_state_cs = {};
        sys::ServiceCriticalSection keyboard_state_cs = {};

        KeyboardData                raw_keyboard = {};
        KeyboardState               frame_keyboard_state;
        KeyState                    pressed_state;
        KeyState                    pressed_state_last;
        KeyState                    released_state;
        KeyState                    released_state_last;
        KeyState                    aheld_state;

        void SetMouseState() {
            const RAWMOUSE *last_mouse = std::addressof(last_raw_mouse.raw_input.data.mouse);
            const RAWMOUSE *mouse = std::addressof(raw_mouse.raw_input.data.mouse);

            /* Get monitor dimensions */
            s32 width = 0, height = 0;
            if ((last_mouse->usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP) {
                width = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
                height = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
            } else {
                width = ::GetSystemMetrics(SM_CXSCREEN);
                height = ::GetSystemMetrics(SM_CYSCREEN);
            }

            /* Set Mouse State */
            {
                std::scoped_lock l(mouse_state_cs);

                if ((last_mouse->usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE) {
                    interim_mouse_state.absolute_x = static_cast<s32>(((mouse->lLastX / 65535.0f) * width));
                    interim_mouse_state.absolute_y = static_cast<s32>(((mouse->lLastY / 65535.0f) * height));

                    /* This will be the same as absolute for the first frame */
                    interim_mouse_state.delta_x += mouse->lLastX - last_mouse->lLastX;
                    interim_mouse_state.delta_y += mouse->lLastY - last_mouse->lLastY;
                } else if ((last_mouse->usFlags & MOUSE_MOVE_RELATIVE) == MOUSE_MOVE_RELATIVE) {
                    interim_mouse_state.delta_x += mouse->lLastX;
                    interim_mouse_state.delta_y += mouse->lLastY;
                }

                if ((mouse->usButtonFlags & RI_MOUSE_WHEEL) == RI_MOUSE_WHEEL) {
                    interim_mouse_state.delta_vwheel += mouse->usButtonData;
                }

                if ((mouse->usButtonFlags & RI_MOUSE_HWHEEL) == RI_MOUSE_HWHEEL) {
                    interim_mouse_state.delta_hwheel += mouse->usButtonData;
                }

                interim_mouse_state.button_held |= mouse->usButtonFlags;
            }

            ::memcpy(std::addressof(last_raw_mouse), std::addressof(raw_mouse), sizeof(MouseData));
        }
        
        void SetKeyboardState() {
            std::scoped_lock l(keyboard_state_cs);

            /* Process key */
            if (raw_keyboard.raw_input.data.keyboard.Flags == RI_KEY_MAKE) {
                /* Set pressed key */
                pressed_state.SetKeyState(raw_keyboard.raw_input.data.keyboard.VKey);
            } else if (raw_keyboard.raw_input.data.keyboard.Flags == RI_KEY_BREAK) {
                /* Set released key */
                released_state.SetKeyState(raw_keyboard.raw_input.data.keyboard.VKey);
                pressed_state.ClearKeyState(raw_keyboard.raw_input.data.keyboard.VKey);
                pressed_state.ClearKeyState(raw_keyboard.raw_input.data.keyboard.VKey);
            }
        }
    }

    void BeginFrame() {

        {
            std::scoped_lock l(mouse_state_cs);

            /* Determine held state */
            const u16 held_state = (frame_mouse_state.button_held | (interim_mouse_state.button_state & MouseState::ButtonDownMask)) & ~((interim_mouse_state.button_state >> 1) & MouseState::ButtonDownMask);

            /* Copy last mouse state */
            ::memcpy(std::addressof(frame_mouse_state), std::addressof(interim_mouse_state), sizeof(MouseState));
            ::memset(std::addressof(interim_mouse_state), 0, sizeof(MouseState));

            /* Get absolute cursor pos */
            POINT absolute_pos = {};
            const bool result0 = ::GetCursorPos(std::addressof(absolute_pos));
            VP_ASSERT(result0 == true);

            frame_mouse_state.absolute_x = absolute_pos.x;
            frame_mouse_state.absolute_y = absolute_pos.y;

            /* Set held state */
            frame_mouse_state.button_held = held_state;
        }

        {
            std::scoped_lock l(keyboard_state_cs);

            /* Copy keyboard state */
            aheld_state = aheld_state | (pressed_state & (~released_state));
            frame_keyboard_state.released_keys = released_state;
            frame_keyboard_state.held_keys     = pressed_state | (pressed_state_last & (~released_state_last));
            frame_keyboard_state.pressed_keys  = pressed_state & (~pressed_state_last);

            ::memcpy(std::addressof(pressed_state_last), std::addressof(pressed_state), sizeof(KeyState));
            ::memcpy(std::addressof(released_state_last), std::addressof(released_state), sizeof(KeyState));
            ::memset(std::addressof(released_state), 0, sizeof(KeyState));
        }
    }

    void SetLastRawInput(HRAWINPUT input_handle) {

        /* Get raw input header */
        RAWINPUTHEADER raw_header = {};
        u32 struct_size = sizeof(RAWINPUTHEADER);
        u32 result = ::GetRawInputData(input_handle, RID_HEADER, std::addressof(raw_header), std::addressof(struct_size), sizeof(RAWINPUTHEADER));
        VP_ASSERT(result == sizeof(RAWINPUTHEADER));

        /* Get raw input data */
        if (raw_header.dwType == RIM_TYPEMOUSE) {
            struct_size = sizeof(MouseData);
            result = ::GetRawInputData(input_handle, RID_INPUT, std::addressof(raw_mouse.raw_input), std::addressof(struct_size), sizeof(RAWINPUTHEADER));
            VP_ASSERT(sizeof(RAWMOUSE) <= result && result <= sizeof(MouseData));

            SetMouseState();
        } else if (raw_header.dwType == RIM_TYPEKEYBOARD) {
            struct_size = sizeof(KeyboardData);
            result = ::GetRawInputData(input_handle, RID_INPUT, std::addressof(raw_keyboard.raw_input), std::addressof(struct_size), sizeof(RAWINPUTHEADER));
            VP_ASSERT(sizeof(RAWKEYBOARD) <= result && result <= sizeof(KeyboardData));

            SetKeyboardState();
        }
    }

    const KeyboardState GetKeyboardState() {
        std::scoped_lock l(keyboard_state_cs);

        return frame_keyboard_state;
    }

    const MouseState GetMouseState() {
        std::scoped_lock l(mouse_state_cs);

        return frame_mouse_state;
    }
}

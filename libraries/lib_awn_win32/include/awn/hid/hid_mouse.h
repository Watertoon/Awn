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

namespace awn::hid {

    struct MouseState {
        u64 sampling_count;
        s32 x;
        s32 y;
        s32 delta_x;
        s32 delta_y;
        s32 delta_wheel_v;
        s32 delta_wheel_h;
        union {
            u32 button_state;
            struct {
                u32 left     : 1;
                u32 right    : 1;
                u32 middle   : 1;
                u32 forward  : 1;
                u32 back     : 1;
                u32 reserve0 : 27;
            };
        };
        u32 is_connected;
    };

	void InitializeMouse(mem::Heap *heap, u32 max_mice);
    void FinalizeMouse();

    void GetMouseStates(MouseState *out_mouse_states, u32 state_count, u32 device_index);
}

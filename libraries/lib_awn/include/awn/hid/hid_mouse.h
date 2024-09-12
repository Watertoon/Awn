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

    enum class MouseButton : u32 {
        Left = 0,
        Right = 1,
        Middle = 2,
        Forward = 3,
        Backward = 4,
    };
    using MouseButtonSet = vp::util::FixedConnection<u32, 0x20>;
    using MouseAttributeSet = vp::util::FixedConnection<u32, 0x20>;

    struct MouseState {
        s64 sampling_count;
        vp::util::Int2 position;
        vp::util::Int2 delta;
        vp::util::Int2 delta_wheel;
        MouseButtonSet button_set;
        MouseAttributeSet attribute_set;

        constexpr bool IsButtonDown(MouseButton mouse_button) {
            return this->button_set.GetBit(static_cast<u32>(mouse_button));
        }
    };

	void InitializeMouse(mem::Heap *heap, u32 max_mice);
    void FinalizeMouse();
    void GetMouseStates(MouseState *out_mouse_states, u32 state_count, u32 device_index);
}

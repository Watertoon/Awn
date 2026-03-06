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

    static constexpr u32 cMaxMouseStateCount = 0x10;
    static constexpr Int2 cInvalidMousePosition = { 0x7fff'ffff, 0x7fff'ffff, };

    struct MouseState {
        s64 sampling_count;
        Int2 position;
        Int2 delta;
        Int2 delta_wheel;
        MouseButtonSet button_set;
        MouseAttributeSet attribute_set;

        constexpr void SetDefaults() {
            this->sampling_count = 0;
            this->position = cInvalidMousePosition;
            this->delta = VectorConstant<Int2>::cZero;
            this->delta_wheel = VectorConstant<Int2>::cZero;
            this->button_set.Clear();
            this->attribute_set.Clear();
        }
        constexpr bool IsButton(MouseButton mouse_button) {
            return this->button_set.GetBit(static_cast<u32>(mouse_button));
        }
    };

	void InitializeMouse(mem::Heap *heap, u32 max_mouse_count);
    void FinalizeMouse();
    u32 GetMouseStates(MouseState *out_mouse_states, u32 state_count, u32 mouse_id);

    void SwapMouseAssignment(u32 mouse_id0, u32 mouse_id1);
    void DisconnectMouse(u32 mouse_id);
}

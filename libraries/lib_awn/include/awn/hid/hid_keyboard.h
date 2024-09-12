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

    using KeyboardModifierSet = vp::util::FixedConnection<u32, 0x20>;
    using KeyboardAttributeSet = vp::util::FixedConnection<u32, 0x20>;
    using KeyboardKeySet = vp::util::FixedConnection<u64, 0x100>;

	struct KeyboardState {
		s64 sampling_count;
        KeyboardModifierSet modifier_set;
        KeyboardAttributeSet attribute_set;
		KeyboardKeySet key_set;

        constexpr bool IsKeyDown(KeyboardKey keyboard_key) const {
            return this->key_set.GetBit(static_cast<u32>(keyboard_key));
        }
        constexpr bool IsModifier(KeyboardModifier keyboard_modifier) const {
            return this->modifier_set.GetBit(static_cast<u32>(keyboard_modifier));
        }
	};

	void InitializeKeyboard(mem::Heap *heap, u32 max_keyboards);
    void FinalizeKeyboard();
    void GetKeyboardStates(KeyboardState *out_keyboard_states, u32 state_count, u32 device_index);
}

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

    using KeyMask         = vp::util::FixedConnection<u64, 0x100>;
    using KeyModifierMask = vp::util::FixedConnection<u64, 0x40>;

	struct KeyboardState {
		u64             sampling_count;
        KeyModifierMask key_modifier_state;
		KeyMask         key_state;

        constexpr bool GetKey(KeyLocation key_location) const {
            return key_state.GetBit(static_cast<u32>(key_location));
        }
        constexpr bool GetKeyModifier(KeyModifierLocation key_modifier_location) const {
            return key_state.GetBit(static_cast<u32>(key_modifier_location));
        }
	};

	void InitializeKeyboard(mem::Heap *heap, u32 max_mice);
    void FinalizeKeyboard();

    void GetKeyboardStates(KeyboardState *out_keyboard_states, u32 state_count, u32 device_index);
}

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

        constexpr bool IsKeySet(KeyboardKey keyboard_key) const {
            return this->key_set.GetBit(static_cast<u32>(keyboard_key));
        }
        constexpr bool IsModifier(KeyboardModifier keyboard_modifier) const {
            return this->modifier_set.GetBit(static_cast<u32>(keyboard_modifier));
        }
	};

    struct KeyboardControl {
        s64 sampling_count = 0;
        KeyboardModifierSet modifier_set_held;
        KeyboardModifierSet modifier_set_press;
        KeyboardAttributeSet attribute_set_held;
        KeyboardAttributeSet attribute_set_press;
		KeyboardKeySet key_set_held;
		KeyboardKeySet key_set_press;

        constexpr bool IsKeyHeld(KeyboardKey keyboard_key) const {
            return this->key_set_held.GetBit(static_cast<u32>(keyboard_key));
        }

        constexpr bool IsKeyPress(KeyboardKey keyboard_key) const {
            return this->key_set_press.GetBit(static_cast<u32>(keyboard_key));
        }

        constexpr bool IsModifierHeld(KeyboardModifier keyboard_modifier) const {
            return this->modifier_set_held.GetBit(static_cast<u32>(keyboard_modifier));
        }

        constexpr bool IsModifierPress(KeyboardModifier keyboard_modifier) const {
            return this->modifier_set_press.GetBit(static_cast<u32>(keyboard_modifier));
        }

        constexpr bool IsAttributeHeld(KeyboardAttribute keyboard_attribute) const {
            return this->attribute_set_held.GetBit(static_cast<u32>(keyboard_attribute));
        }

        constexpr bool IsAttributePress(KeyboardAttribute keyboard_attribute) const {
            return this->attribute_set_press.GetBit(static_cast<u32>(keyboard_attribute));
        }

        constexpr void CombineKeyboardState(const KeyboardState *key_state_array, u32 count) {

            /* Generate anti-press mask from last held */
            KeyboardModifierSet modifier_set_temp = {};
            modifier_set_temp.Not(this->modifier_set_held);
            KeyboardAttributeSet attribute_set_temp = {};
            attribute_set_temp.Not(this->attribute_set_held);
            KeyboardKeySet key_set_temp = {};
            key_set_temp.Not(this->key_set_held);

            this->modifier_set_press.Clear();
            this->modifier_set_held.Clear();
            this->attribute_set_press.Clear();
            this->attribute_set_held.Clear();
            this->key_set_press.Clear();
            this->key_set_held.Clear();
            for (u32 i = 0; i < count; ++i) {

                /* Ensure later sample */
                const KeyboardState *in_state = std::addressof(key_state_array[i]);
                if (in_state->sampling_count < this->sampling_count) { continue; }
                this->sampling_count = in_state->sampling_count;

                /* Pressed = (press_state | in_state) & (~prev_held) */
                this->modifier_set_press.Or(in_state->modifier_set);
                this->modifier_set_press.And(modifier_set_temp);

                this->attribute_set_press.Or(in_state->attribute_set);
                this->attribute_set_press.And(attribute_set_temp);
    
                this->key_set_press.Or(in_state->key_set);
                this->key_set_press.And(key_set_temp);

                /* Held = in_state */
                this->modifier_set_held.Or(in_state->modifier_set);
                this->attribute_set_held.Or(in_state->attribute_set);
                this->key_set_held.Or(in_state->key_set);
            }

            return;
        }

        constexpr void CombineKeyboardControl(const KeyboardControl *keyboard_control_array, u32 count) {

            this->modifier_set_press.Clear();
            this->modifier_set_held.Clear();
            this->attribute_set_press.Clear();
            this->attribute_set_held.Clear();
            this->key_set_press.Clear();
            this->key_set_held.Clear();
            for (u32 i = 0; i < count; ++i) {
                const KeyboardControl *in_control = std::addressof(keyboard_control_array[i]);
                this->modifier_set_press.Or(in_control->modifier_set_press);
                this->modifier_set_held.Or(in_control->modifier_set_held);
                this->attribute_set_press.Or(in_control->attribute_set_press);
                this->attribute_set_held.Or(in_control->attribute_set_held);
                this->key_set_press.Or(in_control->key_set_press);
                this->key_set_held.Or(in_control->key_set_held);
            }

            return;
        }
    };

	void InitializeKeyboard(mem::Heap *heap, u32 max_keyboards);
    void FinalizeKeyboard();
    void GetKeyboardStates(KeyboardState *out_keyboard_states, u32 state_count, u32 device_index);
}

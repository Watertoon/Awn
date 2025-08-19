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

	void InitializeKeyboard(mem::Heap *heap, u32 max_keyboard_count) {
        impl::KeyboardManager *mgr = impl::KeyboardManager::CreateInstance(heap);
        RESULT_ABORT_UNLESS(mgr->Initialize(heap, max_keyboard_count));
        return;
    }

    void FinalizeKeyboard() {
        impl::KeyboardManager *mgr = impl::KeyboardManager::GetInstance();
        mgr->Finalize();
        impl::KeyboardManager::DeleteInstance();
        return;
    }

    u32 GetKeyboardStates(KeyboardState *out_keyboard_states, u32 state_count, u32 keyboard_id) {
        impl::KeyboardManager *mgr = impl::KeyboardManager::GetInstance();
        return mgr->GetKeyboardStates(out_keyboard_states, state_count, keyboard_id);
    }

    void SwapKeyboardAssignment(u32 keyboard_id0, u32 keyboard_id1) {
        impl::KeyboardManager *mgr = impl::KeyboardManager::GetInstance();
        mgr->SwapKeyboardAssignment(keyboard_id0, keyboard_id1);
        return;
    }

    void DisconnectKeyboard(u32 keyboard_id) {
        impl::KeyboardManager *mgr = impl::KeyboardManager::GetInstance();
        mgr->DisconnectKeyboard(keyboard_id);
        return;
    }
}

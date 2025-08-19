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

	void InitializeMouse(mem::Heap *heap, u32 max_mouse_count) {
        impl::MouseManager *mgr = impl::MouseManager::CreateInstance(heap);
        RESULT_ABORT_UNLESS(mgr->Initialize(heap, max_mouse_count));
        return;
    }

    void FinalizeMouse() {
        impl::MouseManager *mgr = impl::MouseManager::GetInstance();
        mgr->Finalize();
        impl::MouseManager::DeleteInstance();
        return;
    }

    u32 GetMouseStates(MouseState *out_mouse_states, u32 state_count, u32 mouse_id) {
        impl::MouseManager *mgr = impl::MouseManager::GetInstance();
        return mgr->GetMouseStates(out_mouse_states, state_count, mouse_id);
    }

    void SwapMouseAssignment(u32 mouse_id0, u32 mouse_id1) {
        impl::MouseManager *mgr = impl::MouseManager::GetInstance();
        mgr->SwapMouseAssignment(mouse_id0, mouse_id1);
        return;
    }

    void DisconnectMouse(u32 mouse_id) {
        impl::MouseManager *mgr = impl::MouseManager::GetInstance();
        mgr->DisconnectMouse(mouse_id);
        return;
    }
}

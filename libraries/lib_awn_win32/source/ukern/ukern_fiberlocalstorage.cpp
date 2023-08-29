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

namespace awn::ukern {

    bool FiberLocalStorage::IsSchedulable(u32 core_number, u64 time) {
        if ((core_mask & (1 << core_number)) == 0)                                           { return false; }
        if (waitable_object != nullptr && timeout < time)                                    { waitable_object->CancelWait(this, ResultTimeout); }
        if (fiber_state != FiberState_Scheduled && fiber_state != FiberState_ScheduledLocal) { return false; }

        return true;
    }

    void FiberLocalStorage::ReleaseLockWaitListUnsafe() {

        VP_ASSERT(this->wait_list.IsEmpty() == false);

        /* Get waiter to signal */
        FiberLocalStorage &next_owner = this->wait_list.PopFront();

        /* Set wait tag and clear state */
        *next_owner.lock_address = (this->wait_list.IsEmpty() == true) ? next_owner.wait_tag : next_owner.wait_tag | HasChildWaitersBit;
        next_owner.lock_address  = nullptr;
        next_owner.wait_tag      = 0;

        /* Transfer waiters to new owner */
        for (FiberLocalStorage &waiter : this->wait_list) {
            waiter.wait_list_node.Unlink();
            next_owner.wait_list.PushBack(waiter);
        }

        /* End next owner's wait */
        next_owner.waitable_object->EndWait(std::addressof(next_owner), ResultSuccess);
        next_owner.waitable_object = nullptr;

        return;
    }
}

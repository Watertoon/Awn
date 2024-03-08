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

namespace awn::sys {

    sys::ThreadBase *GetCurrentThread() {
        return sys::ThreadManager::GetInstance()->GetCurrentThread();
    }

    void SleepThread(vp::TimeSpan timeout_ns) {
        sys::ThreadManager::GetInstance()->GetCurrentThread()->SleepThread(timeout_ns);
    }

    TlsSlot AllocateTlsSlot(TlsDestructor destructor) {
        TlsSlot    slot   = 0;
        const bool result = sys::ThreadManager::GetInstance()->AllocateTlsSlot(std::addressof(slot), destructor, false);
        VP_ASSERT(result == true);
        return slot;
    }

    void FreeTlsSlot(TlsSlot slot) {
        sys::ThreadManager::GetInstance()->FreeTlsSlot(slot);
    }

    void *GetTlsData(TlsSlot slot) {
        return sys::GetCurrentThread()->GetTlsData(slot);
    }
    void SetTlsData(TlsSlot slot, void *data) {
        sys::GetCurrentThread()->SetTlsData(slot, data);
    }    

        bool IsThreadValid(sys::ThreadBase *thread) {
            return sys::ThreadManager::GetInstance()->IsThreadValid(thread);
        }
}

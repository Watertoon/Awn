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
        sys::ThreadManager *mgr = sys::ThreadManager::GetInstance();
        if (mgr == nullptr) { return nullptr; }
        return mgr->GetCurrentThread();
    }

    void SleepThread(vp::TimeSpan timeout_ns) {
        sys::ThreadManager::GetInstance()->GetCurrentThread()->SleepThread(timeout_ns);
    }

#ifdef VP_TARGET_PLATFORM_win32

    u32 GetCurrentThreadId() {
        return ::GetCurrentThreadId();
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

    uintptr_t GetTlsValue(TlsSlot slot) {
        return sys::GetCurrentThread()->GetTlsValue(slot);
    }
    void SetTlsValue(TlsSlot slot, uintptr_t data) {
        sys::GetCurrentThread()->SetTlsValue(slot, data);
    }    
#elif VP_TARGET_PLATFORM_nx
    TlsSlot AllocateTlsSlot(TlsDestructor destructor) {
        TlsSlot slot = 0;
        RESULT_ABORT_UNLESS(nn::os::AllocateTlsSlot(std::addressof(slot), destructor));
        return slot;
    }

    void FreeTlsSlot(TlsSlot slot) {
        nn::os::FreeTlsSlot(slot);
    }

    uintptr_t GetTlsValue(TlsSlot slot) {
        return reinterpret_cast<void*>(nn::os::GetTlsValue(slot));
    }
    void SetTlsValue(TlsSlot slot, uintptr_t data) {
        nn::os::SetTlsValue(slot, data);
    }
#endif

    bool IsThreadValid(sys::ThreadBase *thread) {
        return sys::ThreadManager::GetInstance()->IsThreadValid(thread);
    }
}

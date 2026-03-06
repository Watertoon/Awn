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

    void SleepThread(vp::TimeSpan timeout_ns) {
        sys::GetCurrentThread()->SleepThread(timeout_ns);
    }

#ifdef VP_TARGET_PLATFORM_win32
    sys::ThreadBase *GetCurrentThread() {
        sys::ThreadManager *mgr = sys::ThreadManager::GetInstance();
        if (mgr == nullptr) { return nullptr; }
        return mgr->GetCurrentThread();
    }
    u32 GetCurrentThreadId() {
        return ::GetCurrentThreadId();
    }
#elifdef VP_TARGET_PLATFORM_linux
    thread_local sys::ThreadBase *tCurrentThread = nullptr;

    sys::ThreadBase *GetCurrentThread() {
        return tCurrentThread;
    }
    namespace impl {
        void SetCurrentThread(sys::ThreadBase *thread) {
            tCurrentThread = thread;
        }
    }
    u32 GetCurrentThreadId() {
        const sys::Thread *thread = reinterpret_cast<sys::Thread*>(sys::GetCurrentThread());
        return (VP_LIKELY(thread != nullptr)) ? thread->GetTid() : ::gettid();
    }
#endif

#if defined(VP_TARGET_PLATFORM_win32) || defined(VP_TARGET_PLATFORM_linux)

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

#elifdef VP_TARGET_PLATFORM_nx

    TlsSlot AllocateTlsSlot(TlsDestructor destructor) {
        TlsSlot slot = 0;
        RESULT_ABORT_UNLESS(nn::os::AllocateTlsSlot(std::addressof(slot), destructor));
        return slot;
    }

    void FreeTlsSlot(TlsSlot slot) {
        nn::os::FreeTlsSlot(slot);
    }

    uintptr_t GetTlsValue(TlsSlot slot) {
        return reinterpret_cast<uintptr_t>(nn::os::GetTlsValue(slot));
    }

    void SetTlsValue(TlsSlot slot, uintptr_t data) {
        nn::os::SetTlsValue(slot, data);
    }

#endif

    bool IsThreadValid(sys::ThreadBase *thread) {
        return sys::ThreadManager::GetInstance()->IsThreadValid(thread);
    }
}

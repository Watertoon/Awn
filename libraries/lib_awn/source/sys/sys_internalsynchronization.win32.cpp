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

namespace awn::sys::impl {

    void ArbitrateLockImpl(u32 *address, u32 tag) {

        /* Wait until we acquire the tag */
        u32 prev;
        /* Claim tag by thread id */
        if (vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), address, tag, 0u)) { return; }
        for (;;) {
            VP_ASSERT((prev & cTagLockClearMask) != tag);
            /* Set waiters bit */
            if ((prev & cTagLockSetMask) == 0) {
                if (!vp::util::InterlockedCompareExchangeRelaxed(std::addressof(prev), address, prev | cTagLockSetMask, prev)) { continue; }
                prev |= cTagLockSetMask;
            }
            /* Lock by system */
            const bool result = ::WaitOnAddress(address, std::addressof(prev), sizeof(u32), INFINITE);
            VP_ASSERT(result);
            /* Ensure we own the lock */
            prev = vp::util::InterlockedLoadAcquire(address);
            if ((prev & cTagLockClearMask) == tag) { return; }
            if (prev == 0) {
                if (vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), address, tag, 0u)) { return; }
            }
        }
        return;
    }

    void ArbitrateUnlockImpl(u32 *address, u32 tag) {

        /* Release the tag, wake all waiters for acquisition to ensure multi-waits set the lock bit again */
        u32 prev;
        const bool is_rel = vp::util::InterlockedCompareExchangeRelease(std::addressof(prev), address, 0u, tag);
        if (!is_rel) {
            //VP_ASSERT(prev == (tag | cTagLockSetMask));
            const bool is_rel1 = vp::util::InterlockedCompareExchangeRelease(std::addressof(prev), address, 0u, tag | cTagLockSetMask);
            VP_ASSERT(is_rel1);
            ::WakeByAddressAll(address);
        }

        return;
    }

    void WaitProcessWideKeyImpl(u32 *address_cs, u32 *address_cv, u32 tag) {

        /* Try to release our tag for no contention */
        ArbitrateUnlockImpl(address_cs, tag);

        /* Wait for cv to change from initial value */
        u32 prev;
        u32 wait_val = *address_cv;
        for (;;) {
            const bool wait_result = ::WaitOnAddress(address_cv, std::addressof(wait_val), sizeof(u32), INFINITE);
            VP_ASSERT(wait_result);

            /* Succeed if clear single wake bit */
            const bool is_single_clear = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), address_cv, wait_val, wait_val | cWideKeySingleMask);
            if (is_single_clear) { break; }
            /* Succeed if broadcast id advanced */
            if (wait_val != prev) { break; }
        }

        /* Try reclaim tag */
        ArbitrateLockImpl(address_cs, tag);

        return;
    }

    void SignalWideKey(u32 *address_cv, bool is_all) {
        if (is_all) {

            /* Increment broadcast id */
            u32 prev = *address_cv;
            for (;;) {
                const u32 single_bit = (prev & cWideKeyBroadcastMask) + 1;
                const u32 next = (single_bit == cWideKeyBroadcastMask) ? 0 : single_bit;
                const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(prev), address_cv, next | (prev & cWideKeySingleMask), prev);
                if (result) { break; }
            }

            /* Wake all waiters */
            ::WakeByAddressAll(address_cv);
        } else {
            /* Set wake single waiter bit */
            vp::util::InterlockedOrRelease(address_cv, cWideKeySingleMask);

            /* Wake a single waiter */
            ::WakeByAddressSingle(address_cv);
        }
        return;
    }

    void InternalCriticalSectionImplByWin32::Enter() {
        const u32 id = sys::GetCurrentThreadId();
        ArbitrateLockImpl(std::addressof(m_tag), id);
        return;
    }

    bool InternalCriticalSectionImplByWin32::TryEnter() {
        /* Try claim tag */
        const u32 id = sys::GetCurrentThreadId();
        u32 prev;
        const bool is_acq = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(m_tag), id, 0u);
        VP_ASSERT(is_acq || (prev & cTagLockClearMask) != id);
        return is_acq;
    }

    void InternalCriticalSectionImplByWin32::Leave() {
        const u32 id = sys::GetCurrentThreadId();
        ArbitrateUnlockImpl(std::addressof(m_tag), id);
    }

    bool InternalCriticalSectionImplByWin32::IsLockedByCurrentThread() {
        const u32 thread_id = sys::GetCurrentThreadId();
        return (m_tag & cTagLockClearMask) == thread_id;
    }

    void InternalConditionVariableImplByWin32::Wait(InternalCriticalSection *cs) {
        const u32 thread_id = sys::GetCurrentThreadId();
        WaitProcessWideKeyImpl(std::addressof(cs->m_tag), std::addressof(m_cv), thread_id);
    }

    void InternalConditionVariableImplByWin32::Signal() {
        //if (m_cv == 0) { return; }
        SignalWideKey(std::addressof(m_cv), false);
    }

    void InternalConditionVariableImplByWin32::Broadcast() {
        //if (m_cv == 0) { return; }
        SignalWideKey(std::addressof(m_cv), true);
    }
}

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
        u32 prev = 0;
        for (;;) {

            /* Try claim tag */
            while (prev == 0) {
                const bool cmp_result = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), address, tag, 0u);
                if (cmp_result) { return; }
            }

            /* Set lock bit */
            u32 wait_tag = prev | cTagLockSetMask;
            if (!((prev >> cTagLockBitOffset) & 1)) {
                const bool tag_result = vp::util::InterlockedCompareExchange(std::addressof(prev), address, wait_tag, prev);
                if (!tag_result) { continue; }
            }

            /* System wait */
            const bool wait_result = ::WaitOnAddress(address, std::addressof(wait_tag), sizeof(u32), INFINITE);
            VP_ASSERT(wait_result);
        }
    }

    void ArbitrateUnlockImpl(u32 *address) {

        /* Release the tag, wake all waiters for acquisition to ensure multi-waits set the lock bit again */
        const u32 prev = vp::util::InterlockedExchangeRelease(address, 0u);
        if ((prev >> cTagLockBitOffset) & 1) {
            ::WakeByAddressAll(address);
        }

        return;
    }

    void WaitProcessWideKeyImpl(u32 *address_cs, u32 *address_cv, u32 tag) {

        /* Try to release our tag for no contention */
        u32 last_cs;
        const bool cs_release_result = vp::util::InterlockedCompareExchangeRelease(std::addressof(last_cs), address_cs, 0u, tag);
        if ((last_cs >> cTagLockBitOffset) & 1) {
            /* Release for contention */
            VP_ASSERT((last_cs & cTagLockClearMask) == tag);
            ArbitrateUnlockImpl(address_cs);
        } else {
            VP_ASSERT(cs_release_result);
        }

        /* Wait for cv to change from initial value */
        u32 prev;
        u32 wait_val = *address_cv;
        for (;;) {
            const bool wait_result = ::WaitOnAddress(address_cv, std::addressof(wait_val), sizeof(u32), INFINITE);
            VP_ASSERT(wait_result);

            /* Succeed if clear single wake bit */
            const bool is_single_clear = vp::util::InterlockedCompareExchange(std::addressof(prev), address_cv, wait_val, wait_val | cWideKeySingleMask);
            if (is_single_clear) { break; }
            /* Succeed if broadcast id advanced */
            if (wait_val != prev) { break; }
        }

        /* Try reclaim tag */
        const bool cs_rec_result = vp::util::InterlockedCompareExchangeAcquire(std::addressof(last_cs), address_cs, tag, 0u);
        if (!cs_rec_result) {
            /* Move to an arbitrate lock acquire/wait */
            ArbitrateLockImpl(address_cs, tag);
        }

        return;
    }

    void SignalWideKey(u32 *address_cv, bool is_all) {
        if (is_all) {

            /* Increment broadcast id */
            u32 prev = *address_cv;
            for (;;) {
                const u32 single_bit = prev & cWideKeySingleMask;
                const u32 next = ((prev & cWideKeyBroadcastMask) == cWideKeyBroadcastMask) ? single_bit : single_bit + 1;
                const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(prev), address_cv, next, prev);
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
        const u32 thread_id = sys::GetCurrentThreadId();
        ArbitrateLockImpl(std::addressof(this->tag), thread_id);
        return;
    }

    bool InternalCriticalSectionImplByWin32::TryEnter() {

        /* Try claim tag */
        const u32 thread_id = sys::GetCurrentThreadId();
        u32 prev;
        const bool cmp_result = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(this->tag), thread_id, 0u);
        return cmp_result;
    }

    void InternalCriticalSectionImplByWin32::Leave() {

        /* Release tag and arbitrate unlock if necessary */
        const u32 thread_id = sys::GetCurrentThreadId();
        u32 prev;
        const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(prev), std::addressof(this->tag), 0u, thread_id);
        if (result) { return; }
        ArbitrateUnlockImpl(std::addressof(this->tag));
    }

    bool InternalCriticalSectionImplByWin32::IsLockedByCurrentThread() {
        const u32 thread_id = sys::GetCurrentThreadId();
        return (this->tag & cTagLockClearMask) == thread_id;
    }

    void InternalConditionVariableImplByWin32::Wait(InternalCriticalSection *cs) {
        const u32 thread_id = sys::GetCurrentThreadId();
        VP_ASSERT(thread_id == (cs->tag & cTagLockClearMask));
        WaitProcessWideKeyImpl(std::addressof(cs->tag), std::addressof(this->cv), thread_id);
    }

    void InternalConditionVariableImplByWin32::Signal() {
        //if (m_cv == 0) { return; }
        SignalWideKey(std::addressof(this->cv), false);
    }

    void InternalConditionVariableImplByWin32::Broadcast() {
        //if (m_cv == 0) { return; }
        SignalWideKey(std::addressof(this->cv), true);
    }
}

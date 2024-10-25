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

    void ReaderWriterLock::AcquireReadLockAsWriter() {
        ++m_writer_read_count;
    }

    void ReaderWriterLock::ReleaseReadLockAsWriter() {
        --m_writer_read_count;
        this->ReleaseWriteLockImpl();
        return;
    }

    void ReaderWriterLock::ReleaseWriteLockImpl() {

        /* Fail if we have readers or writers */
        if (m_writer_read_count != 0 || m_writer_count != 0) { return; }

        /* Release owner */
        m_writer_thread_id = 0;

        /* Wake the next write waiter, else wake all the reader waiters */
        u64 state = m_state;
        if (((state >> 0x20) & cWriterWaitCountMaskU32) != 0) {
            m_write_cv.Signal();
        } else if (((state >> 0x20) & cReaderWaitCountMaskU32) != 0) {
            m_read_cv.Broadcast();
        }

        for (;;) {
            /* Release CriticalSection and release lock bit */
            while ((state >> impl::cTagLockBitOffset) & 1) {
                const u64 next_state = state & (~cWriterLockBitU64);
                const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(state), std::addressof(m_state), next_state, state);
                if (result) {
                    impl::ArbitrateUnlockImpl(std::addressof(m_cs.tag));
                    return; 
                }
            }
            const u64 next_state = state & ((~cWriterLockBitU64) & (~cTagBitU64));
            const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(state), std::addressof(m_state), next_state, state);
            if (result) { return; }
        }
    }

    void ReaderWriterLock::AcquireWriteLock() {

        /* Increment writer count if we already own the lock */
        const u32 thread_id = sys::GetCurrentThreadId();
        if (thread_id == m_writer_thread_id) {
            ++m_writer_count;
            return;
        }

        /* Set thread id and increment writer count on acquisition */
        ON_SCOPE_EXIT {
            m_writer_thread_id = thread_id;
            ++m_writer_count;
        };

        bool is_retry = false;
        u64 prev = m_state;
        for (;;) {
            u32 p_tag;
            u32 counts;
            /* Try to take Critical Section if no readers and available */
            for (;;) {
                p_tag = static_cast<u32>(prev);
                counts = static_cast<u32>(prev >> 0x20);
                if ((counts & cReaderCountMaskU32) == 0 && p_tag == 0) { break; }
                /* Preserve high, decrement writer wait count on retry */
                const u32 next_high = (!is_retry) ? counts : ((counts & cWriterWaitCountMaskU32) - 0x10'0000) | (counts & (~cWriterWaitCountMaskU32));
                /* Try to take critical section and set write lock bit */
                const u64 next = (static_cast<u64>(next_high) << 0x20) | cWriterLockBitU64 | thread_id;
                const bool result = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(m_state), next, prev);
                if (result) { return; }
            }
            /* Set wait bit if critical section is taken, else take it */
            const u32 next_low = (p_tag != 0) ? p_tag | impl::cTagLockSetMask : thread_id;
            /* Increment write waiter count unless we are retrying */
            const u32 next_high = (is_retry) ? counts : ((counts & cWriterWaitCountMaskU32) + 0x10'0000) | (counts & (~cWriterWaitCountMaskU32));
            const u64 next = (static_cast<u64>(next_high) << 0x20) | next_low;
            const bool result = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(m_state), next, prev);
            if (!result) { continue; }
            /* If owned wait on Critical Section for previous writer or reader */
            if (p_tag != 0) {
                impl::ArbitrateLockImpl(/*next_low & impl::cTagLockClearMask,*/ std::addressof(m_cs.tag), thread_id);
                prev = m_state;
                is_retry = true;
                if (((thread_id ^ static_cast<u32>(prev)) & impl::cTagLockClearMask) != 0) { continue; }
            } else {
                prev = next;
            }
            /* Transition to wait acquire loop */
            break;
        }

        for (;;) {
            /* Wait for readers to drain */
            while ((prev & cReaderCountMaskU64) != 0) {
                m_write_cv.Wait(std::addressof(m_cs));
                prev = m_state;
            }
            /* Decrement write waiter count and acquire write lock */
            const u32 counts = static_cast<u32>(prev >> 0x20);
            const u32 next_high = ((counts & cWriterWaitCountMaskU32) - 0x10'0000) | (counts & (~cWriterWaitCountMaskU32));
            const u64 next = (static_cast<u64>(next_high) << 0x20) | cWriterLockBitU64 | (prev & cTagBitU32);
            const bool result = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(m_state), next, prev);
            if (result) { return; }
        }
    }

    bool ReaderWriterLock::TryAcquireWriteLock() {

        /* Increment writer count if we are the owner */
        const u32 thread_id = sys::GetCurrentThreadId();
        if (thread_id == m_writer_thread_id) {
            ++m_writer_count;
            return true;
        }

        u64 prev = m_state;
        for (;;) {
            /* Fail to take thread if the Criticals Section is unavailable or readers exist */
            if ((prev & (cTagBitU64 | cReaderCountMaskU64)) != 0) { return false; }
            /* Take the critical section and set the lock bit */
            const u64 next = (prev & (cTagBitU64)) | cWriterLockBitU64 | thread_id;
            const bool result = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(m_state), next, prev);
            if (result) {
                m_writer_thread_id = thread_id;
                ++m_writer_count;
                return true;
            }
        }
    }

    void ReaderWriterLock::ReleaseWriteLock() {
        --m_writer_count;
        this->ReleaseWriteLockImpl();
    }

    void ReaderWriterLock::AcquireReadLock() {

        /* Handle if we are the writer */
        const u32 thread_id = sys::GetCurrentThreadId();
        if (thread_id == m_writer_thread_id) {
            this->AcquireReadLockAsWriter();
            return;
        }

        u32 loop_result;
        u32 low;
        u32 high;
        bool is_retry = false;
        u64 prev = m_state;
        do {
            bool is_tag;
            high = static_cast<u32>(prev >> 0x20);
            if ((high & cWriterWaitCountMaskU32) == 0) {
                is_tag = false;
                const u32 read_wait_dec = (!is_retry) ? high : ((high & cReaderWaitCountMaskU32) - 0x400) | (high & (~cReaderWaitCountMaskU32));
                high = ((read_wait_dec & cReaderCountMaskU32) + 1) | (read_wait_dec & (~cReaderCountMaskU32));
                low = static_cast<u32>(prev);
            } else {
                high = (is_retry) ? high : ((high & cReaderWaitCountMaskU32) + 0x400) | (high & (~cReaderWaitCountMaskU32));
                is_tag = static_cast<u32>(prev) != 0;
                low = (!is_retry) ? thread_id : static_cast<u32>(prev) | impl::cTagLockSetMask;
            }

            const u64 next = low | (static_cast<u64>(high) << 0x20);
            const bool result0 = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(m_state), next, prev);
            if (!result0) { loop_result = 0; continue; }
            
            const bool is_write_waiter = (prev & (cWriterLockBitU64 | cWriterWaitCountMaskU64)) != 0;
            const bool is_locked = (is_write_waiter) ? is_tag : false;
            loop_result = (is_write_waiter) ? 2 : 1;

            if (is_locked) {
                impl::ArbitrateLockImpl(/*low & impl::cTagLockClearMask,*/ std::addressof(m_cs.tag), thread_id);
                prev = m_state;
                low = static_cast<u32>(m_state);
                high = static_cast<u32>(m_state >> 0x20);
                if (((thread_id ^ low) & impl::cTagLockClearMask) == 0) { loop_result = 2; break; }
                is_retry = true;
                loop_result = 0;
            } else {
                is_retry |= is_write_waiter;
            }
        } while (loop_result == 0);
        if (loop_result != 2) { return; }
        prev = low | (static_cast<u64>(high) << 0x20);
        for (;;) {
            /* Wait until we no longer have writer waiters */
            while ((prev & cWriterWaitCountMaskU64) != 0) {
                m_read_cv.Wait(std::addressof(m_cs));
                prev = m_state;
            }
            /* Decrement read wait count and increment reader count */
            const u32 f_high = prev >> 0x20;
            const u32 f_adj = ((f_high & cReaderWaitCountMaskU32) - 0x400) | ((f_high & cReaderCountMaskU32) + 1) | (f_high & (~(cReaderWaitCountMaskU32 | cReaderCountMaskU32)));
            const u64 fin = static_cast<u64>(f_adj) << 0x20;
            /* Unlock Critical Section if it was acquired */
            if ((prev >> impl::cTagLockBitOffset) & 1) {
                const bool result0 = vp::util::InterlockedCompareExchangeRelease(std::addressof(prev), std::addressof(m_state), fin | static_cast<u32>(prev), prev);
                if (result0) {
                    impl::ArbitrateUnlockImpl(std::addressof(m_cs.tag));
                    return;
                }
            } else {
                const bool result1 = vp::util::InterlockedCompareExchangeRelease(std::addressof(prev), std::addressof(m_state), fin, prev);
                if (result1) { return; }
            }
        }
    }

    bool ReaderWriterLock::TryAcquireReadLock() {

        /* Handle if we are the writer */
        const u32 thread_id = sys::GetCurrentThreadId();
        if (thread_id == m_writer_thread_id) {
            this->AcquireReadLockAsWriter();
            return true;
        }

        u64 prev = m_state;
        for (;;) {
            /* Fail if we have a writer */
            const u64 write_lock_mask = prev & (cWriterLockBitU64 | cWriterWaitCountMaskU64);
            if (write_lock_mask != 0) { return write_lock_mask == 0; }
            /* Try increment reader count while we don't have a writer */
            const u32 high = static_cast<u32>(prev >> 0x20);
            const u32 next_high = ((high & cReaderCountMaskU32) + 1) | (high & (~cReaderCountMaskU32));
            const u64 next = (prev & cTagBitU32) | (static_cast<u64>(next_high) << 0x20);
            const bool result = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(m_state), next, prev);
            if (result) { return write_lock_mask == 0; }
        }
    }

    void ReaderWriterLock::ReleaseReadLock() {

        /* Handle if we are the writer */
        const u32 thread_id = sys::GetCurrentThreadId();
        if (thread_id == m_writer_thread_id) {
            this->ReleaseReadLockAsWriter();
            return;
        }

        u64 prev = m_state;
        for (;;) {
            /* If we are the last reader to leave we need more than a decrement */
            const u32 high = prev >> 0x20;
            const u32 next_read_count = ((high & cReaderCountMaskU32) - 1);
            if (next_read_count == 0 && (prev & cWriterWaitCountMaskU64) != 0) { break; }
            /* Decrement reader count */
            const u64 next = (prev & cTagBitU32) | (static_cast<u64>(next_read_count | (high & (~cReaderCountMaskU32))) << 0x20);
            const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(prev), std::addressof(m_state), next, prev);
            if (result) { return; }
        }

        do {
            u32 tag;
            /* Try to acquire the Critical Section, set lock bit of taken */
            for (;;) {
                tag = (static_cast<u32>(prev) != 0) ? prev | impl::cTagLockSetMask : thread_id;
                const u64 next = (prev & (~cTagBitU64)) | tag;
                const bool result = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(m_state), next, prev);
                if (result) { break; }
            }
            /* We already own it, congrats */
            if (tag == thread_id) { break; }
            /* Wait until we have acquired the Critical Section */
            impl::ArbitrateLockImpl(/*tag & impl::cTagLockClearMask,*/ std::addressof(m_cs.tag), thread_id);
            prev = m_state;
        } while (((thread_id ^ static_cast<u32>(prev)) & impl::cTagLockClearMask) != 0);

        u64 next;
        for (;;) {
            /* Decrement reader count */
            const u32 high = prev >> 0x20;
            next = (prev & cTagBitU32) | ((high & 0x7fff) - 1) | (high & 0xffff'8000);
            const bool result = vp::util::InterlockedCompareExchangeWeak(std::addressof(prev), std::addressof(m_state), next, prev);
            if (result) { break; }
        }

        /* Unlock a writer */
        m_write_cv.Signal();

        for (;;) {
            /* Release Critical Section if the tag lock is not set */
            const u32 tag = static_cast<u32>(next) & (static_cast<s32>(static_cast<u32>(next) << (0x1f - impl::cTagLockBitOffset)) >> 0x1f);
            const u64 fin = (next & (~cTagBitU64)) | tag;
            const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(next), std::addressof(m_state), fin, next);
            if (!result) { continue; }
            /* Unlock tag lock if set */
            if ((tag >> impl::cTagLockBitOffset) & 1) {
                impl::ArbitrateUnlockImpl(std::addressof(m_cs.tag));
            }
            return;
        }
    }

    bool ReaderWriterLock::IsWriteLockedByCurrentThread() const {
        return sys::GetCurrentThreadId() == m_writer_thread_id && m_writer_count != 0;
    }
}

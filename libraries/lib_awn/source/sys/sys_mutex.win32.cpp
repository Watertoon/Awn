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

    void Mutex::Enter() {

        /* Acquire if not owner */
        const u32 thread_id = sys::GetCurrentThreadId();
        if (thread_id != m_tag) {
            ::AcquireSRWLockExclusive(std::addressof(m_srwlock));
            m_tag = thread_id;
        }

        /* Increment count as the owner */
        ++m_lock_count;

        return;
    }

    bool Mutex::TryEnter() {

        /* Recursively acquire if owner */
        const u32 thread_id = sys::GetCurrentThreadId();
        if (m_tag == thread_id) {
            ++m_lock_count;
            return true;
        }

        /* Try acquire if not owner */
        const bool is_acq = ::TryAcquireSRWLockExclusive(std::addressof(m_srwlock));
        if (is_acq) {
            m_tag = thread_id;
            ++m_lock_count;
        }

        return is_acq;
    }

    void Mutex::Leave() {

        /* Decrement count */
        const u32 dec_lock_count = m_lock_count - 1;
        m_lock_count = dec_lock_count;
        if (0 < dec_lock_count) { return; }

        /* Leave internal cs when count drops to 0 */
        m_tag = 0;
        ::ReleaseSRWLockExclusive(std::addressof(m_srwlock));

        return;
    }

    bool Mutex::IsLockedByCurrentThread() {
        const u32 thread_id = sys::GetCurrentThreadId();
        return m_tag == thread_id;
    }
}

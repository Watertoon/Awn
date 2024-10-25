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

        /* Defer to the internal cs if this thread is not the owner */
        if (!m_cs.IsLockedByCurrentThread()) {
            m_cs.Enter();
        }

        /* Increment count as the owner */
        ++m_lock_count;

        return;
    }

    bool Mutex::TryEnter() {

        /* Defer to the internal cs if this thread is not the owner */
        if (!m_cs.IsLockedByCurrentThread() && !m_cs.TryEnter()) { return false; }

        /* Increment count as the owner */
        m_lock_count = m_lock_count + 1;

        return true;
    }

    void Mutex::Leave() {

        /* Decrement count */
        const u32 dec_lock_count = m_lock_count - 1;
        m_lock_count = dec_lock_count;
        if (0 < dec_lock_count) { return; }

        /* Leave internal cs when count drops to 0 */
        m_cs.Leave();

        return;
    }

    bool Mutex::IsLockedByCurrentThread() {
        return m_cs.IsLockedByCurrentThread();
    }
}

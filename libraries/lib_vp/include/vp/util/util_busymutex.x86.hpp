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
#pragma once

namespace vp::util {

    class BusyMutex {
        private:
        union {
            struct {
                u16 m_release_count;
                u16 m_lock_count;
            };
            u32 m_counter;
        };
        public:
            constexpr ALWAYS_INLINE BusyMutex() : m_counter(0) {/*...*/}
            constexpr ALWAYS_INLINE ~BusyMutex() {/*...*/}

            ALWAYS_INLINE void Enter() {

                /* Increment lock count */
                u32 wait = util::InterlockedFetchAdd(std::addressof(m_counter), 0x1'0000u);

                /* Wait until release count is matched */
                if ((wait & 0xffff) != ((wait >> 0x10) & 0xffff)) {
                    do {
                        /* Signal the processor to not aggressively speculatively execute for a bit */
                        util::x86::pause();
                    } while (m_release_count != ((wait >> 0x10) & 0xffff));
                }

                return;
            }

            ALWAYS_INLINE void Leave() {

                /* Increment release */
                util::InterlockedIncrementRelease(std::addressof(m_release_count));
            }
    };
    static_assert(sizeof(volatile long int) == sizeof(u32));
    static_assert(sizeof(volatile short) == sizeof(u16));

    class ScopedBusyMutex {
        private:
            BusyMutex *m_mutex;
        public:
            ALWAYS_INLINE ScopedBusyMutex(BusyMutex *mutex) : m_mutex(mutex) {
                m_mutex->Enter();
            }
            ALWAYS_INLINE ~ScopedBusyMutex() {
                m_mutex->Leave();
            }
    };
}


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

    template<typename T, auto SampleCountMember, u32 MaxCount>
    class FixedAtomicLifo {
        public:
            struct AtomicState {
                u64 sample_count;
                T state;
            };
        private:
            u64 m_sample_iter;
            u32 m_count;
            u32 m_offset;
            AtomicState m_array[MaxCount];
        public:
            constexpr FixedAtomicLifo() : m_sample_iter(1), m_count(), m_offset(), m_array() {/*...*/}
            constexpr ~FixedAtomicLifo() {/*...*/}

            void PushState(T *push_state) {

                /* Get old and latest state */
                const u32 count = m_count;
                push_state->*SampleCountMember = m_sample_iter;
                if (count < 1) {
                    AtomicState *state = m_array;
                    state->state = *push_state;
                    state->sample_count = m_sample_iter;
                    vp::util::InterlockedIncrement(std::addressof(m_count));
                    ++m_sample_iter;
                    return;
                }

                /* Get old and latest state */
                const u32 offset = m_offset;
                u32 base_id = (offset - count) + (MaxCount + 1);
                const u32 old_id = base_id % MaxCount;
                AtomicState *next = (MaxCount == count) ? m_array + old_id : m_array + count;
                next->state = *push_state;
                vp::util::MemoryBarrierInnerWrite();
                next->sample_count = m_sample_iter;
                vp::util::InterlockedIncrement(std::addressof(m_offset));
                if (count < MaxCount) {
                    vp::util::InterlockedIncrement(std::addressof(m_count));
                }
                ++m_sample_iter;

                return;
            }

            void ReadState(T *state) {

                /* Try read most recent state */
                u32 count = vp::util::InterlockedLoadAcquire(std::addressof(m_count));
                count = vp::util::Min(1u, count);
                while (0 < count) {

                    /* Copy most recent state */
                    const u32 offset = vp::util::InterlockedLoadAcquire(std::addressof(m_offset));
                    const u32 id = (offset + MaxCount) % MaxCount;
                    AtomicState *at_state = m_array + id;
                    const u64 iter = vp::util::InterlockedLoadAcquire(std::addressof(at_state->sample_count));
                    *state = at_state->state;

                    /* Check for state invalidation */
                    vp::util::MemoryBarrierInnerRead();
                    const u64 sampling = at_state->sample_count;
                    if (iter == sampling) { break; }
                    count = vp::util::InterlockedLoadAcquire(std::addressof(m_count));
                    count = vp::util::Min(1u, count);
                }

                return;
            }

            void ReadStates(u32 *out_read_count, T *state_array, u32 state_count) {

                /* Try read states from least to most recent */
                u32 offset = vp::util::InterlockedLoadAcquire(std::addressof(m_offset));
                u32 count_base = vp::util::InterlockedLoadAcquire(std::addressof(m_count));
                u32 count = vp::util::Min(state_count, count_base);
                while (0 < count) {

                    T *prev = state_array + count;
                    T *next = state_array + (count - 1);
                    u32 base_id = (offset - count) + (MaxCount + 1);
                    u32 l_i = count;
                    for (;;) {

                        /* Copy a state */
                        const u32 id = base_id % MaxCount;
                        AtomicState *at_state = m_array + id;
                        const u64 iter = vp::util::InterlockedLoadAcquire(std::addressof(at_state->sample_count));
                        *next = at_state->state;

                        /* Check for state invalidation */
                        vp::util::MemoryBarrierInnerRead();
                        const u64 sampling = at_state->sample_count;
                        if (iter != sampling || (l_i < count && (next->*SampleCountMember - prev->*SampleCountMember) != 1)) { break; }

                        /* Iterate to more recent state */
                        --prev;
                        --next;
                        ++base_id;
                        --l_i;
                        if (l_i < 1) {
                            *out_read_count = count;
                            return;
                        }
                    }

                    /* Refresh shared state */
                    const u32 reload_count = vp::util::InterlockedLoadAcquire(std::addressof(m_count));
                    count = vp::util::Min(state_count, count);
                    offset = vp::util::InterlockedLoadAcquire(std::addressof(m_offset));
                    count = vp::util::Max(reload_count, count);
                }
                *out_read_count = count;
                return;
            }
    };
}

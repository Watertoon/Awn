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

namespace awn::ukern {

    class HandleTable {
        public:
            static constexpr size_t MaxHandles = 256;
            static constexpr size_t CounterBitOffset = 0xf;
        private:
            s16        m_counters[MaxHandles];
            void      *m_objects[MaxHandles];
            BusyMutex  m_table_mutex;
            s32        m_indice_iter;
            u16        m_active_handles;
            u16        m_counter_value;
        public:
            constexpr ALWAYS_INLINE HandleTable() : m_counters{}, m_objects{}, m_table_mutex(), m_indice_iter(-1), m_active_handles(0), m_counter_value(1) {/*...*/}

            constexpr ALWAYS_INLINE void Initialize() {
                m_active_handles = 0;
                m_counter_value  = 1;
                for (u32 i = 0; i < MaxHandles; ++i) {
                    m_objects[i]  = nullptr;
                    m_counters[i] = i - 1;
                    m_indice_iter = i;
                }
            }

            bool ReserveHandle(u32 *out_handle, void *object) {
                ScopedBusyMutex lock(std::addressof(m_table_mutex));

                if (MaxHandles <= m_active_handles) { return false; }

                const u16 index = m_indice_iter;
                m_indice_iter   = m_counters[index];

                const u16 counter = m_counter_value;
                m_counters[index] = counter;
                m_objects[index] = object;

                ++m_active_handles;

                u16 next = 1;
                if (-1 < static_cast<s16>(m_counter_value + 1)) {
                    next = m_counter_value + 1;
                }
                m_counter_value = next;

                *out_handle = (index & 0x7fff) | (counter << CounterBitOffset);

                return true;
            }

            bool FreeHandle(u32 handle) {
                ScopedBusyMutex lock(std::addressof(m_table_mutex));

                if ((handle == 0) || ((handle >> CounterBitOffset) == 0)) { return false; }

                const u32 index = (handle & 0x7fff);
                if (MaxHandles < index || m_objects[index] == nullptr || static_cast<s16>(handle >> CounterBitOffset) != m_counters[index]) { return false; }

                m_objects[index] = nullptr;
                m_counters[index] = m_indice_iter;
                m_indice_iter = index;
                --m_active_handles;

                return true;
            }

            void *GetObjectByHandle(u32 handle) {
                ScopedBusyMutex lock(std::addressof(m_table_mutex));

                if (((handle >> 0x1e) != 0) || (handle == 0) || ((handle >> 0xf) == 0)) { return nullptr; }

                const u32 index = (handle & 0x7fff);
                if ((MaxHandles < index) || (static_cast<s16>(handle >> 0xf) != m_counters[index])) { return nullptr; }

                return m_objects[index];
            }
    };
}

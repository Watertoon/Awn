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

    constexpr ALWAYS_INLINE u32 GetHandleIndex(u32 handle) {
        return handle & 0x7fff;
    }

    template <size_t HandleCount>
        requires (HandleCount <= 0x7fff)
    class FixedHandleTable {
        public:
            static constexpr s32    cMaxHandles             = HandleCount;
            static constexpr size_t cCounterBitOffset       = 0xf;
            static constexpr size_t cHandleReserveBitOffset = 0x1e;
        private:
            s16        m_counter_array[cMaxHandles];
            void      *m_object_array[cMaxHandles];
            BusyMutex  m_table_mutex;
            s32        m_indice_iter;
            u16        m_active_handles;
            u16        m_counter_value;
        public:
            constexpr ALWAYS_INLINE FixedHandleTable() : m_counter_array{}, m_object_array{}, m_table_mutex(), m_indice_iter(-1), m_active_handles(0), m_counter_value(1) {/*...*/}
            constexpr ~FixedHandleTable() {/*...*/}

            constexpr ALWAYS_INLINE void Initialize() {

                /* Initialize handle table to valid state */
                m_active_handles = 0;
                m_counter_value  = 1;
                for (s32 i = 0; i < cMaxHandles; ++i) {
                    m_object_array[i]  = nullptr;
                    m_counter_array[i] = i - 1;
                    m_indice_iter      = i;
                }
            }

            bool ReserveHandle(u32 *out_handle, void *object) {
                
                /* Lock the handle table */
                ScopedBusyMutex lock(std::addressof(m_table_mutex));

                /* Integrity check handle count */
                if (cMaxHandles <= m_active_handles) { return false; }

                /* Get the next indice from the free counter_array */
                const u16 index = m_indice_iter;
                m_indice_iter   = m_counter_array[index];

                /* Set the table state and increment handle count */
                const u16 counter      = m_counter_value;
                m_counter_array[index] = counter;
                m_object_array[index]  = object;
                ++m_active_handles;

                /* Find next valid counter value with wrap around */
                u16 next = 1;
                if (-1 < static_cast<s16>(m_counter_value + 1)) {
                    next = m_counter_value + 1;
                }
                m_counter_value = next;

                /* Set return handle */
                *out_handle = (index & 0x7fff) | (counter << cCounterBitOffset);

                return true;
            }

            bool FreeHandle(u32 handle) {

                /* Lock the handle table */
                ScopedBusyMutex lock(std::addressof(m_table_mutex));

                /* Integrity check the handle is a valid form */
                if ((handle == 0) || ((handle >> cCounterBitOffset) == 0)) { return false; }

                /* Get the handle index and integrity check that the handle is currently valid */
                const u32 index = (handle & 0x7fff);
                if (cMaxHandles < index || m_object_array[index] == nullptr || (handle >> cCounterBitOffset) != static_cast<u32>(m_counter_array[index])) { return false; }

                /* Clear and free the handle state */
                m_object_array[index] = nullptr;
                m_counter_array[index] = m_indice_iter;
                m_indice_iter = index;
                --m_active_handles;

                return true;
            }

            void *GetObjectByHandle(u32 handle) {

                /* Lock the handle table */
                ScopedBusyMutex lock(std::addressof(m_table_mutex));

                /* Integrity check the handle is a valid form */
                if (((handle >> cHandleReserveBitOffset) != 0) || (handle == 0) || ((handle >> cCounterBitOffset) == 0)) { return nullptr; }

                /* Get the handle index and integrity check that the handle is currently valid */
                const u32 index = (handle & 0x7fff);
                if ((cMaxHandles < index) || ((handle >> cCounterBitOffset) != static_cast<u32>(m_counter_array[index]))) { return nullptr; }

                /* Get object */
                return m_object_array[index];
            }
    };
}

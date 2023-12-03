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

namespace awn::gfx {

    /* Only valid for service threads */
    class Sync {
        private:
            VkSemaphore m_vk_semaphore;
            size_t      m_expected_value;
        public:
            constexpr Sync() : m_vk_semaphore(VK_NULL_HANDLE), m_expected_value(0) {/*...*/}
            constexpr ~Sync() {/*...*/}

            void Initialize(bool is_for_present = false);
            void Finalize();

            void Wait();
            void TimedWait(TimeSpan timeout);

            void Signal();

            constexpr ALWAYS_INLINE void IncrementExpectedValue() { ++m_expected_value; }
            constexpr ALWAYS_INLINE u64  GetExpectedValue() const { return m_expected_value; }

            constexpr ALWAYS_INLINE VkSemaphore GetVkSemaphore() const { return m_vk_semaphore; }
    };
}

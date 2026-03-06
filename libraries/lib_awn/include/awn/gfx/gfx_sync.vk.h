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

    class Sync {
        private:
            VkSemaphore m_vk_semaphore;
            u64 m_expected_value;
        public:
            constexpr Sync() : m_vk_semaphore(VK_NULL_HANDLE), m_expected_value() {/*...*/}
            constexpr ~Sync() {/*...*/}

            void Initialize(bool is_for_present = false);
            void Finalize();

            void Wait();
            void TimedWait(TimeSpan timeout);
        public:
            void Signal();

            void InitializeImpl(const void *pnext, bool is_binary);

            constexpr void IncrementExpectedValue() { ++m_expected_value; }
            constexpr u64 GetExpectedValue() const { return m_expected_value; }
            u64 GetValue() const;

            constexpr bool IsValid() const { return m_vk_semaphore != VK_NULL_HANDLE; }
            constexpr VkSemaphore GetVkSemaphore() const { return m_vk_semaphore; }
    };
}

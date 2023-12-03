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

namespace awn::gfx {

    void Sync::Initialize(bool is_for_present) {

        /* Intialize semaphore */
        const VkSemaphoreTypeCreateInfo semaphore_type_info = {
            .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .semaphoreType = (is_for_present == true) ?  VK_SEMAPHORE_TYPE_BINARY : VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue  = 0,
        };
        const VkSemaphoreCreateInfo semaphore_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = std::addressof(semaphore_type_info),
        };
        const u32 result = ::pfn_vkCreateSemaphore(Context::GetInstance()->GetVkDevice(), std::addressof(semaphore_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_semaphore));
        VP_ASSERT(result == VK_SUCCESS);

        /* Set next expected */
        m_expected_value = 1;

        return;
    }

    void Sync::Finalize() {

        /* Reset expected */
        m_expected_value = 0;

        /* Null check */
        if (m_vk_semaphore == VK_NULL_HANDLE) { return; }

        /* Destroy semaphore */
        ::pfn_vkDestroySemaphore(Context::GetInstance()->GetVkDevice(), m_vk_semaphore, Context::GetInstance()->GetVkAllocationCallbacks());
        m_vk_semaphore = VK_NULL_HANDLE;

        return;
    }

    void Sync::Wait() {

        /* Wait on semaphore */
        const u64 value = m_expected_value;
        const VkSemaphoreWaitInfo wait_info = {
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .semaphoreCount = 1,
            .pSemaphores    = std::addressof(m_vk_semaphore),
            .pValues        = std::addressof(value)
        };
        const u32 result = ::pfn_vkWaitSemaphores(Context::GetInstance()->GetVkDevice(), std::addressof(wait_info), 0xffff'ffff'ffff'ffff);
        VP_ASSERT(result == VK_SUCCESS);

        return;
    }

    void Sync::TimedWait(TimeSpan timeout) {

        /* Wait on semaphore */
        const u64 value = m_expected_value;
        const VkSemaphoreWaitInfo wait_info = {
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .semaphoreCount = 1,
            .pSemaphores    = std::addressof(m_vk_semaphore),
            .pValues        = std::addressof(value)
        };
        const u32 result = ::pfn_vkWaitSemaphores(Context::GetInstance()->GetVkDevice(), std::addressof(wait_info), timeout.GetNanoSeconds());
        VP_ASSERT(result == VK_SUCCESS || result == VK_TIMEOUT);

        return;
    }

    void Sync::Signal() {

        /* Signal vk semaphore */
        const VkSemaphoreSignalInfo signal_info = {
            .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
            .semaphore = m_vk_semaphore,
            .value     = m_expected_value,
        };
        const u32 result = ::pfn_vkSignalSemaphore(Context::GetInstance()->GetVkDevice(), std::addressof(signal_info));
        VP_ASSERT(result == VK_SUCCESS);

        /* Increment expected value */
        ++m_expected_value;

        return;
    }
}

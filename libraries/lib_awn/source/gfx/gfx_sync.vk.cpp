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
        this->InitializeImpl(nullptr, is_for_present);
        return;
    }

    void Sync::InitializeImpl(const void *pnext, bool is_binary) {

        /* Intialize semaphore */
        const VkSemaphoreTypeCreateInfo semaphore_type_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = pnext,
            .semaphoreType = (is_binary) ?  VK_SEMAPHORE_TYPE_BINARY : VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = 0,
        };
        const VkSemaphoreCreateInfo semaphore_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = std::addressof(semaphore_type_info),
        };
        impl::Context *context = impl::Context::GetInstance();
        const VkResult vk_result = ::pfn_vkCreateSemaphore(context->GetVkDevice(), std::addressof(semaphore_info), context->GetVkAllocationCallbacks(), std::addressof(m_vk_semaphore));
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(vk_result));

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
        ::pfn_vkDestroySemaphore(impl::Context::GetInstance()->GetVkDevice(), m_vk_semaphore, impl::Context::GetInstance()->GetVkAllocationCallbacks());
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
        const VkResult vk_result = ::pfn_vkWaitSemaphores(impl::Context::GetInstance()->GetVkDevice(), std::addressof(wait_info), 0xffff'ffff'ffff'ffff);
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(vk_result));

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
        const VkResult vk_result = ::pfn_vkWaitSemaphores(impl::Context::GetInstance()->GetVkDevice(), std::addressof(wait_info), timeout.GetNanoSeconds());
        const Result result = impl::ConvertVkResultToResult(vk_result);
        if (!RESULT_MATCHES(result, ResultTimeout)) {            
            RESULT_ABORT_UNLESS(result);
        }

        return;
    }

    void Sync::Signal() {

        /* Signal vk semaphore */
        const VkSemaphoreSignalInfo signal_info = {
            .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
            .semaphore = m_vk_semaphore,
            .value     = m_expected_value,
        };
        const VkResult vk_result = ::pfn_vkSignalSemaphore(impl::Context::GetInstance()->GetVkDevice(), std::addressof(signal_info));
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(vk_result));

        /* Increment expected value */
        ++m_expected_value;

        return;
    }

    u64 Sync::GetValue() const {
        u64 counter = 0;
        const VkResult result0 = ::pfn_vkGetSemaphoreCounterValue(impl::Context::GetInstance()->GetVkDevice(), m_vk_semaphore, std::addressof(counter));
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(result0));
        return counter;
    }
}

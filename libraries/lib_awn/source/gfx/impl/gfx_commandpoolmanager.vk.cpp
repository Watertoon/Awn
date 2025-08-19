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

namespace awn::gfx::impl {

    AWN_SINGLETON_TRAITS_IMPL(CommandPoolManager);

    void CommandPoolManager::Initialize(mem::Heap *heap, u32 pool_count) {
        m_handle_allocator.Initialize(heap, pool_count);
        m_unit_array.Initialize(heap, pool_count);
    }

    void CommandPoolManager::Finalize() {
        m_handle_allocator.Finalize();
        m_unit_array.Finalize();
    }

    Result CommandPoolManager::CreateCommandPool(CommandPoolId *out_id, QueueType queue_type) {

        const CommandPoolId handle = m_handle_allocator.Allocate();
        VP_ASSERT(handle != HandleAllocator::cInvalidEntryIndex);
        CommandPoolUnit *unit = std::addressof(m_unit_array[handle]);
        unit->queue_type = queue_type;
        unit->ref_count = 1;
        Context *context = Context::GetInstance();
        const VkCommandPoolCreateInfo command_pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = context->GetQueueFamilyIndex(queue_type),
        };
        const VkResult vk_result = ::pfn_vkCreateCommandPool(context->GetVkDevice(), std::addressof(command_pool_create_info), context->GetVkAllocationCallbacks(), std::addressof(unit->vk_command_pool));
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));

        *out_id = handle;

        RESULT_RETURN_SUCCESS;
    }

    void CommandPoolManager::FreeCommandPool(CommandPoolId pool_id) {

        /* Release ref */
        CommandPoolUnit *unit = std::addressof(m_unit_array[pool_id]);
        const u32 count = vp::util::InterlockedDecrement(std::addressof(unit->ref_count));
        if (count != 0) { return; }

        /* Release command pool */
        Context *context = Context::GetInstance();
        ::pfn_vkDestroyCommandPool(context->GetVkDevice(), unit->vk_command_pool, context->GetVkAllocationCallbacks());
        unit->vk_command_pool = VK_NULL_HANDLE;
        m_handle_allocator.Free(pool_id);

        return;
    }

    Result CommandPoolManager::CreateCommandBuffer(VkCommandBuffer *out_vk_command_buffer, CommandPoolId pool_id, bool is_primary) {

        /* Reference command pool */
        VP_ASSERT(pool_id != cInvalidCommandPoolId);
        CommandPoolUnit *unit = std::addressof(m_unit_array[pool_id]);
        vp::util::InterlockedIncrement(std::addressof(unit->ref_count));
        const VkCommandPool vk_command_pool = unit->vk_command_pool;

        /* Allocate command buffer */
        Context *context = Context::GetInstance();
        const VkCommandBufferAllocateInfo allocate_info {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = vk_command_pool,
            .level              = (is_primary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
            .commandBufferCount = 1,
        };
        const VkResult vk_result = ::pfn_vkAllocateCommandBuffers(context->GetVkDevice(), std::addressof(allocate_info), out_vk_command_buffer);
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));

        RESULT_RETURN_SUCCESS;
    }

    void CommandPoolManager::FreeCommandBuffer(VkCommandBuffer vk_command_buffer, CommandPoolId pool_id) {

        /* Free command buffer */
        VP_ASSERT(vk_command_buffer != VK_NULL_HANDLE);
        Context *context = Context::GetInstance();
        CommandPoolUnit *unit = std::addressof(m_unit_array[pool_id]);
        ::pfn_vkFreeCommandBuffers(context->GetVkDevice(), unit->vk_command_pool, 1, std::addressof(vk_command_buffer));
        this->FreeCommandPool(pool_id);

        return;
    }
}

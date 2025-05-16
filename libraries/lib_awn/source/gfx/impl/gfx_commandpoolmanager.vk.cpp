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

    CommandList CommandPoolManager::CreateCommandList(QueueType queue_type, CommandPoolId pool_id, bool is_primary) {

        /* Allocate new command pool unit if one doesn't exist */
        Context *context = Context::GetInstance();
        {
            std::scoped_lock l(m_command_pool_cs);
            if (pool_id == cInvalidHandle) {
                const u32 handle = m_handle_allocator.Allocate();
                VP_ASSERT(handle != HandleAllocator::cInvalidEntryIndex);
                CommandPoolUnit *unit = std::addressof(m_unit_array[handle]);
                unit->queue_type = queue_type;
                unit->ref_count = 1;
                const VkCommandPoolCreateInfo command_pool_create_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .queueFamilyIndex = context->GetQueueFamilyIndex(queue_type),
                };
                const VkResult vk_result = ::pfn_vkCreateCommandPool(context->GetVkDevice(), std::addressof(command_pool_create_info), context->GetVkAllocationCallbacks(), std::addressof(unit->vk_command_pool));
                RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(vk_result));
                pool_id = handle;
            } else {
                VP_ASSERT(m_unit_array[pool_id].vk_command_pool != VK_NULL_HANDLE);
                VP_ASSERT(m_unit_array[pool_id].queue_type == queue_type);
                m_unit_array[pool_id].ref_count += 1;
            }
        }

        /* Get command pool */
        VkCommandPool vk_command_pool = m_unit_array[pool_id].vk_command_pool;

        /* Allocate command buffer */
        const VkCommandBufferAllocateInfo allocate_info {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = vk_command_pool,
            .level              = (is_primary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
        const VkResult vk_result = ::pfn_vkAllocateCommandBuffers(context->GetVkDevice(), std::addressof(allocate_info), std::addressof(vk_command_buffer));
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(vk_result));

        return { vk_command_buffer, pool_id, };
    }

    void CommandPoolManager::FinalizeCommandList(CommandList command_list) {

        VP_ASSERT(command_list.IsValid());

        /* Free secondary command buffer */
        Context *context = Context::GetInstance();
        CommandPoolUnit *unit = std::addressof(m_unit_array[command_list.pool_id]);
        ::pfn_vkFreeCommandBuffers(context->GetVkDevice(), unit->vk_command_pool, 1, std::addressof(command_list.vk_command_buffer));

        /* Free pool */
        {            
            std::scoped_lock l(m_command_pool_cs);
            const u32 next_ref = unit->ref_count - 1;
            unit->ref_count = next_ref;
            if (next_ref == 0) {
                ::pfn_vkDestroyCommandPool(context->GetVkDevice(), unit->vk_command_pool, context->GetVkAllocationCallbacks());
                unit->vk_command_pool = VK_NULL_HANDLE;
                m_handle_allocator.Free(command_list.pool_id);
            }
        }

        return;
    }
}

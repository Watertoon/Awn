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
    struct CommandList;
}

namespace awn::gfx::impl {

    static constexpr u32 cMaxCommandBufferCount = 0x8000;
    using CommandPoolId = vp::util::MinUIntType<cMaxCommandBufferCount>;

    class CommandPoolManager {
        public:
            struct CommandPoolUnit {
                u32 ref_count;
                VkCommandPool vk_command_pool;
                QueueType queue_type;
            };
        public:
            static constexpr CommandPoolId cInvalidHandle = static_cast<CommandPoolId>(0xffff'ffff'ffff'ffff);
            static constexpr size_t cMaxCommandBufferThreadCount = 0x80;
        public:
            using HandleAllocator = vp::util::IndexAllocator<CommandPoolId>;
            using CommandPoolUnitArray = vp::util::HeapArray<CommandPoolUnit>;
        private:
            sys::Mutex m_command_pool_cs;
            HandleAllocator m_handle_allocator;
            CommandPoolUnitArray m_unit_array;
        public:
            AWN_SINGLETON_TRAITS(CommandPoolManager);
        public:
            constexpr ALWAYS_INLINE  CommandPoolManager() : m_command_pool_cs(), m_handle_allocator(), m_unit_array() {/*...*/}
            constexpr ALWAYS_INLINE ~CommandPoolManager() {/*...*/}

            void Initialize(mem::Heap *heap, u32 pool_count);
            void Finalize();

            CommandList CreateCommandList(QueueType queue_type, CommandPoolId pool_id, bool is_primary);
            void FinalizeCommandList(CommandList command_list);

            constexpr QueueType GetQueueType(CommandPoolId pool_id) {
                VP_ASSERT(pool_id != cInvalidHandle);
                return m_unit_array[pool_id].queue_type;
            }
    };
}

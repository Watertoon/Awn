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
#include <vp.hpp>

namespace vp::resbui {

    void NintendoWareStringPoolBuilder::ResolveCollisions() {

        /* Ensure all collisions are collisions */
        for (StringPoolStringBase &collided_string : m_collision_list) {

            StringPoolStringBase *no_longer_colliding_string = std::addressof(collided_string);
            const u32 col_length = collided_string.GetLength();
            for (StringPoolStringBase &string : m_string_list) {
                if (col_length != string.GetLength() || !vp::util::StringCompare(collided_string.GetString(), string.GetString())) { continue; }
                no_longer_colliding_string = nullptr;
                break;
            }

            /* Readd string if it is no longer colliding */
            if (no_longer_colliding_string != nullptr) {
                no_longer_colliding_string->m_pool_builder_node.Unlink();
                RESULT_ABORT_UNLESS(this->AddString(no_longer_colliding_string));
            }
        }

        return;
    }

    void NintendoWareStringPoolBuilder::Finalize() {
        m_string_list.Clear();
        m_collision_list.Clear();
    }

    Result NintendoWareStringPoolBuilder::AddString(StringPoolStringBase *pool_string) {

        /* Integrity checks */
        RESULT_RETURN_IF(pool_string == nullptr,                      ResultNullArgument);
        RESULT_RETURN_IF(pool_string->m_pool_builder_node.IsLinked(), ResultAlreadyLinked);

        /* Check against unique strings */
        const u32 new_length = pool_string->GetLength();
        const char *new_string = pool_string->GetString();
        for (StringPoolStringBase &string : m_string_list) {
            if (new_length == string.GetLength() && vp::util::StringCompare(string.GetString(), new_string)) {
                /* Add to duplicate list */
                m_collision_list.PushBack(*pool_string);
                RESULT_RETURN_SUCCESS;
            }
        }

        /* Add to unique list */
        m_string_list.PushBack(*pool_string);

        RESULT_RETURN_SUCCESS;
    }

    void NintendoWareStringPoolBuilder::Serialize(const NintendoWareStringPoolBuilderMemoryInfo *memory_info, void *output_buffer, size_t buffer_size) {
        VP_UNUSED(buffer_size);

        /* Calculate location offsets */
        vp::res::ResNintendoWareStringPool *header      = reinterpret_cast<vp::res::ResNintendoWareStringPool*>(reinterpret_cast<uintptr_t>(output_buffer) + memory_info->location_header.offset);
        char                               *string_pool = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(output_buffer) + memory_info->location_string_pool.offset);

        /* Write header */
        header->magic        = vp::res::ResNintendoWareStringPool::cMagic;
        header->string_count = memory_info->string_count;
        header->section_size = static_cast<u32>(memory_info->total_size);

        /* Stream out unique strings */
        char *pool_iter = string_pool;
        for (StringPoolStringBase &string : m_string_list) {
            const u16 length = string.GetLength();
            *reinterpret_cast<u16*>(pool_iter) = length;
            ::strcpy(pool_iter + sizeof(u16), string.GetString());
            *(pool_iter + length + sizeof(u16)) = '\0';
            pool_iter = vp::util::AlignUp(pool_iter + sizeof(u16) + length + 1, alignof(u16));
        }

        return;
    }

    void NintendoWareStringPoolBuilder::CalculateMemoryInfo(NintendoWareStringPoolBuilderMemoryInfo *memory_info) {

        /* Calculate locations */
        size_t size = 0;
        this->CalculateHeaderLocation(std::addressof(memory_info->location_header), 0);
        size = vp::util::AlignUp(size + memory_info->location_header.size, alignof(u32));

        this->ResolveCollisions();
        const u32 count = this->CalculateStringPoolLocation(std::addressof(memory_info->location_string_pool), size);
        size = vp::util::AlignUp(size + memory_info->location_string_pool.size, alignof(size_t));

        /* Set output */
        memory_info->total_size    = size;
        memory_info->max_alignment = alignof(size_t);
        memory_info->string_count  = count;
    }
}

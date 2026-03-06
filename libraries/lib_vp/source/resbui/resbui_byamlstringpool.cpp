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

	void ByamlStringPoolBuilder::ResolveCollisions() {

		/* Ensure all collisions are collisions */
		for (auto str_iter = m_collision_list.begin(); str_iter != m_collision_list.end(); ) {

			ByamlStringPoolString *collided_string = std::addressof(*str_iter);
			ByamlStringPoolString *no_longer_colliding_string = collided_string;
            ++str_iter;

			for (ByamlStringPoolString &string : m_string_list) {
				if (::strcmp(collided_string->GetString(), string.GetString()) != 0) { continue; }
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

	Result ByamlStringPoolBuilder::AddString(ByamlStringPoolString *pool_string) {

		/* Non-sorted insertion */
		if (m_is_sorted == false) {

			/* Check for collisions */    
			for (ByamlStringPoolString &string : m_string_list) {
				if (::strcmp(string.GetString(), pool_string->GetString()) != 0) { continue; }
				m_collision_list.PushBack(*pool_string);
				RESULT_RETURN_SUCCESS;
			}

			/* Insert */
			m_string_list.PushBack(*pool_string);

			RESULT_RETURN_SUCCESS;
		}

		/* Find insert location */
		ByamlStringPoolString *prev_string = nullptr;
		for (ByamlStringPoolString &string : m_string_list) {
            const s32 result = ::strcmp(string.GetString(), pool_string->GetString());
			if (result < 0) { prev_string = std::addressof(string); continue; }
			if (result > 0) { break; }
            m_collision_list.PushBack(*pool_string);
            RESULT_RETURN_SUCCESS;
		}

		/* Insert */
        if (prev_string == nullptr) {
			m_string_list.PushFront(*pool_string);
		} else {
			prev_string->m_pool_builder_node.LinkNext(std::addressof(pool_string->m_pool_builder_node));
		}

		RESULT_RETURN_SUCCESS;
	}

	void ByamlStringPoolBuilder::Serialize(void *output_buffer, [[maybe_unused]] size_t buffer_size, ByamlStringPoolBuilderMemoryInfo *memory_info) {

		/* Nothing to serialize if no strings */
		if (m_string_list.IsEmpty()) { return; }

		/* Calculate locations */
        const u32 string_array_offset                 = sizeof(vp::res::ResByamlContainer) + sizeof(u32) * (memory_info->string_count + 1);
		vp::res::ResByamlContainer *header            = reinterpret_cast<vp::res::ResByamlContainer*>(output_buffer);
		u32                        *offset_array      = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(output_buffer) + sizeof(vp::res::ResByamlContainer));
		char                       *string_array_base = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(output_buffer) + string_array_offset);

		/* Set header */
		header->data_type = static_cast<u8>(vp::res::ByamlDataType::KeyTable);
		header->count     = memory_info->string_count;
		if (m_is_reverse_endian) { header->SwapEndian(); }

		/* Stream out offset and string arrays */
		u32 i           = 1;
		u32 string_iter = 0;
		offset_array[0] = (!m_is_reverse_endian) ? string_array_offset : vp::util::SwapEndian(string_array_offset);
		for (ByamlStringPoolString &string : m_string_list) {
			
			/* Calculate offsets */
			const u32  size        = string.GetLength();
			char      *string_base = string_array_base + string_iter;

			/* Serialize string */
			::memcpy(string_base, string.GetString(), size);
			string_base[size] = '\0';

			/* Serialize container offset to string */
			const u32 iter_offset = string_iter + size + 1;
			const u32 offset      = string_array_offset + iter_offset;
			offset_array[i]  = (!m_is_reverse_endian) ? offset : vp::util::SwapEndian(offset);

			/* Record string index to string */
            const u32 key_index = i - 1;
			string.m_index = key_index;
            for (ByamlStringPoolString &collide_string : m_collision_list) {
                if (::strcmp(collide_string.GetString(), string_base) == 0) { collide_string.m_index = key_index;}
            }

			/* Advance iterators */
			string_iter = iter_offset;
			++i;
		}

		return;
	}

	void ByamlStringPoolBuilder::CalculateMemoryInfo(ByamlStringPoolBuilderMemoryInfo *out_memory_info) {

		/* Resolve any changed collisions */
		this->ResolveCollisions();

		/* Nothing if no string */
		if (m_string_list.IsEmpty() == true) { return; }

		/* Calculate count and size */
		size_t string_array_size = 0;
		u32 count = 0;
		for (ByamlStringPoolString &string : m_string_list) {
			string_array_size += string.GetLength() + 1;
			++count;
		}

		/* Set string count */
		out_memory_info->string_count = count;

		/* Set max */
		out_memory_info->total_size    = vp::util::AlignUp(sizeof(vp::res::ResByamlContainer) + sizeof(u32) * (count + 1) + string_array_size, alignof(u32));
		out_memory_info->max_alignment = alignof(u32);

		return;
	}
}

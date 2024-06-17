#include <vp.hpp>

namespace vp::resbui {

	void ByamlStringPoolBuilder::ResolveCollisions() {

		/* Ensure all collisions are collisions */
		for (ByamlStringPoolString &collided_string : m_collision_list) {

			ByamlStringPoolString *no_longer_colliding_string = std::addressof(collided_string);
			for (ByamlStringPoolString &string : m_string_list) {
				if (::strcmp(collided_string.GetString(), string.GetString()) != 0) { continue; }
				no_longer_colliding_string = nullptr;
				break;
			}

			/* Readd string if it is no longer colliding */
			if (no_longer_colliding_string != nullptr) {
				no_longer_colliding_string->m_pool_builder_node.Unlink();
				this->AddString(no_longer_colliding_string);
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
		ByamlStringPoolString *prev_string = std::addressof(m_string_list.Back());
		for (ByamlStringPoolString &string : m_string_list) {
			if (::strcmp(string.GetString(), pool_string->GetString()) < 0) { prev_string = std::addressof(string); continue; }
			break;
		}

		/* Insert */
		if (prev_string == std::addressof(m_string_list.Back())) {
			m_collision_list.PushBack(*pool_string);
		} else {
			prev_string->m_pool_builder_node.LinkNext(std::addressof(pool_string->m_pool_builder_node));
		}

		RESULT_RETURN_SUCCESS;
	}

	void ByamlStringPoolBuilder::Serialize(void *output_buffer, [[maybe_unused]] size_t buffer_size, ByamlStringPoolBuilderMemoryInfo *memory_info) {

		/* Nothing to serialize if no strings */
		if (m_string_list.IsEmpty() == true) { return; }

		/* Calculate locations */
		vp::res::ResByamlContainer *header            = reinterpret_cast<vp::res::ResByamlContainer*>(output_buffer);
		u32                        *offset_array      = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(output_buffer) + sizeof(vp::res::ResByamlContainer));
		char                       *string_array_base = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(output_buffer) + sizeof(vp::res::ResByamlContainer) + sizeof(u32) * memory_info->string_count);

		/* Set header */
		header->data_type = static_cast<u8>(vp::res::ByamlDataType::KeyTable);
		header->count     = memory_info->string_count;
		if (m_is_reverse_endian) { header->SwapEndian(); }

		/* Stream out offset and string arrays */
		u32 i           = 1;
		u32 string_iter = 0;
		offset_array[0] = string_iter;
		for (ByamlStringPoolString &string : m_string_list) {
			
			/* Calculate offsets */
			const u32  size        = string.GetStringLength();
			char      *string_base = string_array_base + string_iter;

			/* Serialize string */
			::memcpy(string_base, string.GetString(), size);
			string_base[size] = '\0';

			/* Serialize container offset to string */
			const u32 offset = static_cast<u32>(reinterpret_cast<uintptr_t>(string_base) - reinterpret_cast<uintptr_t>(header));
			offset_array[i]  = (!m_is_reverse_endian) ? offset : vp::util::SwapEndian(offset);

			/* Record string index to string */
			string.m_index    = i - 1;

			/* Advance iterators */
			string_iter       = string_iter + size + 1;
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
			string_array_size += string.GetStringLength() + 1;
			++count;
		}

		/* Set string count */
		out_memory_info->string_count = count;

		/* Set max */
		out_memory_info->total_size    = vp::util::AlignUp(sizeof(vp::res::ResByamlContainer) + sizeof(u32) * count + string_array_size, alignof(u32));
		out_memory_info->max_alignment = alignof(u32);

		return;
	}
}

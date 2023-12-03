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

namespace vp::resbui {

    class ByamlStringPoolBuilder;

    class ByamlStringPoolString : public StringPoolString {
        public:
            friend class ByamlStringPoolBuilder;
        public:
            static constexpr u32 cInvalidIndex = 0xffff'ffff;
        private:
            u32 m_index;
        public:
            constexpr ByamlStringPoolString() : StringPoolString(), m_index(cInvalidIndex) {/*...*/}
            constexpr ByamlStringPoolString(const char *string) : StringPoolString(string), m_index(cInvalidIndex) {/*...*/}
            virtual constexpr ~ByamlStringPoolString() override {/*...*/}

            constexpr u32 GetKeyIndex() const { return m_index; }
    };

    struct ByamlStringPoolBuilderMemoryInfo {
        size_t total_size;
        size_t max_alignment;
        u32    string_count;
    };

	class ByamlStringPoolBuilder {
        public:
            using StringList = vp::util::IntrusiveListTraits<ByamlStringPoolString, &ByamlStringPoolString::m_pool_builder_node>::List;
		private:
			StringList m_string_list;
			StringList m_collision_list;
            bool       m_is_sorted;
        public:
            void ResolveCollisions() {

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
		public:
			explicit constexpr ByamlStringPoolBuilder(bool is_sorted) : m_string_list(), m_collision_list(), m_is_sorted(is_sorted) {/*...*/}
			constexpr ~ByamlStringPoolBuilder() { this->Finalize(); }

            constexpr void Finalize() {
                m_string_list.Clear();
                m_collision_list.Clear();
            }

			Result AddString(ByamlStringPoolString *pool_string) {

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

            void Serialize(void *output_buffer, [[maybe_unused]] size_t buffer_size, ByamlStringPoolBuilderMemoryInfo *memory_info) {

                /* Nothing to serialize if no strings */
                if (m_string_list.IsEmpty() == true) { return; }

                /* Calculate locations */
                vp::res::ResByamlContainer *header            = reinterpret_cast<vp::res::ResByamlContainer*>(output_buffer);
                u32                        *offset_array      = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(output_buffer) + sizeof(vp::res::ResByamlContainer));
                char                       *string_array_base = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(output_buffer) + sizeof(vp::res::ResByamlContainer) + sizeof(u32) * memory_info->string_count);

                /* Set header */
                header->data_type = static_cast<u8>(vp::res::ByamlDataType::KeyTable);
                header->count     = memory_info->string_count;

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
                    offset_array[i]   = static_cast<u32>(reinterpret_cast<uintptr_t>(string_base) - reinterpret_cast<uintptr_t>(header));

                    /* Record string index to string */
                    string.m_index    = i - 1;
    
                    /* Advance iterators */
                    string_iter       = string_iter + size + 1;
                    ++i;
                }

                return;
            }

            void CalculateMemoryInfo(ByamlStringPoolBuilderMemoryInfo *out_memory_info) {

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
	};
}

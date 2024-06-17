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
            bool       m_is_reverse_endian;
        public:
            void ResolveCollisions();
		public:
			explicit constexpr ByamlStringPoolBuilder(bool is_sorted) : m_string_list(), m_collision_list(), m_is_sorted(is_sorted), m_is_reverse_endian(false) {/*...*/}
			constexpr ~ByamlStringPoolBuilder() { this->Finalize(); }

            constexpr void Finalize() {
                m_string_list.Clear();
                m_collision_list.Clear();
            }

            constexpr void SetIsReverseEndian(bool is_reverse_endian) { m_is_reverse_endian = is_reverse_endian; }

			Result AddString(ByamlStringPoolString *pool_string);

            void Serialize(void *output_buffer, [[maybe_unused]] size_t buffer_size, ByamlStringPoolBuilderMemoryInfo *memory_info);

            void CalculateMemoryInfo(ByamlStringPoolBuilderMemoryInfo *out_memory_info);
    };
}

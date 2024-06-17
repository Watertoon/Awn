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

	struct ByamlBuilderMemoryInfo {
		size_t 		   total_size;
		size_t 		   max_alignment;
		BufferLocation location_header;
		BufferLocation location_key_table;
		BufferLocation location_string_pool;
		BufferLocation location_big_data;
		BufferLocation location_container;
	};

	class ByamlBuilder {
		private:
			u16            m_version;
			bool           m_is_reverse_endian;
			ByamlNodeBase *m_root_node;
		public:
			constexpr  ByamlBuilder() : m_version(vp::res::ResByaml::cTargetVersion), m_is_reverse_endian(false), m_root_node() {/*...*/}
			constexpr ~ByamlBuilder() {/*...*/}

            constexpr bool IsValid() const {
                return (m_root_node != nullptr);
            }

            constexpr void SetRootNode(ByamlNodeBase *node) {
                m_root_node = node;
            }
            constexpr void SetRootNode(bool is_reverse_endian) {
                m_is_reverse_endian = is_reverse_endian;
            }
            constexpr void SetVersion(u16 version) {
                m_version = version;
            }

			void Serialize(void *output, size_t output_size, ByamlBuilderMemoryInfo *memory_info);

            void CalculateMemoryInfo(ByamlBuilderMemoryInfo *out_memory_info);
	};
}

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
		size_t total_size;
		size_t max_alignment;
        size_t offset_header;
        size_t offset_key_table;
        size_t offset_string_pool;
        size_t offset_big_data;
        size_t offset_container;
        
        constexpr size_t GetTotalSize() const { return this->total_size; }
        constexpr size_t GetMaxAlignment() const { return this->max_alignment; }
	};

	class ByamlBuilder {
        public:
            struct Cache {
                ByamlStringPoolBuilderMemoryInfo key_table_memory_info;
                ByamlStringPoolBuilderMemoryInfo string_table_memory_info;
                bool is_cache;
            };
		private:
			u16 m_version;
			bool m_is_reverse_endian;
			ByamlNodeBase *m_root_node;
            ByamlStringPoolBuilder m_key_table_builder;
            ByamlStringPoolBuilder m_string_table_builder;

            Cache m_cache;
		public:
			constexpr  ByamlBuilder() : m_version(vp::res::ResByaml::cTargetVersion), m_is_reverse_endian(false), m_root_node(), m_key_table_builder(true), m_string_table_builder(true), m_cache() {/*...*/}
			constexpr ~ByamlBuilder() {/*...*/}

            constexpr bool IsValid() const {
                return (m_root_node != nullptr);
            }

            constexpr void SetRootNode(ByamlNodeBase *node) {
                m_root_node = node;
            }
            constexpr void SetIsReverseEndian(bool is_reverse_endian) {
                m_is_reverse_endian = is_reverse_endian;
            }
            constexpr void SetVersion(u16 version) {
                m_version = version;
            }

            void CalculateMemoryInfo(ByamlBuilderMemoryInfo *out_memory_info);
			void Serialize(void *output, size_t output_size, ByamlBuilderMemoryInfo *memory_info);
	};
}

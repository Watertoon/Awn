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
			ByamlNodeBase *m_root_node;
		public:
			constexpr  ByamlBuilder() : m_root_node() {/*...*/}
			constexpr ~ByamlBuilder() {/*...*/}

            constexpr void SetRootNode(ByamlNodeBase *node) {
                m_root_node = node;
            }

			void Serialize(void *output, size_t output_size, ByamlBuilderMemoryInfo *memory_info) {

                /* Clear memory */
                ::memset(output, 0, output_size);

                /* Get regions */
                vp::res::ResByaml *head           = reinterpret_cast<vp::res::ResByaml*>(reinterpret_cast<uintptr_t>(output) + memory_info->location_header.offset);
                void              *key_table      = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output) + memory_info->location_key_table.offset);
                void              *string_pool    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output) + memory_info->location_string_pool.offset);
                void              *big_data_base  = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output) + memory_info->location_big_data.offset);
                void              *container_base = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output) + memory_info->location_container.offset);

                /* Set header info */
                head->magic   = vp::res::ResByaml::cMagic;
                head->version = vp::res::ResByaml::cTargetVersion;

                /* Nothing else if null root */
                if (m_root_node == nullptr) { return; }

                /* Handle key table */
                if (memory_info->location_key_table.size != 0) {

                    /* Gather key table */
                    ByamlStringPoolBuilder key_table_builder(true);
                    m_root_node->PushKeys(std::addressof(key_table_builder));

                    /* Calculate memory info */
                    ByamlStringPoolBuilderMemoryInfo key_table_memory_info = {};
                    key_table_builder.CalculateMemoryInfo(std::addressof(key_table_memory_info));

                    /* Serialize */
                    key_table_builder.Serialize(key_table, memory_info->location_key_table.size, std::addressof(key_table_memory_info));

                    /* Finalize builder */
                    key_table_builder.Finalize();

                    /* Set header offset */
                    head->key_table_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(key_table) - reinterpret_cast<uintptr_t>(head));
                }

                /* Handle string pool */
                if (memory_info->location_string_pool.size != 0) {

                    /* Gather string pool */
                    ByamlStringPoolBuilder string_pool_builder(false);
                    m_root_node->PushStrings(std::addressof(string_pool_builder));

                    /* Calculate memory info */
                    ByamlStringPoolBuilderMemoryInfo string_pool_memory_info = {};
                    string_pool_builder.CalculateMemoryInfo(std::addressof(string_pool_memory_info));

                    /* Serialize */
                    string_pool_builder.Serialize(string_pool, memory_info->location_string_pool.size, std::addressof(string_pool_memory_info));

                    /* Finalize builder */
                    string_pool_builder.Finalize();

                    /* Set header offset */
                    head->string_table_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(string_pool) - reinterpret_cast<uintptr_t>(head));
                }

                /* Complete if the container is empty */
                if (memory_info->location_container.size == 0) { return; }

                /* Check if big data */
                if (ByamlNodeBigData::CheckRuntimeTypeInfo(m_root_node) == true) {

                    /* Serialize big data */
                    uintptr_t big_data_iter  = reinterpret_cast<uintptr_t>(big_data_base);
                    uintptr_t container_iter = reinterpret_cast<uintptr_t>(container_base);
                    reinterpret_cast<ByamlNodeBigData*>(m_root_node)->SerializeForBigData(std::addressof(big_data_iter), std::addressof(container_iter), head);
                }

                /* Set head */
                head->data_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(container_base) - reinterpret_cast<uintptr_t>(head));

                /* Write single data type for non container */
                void *root_value_offset           = container_base;
                res::ByamlDataType root_data_type = m_root_node->GetByamlDataType();
                if (res::IsContainerType(root_data_type) == false && root_data_type != res::ByamlDataType::Null) {

                    u8 *data_type     = reinterpret_cast<u8*>(container_base);
                    *data_type        = static_cast<u8>(root_data_type);
                    root_value_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(container_base) + sizeof(u32));
                }

                /* Serialize root value */
                m_root_node->Serialize(root_value_offset, head);

                return;
            }

            void CalculateMemoryInfo(ByamlBuilderMemoryInfo *out_memory_info) {

                /* Set header location */
                out_memory_info->location_header.offset    = 0;
                out_memory_info->location_header.size      = sizeof(vp::res::ResByaml);
                out_memory_info->location_header.alignment = alignof(u32);

                /* Nothins else if null root */
                if (m_root_node == nullptr) {
                    out_memory_info->total_size    = sizeof(vp::res::ResByaml);
                    out_memory_info->max_alignment = alignof(u32);
                    return;
                }

                /* Gather key table */
                ByamlStringPoolBuilder key_table_builder(true);
                m_root_node->PushKeys(std::addressof(key_table_builder));

                /* Calculate memory info */
                ByamlStringPoolBuilderMemoryInfo key_table_memory_info = {};
                key_table_builder.CalculateMemoryInfo(std::addressof(key_table_memory_info));

                /* Finalize builder */
                key_table_builder.Finalize();

                /* Set key table location */
                out_memory_info->location_key_table.offset    = out_memory_info->location_header.size;
                out_memory_info->location_key_table.size      = key_table_memory_info.total_size;
                out_memory_info->location_key_table.alignment = key_table_memory_info.max_alignment;

                /* Gather string pool */
                ByamlStringPoolBuilder string_pool_builder(false);
                m_root_node->PushStrings(std::addressof(string_pool_builder));

                /* Calculate memory info */
                ByamlStringPoolBuilderMemoryInfo string_pool_memory_info = {};
                string_pool_builder.CalculateMemoryInfo(std::addressof(string_pool_memory_info));

                /* Finalize builder */
                string_pool_builder.Finalize();

                /* Set string pool location */
                out_memory_info->location_string_pool.offset    = out_memory_info->location_key_table.offset + out_memory_info->location_key_table.size;
                out_memory_info->location_string_pool.size      = string_pool_memory_info.total_size;
                out_memory_info->location_string_pool.alignment = string_pool_memory_info.max_alignment;

                const size_t big_data_base = out_memory_info->location_string_pool.offset + out_memory_info->location_string_pool.size;
                uintptr_t container_iter  = big_data_base;
                uintptr_t big_data_iter   = big_data_base;
                if (ByamlNodeBigData::CheckRuntimeTypeInfo(m_root_node) == true) {

                    /* Calculate big data and container end */
                    reinterpret_cast<ByamlNodeBigData*>(m_root_node)->CalculateBigDataEndOffsets(std::addressof(big_data_iter), std::addressof(container_iter), 0);

                    container_iter  = big_data_iter;
                    big_data_iter   = big_data_base;
                    reinterpret_cast<ByamlNodeBigData*>(m_root_node)->CalculateBigDataEndOffsets(std::addressof(big_data_iter), std::addressof(container_iter), 0);

                    /* Set big data location */
                    out_memory_info->location_big_data.offset    = big_data_base;
                    out_memory_info->location_big_data.size      = big_data_iter - big_data_base;
                    out_memory_info->location_big_data.alignment = alignof(u32);
                }

                /* Set container location */
                out_memory_info->location_container.offset    = big_data_iter;
                out_memory_info->location_container.size      = (container_iter - big_data_iter == 0) ? (sizeof(u32) * 2) : container_iter - big_data_iter;
                out_memory_info->location_container.alignment = alignof(u32);

                /* Set totals */
                out_memory_info->total_size    = out_memory_info->location_container.offset + out_memory_info->location_container.size;
                out_memory_info->max_alignment = alignof(u32);

                return;
            }
	};
}

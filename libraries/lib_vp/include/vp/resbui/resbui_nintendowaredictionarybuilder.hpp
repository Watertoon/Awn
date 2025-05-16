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

    struct NintendoWareDictionaryBuilderMemoryInfo {
        size_t         total_size;
        size_t         max_alignment;
        u32            total_relocation_count;
        BufferLocation location_header;
        BufferLocation location_node_array;
    };

    class NintendoWareDictionaryBuilder {
        public:
            using DictionaryNodeList = vp::util::IntrusiveListTraits<DictionaryStringBase, &DictionaryStringBase::m_dictionary_builder_node>::List;
        private:
            DictionaryNodeList m_node_list;
            u32 m_node_count;
        public:
            constexpr ALWAYS_INLINE void CalculateHeaderLocation(BufferLocation *out_location, size_t offset) {
                out_location->offset           = offset;
                out_location->size             = sizeof(vp::res::ResNintendoWareDictionary);
                out_location->alignment        = alignof(size_t);
                out_location->relocation_count = 0;
            }
            constexpr ALWAYS_INLINE void CalculateNodeArrayLocation(BufferLocation *out_location, size_t offset) {
                out_location->offset           = offset;
                out_location->size             = sizeof(vp::res::ResNintendoWareDictionary::ResDicNode) * m_node_count;
                out_location->alignment        = alignof(size_t);
                out_location->relocation_count = 1;
            }
        public:
            constexpr  NintendoWareDictionaryBuilder() : m_node_list(), m_node_count() {/*...*/}
            constexpr ~NintendoWareDictionaryBuilder() { this->Finalize(); }

            constexpr void Finalize() {
                m_node_list.Clear();
                m_node_count = 0;
            }

            Result AddNode(DictionaryStringBase *dictionary_string);
            void Remove(DictionaryStringBase *dictionary_string);
            bool IsSame(vp::res::ResNintendoWareDictionary *res_dic);
            bool IsSame(NintendoWareDictionaryBuilder *builder);
            constexpr u32 GetCount() const { return m_node_count; }
        public:
            void Serialize(NintendoWareDictionaryBuilderMemoryInfo *memory_info, NintendoWareRelocationTableStream *relocation_table_stream, const vp::res::ResNintendoWareStringPool *string_pool, void *output_buffer, size_t buffer_size);

            constexpr void CalculateMemoryInfo(NintendoWareDictionaryBuilderMemoryInfo *out_memory_info) {

                /* Calculate locations */
                size_t size = 0;
                this->CalculateHeaderLocation(std::addressof(out_memory_info->location_header), 0);
                size = size + out_memory_info->location_header.size;

                this->CalculateNodeArrayLocation(std::addressof(out_memory_info->location_node_array), size);
                size = size + out_memory_info->location_node_array.size;

                /* Set totals */
                out_memory_info->total_size    = size;
                out_memory_info->max_alignment = alignof(size_t);
                out_memory_info->total_relocation_count = 1;
                
                return;
            }

            static constexpr size_t GetDictionarySize(u32 node_count) {
                return sizeof(vp::res::ResNintendoWareDictionary) + node_count * sizeof(vp::res::ResNintendoWareDictionary::ResDicNode);
            }
    };
}

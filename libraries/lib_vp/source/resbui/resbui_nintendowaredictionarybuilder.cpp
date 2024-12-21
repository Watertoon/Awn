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

    Result NintendoWareDictionaryBuilder::AddNode(DictionaryStringBase *dictionary_string) {

        /* Integrity checks */
        RESULT_RETURN_IF(dictionary_string == nullptr, ResultNullArgument);
        const char *new_string = dictionary_string->GetString();
        RESULT_RETURN_IF(new_string == nullptr || new_string[0] == '\0', ResultNullArgument);

        for (DictionaryStringBase &string : m_node_list) {
            const char *other_string = string.GetString();
            RESULT_RETURN_IF(::strcmp(new_string, other_string) == 0, ResultDuplicateString);
        }

        /* Insert new node */
        m_node_list.PushBack(*dictionary_string);

        RESULT_RETURN_SUCCESS;
    }

    void NintendoWareDictionaryBuilder::Serialize(NintendoWareDictionaryBuilderMemoryInfo *memory_info, NintendoWareRelocationTableStream *relocation_table_stream, vp::res::ResNintendoWareStringPool *string_pool, void *output_buffer, size_t buffer_size) {
        VP_UNUSED(buffer_size);

        /* Calculate locations */
        vp::res::ResNintendoWareDictionary             *header     = reinterpret_cast<vp::res::ResNintendoWareDictionary*>(reinterpret_cast<uintptr_t>(output_buffer) + memory_info->location_header.offset);
        vp::res::ResNintendoWareDictionary::ResDicNode *node_array = reinterpret_cast<vp::res::ResNintendoWareDictionary::ResDicNode*>(reinterpret_cast<uintptr_t>(output_buffer) + memory_info->location_node_array.offset);

        /* Relocate strings into nodes */
        u32 i = 0;
        for (DictionaryStringBase &string : m_node_list) {
            node_array[i].key = string_pool->FindString(string.GetString());
            ++i;
        }

        /* Set header */
        header->magic = vp::res::ResNintendoWareDictionary::cMagic;
        header->node_count = i;
        header->root_node.key = string_pool->FindString("");

        /* Build dictionary */
        const bool build_result = header->Build();
        VP_ASSERT(build_result == true);

        /* Add relocation entry */
        relocation_table_stream->AddEntry(std::addressof(header->root_node.key), i + 1, 1, 1);

        return;
    }
}

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

    bool NintendoWareDictionaryBuilder::IsSame(vp::res::ResNintendoWareDictionary *res_dic) {
        if (res_dic->node_count != static_cast<s32>(m_node_count)) { return false; }
        u32 i = 0;
        for (DictionaryStringBase &string : m_node_list) {
            const char *other_string = std::addressof(res_dic->root_node)[1 + i].key + 2;
            ++i;
            if (!vp::util::StringCompare(string.GetString(), other_string)) { return false; }
        }
        return true;
    }

    bool NintendoWareDictionaryBuilder::IsSame(NintendoWareDictionaryBuilder *builder) {
        if (builder->m_node_count != m_node_count) { return false; }
        auto iter = m_node_list.begin();
        auto iter_end = m_node_list.end();
        auto other_iter = builder->m_node_list.begin();
        auto other_end = builder->m_node_list.end();
        while (iter != iter_end && other_iter != other_end) {
            const char *string = (*iter).GetString();
            const char *other_string = (*other_iter).GetString();
            ++iter;
            ++other_iter;
            if (!vp::util::StringCompare(string, other_string)) { return false; }
        }
        if (iter != iter_end || other_iter != other_end) { return false; }
        return true;
    }

    Result NintendoWareDictionaryBuilder::AddNode(DictionaryStringBase *dictionary_string) {

        /* Integrity checks */
        RESULT_RETURN_IF(dictionary_string == nullptr, ResultNullArgument);
        const char *new_string = dictionary_string->GetString();
        RESULT_RETURN_IF(new_string == nullptr || new_string[0] == '\0', ResultNullArgument);

        for (DictionaryStringBase &string : m_node_list) {
            const char *other_string = string.GetString();
            RESULT_RETURN_IF(vp::util::StringCompare(new_string, other_string), ResultDuplicateString);
        }

        /* Insert new node */
        m_node_list.PushBack(*dictionary_string);
        ++m_node_count;

        RESULT_RETURN_SUCCESS;
    }

    void NintendoWareDictionaryBuilder::Remove(DictionaryStringBase *dictionary_string) {
        if (!dictionary_string->m_dictionary_builder_node.IsLinked()) { return; }
        dictionary_string->m_dictionary_builder_node.Unlink();
        --m_node_count;
    }

    void NintendoWareDictionaryBuilder::Serialize(NintendoWareDictionaryBuilderMemoryInfo *memory_info, NintendoWareRelocationTableStream *relocation_table_stream, const vp::res::ResNintendoWareStringPool *string_pool, void *output_buffer, size_t buffer_size) {
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

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

    class SarcBuilder;

    class SarcFileNode {
        public:
            friend class SarcBuilder;
        private:
            vp::util::IntrusiveListNode  m_builder_node;
            const char                  *m_file_path;
            void                        *m_file;
            u32                          m_file_size;
            u32                          m_file_alignment;
        public:
            constexpr  SarcFileNode() : m_builder_node(), m_file_path(nullptr), m_file(nullptr), m_file_size(0), m_file_alignment(alignof(u32)) {/*...*/}
            constexpr ~SarcFileNode() {/*...*/}

            Result SetFile(const char *file_path, void *file, u32 file_size, u32 file_alignment);
    };

    struct SarcBuilderMemoryInfo {
        size_t         total_file_size;
        size_t         max_alignment;
        BufferLocation location_header;
        BufferLocation location_sfat;
        BufferLocation location_sfnt;
        BufferLocation location_file_region;
    };

    class SarcBuilder {
        public:
            static constexpr u32 cDefaultHashSeed = 0x65;
        private:
            using FileList = vp::util::IntrusiveListTraits<SarcFileNode, &SarcFileNode::m_builder_node>::List;
        public:
            FileList m_file_list;
            u32      m_hash_seed;
        public:
            constexpr  SarcBuilder() : m_file_list(), m_hash_seed(cDefaultHashSeed) {/*...*/}
            constexpr ~SarcBuilder() {/*...*/}

            Result AddFile(SarcFileNode *file_node);

            void Serialize(void *out_buffer, size_t buffer_size, SarcBuilderMemoryInfo *memory_info);

            void CalculateMemoryInfo(SarcBuilderMemoryInfo *out_memory_info);
    };
}

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

            Result SetFile(const char *file_path, void *file, u32 file_size, u32 file_alignment) {

                RESULT_RETURN_IF(m_builder_node.IsLinked() == true, ResultAlreadyLinked);

                m_file_path      = file_path;
                m_file           = file;
                m_file_size      = file_size;
                m_file_alignment = file_alignment;

                RESULT_RETURN_SUCCESS;
            }
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

            Result AddFile(SarcFileNode *file_node) {

                /* Integrity checks */
                RESULT_RETURN_IF(file_node == nullptr,                                                 ResultNullArgument);
                RESULT_RETURN_IF(file_node->m_file_path == nullptr || *file_node->m_file_path == '\0', ResultInvalidPath);

                /* Sorted insertion */
                const u32 path_hash = vp::res::CalculateSarcHash(m_hash_seed, file_node->m_file_path);
                for (SarcFileNode &node : m_file_list) {
                    RESULT_RETURN_IF(::strcmp(file_node->m_file_path, node.m_file_path) == 0, ResultDuplicatePath);
                    if (path_hash > vp::res::CalculateSarcHash(m_hash_seed, node.m_file_path)) { continue; }

                    node.m_builder_node.LinkPrev(std::addressof(file_node->m_builder_node));
                    RESULT_RETURN_SUCCESS;
                }

                /* Push to back if at the end */
                m_file_list.PushBack(*file_node);

                RESULT_RETURN_SUCCESS;
            }

            void Serialize(void *out_buffer, size_t buffer_size, SarcBuilderMemoryInfo *memory_info) {

                /* Integrity checks */
                VP_ASSERT(out_buffer != nullptr);
                VP_ASSERT(memory_info != nullptr);
                VP_ASSERT(memory_info->total_file_size <= buffer_size);

                /* Clear memory */
                ::memset(out_buffer, 0, memory_info->total_file_size);

                /* Setup locations */
                vp::res::ResSarc     *sarc        = reinterpret_cast<vp::res::ResSarc*>(reinterpret_cast<uintptr_t>(out_buffer) + memory_info->location_header.offset);
                vp::res::ResSarcSfat *sfat        = reinterpret_cast<vp::res::ResSarcSfat*>(reinterpret_cast<uintptr_t>(out_buffer) + memory_info->location_sfat.offset);
                vp::res::ResSarcSfnt *sfnt        = reinterpret_cast<vp::res::ResSarcSfnt*>(reinterpret_cast<uintptr_t>(out_buffer) + memory_info->location_sfnt.offset);
                uintptr_t             file_region = reinterpret_cast<uintptr_t>(out_buffer) + memory_info->location_file_region.offset;

                /* Write header */
                sarc->magic             = vp::res::ResSarc::cMagic;
                sarc->header_size       = sizeof(vp::res::ResSarc);
                sarc->endianess         = vp::res::ByteOrder_Native;
                sarc->file_size         = memory_info->total_file_size;
                sarc->file_array_offset = memory_info->location_file_region.offset;
                sarc->version           = vp::res::ResSarc::cTargetVersion;
                sarc->reserve0          = 0;

                /* Write sfat header */
                sfat->magic       = vp::res::ResSarcSfat::cMagic;
                sfat->header_size = sizeof(vp::res::ResSarcSfat);
                sfat->hash_seed   = m_hash_seed;

                /* Write sfnt header */
                sfnt->magic       = vp::res::ResSarcSfnt::cMagic;
                sfnt->header_size = sizeof(vp::res::ResSarcSfnt);

                /* Write file paths and file region */
                u32        file_count      = 0;
                u32        last_hash       = 0;
                u32        collision_count = 1;
                uintptr_t  sfnt_start      = reinterpret_cast<uintptr_t>(sfnt) + sizeof(vp::res::ResSarcSfnt);
                void      *sfnt_iter       = reinterpret_cast<void*>(sfnt_start);
                void      *file_iter       = reinterpret_cast<void*>(file_region);
                for (SarcFileNode &node : m_file_list) {

                    /* Handle collision */
                    const u32 hash = vp::res::CalculateSarcHash(m_hash_seed, node.m_file_path);

                    /* Check if the next element in the hash sorted list is the same hash */
                    if (0 < file_count && hash == last_hash) {
                        ++collision_count;
                        VP_ASSERT(collision_count < 0x100);
                    } else {
                        last_hash       = hash;
                        collision_count = 1;
                    }

                    /* Write sfat entry */
                    sfat->entry_array[file_count].file_name_hash          = hash;
                    sfat->entry_array[file_count].file_name_offset        = (reinterpret_cast<uintptr_t>(sfnt_iter) - sfnt_start) >> 2;
                    sfat->entry_array[file_count].hash_collision_index    = collision_count;

                    /* Write sfnt path */
                    const u32 string_size = ::strlen(node.m_file_path) + 1;
                    ::memcpy(sfnt_iter, node.m_file_path, string_size);
                    sfnt_iter = reinterpret_cast<void*>(vp::util::AlignUp(reinterpret_cast<uintptr_t>(sfnt_iter) + string_size, alignof(u32)));

                    /* Write file */
                    file_iter = reinterpret_cast<void*>(vp::util::AlignUp(reinterpret_cast<uintptr_t>(file_iter), node.m_file_alignment));
                    sfat->entry_array[file_count].file_array_start_offset = reinterpret_cast<uintptr_t>(file_iter) - file_region;

                    ::memcpy(file_iter, node.m_file, node.m_file_size);

                    sfat->entry_array[file_count].file_array_end_offset = reinterpret_cast<uintptr_t>(file_iter) + node.m_file_size - file_region;
                    file_iter = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(file_iter) + node.m_file_size);

                    ++file_count;
                }

                sfat->file_count  = file_count;

                return;
            }

            void CalculateMemoryInfo(SarcBuilderMemoryInfo *out_memory_info) {

                /* Calculate memory sizes */
                u32    file_count       = 0;
                u32    sfnt_size        = sizeof(vp::res::ResSarcSfnt);
                u32    file_region_size = 0;
                size_t max_align        = alignof(u32);
                for (SarcFileNode &node : m_file_list) {

                    /* Add string and file size */
                    sfnt_size = vp::util::AlignUp(sfnt_size + ::strlen(node.m_file_path) + 1, alignof(u32));

                    /* Add file size */
                    if (file_region_size != 0) {
                        file_region_size = vp::util::AlignUp(file_region_size, node.m_file_alignment);
                    }
                    file_region_size = file_region_size + node.m_file_size;

                    /* Set max align */
                    if (max_align < node.m_file_alignment) {
                        max_align = node.m_file_alignment;
                    }

                    /* Increment count */
                    ++file_count;
                }

                /* Calculate locations */
                out_memory_info->location_header.offset    = 0;
                out_memory_info->location_header.size      = sizeof(vp::res::ResSarc);
                out_memory_info->location_header.alignment = alignof(u32);

                out_memory_info->location_sfat.offset    = sizeof(vp::res::ResSarc);
                out_memory_info->location_sfat.size      = sizeof(vp::res::ResSarcSfat) + sizeof(vp::res::ResSarcSfatEntry) * file_count;
                out_memory_info->location_sfat.alignment = alignof(u32);

                out_memory_info->location_sfnt.offset    = out_memory_info->location_sfat.offset + out_memory_info->location_sfat.size;
                out_memory_info->location_sfnt.size      = sfnt_size;
                out_memory_info->location_sfnt.alignment = alignof(u32);

                out_memory_info->location_file_region.offset    = vp::util::AlignUp(out_memory_info->location_sfnt.offset + sfnt_size, m_file_list.Front().m_file_alignment);
                out_memory_info->location_file_region.size      = file_region_size - out_memory_info->location_file_region.offset;
                out_memory_info->location_file_region.alignment = alignof(u32);

                out_memory_info->total_file_size = out_memory_info->location_file_region.offset + out_memory_info->location_file_region.size;
                out_memory_info->max_alignment   = max_align;

                return;
            }
    };
}

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

    struct NintendoWareStringPoolBuilderMemoryInfo {
        size_t         total_size;
        size_t         max_alignment;
        u32            string_count;
        BufferLocation location_header;
        BufferLocation location_string_pool;
    };

    class NintendoWareStringPoolBuilder {
        public:
            using StringList = vp::util::IntrusiveListTraits<StringPoolStringBase, &StringPoolStringBase::m_pool_builder_node>::List;
        private:
            StringList m_string_list;
            StringList m_collision_list;
        public:
            constexpr ALWAYS_INLINE void CalculateHeaderLocation(BufferLocation *out_location, size_t offset) {
                out_location->offset           = offset;
                out_location->size             = sizeof(vp::res::ResNintendoWareStringPool);
                out_location->alignment        = alignof(size_t);
                out_location->relocation_count = 0;
            }
            constexpr ALWAYS_INLINE u32 CalculateStringPoolLocation(BufferLocation *out_location, size_t offset) {

                /* Calculate string pool size */
                size_t size = 0;
                u32 count = 0;
                for (StringPoolStringBase &string : m_string_list) {
                    size += vp::util::AlignUp(size + string.GetLength() + 2, sizeof(u16));
                    ++count;
                }

                out_location->offset           = offset;
                out_location->size             = size;
                out_location->alignment        = alignof(size_t);
                out_location->relocation_count = 0;
                return count;
            }
            void ResolveCollisions();
        public:
            constexpr NintendoWareStringPoolBuilder() : m_string_list(), m_collision_list() {/*...*/}
            ~NintendoWareStringPoolBuilder() { this->Finalize(); }

            void Finalize();

            Result AddString(StringPoolStringBase *pool_string);

            void Serialize(const NintendoWareStringPoolBuilderMemoryInfo *memory_info, void *output_buffer, size_t buffer_size);

            void CalculateMemoryInfo(NintendoWareStringPoolBuilderMemoryInfo *memory_info);
    };
}

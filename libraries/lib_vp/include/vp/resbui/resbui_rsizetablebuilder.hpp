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

    class ResourceSizeTableBuilder;

    class ResourceSizeNode {
        public:
            friend class ResourceSizeTableBuilder;
        private:
            vp::util::IntrusiveRedBlackTreeNode<const char*>  collision_node;
            vp::util::IntrusiveRedBlackTreeNode<u32>          hash_node;
            u32                                               size;
        public:
            explicit constexpr  ResourceSizeNode(u32 path_crc32, u32 resource_size) : collision_node(), hash_node(path_crc32), size(resource_size) {/*...*/}
            explicit constexpr  ResourceSizeNode(const char *file_path, u32 resource_size) : collision_node(file_path), hash_node(), size(resource_size) {

                /* Integrity checks */
                VP_ASSERT(file_path != nullptr && *file_path != '\0');

                /* Calculate path hash */
                const u32 path_crc32 = vp::util::HashCrc32b(file_path);
                hash_node.SetKey(path_crc32);

                return;
            }
            constexpr ~ResourceSizeNode() {/*...*/}
    };

    class ResourceSizeTableBuilder {
        public:
            using EntryHashList      = vp::util::IntrusiveRedBlackTreeTraits<ResourceSizeNode, &ResourceSizeNode::hash_node>::Tree;
            using EntryCollisionList = vp::util::IntrusiveRedBlackTreeTraits<ResourceSizeNode, &ResourceSizeNode::collision_node>::Tree;
        public:
            EntryHashList      m_crc32_entry_list;
            EntryCollisionList m_collision_entry_list;
        private:
            ResourceSizeNode *SearchEntry(const char *path);
        public:
            constexpr ResourceSizeTableBuilder() : m_crc32_entry_list(), m_collision_entry_list() {/*...*/}
            constexpr ~ResourceSizeTableBuilder() {/*...*/}

            bool AddEntry(ResourceSizeNode *new_entry);

            ResourceSizeNode *RemoveEntry(const char *path);

            bool UpdateSize(const char *path, u32 new_size);

            void Serialize(void *file, size_t file_size, u32 max_path_length = 0x80);

            size_t CalculateSerializedSize(u32 max_path_length = 0x80);
    };
}

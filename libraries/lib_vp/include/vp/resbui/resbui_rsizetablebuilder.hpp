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
            ResourceSizeNode *SearchEntry(const char *path) {

                /* Search by crc32 */
                {
                    const u32 path_crc32 = util::HashCrc32b(path);
                    ResourceSizeNode *node = m_crc32_entry_list.Find(path_crc32);
                    if (node != nullptr) { return node; }
                }

                /* Search by path */
                {
                    ResourceSizeNode *node = m_collision_entry_list.Find(path);
                    if (node != nullptr) { return node; }
                }

                return nullptr;
            }
        public:
            constexpr ResourceSizeTableBuilder() : m_crc32_entry_list(), m_collision_entry_list() {/*...*/}
            constexpr ~ResourceSizeTableBuilder() {/*...*/}

            bool AddEntry(ResourceSizeNode *new_entry) {

                /* Integrity check */
                VP_ASSERT(new_entry != nullptr);

                /* Find if there is a hash collision */
                ResourceSizeNode *collision_node = m_crc32_entry_list.Find(new_entry->hash_node.GetKey());

                /* Add node to hash tree if there was no collision */
                if (collision_node == nullptr) {

                    m_crc32_entry_list.Insert(new_entry);

                    return true;
                }

                /* Check whether the new node has a valid path */
                const char *new_path = new_entry->collision_node.GetKey();
                if (new_path == nullptr || *new_path == '\0') { return false; }

                /* Check whether the path collides */
                if (m_collision_entry_list.Find(new_entry->collision_node.GetKey()) == nullptr) { return false; }

                /* Check whether the colliding node has a valid path */
                const char *collide_path = new_entry->collision_node.GetKey();
                if (collide_path == nullptr || *collide_path == '\0') { return false; }

                /* Remove colliding node */
                m_crc32_entry_list.Remove(collision_node);

                /* Add both nodes to the collision list */
                m_collision_entry_list.Insert(collision_node);
                m_collision_entry_list.Insert(new_entry);

                return true;
            }

            ResourceSizeNode *RemoveEntry(const char *path) {

                /* Find and unlink Entry */
                ResourceSizeNode *entry = this->SearchEntry(path);
                if (entry == nullptr) { return nullptr; }

                /* Try remove */
                if (entry->hash_node.IsLinked() == true)      { m_crc32_entry_list.Remove(entry); }
                if (entry->collision_node.IsLinked() == true) {

                    m_collision_entry_list.Remove(entry); 

                    /* Try to readd a no longer colliding node */
                    const u32         hash                     = entry->hash_node.GetKey();
                    ResourceSizeNode *no_longer_colliding_node = nullptr;
                    ResourceSizeNode *node_iter                = m_collision_entry_list.Start();
                    while (node_iter != nullptr) {
                        if (node_iter->hash_node.GetKey() == hash) {
                            if (no_longer_colliding_node != nullptr) { return entry; }
                            no_longer_colliding_node = node_iter; 
                        }
                        node_iter = m_collision_entry_list.GetNext(node_iter);
                    }
                    if (no_longer_colliding_node != nullptr) {
                        m_collision_entry_list.Remove(no_longer_colliding_node);
                        m_crc32_entry_list.Insert(no_longer_colliding_node); 
                    }
                }

                return entry;
            }

            bool UpdateSize(const char *path, u32 new_size) {

                /* Find and remove Entry */
                ResourceSizeNode *entry = this->SearchEntry(path);
                if (entry == nullptr) { return false; }

                /* Update state */
                entry->size = new_size;

                return true;
            }

            void Serialize(void *file, size_t file_size, u32 max_path_length = 0x80) {

                /* Integrity checks */
                VP_ASSERT(file != nullptr && this->CalculateSerializedSize(max_path_length) <= file_size);

                /* Clear memory */
                ::memset(file, 0, file_size);

                /* Write header */
                res::ResRsizetable *rsizetable = reinterpret_cast<res::ResRsizetable*>(file);
                rsizetable->magic0             = res::ResRsizetable::cMagic0;
                rsizetable->magic1             = res::ResRsizetable::cMagic1;
                rsizetable->version            = res::ResRsizetable::cTargetVersion;
                rsizetable->max_path_length    = max_path_length;

                /* Stream out crc32 entries  */
                u32 crc32_count = 0;
                ResourceSizeNode *hash_node = m_crc32_entry_list.Start();
                while (hash_node != nullptr) {
                    rsizetable->resource_size_crc32_array[crc32_count].path_crc32    = hash_node->hash_node.GetKey();
                    rsizetable->resource_size_crc32_array[crc32_count].resource_size = hash_node->size;

                    hash_node = m_crc32_entry_list.GetNext(hash_node);
                    ++crc32_count;
                }
                rsizetable->resource_size_crc32_count = crc32_count;

                /* Stream out collision entries */
                u32               collision_count = 0;
                void             *collision_array = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(rsizetable) + sizeof(res::ResRsizetable) + crc32_count * sizeof(res::ResRsizetableCrc32));
                ResourceSizeNode *collision_node = m_collision_entry_list.Start();
                while (collision_node != nullptr) {
                    char *output_string = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(collision_array) + (sizeof(u32) + max_path_length) * collision_count);
                    ::strncpy(output_string, collision_node->collision_node.GetKey(), max_path_length);
                    *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(output_string) + max_path_length) = collision_node->size;

                    collision_node = m_collision_entry_list.GetNext(collision_node);
                    ++collision_count;
                }
                rsizetable->resource_size_collision_count = collision_count;

                return;
            }

            size_t CalculateSerializedSize(u32 max_path_length = 0x80) {
                return sizeof(res::ResRsizetable) + sizeof(res::ResRsizetableCrc32) * m_crc32_entry_list.GetCount() + (sizeof(u32) + max_path_length) * m_collision_entry_list.GetCount();
            }
    };
}

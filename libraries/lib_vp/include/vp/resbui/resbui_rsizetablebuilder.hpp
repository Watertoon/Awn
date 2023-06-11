#pragma once

namespace vp::resbui {

    class ResourceSizeTableBuilder {
        public:
            class EntryNode {
                public:
                    friend class ResourceSizeTableBuilder;
                private:
                    u32                                       path_crc32;
                    u32                                       size;
                    vp::util::FixedString<vp::util::cMaxPath> path;
                    vp::util::IntrusiveListNode               node;
                public:
                    explicit constexpr  EntryNode(u32 path_crc32, u32 resource_size) : path_crc32(path_crc32), size(resource_size), path(nullptr), node() {/*...*/}
                    explicit constexpr  EntryNode(const char *file_path, u32 resource_size) : path_crc32(0), size(resource_size), path(file_path), node() {

                        /* Integrity checks */
                        VP_ASSERT(file_path != nullptr && *file_path != '\0');

                        /* Calculate path hash */
                        path_crc32 = vp::util::HashCrc32b(file_path);

                        return;
                    }
                    constexpr ~EntryNode() {/*...*/}
            };
        public:
            using EntryList = vp::util::IntrusiveListTraits<EntryNode, &EntryNode::node>::List;
        public:
            EntryList m_crc32_entry_list;
            EntryList m_collision_entry_list;
        private:
            bool LinkEntryPath(EntryNode *link_node) {

                /* Find neighboring nodes by path */
                EntryList::iterator next_iter0 = m_crc32_entry_list.begin();
                while (next_iter0 != m_crc32_entry_list.end() && ::strcmp((*next_iter0).path.GetString(), link_node->path.GetString()) < 0) { ++next_iter0; }

                /* Ensure */
                if (::strcmp((*next_iter0).path.GetString(), link_node->path.GetString()) < 0) { return false; }

                /* Link nodes */
                (*next_iter0).node.prev()->LinkNext(std::addressof(link_node->node));
                (*next_iter0).node.LinkPrev(std::addressof(link_node->node));

                return true;
            }
            
            EntryNode *SearchEntry(const char *path) {

                /* Search by crc32 */
                const u32 path_crc32 = util::HashCrc32b(path);
                EntryList::iterator next_iter0 = m_crc32_entry_list.begin();
                while (next_iter0 != m_crc32_entry_list.end() && (*next_iter0).path_crc32 < path_crc32) { ++next_iter0; }

                /* Return if found */
                if ((*next_iter0).path_crc32 == path_crc32) { return std::addressof(*next_iter0); }

                /* Search by path */
                EntryList::iterator next_iter1 = m_crc32_entry_list.begin();
                while (next_iter1 != m_crc32_entry_list.end() && ::strcmp((*next_iter1).path.GetString(), path) < 0) { ++next_iter1; }

                /* Return if found */
                if (::strcmp((*next_iter1).path.GetString(), path) == 0) { return std::addressof(*next_iter1); } 

                return nullptr;
            }
        public:
            constexpr ResourceSizeTableBuilder() : m_crc32_entry_list(), m_collision_entry_list() {/*...*/}
            ~ResourceSizeTableBuilder() {/*...*/}

            bool AddEntry(EntryNode *new_entry) {

                /* Integrity check */
                VP_ASSERT(new_entry != nullptr);

                /* Find if there is already a hash collision */
                EntryList::iterator next_iter0 = m_collision_entry_list.begin();
                while (next_iter0 != m_collision_entry_list.end() && ::strcmp((*next_iter0).path.GetString(), new_entry->path.GetString()) < 0) { ++next_iter0; }

                /* Link nodes if there was a collision */
                if (next_iter0 != m_collision_entry_list.end()) {

                    /* Fail if the path is identical to the next node */
                    if ((*next_iter0).path == new_entry->path) { return false; }

                    (*next_iter0).node.LinkPrev(std::addressof(new_entry->node));
                    return true;
                }

                /* Find neighboring nodes by crc32 */
                EntryList::iterator next_iter1 = m_crc32_entry_list.begin();
                while (next_iter1 != m_crc32_entry_list.end() && (*next_iter1).path_crc32 < new_entry->path_crc32) { ++next_iter1; }
                if (next_iter1 == m_crc32_entry_list.end()) {
                    next_iter1 = m_crc32_entry_list.begin();
                }

                /* Link nodes unless a collision occured */
                EntryNode &next_entry = (*next_iter1);
                if (next_entry.path_crc32 != new_entry->path_crc32) {
                    next_entry.node.LinkPrev(std::addressof(new_entry->node));
                    return true;
                }

                /* Fail if the node has no path or collides */
                if (next_entry.path.IsNullString() == true || next_entry.path == new_entry->path) { return false; }

                /* Unlink newly colliding node */
                next_entry.node.Unlink();

                /* Link new node and colliding node to path table */
                const bool result0 = this->LinkEntryPath(std::addressof(next_entry));
                VP_ASSERT(result0 != false);
                const bool result1 = this->LinkEntryPath(new_entry);
                VP_ASSERT(result1 != false);

                return true;
            }

            EntryNode *RemoveEntry(const char *path) {

                /* Find and unlink Entry */
                EntryNode *entry = this->SearchEntry(path);
                if (entry == nullptr) { return nullptr; }
                entry->node.Unlink();

                return entry;
            }

            bool UpdateSize(const char *path, u32 new_size) {

                /* Find and remove Entry */
                EntryNode *entry = this->SearchEntry(path);
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
                rsizetable->magic0          = res::ResRsizetable::cMagic0;
                rsizetable->magic1          = res::ResRsizetable::cMagic1;
                rsizetable->version         = res::ResRsizetable::cTargetVersion;
                rsizetable->max_path_length = max_path_length;

                /* Stream out crc32 entries  */
                u32 crc32_count = 0;
                for (const EntryNode &entry_node : m_crc32_entry_list) {
                    rsizetable->resource_size_crc32_array[crc32_count].path_crc32    = entry_node.path_crc32;
                    rsizetable->resource_size_crc32_array[crc32_count].resource_size = entry_node.size;
                    ++crc32_count;
                }
                rsizetable->resource_size_crc32_count = crc32_count;

                /* Stream out collision entries */
                u32 collision_count = 0;
                void *collision_array = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(rsizetable) + sizeof(res::ResRsizetable) + crc32_count * sizeof(res::ResRsizetableCrc32));
                for (const EntryNode &entry_node : m_collision_entry_list) {
                    char *output_string = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(collision_array) + (sizeof(u32) + max_path_length) * collision_count);
                    ::strncpy(output_string, entry_node.path.GetString(), max_path_length);
                    *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(output_string) + max_path_length) = entry_node.size;
                    ++collision_count;
                }
                rsizetable->resource_size_collision_count = collision_count;

                return;
            }

            constexpr size_t CalculateSerializedSize(u32 max_path_length = 0x80) {
                return sizeof(res::ResRsizetable) + sizeof(res::ResRsizetableCrc32) * m_crc32_entry_list.GetCount() + (sizeof(u32) + max_path_length) * m_collision_entry_list.GetCount();
            }
    };
}

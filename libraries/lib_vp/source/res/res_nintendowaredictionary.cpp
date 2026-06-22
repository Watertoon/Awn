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

namespace vp::res {

	bool ResNintendoWareDictionary::Build() {

        /* Nothing to do if there are no nodes */
        if (node_count <= 0) { return true; }

        /* Default initialize nodes */
        const u32   node_count_root = node_count + 1;
        ResDicNode *root_node_array = std::addressof(root_node);
        for (u32 i = 0; i < node_count_root; ++i) {
            root_node_array[i].ref_bit    = static_cast<u32>(cNPos);
            root_node_array[i].left_node  = i;
            root_node_array[i].right_node = i;
        }

        /* Build loop */
        ResDicNode *node_iter = std::addressof(root_node) + 1;
        for (u32 i = 1; i < node_count_root; ++i) {

            /* Advance node iter */
            const NWString8 *iter_key = node_iter->key;
            const u16 key_length = iter_key->GetLength();

            /* Traverse tree to find insert key */
            u32 last_ref_iter = root_node.ref_bit;
            s16 insert_iter   = root_node.left_node;
            u32 ref_iter      = root_node_array[insert_iter].ref_bit;
            while (static_cast<s32>(last_ref_iter) < static_cast<s32>(ref_iter)) {

                /* Advance last ref */
                last_ref_iter = ref_iter;

                /* Determine next side */
                const u32  last_length = last_ref_iter >> 3;
                const bool side = (last_length < key_length) ? (iter_key->GetString()[key_length + ~last_length] >> (last_ref_iter & 0x7)) & 0x1 : false;

                /* Advance iterators */
                insert_iter = std::addressof(root_node_array[insert_iter].left_node)[side];
                ref_iter    = root_node_array[insert_iter].ref_bit;
            }
            const NWString8 *insert_key = root_node_array[insert_iter].key;

            /* Select max key length */
            const u16 insert_length = insert_key->GetLength();
            const u16 max_length    = (insert_length < key_length) ? key_length : insert_length;

            /* Integrity check max length is valid */
            if (max_length == 0) { return false; }

            /* Find reference bit between insert node and current node's strings */
            u32 new_ref = 0;
            for (;;) {
                const u32  ref_length  = new_ref >> 3;
                const bool iter_ref    = (ref_length < key_length)    ? (iter_key->GetString()[key_length + ~ref_length] >> (new_ref & 0x7)) & 0x1 : false;
                const bool insert_ref  = (ref_length < insert_length) ? (insert_key->GetString()[insert_length + ~ref_length] >> (new_ref & 0x7)) & 0x1  : false;
                if (iter_ref != insert_ref) { break; }
                new_ref = new_ref + 1;
                if ((static_cast<u32>(max_length) << 3) == new_ref) { node_iter->ref_bit = static_cast<u32>(cNPos); return false; }
            }

            /* Set new ref bit */
            node_iter->ref_bit = new_ref;
            if (new_ref == static_cast<u32>(cNPos)) {
                return false;
            }

            /* Find insert location */
            ResDicNode *parent_iter = std::addressof(root_node_array[0]);
            ResDicNode *child_iter  = std::addressof(root_node_array[root_node.left_node]);
            ref_iter      = child_iter->ref_bit;
            while (static_cast<s32>(parent_iter->ref_bit) < static_cast<s32>(ref_iter) && static_cast<s32>(ref_iter) < static_cast<s32>(new_ref)) {
                
                /* Advance last ref */
                parent_iter = child_iter;
    
                /* Determine next side */
                const u32  child_length = ref_iter >> 3;
                const bool side         = (child_length < key_length) ? (iter_key->GetString()[key_length + ~child_length] >> (ref_iter & 0x7)) & 0x1 : false;
    
                /* Advance iterators */
                const u32 last_index = std::addressof(parent_iter->left_node)[side];
                child_iter = std::addressof(root_node_array[last_index]);
                ref_iter   = child_iter->ref_bit;
            }

            /* Set parent's child to current node */
            const u32  parent_length = parent_iter->ref_bit >> 3;
            const bool side0 = (parent_length < key_length) ? (iter_key->GetString()[key_length + ~parent_length] >> (parent_iter->ref_bit & 0x7)) & 0x1 : false;
            std::addressof(parent_iter->left_node)[side0] = i;

            /* Move child from parent node to child of current node */
            const u32 child_length = child_iter->key->GetLength();
            const u32 node_length = node_iter->ref_bit >> 3;
            const bool side1 = (child_length > node_length) ? (child_iter->key->GetString()[child_length + ~node_length] >> (node_iter->ref_bit & 0x7)) & 0x1 : false;
            std::addressof(node_iter->left_node)[side1] = (reinterpret_cast<uintptr_t>(child_iter) - reinterpret_cast<uintptr_t>(root_node_array)) / sizeof(ResDicNode);

            node_iter = node_iter + 1;
        }

        return true;
	}

    u32 ResNintendoWareDictionary::TryGetEntryIndexByKey(const char *key, u32 length) const {

        /* Dictionary search */
        const u32         len       = length;
        s16               next_idx  = root_node.left_node;
        const ResDicNode *node_iter = std::addressof(root_node) + next_idx;
        const ResDicNode *last_iter = std::addressof(root_node);
        s32               last_ref  = root_node.ref_bit;
        s32               iter_ref  = node_iter->ref_bit;
        while (last_ref < iter_ref) {
            const u32 ref_len = iter_ref >> 3;
            bool is_right = false;
            if (ref_len < len) {
                is_right = key[len + ~ref_len] >> (iter_ref & 7) & 1;
            }
            last_iter = std::addressof(root_node) + next_idx;
            next_idx  = std::addressof(last_iter->left_node)[is_right];
            node_iter = std::addressof(root_node) + next_idx;
            last_ref  = last_iter->ref_bit;
            iter_ref  = node_iter->ref_bit;
        }

        /* Find larger string size */
        u16 key_len = node_iter->key->GetLength();
        u16 max_len = len;
        if (key_len <= len) {
            max_len = key_len;
        }

        /* Ensure strings are identical */
        if (max_len == 0 && key_len < len) { return cInvalidEntryIndex; }
        const s32 result = std::memcmp(node_iter->key->GetString(), key, max_len);
        if (result != 0)   { return cInvalidEntryIndex; }
        if (len < key_len) { return cInvalidEntryIndex; }
        if (key_len < len) { return cInvalidEntryIndex; }

        /* Find entry index through ptrdiff */
        ptrdiff_t ptr_diff = reinterpret_cast<uintptr_t>(node_iter) - sizeof(ResNintendoWareDictionary) - reinterpret_cast<uintptr_t>(this);
        if ((~ptr_diff & 0xf'ffff'fff0) != 0) {
            return ptr_diff >> 4;
        }

        return cInvalidEntryIndex;
    }
}

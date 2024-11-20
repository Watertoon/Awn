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

namespace vp::res {

	struct ResNintendoWareDictionary {
        struct ResDicNode {
            union {
                u32     ref_bit;
                struct {
                    u32 ref_bit_rshift : 3;
                    u32 ref_bit_length : 29;
                };
            };
            s16         left_node;
            s16         right_node;
            const char *key; /* string starts at pos 2, 0-1 is a u16 of the string size */
        };
        static_assert(sizeof(ResDicNode) == 0x10);

        u32        magic;
        s32        node_count; /* Note; does not include Root Node */
        ResDicNode root_node;

        static constexpr inline u32 cMagic             = util::TCharCode32("_DIC");
        static constexpr inline s32 cNPos              = -1;
        static constexpr inline u32 cInvalidEntryIndex = 0xffff'ffff;

        struct impl {
            static constexpr u16 ReadU16(const char *string) {
                const u32 c1 = *(string);
                const u32 c2 = *(string + 1);
                return static_cast<u16>((c1) | (c2 << 8));
            }
        };

        constexpr bool TBuild(ResDicNode *node_array) {

            /* Nothing to do if there are no nodes */
            if (node_count <= 0) { return true; }

            /* Default initialize nodes */
            root_node.ref_bit    = static_cast<u32>(cNPos);
            root_node.left_node  = 0;
            root_node.right_node = 0;
            for (s32 i = 0; i < node_count; ++i) {
                node_array[i].ref_bit    = static_cast<u32>(cNPos);
                node_array[i].left_node  = i + 1;
                node_array[i].right_node = i + 1;
            }

            /* Build loop */
            ResDicNode *node_iter = node_array;
            for (s32 i = 0; i < node_count; ++i) {

                /* Advance node iter */
                const char *iter_key = node_iter->key;
                const u16 key_length = impl::ReadU16(iter_key);

                /* Traverse tree to find insert key */
                u32 last_ref_iter = root_node.ref_bit;
                s16 insert_iter   = root_node.left_node;
                u32 ref_iter      = (insert_iter == 0) ? root_node.ref_bit : node_array[insert_iter - 1].ref_bit;
                while (static_cast<s32>(last_ref_iter) < static_cast<s32>(ref_iter)) {

                    /* Advance last ref */
                    last_ref_iter = ref_iter;

                    /* Determine next side */
                    const u32  last_length = last_ref_iter >> 3;
                    const bool side = (last_length < key_length) ? (iter_key[key_length + ~last_length + 2] >> (last_ref_iter & 0x7)) & 0x1 : false;

                    /* Advance iterators */
                    if (insert_iter == 0) {
                        insert_iter = (side) ? root_node.right_node : root_node.left_node;
                    } else {
                        insert_iter = (side) ? node_array[insert_iter - 1].right_node : node_array[insert_iter - 1].left_node;
                    }
                    ref_iter = (insert_iter == 0) ? root_node.ref_bit : node_array[insert_iter - 1].ref_bit;
                }
                const char *insert_key = (insert_iter == 0) ? root_node.key : node_array[insert_iter - 1].key;

                /* Select max key length */
                const u16 insert_length = impl::ReadU16(insert_key);
                const u16 max_length    = (insert_length < key_length) ? key_length : insert_length;

                /* Integrity check max length is valid */
                if (max_length == 0) { return false; }

                /* Find reference bit between insert node and current node's strings */
                u32 new_ref = 0;
                for (;;) {
                    const u32  ref_length  = new_ref >> 3;
                    const bool iter_ref    = (ref_length < key_length)    ? (iter_key[key_length + ~ref_length + 2] >> (new_ref & 0x7)) & 0x1 : false;
                    const bool insert_ref  = (ref_length < insert_length) ? (insert_key[insert_length + ~ref_length + 2] >> (new_ref & 0x7)) & 0x1  : false;
                    if (iter_ref != insert_ref) { break; }
                    new_ref = new_ref + 1;
                    if ((static_cast<u32>(max_length) << 3) == new_ref || static_cast<s32>(new_ref) < 0) { node_iter->ref_bit = static_cast<u32>(cNPos); return false; }
                }

                /* Set new ref bit */
                node_iter->ref_bit = new_ref;
                if (new_ref == static_cast<u32>(cNPos)) { return false; }

                /* Find insert location */
                ResDicNode *parent_iter = std::addressof(root_node);
                ResDicNode *child_iter  = (root_node.left_node == 0) ? std::addressof(root_node) : std::addressof(node_array[root_node.left_node - 1]);
                s16         last_index  = root_node.left_node;
                ref_iter      = child_iter->ref_bit;
                while (static_cast<s32>(parent_iter->ref_bit) < static_cast<s32>(ref_iter) && static_cast<s32>(new_ref) > static_cast<s32>(ref_iter)) {

                    /* Advance last ref */
                    parent_iter   = child_iter;

                    /* Determine next side */
                    const u32  child_length = ref_iter >> 3;
                    const bool side         = (child_length < key_length) ? (iter_key[key_length + ~child_length + 2] >> (ref_iter & 0x7)) & 0x1 : false;

                    /* Advance iterators */
                    if (last_index == 0) {
                        last_index = (side) ? root_node.right_node : root_node.left_node;
                    } else {
                        last_index = (side) ? node_array[last_index - 1].right_node : node_array[last_index - 1].left_node;
                    }
                    child_iter = (last_index == 0) ? std::addressof(root_node) : std::addressof(node_array[last_index - 1]);
                    ref_iter   = child_iter->ref_bit;
                }

                /* Set parent's child to current node */
                const u32  parent_length = parent_iter->ref_bit >> 3;
                const bool side0 = (parent_length < key_length) ? (iter_key[key_length + ~parent_length + 2] >> (parent_iter->ref_bit & 0x7)) & 0x1 : false;

                u32 count0 = 0;
                ResDicNode *iter0 = std::addressof(root_node);
                while (iter0 != node_iter && count0 < 0xffff) {
                    if (count0 == 0) { iter0 = node_array; } else { iter0 = iter0 + 1; }
                    ++count0;
                }
                if (side0) {
                    parent_iter->right_node = count0;
                } else {
                    parent_iter->left_node = count0;
                }

                /* Move child from parent node to child of current node */
                const u32 child_length = impl::ReadU16(child_iter->key);
                const u32 node_length = node_iter->ref_bit >> 3;
                const bool side1 = (child_length > node_length) ? (child_iter->key[child_length + ~node_length + 2] >> (node_iter->ref_bit & 0x7)) & 0x1 : false;

                u32 count1 = 0;
                ResDicNode *iter1 = std::addressof(root_node);
                while (iter1 != child_iter && count1 < 0xffff) {
                    if (count1 == 0) { iter1 = node_array; } else { iter1 = iter1 + 1; }
                    ++count1;
                }
                if (side1) {
                    node_iter->right_node = count1;
                } else {
                    node_iter->left_node = count1;
                }

                node_iter = node_iter + 1;
            }

            return true;
        }

        bool Build();

        u32 FindRefBit();

        u32 TryGetEntryIndexByKey(const char *key, u32 length) const {

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
            u16 key_len = *reinterpret_cast<const u16*>(node_iter->key);
            u16 max_len = len;
            if (key_len <= len) {
                max_len = key_len;
            }

            /* Ensure strings are identical */
            if (max_len == 0 && key_len < len) { return -1; }
            const s32 result = ::memcmp(node_iter->key + 2, key, max_len);
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

        u32 TryGetEntryIndexByKey(const char *key) const {

            /* Dictionary search */
            const u32         len       = (key != nullptr) ? vp::util::StringLength(key) : 0;
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
            u16 key_len = *reinterpret_cast<const u16*>(node_iter->key);
            u16 max_len = len;
            if (key_len <= len) {
                max_len = key_len;
            }

            /* Ensure strings are identical */
            if (max_len == 0 && key_len < len) { return -1; }
            const s32 result = ::memcmp(node_iter->key + 2, key, max_len);
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

        const char *GetKeyByEntryIndex(u32 entry_id) const {
            return (std::addressof(root_node) + 1)[entry_id].key;
        }
    };
    static_assert(sizeof(ResNintendoWareDictionary) == 0x18);
}

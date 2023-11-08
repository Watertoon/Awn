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
        ResDicNode *node_iter = std::addressof(root_node);
        for (u32 i = 1; i < node_count_root; ++i) {

            /* Advance node iter */
            node_iter = node_iter + 1;
            const char *iter_key = node_iter->key;
            const u16 key_length = *reinterpret_cast<const u16*>(iter_key);

            /* Traverse tree to find insert key */
            u32 last_ref_iter = root_node.ref_bit;
            s16 insert_iter   = root_node.left_node;
            u32 ref_iter      = root_node_array[insert_iter].ref_bit;
            while (static_cast<s32>(last_ref_iter) < static_cast<s32>(ref_iter)) {

                /* Advance last ref */
                last_ref_iter = ref_iter;

                /* Determine next side */
                const u32  last_length = last_ref_iter >> 3;
                const bool side = (last_length < key_length) ? (iter_key[key_length + ~last_length + 2] >> (last_ref_iter & 0x7)) & 0x1 : false;

                /* Advance iterators */
                insert_iter = std::addressof(root_node_array[insert_iter].left_node)[side];
                ref_iter    = root_node_array[insert_iter].ref_bit;
            }
            const char *insert_key = root_node_array[insert_iter].key;

            /* Select max key length */
            const u16 insert_length = *reinterpret_cast<const u16*>(insert_key);
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
            if (new_ref == static_cast<u32>(cNPos)) {
                return false;
            }

            /* Find insert location */
            ResDicNode *parent_iter = std::addressof(root_node_array[0]);
            ResDicNode *child_iter  = std::addressof(root_node_array[root_node.left_node]);
            s16         last_index  = root_node.left_node;
            last_ref_iter = root_node.ref_bit;
            ref_iter      = child_iter->ref_bit;
            while (static_cast<s32>(last_ref_iter) < static_cast<s32>(ref_iter) && static_cast<s32>(new_ref) > static_cast<s32>(ref_iter)) {
                
                /* Advance last ref */
                last_ref_iter = ref_iter;
                parent_iter   = child_iter;

                /* Determine next side */
                const u32  child_length = ref_iter >> 3;
                const bool side         = (child_length < key_length) ? (iter_key[key_length + ~child_length + 2] >> (ref_iter & 0x7)) & 0x1 : false;

                /* Advance iterators */
                last_index = std::addressof(root_node_array[last_index].left_node)[side];
                child_iter = std::addressof(root_node_array[last_index]);
                ref_iter   = child_iter->ref_bit;
            }

            /* Set parent's child to current node */
            const u32  parent_length = parent_iter->ref_bit >> 3;
            const bool side0 = (parent_length < key_length) ? (iter_key[key_length + ~parent_length + 2] >> (parent_iter->ref_bit & 0x7)) & 0x1 : false;
            std::addressof(parent_iter->left_node)[side0] = (reinterpret_cast<uintptr_t>(node_iter) - reinterpret_cast<uintptr_t>(root_node_array)) / sizeof(ResDicNode);

            /* Move child from parent node to child of current node */
            const u32 child_length = child_iter->ref_bit >> 3;
            const bool side1 = (child_length < key_length) ? (iter_key[key_length + ~child_length + 2] >> (child_iter->ref_bit & 0x7)) & 0x1 : false;
            std::addressof(node_iter->left_node)[side1] = (reinterpret_cast<uintptr_t>(child_iter) - reinterpret_cast<uintptr_t>(root_node_array)) / sizeof(ResDicNode);
        }

        return true;
	}
}

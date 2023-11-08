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

        bool Build();

        u32 FindRefBit();

        u32 TryGetEntryIndexByKey(const char *key) const {

            /* Find key length */
            u32 len = 0;
            if (key != nullptr) {
                len = ::strlen(key);
            }

            /* Dictionary search */
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

        constexpr const char *GetKeyByEntryIndex(u32 entry_id) const {
            return (std::addressof(root_node) + 1)[entry_id].key;
        }
    };
    static_assert(sizeof(ResNintendoWareDictionary) == 0x18);
}

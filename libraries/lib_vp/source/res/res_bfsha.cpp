#include <vp.hpp>

namespace vp::res {

    u32 ResBfshaShaderOption::ReadStaticKey(const void *static_key) const {
        const u32 *key_array = reinterpret_cast<const u32*>(static_key);
        const u32 base = key_array[this->key_index] & this->key_mask;
        return (base >> (this->key_bit_offset & (BITSIZEOF(u32) - 1)));
    }

    void ResBfshaShaderOption::WriteStaticKey(void *static_key, u32 choice_index) const {
        u32 *key_array = reinterpret_cast<u32*>(static_key);
        const u32 prev = key_array[this->key_index] & (~this->key_mask);
        const u32 next = prev | (choice_index << (this->key_bit_offset & (BITSIZEOF(u32) - 1)));
        key_array[this->key_index] = next;
        return;
    }

    u32 ResBfshaShaderOption::ReadDynamicKey(const void *dynamic_key) const {
        const u32 *key_array = reinterpret_cast<const u32*>(dynamic_key);
        const u32 base = key_array[this->key_index] & this->key_mask;
        return (base >> (this->key_bit_offset & (BITSIZEOF(u32) - 1)));
    }

    void ResBfshaShaderOption::WriteDynamicKey(void *dynamic_key, u32 choice_index) const {
        u32 *key_array = reinterpret_cast<u32*>(dynamic_key);
        const u32 index = this->key_index - this->base_key_index_dynamic;
        const u32 prev = key_array[index] & (~this->key_mask);
        const u32 next = prev | (choice_index << (this->key_bit_offset & (BITSIZEOF(u32) - 1)));
        key_array[index] = next;
        return;
    }

    u32 ResBfshaShadingModel::FindStaticShaderOptionIndex(const char *option_name) const {
        return (this->static_option_dictionary != nullptr) ? this->static_option_dictionary->TryGetEntryIndexByKey(option_name) : cInvalidEntryIndex;
    }

    u32 ResBfshaShadingModel::FindDynamicShaderOptionIndex(const char *option_name) const {
        return (this->dynamic_option_dictionary != nullptr) ? this->dynamic_option_dictionary->TryGetEntryIndexByKey(option_name) : cInvalidEntryIndex;
    }

    u32 ResBfshaShadingModel::FindInputAttributeIndex(const char *attribute_name) const {
        return (this->input_attribute_dictionary != nullptr) ? this->input_attribute_dictionary->TryGetEntryIndexByKey(attribute_name) : cInvalidEntryIndex;
    }

    u32 ResBfshaShadingModel::FindTextureSamplerIndex(const char *texture_sampler_name) const {
        return (this->texture_sampler_dictionary != nullptr) ? this->texture_sampler_dictionary->TryGetEntryIndexByKey(texture_sampler_name) : cInvalidEntryIndex;
    }

    u32 ResBfshaShadingModel::FindImageIndex(const char *image_name) const {
        return (this->image_dictionary != nullptr) ? this->image_dictionary->TryGetEntryIndexByKey(image_name) : cInvalidEntryIndex;
    }

    u32 ResBfshaShadingModel::FindUniformBufferIndex(const char *uniform_buffer_name) const {
        return (this->uniform_buffer_dictionary != nullptr) ? this->uniform_buffer_dictionary->TryGetEntryIndexByKey(uniform_buffer_name) : cInvalidEntryIndex;
    }

    u32 ResBfshaShadingModel::FindStorageBufferIndex(const char *storage_buffer_name) const {
        return (this->storage_buffer_dictionary != nullptr) ? this->storage_buffer_dictionary->TryGetEntryIndexByKey(storage_buffer_name) : cInvalidEntryIndex;
    }

    bool ResBfshaShadingModel::FindProgramRange(BfshaShaderRange *out_range, const void *static_key) const {

        /* Return start of key array if no keys */
        const u32 s_key_size = this->static_key_size * sizeof(u32);
        const u32 d_key_size = this->dynamic_key_size * sizeof(u32);
        const u32 key_size = s_key_size + d_key_size;
        const u32 total_key_size = key_size * this->program_count;
        uintptr_t start = reinterpret_cast<uintptr_t>(this->option_key_array);
        uintptr_t end = reinterpret_cast<uintptr_t>(this->option_key_array);
        ON_SCOPE_EXIT {
            out_range->start_key = reinterpret_cast<void*>(start + s_key_size);
            out_range->end_key = reinterpret_cast<void*>(end + s_key_size);
        };
        if (key_size < total_key_size) { return start != end; }

        /* Key compare function */
        const u32 *comp_key = reinterpret_cast<const u32*>(static_key);
        const auto compare_key_lambda = [=](const u32 *key_range) {
            for (u32 i = 0; i < s_key_size; ++i) {
                if (key_range[i] < comp_key[i]) {
                    return std::strong_ordering::less;
                }
                if (key_range[i] > comp_key[i]) {
                    return std::strong_ordering::greater;
                }
            }
            return std::strong_ordering::equal;
        };

        /* Binary search for start of dynamic program range */
        bool is_static_key = true;
        uintptr_t next_key = 0;
        u32 end_count = 0;
        const auto search_start_lambda = [&]() {

            /* No static key */
            u32 key_idx = this->program_count;
            if (this->static_key_size < 1) {
                const u32 next_idx = key_idx >> 1;
                next_key = start + next_idx * key_size;
                end = next_key + key_size;
                end_count = start + key_size - end;
                is_static_key = false;
                return true;
            }

            /* Find program with static key */
            uintptr_t end_key = start + key_size;
            u32 next_key_offset;
            for (;;) {
                const u32 next_idx = key_idx >> 1;
                next_key_offset = next_idx * key_size;
                next_key = start + next_key_offset;

                std::strong_ordering order = compare_key_lambda(reinterpret_cast<u32*>(next_key));
                if (order == std::strong_ordering::less) {
                    start = next_key + key_size;
                    end = start;
                    key_idx = key_idx - next_idx;
                    if (key_idx == 0) { return false; }
                    continue;
                } else if (order == std::strong_ordering::greater) {
                    end_key = next_key;
                    end = start;
                    key_idx = next_idx;
                    if (key_idx == 0) { return false; }
                    continue;
                } else {
                    break;
                }
            }

            /* Find first program with static key */
            if (key_size <= next_key_offset) {
                u32 key_idx1 = (key_size == 0) ? 0 : next_key_offset / key_size;
                for (;;) {
                    const u32 next_idx = key_idx1 >> 1;
                    next_key_offset = next_idx * key_size;
                    uintptr_t next_key1 = start + next_key_offset;

                    std::strong_ordering order = compare_key_lambda(reinterpret_cast<u32*>(next_key1));
                    if (order == std::strong_ordering::less) {
                        start = next_key1 + key_size;
                        key_idx1 = key_idx1 - next_idx;
                        continue;
                    } else {
                        key_idx1 = next_idx;
                    }
                    if (key_idx1 == 0) { break; }
                }
            }

            is_static_key = true;
            end = next_key + key_size;
            end_count = end_key - end;

            return true;
        };
        const bool is_succeed = search_start_lambda();
        if (!is_succeed) { return start != end; }

        /* Find end program */
        if (key_size <= end_count) {
            u32 key_idx2 = (key_size == 0) ? 0 : end_count / key_size;
            if (!is_static_key) {
                do {
                    const u32 next_idx = key_idx2 >> 1;
                    key_idx2 = key_idx2 - next_idx;
                    next_key = next_key + key_size + key_idx2 * key_size;
                } while (key_idx2 != 0);
                end = next_key + key_size;
                return start != end;
            }
            for (;;) {
                const u32 next_idx = key_idx2 >> 1;
                uintptr_t next_key1 = end + next_idx * key_size;

                std::strong_ordering order = compare_key_lambda(reinterpret_cast<u32*>(next_key1));
                if (order == std::strong_ordering::less) {
                    end = next_key1 + key_size;
                    key_idx2 = key_idx2 - next_idx;
                    continue;
                } else {
                    key_idx2 = next_idx;
                }
                if (key_idx2 == 0) { break; }
            }
        }

        return start != end;
    }

    u32 ResBfshaShadingModel::FindProgramIndex(const BfshaShaderRange *range, const void *dynamic_key) const {
        
        /* Setup key size and iter */
        const u32 s_key_size = this->static_key_size * sizeof(u32);
        const u32 d_key_size = this->dynamic_key_size * sizeof(u32);
        const u32 key_size = s_key_size + d_key_size;
        uintptr_t start = reinterpret_cast<uintptr_t>(range->start_key);
        uintptr_t end = reinterpret_cast<uintptr_t>(range->end_key);

        /* Key compare function */
        const u32 *comp_key = reinterpret_cast<const u32*>(dynamic_key);
        const auto compare_key_lambda = [=](const u32 *key_range) {
            for (u32 i = 0; i < d_key_size; ++i) {
                if (key_range[i] < comp_key[i]) {
                    return std::strong_ordering::less;
                }
                if (key_range[i] > comp_key[i]) {
                    return std::strong_ordering::greater;
                }
            }
            return std::strong_ordering::equal;
        };
        
        /* Find dynamic key */
        const u32 key_range_size = end - start;
        if (key_size <= key_range_size && 0 < this->dynamic_key_size) {
            u32 key_idx = (key_size == 0) ? 0 : key_range_size / key_size;
            for (;;) {
                const u32 next_idx = key_idx >> 1;
                uintptr_t next_key = start + next_idx * key_size;

                std::strong_ordering order = compare_key_lambda(reinterpret_cast<u32*>(next_key));
                if (order == std::strong_ordering::less) {
                    start = next_key + key_size;
                    key_idx = key_idx - next_idx;
                    continue;
                } else if (order == std::strong_ordering::greater) {
                    key_idx = next_idx;
                } else {
                    break;
                }
                if (key_idx == 0) { break; }
            }
        }
        if (start == end) { return cInvalidEntryIndex; }
        const u32 index = (0 < key_size) ? ((start - (reinterpret_cast<uintptr_t>(this->option_key_array) + s_key_size)) / sizeof(u32)) / key_size : 0;
        return index;
    }

    void ResBfshaShadingModel::GetDefaultStaticKey(void *out_default_static_key) const {

        /* Ensure static key */
        if (this->static_key_size == 0) { return; }

        /* Copy specific default static key if it exists */
        const u32 s_key_size = this->static_key_size * sizeof(u32);
        if (this->default_program_index != cInvalidDefaultKeyIndex) {
            const u32 d_key_size = this->dynamic_key_size * sizeof(u32);
            const u32 key_size = s_key_size + d_key_size;
            const u32 *default_static_key = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(this->option_key_array) + (key_size * this->default_program_index));
            ::memmove(out_default_static_key, default_static_key, s_key_size);
            return;
        }

        /* Build dynamic key from options */
        ::memset(out_default_static_key, 0, s_key_size);
        u32 *out_key = reinterpret_cast<u32*>(out_default_static_key);
        for (u32 i = 0; i < this->dynamic_option_count; ++i) {
            ResBfshaShaderOption *s_option = std::addressof(this->dynamic_option_array[i]);
            const u32 key_index = s_option->key_index - s_option->base_key_index_dynamic;
            const u32 mask_prev = out_key[key_index] & (~s_option->key_mask);
            const u32 value = s_option->default_choice_index << (s_option->key_bit_offset & (BITSIZEOF(u32) - 1));
            out_key[key_index] = value | mask_prev;
        }

        return;
    }

    void ResBfshaShadingModel::GetDefaultDynamicKey(void *out_default_dynamic_key) const {

        /* Ensure dynamic key */
        if (this->dynamic_key_size == 0) { return; }

        /* Copy specific default dynamic key if it exists */
        const u32 d_key_size = this->dynamic_key_size * sizeof(u32);
        if (this->default_program_index != cInvalidDefaultKeyIndex) {
            const u32 s_key_size = this->static_key_size * sizeof(u32);
            const u32 key_size = s_key_size + d_key_size;
            const u32 *default_dynamic_key = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(this->option_key_array) + (key_size * this->default_program_index) + s_key_size);
            ::memmove(out_default_dynamic_key, default_dynamic_key, d_key_size);
            return;
        }

        /* Build dynamic key from options */
        ::memset(out_default_dynamic_key, 0, d_key_size);
        u32 *out_key = reinterpret_cast<u32*>(out_default_dynamic_key);
        for (u32 i = 0; i < this->dynamic_option_count; ++i) {
            ResBfshaShaderOption *dyn_option = std::addressof(this->dynamic_option_array[i]);
            const u32 key_index = dyn_option->key_index - dyn_option->base_key_index_dynamic;
            const u32 mask_prev = out_key[key_index] & (~dyn_option->key_mask);
            const u32 value = dyn_option->default_choice_index << (dyn_option->key_bit_offset & (BITSIZEOF(u32) - 1));
            out_key[key_index] = value | mask_prev;
        }

        return;
    }

    void ResBfshaShadingModel::GetInvalidDynamicKey(void *out_invalid_dynamic_key) const {

        /* Ensure dynamic key */
        if (this->dynamic_key_size == 0) { return; }

        /* Set all of key to 0xff */
        const u32 d_key_size = this->dynamic_key_size * sizeof(u32);
        ::memset(out_invalid_dynamic_key, 0xff, d_key_size);

        return;
    }
}

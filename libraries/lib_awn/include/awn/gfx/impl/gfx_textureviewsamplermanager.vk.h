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

namespace awn::gfx::impl {

    struct SamplerKey {
        union {
            u32 packed_state;
            struct {
                u32 wrap_mode_u : 3;
                u32 wrap_mode_v : 3;
                u32 wrap_mode_w : 3;
                u32 compare_op : 3;
                u32 border_color : 2;
                u32 mip_map_filter : 2;
                u32 mag_filter : 2;
                u32 min_filter : 2;
                u32 is_enable_anisotropy : 1;
                u32 is_compare_r_to_texture : 1;
                u32 reduction_filter : 2;
                u32 max_anisotropy : 8;
            };
        };
        float lod_clamp_min;
        float lod_clamp_max;
        float lod_bias;
    };
    constexpr s32 SamplerKeyCompare(const SamplerKey &rhs, const SamplerKey &lhs) {
        if (lhs.packed_state > rhs.packed_state) { return 1; }
        if (lhs.packed_state < rhs.packed_state) { return -1; }
        if (lhs.lod_clamp_min > rhs.lod_clamp_min) { return 1; }
        if (lhs.lod_clamp_min < rhs.lod_clamp_min) { return -1; }
        if (lhs.lod_clamp_max > rhs.lod_clamp_max) { return 1; }
        if (lhs.lod_clamp_max < rhs.lod_clamp_max) { return -1; }
        if (lhs.lod_bias > rhs.lod_bias) { return 1; }
        if (lhs.lod_bias < rhs.lod_bias) { return -1; }
        return -1;
    }
    
    struct TextureViewKey {
        union {
            u32 packed_state;
            struct {
                u32 image_dimension : 4;
                u32 swizzle_x : 3;
                u32 swizzle_y : 3;
                u32 swizzle_z : 3;
                u32 swizzle_w : 3;
                u32 base_mip_level : 7;
                u32 mip_count : 7;
                u32 depth_stencil_mode : 2;
            };
        };
        u16 base_array_layer;
        u16 array_layer_count;
        u32 image_format;
        gfx::Texture *texture;

        constexpr void SetFromInfo(const gfx::TextureViewInfo *info) {
            this->image_format = info->image_format;
            this->base_array_layer = info->base_array_layer;
            this->array_layer_count = info->array_layer_count;
            this->base_mip_level = info->base_mip_level;
            this->mip_count = info->mip_level_count;
            this->image_dimension = info->image_dimension;
            this->depth_stencil_mode = info->depth_stencil_mode;
            this->swizzle_x = info->swizzle_x;
            this->swizzle_y = info->swizzle_y;
            this->swizzle_z = info->swizzle_z;
            this->swizzle_w = info->swizzle_w;
            this->texture = info->texture;
        }

        constexpr ImageFormat GetImageFormat() const {
            return static_cast<ImageFormat>(this->image_format);
        }
        constexpr TextureDepthStencilMode GetTextureDepthStencilMode() const {
            return static_cast<TextureDepthStencilMode>(this->depth_stencil_mode);
        }
    };
    constexpr bool IsSameInfo(const TextureViewKey &key, const gfx::TextureViewInfo *info) {
        if (key.image_format != info->image_format) { return false; }
        if (key.base_array_layer != info->base_array_layer) { return false; }
        if (key.array_layer_count != info->array_layer_count) { return false; }
        if (key.base_mip_level != info->base_mip_level) { return false; }
        if (key.mip_count != info->mip_level_count) { return false; }
        if (key.image_dimension != info->image_dimension) { return false; }
        if (key.depth_stencil_mode != info->depth_stencil_mode) { return false; }
        if (key.swizzle_x != info->swizzle_x) { return false; }
        if (key.swizzle_y != info->swizzle_y) { return false; }
        if (key.swizzle_z != info->swizzle_z) { return false; }
        if (key.swizzle_w != info->swizzle_w) { return false; }
        return true;
    }

    struct TextureViewSamplerManagerInfo {
        u32 max_texture_view_unit_count;
        u32 max_sampler_unit_count;
    };

	class TextureViewSamplerManager {
        public:
            friend class CommandBuffer;
        public:
        public:
            struct SamplerUnit {
                vp::util::IntrusiveRedBlackTreeNode<SamplerKey> rb_node;
                u32 reference_count;
                u32 free_count;

                constexpr  SamplerUnit() : rb_node(), reference_count(), free_count() {/*...*/}
                constexpr ~SamplerUnit()= default;

                constexpr void IncrementFreeCount() {
                    const u32 next_free_count = (this->free_count == (cInvalidDescriptorSlotFreeCount - 1)) ? 0 : this->free_count + 1;
                    this->free_count = next_free_count;
                }

                constexpr u32 IncrementReferenceCount() {
                    return vp::util::InterlockedIncrementRelaxed(std::addressof(this->reference_count));
                }

                constexpr u32 DecrementReferenceCount() {
                    return vp::util::InterlockedDecrementRelaxed(std::addressof(this->reference_count));
                }

                constexpr bool IncrementReferenceCountIfValid() {

                    u32 last_ref_count = this->reference_count;
                    while (last_ref_count != 0) {
                        const u32  next_ref_count = last_ref_count + 1;
                        const bool result = vp::util::InterlockedCompareExchangeRelaxed(std::addressof(last_ref_count), std::addressof(this->reference_count), last_ref_count, next_ref_count);
                        if (result) { return true; }
                    }

                    return false;
                }
            };

            struct TextureViewUnit {
                TextureViewKey key;
                u32 reference_count;
                u32 free_count;
                VkImageView vk_image_view;

                static constexpr inline size_t cInvalidMemoryOffset = 0xffff'ffff'ffff'ffff;

                constexpr  TextureViewUnit() : key(), reference_count(), free_count() , vk_image_view() {/*...*/}
                constexpr ~TextureViewUnit() = default;

                constexpr void IncrementFreeCount() {
                    const u32 next_free_count = (this->free_count == (cInvalidDescriptorSlotFreeCount - 1)) ? 0 : this->free_count + 1;
                    this->free_count = next_free_count;
                }

                constexpr u32 IncrementReferenceCount() {
                    return vp::util::InterlockedIncrementRelaxed(std::addressof(this->reference_count));
                }

                constexpr u32 DecrementReferenceCount() {
                    return vp::util::InterlockedDecrementRelaxed(std::addressof(this->reference_count));
                }
            };
        public:
            using TextureSlotType      = vp::util::MinUIntType<Context::cTargetMaxTextureViewCount - 1>;
            using SamplerSlotType      = vp::util::MinUIntType<Context::cTargetMaxSamplerCount - 1>;
            using TextureViewUnitArray = vp::util::HeapArray<TextureViewUnit>;
            using SamplerUnitArray     = vp::util::HeapArray<SamplerUnit>;
            using TextureSlotAllocator = vp::util::AtomicIndexAllocator<TextureSlotType>;
            using SamplerSlotAllocator = vp::util::AtomicIndexAllocator<SamplerSlotType>;
            using SamplerMap           = vp::util::IntrusiveRedBlackTreeTraits<SamplerUnit, &SamplerUnit::rb_node, SamplerKeyCompare>::Tree;
        private:
            TextureSlotAllocator  m_texture_slot_allocator;
            TextureViewUnitArray  m_texture_view_unit_array;
            SamplerMap            m_sampler_map;
            SamplerSlotAllocator  m_sampler_slot_allocator;
            SamplerUnitArray      m_sampler_unit_array;
            sys::ReaderWriterLock m_sampler_rwlock;
        public:
            AWN_SINGLETON_TRAITS(TextureViewSamplerManager);
        public:
            constexpr TextureViewSamplerManager() : m_texture_slot_allocator(), m_texture_view_unit_array(), m_sampler_map(), m_sampler_slot_allocator(), m_sampler_unit_array(), m_sampler_rwlock() {/*...*/}
            constexpr ~TextureViewSamplerManager() {/*...*/}

            Result Initialize(mem::Heap *heap, const TextureViewSamplerManagerInfo *manager_info);
            void Finalize();

            Result RegisterTextureView(DescriptorSlot *out_texture_slot, const TextureViewInfo *texture_view_info);
            Result ReferenceTextureView(DescriptorSlot *out_texture_slot, DescriptorSlot reference_texture_slot);

            void ReleaseTextureView(DescriptorSlot texture_view_slot);

            Result RegisterSampler(DescriptorSlot *out_sampler_slot, const SamplerInfo *sampler_info);
            Result ReferenceSampler(DescriptorSlot *out_sampler_slot, DescriptorSlot reference_sampler_slot);

            void ReleaseSampler(DescriptorSlot sampler_slot);

            constexpr bool IsTextureViewSlotValid(DescriptorSlot texture_view_slot) const {

                /* Ensure free count is not invalid */
                const u32 free_count = texture_view_slot.free_count;
                if (free_count == cInvalidDescriptorSlotFreeCount) { return false; }
                /* Ensure index is in range */
                const DescriptorSlotIndexType index = texture_view_slot.index;
                if (m_texture_view_unit_array.GetCount() <= index) { return false; }
                /* Ensure free count is valid */
                const TextureViewUnit *texture_view_unit = std::addressof(m_texture_view_unit_array[index]);
                if (texture_view_unit->free_count != free_count) { return false; }

                return true;
            }
            constexpr bool IsSameInfo(DescriptorSlot texture_view_slot, const gfx::TextureViewInfo *texture_view_info) const {
                return (this->IsTextureViewSlotValid(texture_view_slot)) ? impl::IsSameInfo(m_texture_view_unit_array[texture_view_slot.index].key, texture_view_info) : false;
            }

            constexpr bool IsSamplerSlotValid(DescriptorSlot sampler_slot) const {

                /* Ensure free count is not invalid */
                const u32 free_count = sampler_slot.free_count;
                if (free_count == cInvalidDescriptorSlotFreeCount) { return false; }
                /* Ensure index is in range */
                const DescriptorSlotIndexType index = sampler_slot.index;
                if (m_sampler_unit_array.GetCount() <= index) { return false; }
                /* Ensure free count is valid */
                const SamplerUnit *sampler_unit = std::addressof(m_sampler_unit_array[index]);
                if (sampler_unit->free_count != free_count) { return false; }

                return true;
            }
        public:
            constexpr VkImageView GetVkImageView(DescriptorSlot texture_view_slot) const { return m_texture_view_unit_array[texture_view_slot.index].vk_image_view; }

            constexpr u32 GetTextureSamplerDescriptorHandle(DescriptorSlot texture_view_slot, DescriptorSlot sampler_slot) {
                VP_ASSERT(this->IsTextureViewSlotValid(texture_view_slot));
                VP_ASSERT(this->IsSamplerSlotValid(sampler_slot));
                return (texture_view_slot.index & 0xf'ffff) | ((sampler_slot.index & 0xfff) << 0x14);
            }
            constexpr u32 GetImageDescriptorHandle(DescriptorSlot texture_view_slot) {
                VP_ASSERT(this->IsTextureViewSlotValid(texture_view_slot));
                return texture_view_slot.index + m_texture_view_unit_array.GetCount();
            }
            constexpr u32 GetSeperateTextureDescriptorHandle(DescriptorSlot texture_view_slot) {
                VP_ASSERT(this->IsTextureViewSlotValid(texture_view_slot));
                return texture_view_slot.index;
            }
            constexpr u32 GetSeperateSamplerDescriptorHandle(DescriptorSlot sampler_slot) {
                VP_ASSERT(this->IsSamplerSlotValid(sampler_slot));
                return sampler_slot.index;
            }
        public:
            constexpr const TextureViewUnit *GetTextureViewUnit(u32 index) const { return std::addressof(m_texture_view_unit_array[index]); }
            constexpr const SamplerUnit *GetSamplerUnit(u32 index) const { return std::addressof(m_sampler_unit_array[index]); }
    };
}

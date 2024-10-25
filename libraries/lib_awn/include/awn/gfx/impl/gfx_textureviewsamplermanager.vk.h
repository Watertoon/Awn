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

namespace awn::gfx {
    class TextureBinder;
}

namespace awn::gfx::impl {

    struct TextureViewSamplerManagerInfo {
        u32 max_texture_view_unit_count;
        u32 max_sampler_unit_count;
    };

	class TextureViewSamplerManager {
        public:
            friend class CommandBuffer;
        public:
            struct SamplerUnit {
                vp::util::IntrusiveRedBlackTreeNode<u64> rb_node;
                u32                                      reference_count;
                u16                                      free_count;
                VkSampler                                vk_sampler;
                char                                     descriptor_memory[Context::cTargetMaxSamplerDescriptorSize];

                constexpr  SamplerUnit() : rb_node(), reference_count(), free_count(), vk_sampler() {/*...*/}
                constexpr ~SamplerUnit() {/*...*/}

                constexpr void IncrementFreeCount() {
                    const u16 next_free_count = (this->free_count == (cInvalidDescriptorSlotFreeCount - 1)) ? 0 : this->free_count + 1;
                    this->free_count = next_free_count;
                }

                ALWAYS_INLINE u32 IncrementReferenceCount() {
                    return vp::util::InterlockedIncrement(std::addressof(this->reference_count));
                }

                ALWAYS_INLINE u32 DecrementReferenceCount() {
                    return vp::util::InterlockedDecrement(std::addressof(this->reference_count));
                }

                bool IncrementReferenceCountIfValid() {

                    u32 last_ref_count = this->reference_count;
                    while (last_ref_count != 0) {
                        const u32  next_ref_count = last_ref_count + 1;
                        const bool result = vp::util::InterlockedCompareExchange(std::addressof(last_ref_count), std::addressof(this->reference_count), last_ref_count, next_ref_count);
                        if (result) { return true; }
                    }

                    return false;
                }
            };

            struct TextureViewUnit {
                u32           reference_count;
                u16           free_count;
                VkImageView   vk_image_view;
                TextureBinder reference_texture_binder;
                char          descriptor_memory[Context::cTargetMaxTextureDescriptorSize];

                static constexpr inline size_t cInvalidMemoryOffset = 0xffff'ffff'ffff'ffff;

                constexpr  TextureViewUnit() : reference_count(), free_count(), vk_image_view() {/*...*/}
                ~TextureViewUnit() {/*...*/}

                constexpr void IncrementFreeCount() {
                    const u16 next_free_count = (this->free_count == (cInvalidDescriptorSlotFreeCount - 1)) ? 0 : this->free_count + 1;
                    this->free_count = next_free_count;
                }

                ALWAYS_INLINE u32 IncrementReferenceCount() {
                    return vp::util::InterlockedIncrement(std::addressof(this->reference_count));
                }

                ALWAYS_INLINE u32 DecrementReferenceCount() {
                    return vp::util::InterlockedDecrement(std::addressof(this->reference_count));
                }

                bool IncrementReferenceCountIfValid() {

                    u32 last_ref_count = this->reference_count;
                    while (last_ref_count != 0) {
                        const u32  next_ref_count = last_ref_count + 1;
                        const bool result = vp::util::InterlockedCompareExchange(std::addressof(last_ref_count), std::addressof(this->reference_count), last_ref_count, next_ref_count);
                        if (result) { return true; }
                    }

                    return false;
                }
            };
        public:
            using TextureSlotType      = vp::util::MinUIntType<Context::cTargetMaxTextureDescriptorCount - 1>;
            using SamplerSlotType      = vp::util::MinUIntType<Context::cTargetMaxSamplerDescriptorCount - 1>;
            using TextureViewUnitArray = vp::util::HeapArray<TextureViewUnit>;
            using SamplerUnitArray     = vp::util::HeapArray<SamplerUnit>;
            using TextureSlotAllocator = vp::util::AtomicIndexAllocator<TextureSlotType>;
            using SamplerSlotAllocator = vp::util::AtomicIndexAllocator<SamplerSlotType>;
            using SamplerMap           = vp::util::IntrusiveRedBlackTreeTraits<SamplerUnit, &SamplerUnit::rb_node>::Tree;
        private:
            VkBuffer              m_bindless_descriptor_vk_buffer;
            GpuBufferAddress      m_bindless_buffer_address;
            void                 *m_bindless_descriptor_gpu_address;
            size_t                m_texture_base_offset;
            size_t                m_sampler_base_offset;
            TextureSlotAllocator  m_texture_slot_allocator;
            TextureViewUnitArray  m_texture_view_unit_array;
            SamplerMap            m_sampler_map;
            SamplerSlotAllocator  m_sampler_slot_allocator;
            SamplerUnitArray      m_sampler_unit_array;
            sys::Mutex            m_sampler_cs;
        public:
            AWN_SINGLETON_TRAITS(TextureViewSamplerManager);
        public:
            constexpr TextureViewSamplerManager() : m_bindless_descriptor_vk_buffer(), m_bindless_buffer_address(), m_bindless_descriptor_gpu_address(), m_texture_slot_allocator(), m_texture_view_unit_array(), m_sampler_map(), m_sampler_slot_allocator(), m_sampler_unit_array(), m_sampler_cs() {/*...*/}
            constexpr ~TextureViewSamplerManager() {/*...*/}

            void Initialize(mem::Heap *heap, mem::Heap *gpu_heap, const TextureViewSamplerManagerInfo *manager_info);
            void Finalize();

            Result RegisterTextureView(DescriptorSlot *out_texture_slot, const TextureViewInfo *texture_view_info, TextureBinder *reference_texture_binder);
            Result ReferenceTextureView(DescriptorSlot *out_texture_slot, DescriptorSlot reference_texture_slot);

            void ReleaseTextureView(DescriptorSlot texture_view_slot);

            Result RegisterSampler(DescriptorSlot *out_sampler_slot, const SamplerInfo *sampler_info);
            Result ReferenceSampler(DescriptorSlot *out_sampler_slot, DescriptorSlot reference_sampler_slot);

            void ReleaseSampler(DescriptorSlot sampler_slot);

            constexpr GpuBufferAddress GetBindlessDescriptorBufferAddress() const { return m_bindless_buffer_address; }

            constexpr bool IsTextureViewSlotValid(DescriptorSlot texture_view_slot) const {

                /* Ensure free count is not invalid */
                const u16 free_count = texture_view_slot.free_count;
                if (free_count == cInvalidDescriptorSlotFreeCount) { return false; }
                /* Ensure index is in range */
                const DescriptorSlotIndexType index = texture_view_slot.index;
                if (m_texture_view_unit_array.GetCount() <= index) { return false; }
                /* Ensure free count is valid */
                TextureViewUnit *texture_view_unit = std::addressof(m_texture_view_unit_array[index]);
                if (texture_view_unit->free_count != free_count) { return false; }

                return true;
            }
            constexpr VkImageView GetVkImageView(DescriptorSlot texture_view_slot) const { return (this->IsTextureViewSlotValid(texture_view_slot)) ? m_texture_view_unit_array[texture_view_slot.index].vk_image_view : VK_NULL_HANDLE; }
            constexpr const void *GetSeparateTextureDescriptor(DescriptorSlot texture_view_slot) const { return (this->IsTextureViewSlotValid(texture_view_slot)) ? m_texture_view_unit_array[texture_view_slot.index].descriptor_memory : nullptr; }

            constexpr bool IsSamplerSlotValid(DescriptorSlot sampler_slot) const {

                /* Ensure free count is not invalid */
                const u16 free_count = sampler_slot.free_count;
                if (free_count == cInvalidDescriptorSlotFreeCount) { return false; }
                /* Ensure index is in range */
                const DescriptorSlotIndexType index = sampler_slot.index;
                if (m_sampler_unit_array.GetCount() <= index) { return false; }
                /* Ensure free count is valid */
                SamplerUnit *sampler_unit = std::addressof(m_sampler_unit_array[index]);
                if (sampler_unit->free_count != free_count) { return false; }

                return true;
            }
            constexpr VkSampler GetVkSampler(DescriptorSlot sampler_slot) const { return (this->IsSamplerSlotValid(sampler_slot)) ? m_sampler_unit_array[sampler_slot.index].vk_sampler : VK_NULL_HANDLE; }
            constexpr const void *GetSeparateSamplerDescriptor(DescriptorSlot sampler_slot) const { return (this->IsSamplerSlotValid(sampler_slot)) ? m_sampler_unit_array[sampler_slot.index].descriptor_memory : nullptr; }
    };
}

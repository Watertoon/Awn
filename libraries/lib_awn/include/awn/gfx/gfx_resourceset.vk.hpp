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

    class CommandBuffer;

    static constexpr u32 cShaderInterfaceResourceCount = 6;
    static constexpr ShaderInterfaceType cShaderInterfaceResourceArray[cShaderInterfaceResourceCount] = {
        ShaderInterfaceType::TextureSampler,
        ShaderInterfaceType::Image,
        ShaderInterfaceType::UniformBuffer,
        ShaderInterfaceType::StorageBuffer,
        ShaderInterfaceType::SeparateTexture,
        ShaderInterfaceType::SeparateSampler,
    };

    struct ResourceSetInfo {
    #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
        u32 max_location;
    #else
        u32 max_location_texture_sampler;
        u32 max_location_buffer;
    #endif

        constexpr bool AddEntry(ShaderInterfaceType interface_type, u32 location) {
    #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
            this->max_location = vp::util::Max(this->max_location, location);
    #else
            if (interface_type == ShaderInterfaceType::UniformBuffer || interface_type == ShaderInterfaceType::StorageBuffer) {
                this->max_location_buffer = vp::util::Max(this->max_location_buffer, location);                
            } else {
                this->max_location_texture_sampler = vp::util::Max(this->max_location_texture_sampler, location);                
            }
    #endif
            return true;
        }
        constexpr u32 AddEntriesByBnshProgram(vp::res::ResBnshShaderProgram *program) {
            vp::res::ResBnshShaderInterfaceInfoTable *table = program->shader_interface_info_table;
            if (table == nullptr) { return 0; }
            u32 count = 0;
            for (u32 i = 0; i < vp::util::TEnumTraits<ShaderStage>::cEnumCount; ++i) {
                const ShaderStage stage = vp::util::TEnumTraits<ShaderStage>::GetValueByIndex(i);
                vp::res::ResBnshShaderInterfaceInfo *info = table->GetShaderInterfaceInfo(stage);
                if (info == nullptr) { continue; }
                for (u32 j = 0; j < cShaderInterfaceResourceCount; ++j) {
                    const ShaderInterfaceType interface_type = cShaderInterfaceResourceArray[j];
                    const u32 slot_count = info->GetInterfaceCount(interface_type);
                    const u32 *slot_array = info->GetInterfaceSlotArray(interface_type);
                    if (slot_count < 1) { continue; }
                    for (u32 k = 0; k < slot_count; ++k) {
                        if (!this->AddEntry(interface_type, slot_array[k])) { return count; }
                        ++count;
                    }
                }
            }
            return count;
        }
        constexpr u32 AddEntriesByBfshaProgram(vp::res::ResBfshaShaderProgram *program) {
            /* Visit all locations */
            const vp::res::ResBfshaShadingModel *model = program->parent_shading_model;
            if (model == nullptr) { return 0; }
            u8 base_location_array[cShaderStageCount];
            model->GetBaseLocationArray(base_location_array, cShaderStageCount);
            const u32 stage_count = model->shader_stage_count;
            u32 count = 0;
            for (u32 i = 0; i < cShaderStageCount; ++i) {
                const ShaderStage stage = vp::util::TEnumTraits<ShaderStage>::GetValueByIndex(i);
                if (base_location_array[static_cast<u32>(stage)] == vp::res::ResBfshaShadingModel::cInvalidBaseLocation) { continue; }
                for (u32 j = 0; j < cShaderInterfaceResourceCount - 2; ++j) {
                    const ShaderInterfaceType interface_type = cShaderInterfaceResourceArray[j];
                    const u32 slot_count = program->location_count_array[j];
                    const u32 *slot_array = program->location_array[j];
                    for (u32 k = 0; k < slot_count; ++k) {
                        const u32 slot = slot_array[k + base_location_array[static_cast<u32>(stage)]];
                        if (slot == vp::res::ResBfshaShaderProgram::cInvalidLocation) { continue; }
                        if (!this->AddEntry(interface_type, slot)) { return count; }
                        ++count;
                    }
                }
            }
            return count;
        }
    };

    class ResourceSet {
        private:
            friend class CommandBuffer;
        private:
            static constexpr u16 cInvalidBaseLocation = 0xffff;
        #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
        private:
            using SlotArray = vp::util::HeapArray<u64>;
        private:
            SlotArray m_desc_array;
            GpuAddress m_gpu_address;
        #else
            void *m_desc_memory_array[2];
            u32 m_size_array[2];
            VkDeviceSize m_offset_array[2];
        #endif
        public:
            constexpr ResourceSet() : 
        #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
            m_desc_array(), m_gpu_address()
        #else
            m_desc_memory_array(), m_size_array(), m_offset_array()
        #endif
            {/*...*/}
            constexpr ~ResourceSet();

            static consteval bool IsCpuMemory() {
            #if defined(AWN_ENABLE_VK_DESCRIPTOR_HEAP)
                return false;
            #else
                return true;
            #endif
            }
            static consteval bool IsGpuMemory() { return !IsCpuMemory(); }
            static GpuMemoryRequirements GetGpuMemoryRequirements(const ResourceSetInfo *set_info) {
            #if defined(AWN_ENABLE_VK_DESCRIPTOR_HEAP)
                gfx::BufferInfo info;
                info.SetDefaults();
                info.SetSize(set_info->max_location * sizeof(u64));
                info.SetGpuAccessFlags(GpuAccessFlags::UniformBuffer);
                return Buffer::GetGpuMemoryRequirements(std::addressof(info));
            #else
                VP_NOT_IMPLEMENTED();
            #endif
            }

            Result Initialize(mem::Heap *heap, const ResourceSetInfo *info);
            Result Initialize(MemoryPool *pool, size_t offset, const ResourceSetInfo *info);
            void Finalize();
        public:
        #if defined(AWN_ENABLE_VK_DESCRIPTOR_HEAP)
            constexpr void SetTextureSampler(ShaderStage stage, u32 location, gfx::DescriptorSlot texture_slot, gfx::DescriptorSlot sampler_slot) {
                const TextureSamplerHandle handle = gfx::GetTextureSamplerHandle(texture_slot, sampler_slot);
                VP_ASSERT(handle != cInvalidTextureSamplerHandle);
                m_desc_array[location] = handle.handle;
            }
            constexpr void SetImage(ShaderStage stage, u32 location, gfx::DescriptorSlot texture_slot) {
                const ImageHandle handle = gfx::GetImageHandle(texture_slot).handle;
                VP_ASSERT(handle != cInvalidImageHandle);
                m_desc_array[location] = handle.handle;
            }
            constexpr void SetUniformBuffer(ShaderStage stage, u32 location, GpuAddress address, size_t size) {
                const u64 handle = address.vk_device_address;
                VP_ASSERT(address.IsValid());
                m_desc_array[location] = handle;
            }
            constexpr void SetStorageBuffer(ShaderStage stage, u32 location, GpuAddress address, size_t size) {
                const u64 handle = address.vk_device_address;
                VP_ASSERT(address.IsValid());
                m_desc_array[location] = handle;
            }
            constexpr void SetSeparateTexture(ShaderStage stage, u32 location, gfx::DescriptorSlot texture_slot) {
                const SeparateTextureHandle handle = gfx::GetSeparateTextureHandle(texture_slot).handle;
                VP_ASSERT(handle != cInvalidSeparateTextureHandle);
                m_desc_array[location] = handle.handle;
            }
            constexpr void SetSeparateSampler(ShaderStage stage, u32 location, gfx::DescriptorSlot sampler_slot) {
                const SeparateSamplerHandle handle = gfx::GetSeparateSamplerHandle(sampler_slot).handle;
                VP_ASSERT(handle != cInvalidSeparateSamplerHandle);
                m_desc_array[location] = handle.handle;
            }
        #else
            void SetTextureSampler(ShaderStage stage, u32 location, gfx::DescriptorSlot texture_slot, gfx::DescriptorSlot sampler_slot) {
                impl::GetTextureSamplerDescriptor(m_desc_memory_array[0], location, texture_slot, sampler_slot);
            }
            void SetImage(ShaderStage stage, u32 location, gfx::DescriptorSlot texture_slot) {
                impl::GetImageDescriptor(m_desc_memory_array[0], location, texture_slot);
            }
            void SetUniformBuffer(ShaderStage stage, u32 location, GpuAddress address, size_t size) {
                impl::GetUniformBufferDescriptor(m_desc_memory_array[1], location, address, size);
            }
            void SetStorageBuffer(ShaderStage stage, u32 location, GpuAddress address, size_t size) {
                impl::GetStorageBufferDescriptor(m_desc_memory_array[1], location, address, size);
            }
            void SetSeparateTexture(ShaderStage stage, u32 location, gfx::DescriptorSlot texture_slot) {
                impl::GetSeparateTextureDescriptor(m_desc_memory_array[0], location, texture_slot);
            }
            void SetSeparateSampler(ShaderStage stage, u32 location, gfx::DescriptorSlot sampler_slot) {
                impl::GetSeparateSamplerDescriptor(m_desc_memory_array[0], location, sampler_slot);
            }
        #endif
        private:
            void BindImpl(gfx::CommandBuffer *buffer) const;
    };
}

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
        u32 max_location_array[cShaderStageCount];

        constexpr u32 GetTotalDescriptorCount() const {
            u32 count = 0;
            for (u32 i = 0; i < cShaderStageCount; ++i) {
                count += this->max_location_array[i];
            }
            return count;
        }
        constexpr bool AddEntry(ShaderStage stage, ShaderInterfaceType interface_type, u32 location) {
            VP_UNUSED(interface_type);
            this->max_location_array[static_cast<u32>(stage)] = vp::util::Max(this->max_location_array[static_cast<u32>(stage)], location + 1);
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
                        if (!this->AddEntry(stage, interface_type, slot_array[k])) { return count; }
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
                        if (!this->AddEntry(stage, interface_type, slot)) { return count; }
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
        private:
            using SlotArray = vp::util::HeapArray<u64>;
        private:
            SlotArray m_desc_array;
            GpuAddress m_gpu_address;
            u16 m_base_location_array[cShaderStageCount];
            u16 m_descriptor_count;
        public:
            constexpr ResourceSet() : m_desc_array(), m_gpu_address(), m_base_location_array() {/*...*/}
            constexpr ~ResourceSet() = default;

            Result Initialize(mem::Heap *heap, const ResourceSetInfo *info) {
                VP_UNUSED(heap);
                u32 count = 0;
                for (u32 i = 0; i < cShaderStageCount; ++i) {
                    m_base_location_array[i] = count;
                    count += info->max_location_array[i];
                }
                m_descriptor_count = count;
                RESULT_RETURN_SUCCESS;
            }
            void Finalize() {/*...*/}

            static consteval bool IsRequireGpuMemory() { return true; }
            static GpuMemoryRequirements GetGpuMemoryRequirements(const ResourceSetInfo *set_info) {
                gfx::BufferInfo info;
                info.SetDefaults();
                const u32 count = set_info->GetTotalDescriptorCount();
                info.SetSize(count * sizeof(u64));
                info.SetGpuAccessFlags(GpuAccessFlags::UniformBuffer);
                return Buffer::GetGpuMemoryRequirements(std::addressof(info));
            }
            void SetGpuAddress(GpuAddress gpu_address, void *address, size_t memory_size) {
                VP_ASSERT(m_descriptor_count * sizeof(u64) <= memory_size);
                m_desc_array.SetMemory(reinterpret_cast<u64*>(address), m_descriptor_count);
                m_gpu_address = gpu_address;
                return;
            }

            constexpr bool IsValid() const {
                return m_desc_array.IsValid();
            }
        public:
            constexpr void SetTextureSampler(ShaderStage stage, u32 location, DescriptorSlot texture_slot, DescriptorSlot sampler_slot) {
                const TextureSamplerHandle handle = gfx::GetTextureSamplerHandle(texture_slot, sampler_slot);
                VP_ASSERT(handle != cInvalidTextureSamplerHandle);
                m_desc_array[location + m_base_location_array[static_cast<u32>(stage)]] = handle.handle;
            }
            constexpr void SetImage(ShaderStage stage, u32 location, DescriptorSlot texture_slot) {
                const ImageHandle handle = gfx::GetImageHandle(texture_slot);
                VP_ASSERT(handle != cInvalidImageHandle);
                m_desc_array[location + m_base_location_array[static_cast<u32>(stage)]] = handle.handle;
            }
            constexpr void SetUniformBuffer(ShaderStage stage, u32 location, GpuAddress address, size_t size) {
                VP_UNUSED(size);
                const u64 handle = address.GetVkDeviceAddress();
                VP_ASSERT(address.IsValid());
                m_desc_array[location + m_base_location_array[static_cast<u32>(stage)]] = handle;
            }
            constexpr void SetStorageBuffer(ShaderStage stage, u32 location, GpuAddress address, size_t size) {
                VP_UNUSED(size);
                const u64 handle = address.GetVkDeviceAddress();
                VP_ASSERT(address.IsValid());
                m_desc_array[location + m_base_location_array[static_cast<u32>(stage)]] = handle;
            }
            constexpr void SetSeparateTexture(ShaderStage stage, u32 location, DescriptorSlot texture_slot) {
                const SeparateTextureHandle handle = gfx::GetSeparateTextureHandle(texture_slot);
                VP_ASSERT(handle != cInvalidSeparateTextureHandle);
                m_desc_array[location + m_base_location_array[static_cast<u32>(stage)]] = handle.handle;
            }
            constexpr void SetSeparateSampler(ShaderStage stage, u32 location, DescriptorSlot sampler_slot) {
                const SeparateSamplerHandle handle = gfx::GetSeparateSamplerHandle(sampler_slot);
                VP_ASSERT(handle != cInvalidSeparateSamplerHandle);
                m_desc_array[location + m_base_location_array[static_cast<u32>(stage)]] = handle.handle;
            }
        private:
            void BindImpl(gfx::CommandBuffer *buffer) const;
    };
}

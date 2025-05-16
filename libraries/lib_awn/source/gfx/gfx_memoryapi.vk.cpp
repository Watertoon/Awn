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
#include <awn.hpp>

namespace awn::gfx {

    TextureSamplerHandle GetTextureSamplerHandle(DescriptorSlot texture_view_slot, DescriptorSlot sampler_slot) {
        /* Ensure texture sampler manager */
        impl::TextureViewSamplerManager *texture_sampler_manager = impl::TextureViewSamplerManager::GetInstance();
        if (texture_sampler_manager == nullptr) { return cInvalidTextureSamplerHandle; }
        /* Ensure valid descriptor slots */
        if (!texture_sampler_manager->IsTextureViewSlotValid(texture_view_slot)) { return cInvalidTextureSamplerHandle; }
        if (!texture_sampler_manager->IsSamplerSlotValid(sampler_slot)) { return cInvalidTextureSamplerHandle; }
        /* Handle descriptor indexing vs nv bindless */
        //if (!gfx::IsOptionalFeature(OptionalFeature::SupportsNvBindlessTexture)) {
            /* Get indices */
            return TextureSamplerHandle { .handle = texture_view_slot.index | (static_cast<u64>(sampler_slot.index) << 0x20), };
        //}
        /* Get combined image handler */
        /* Note; this is an undocumented dependency */
        //const impl::TextureViewSamplerManager::TextureViewUnit *texture_view_unit = texture_sampler_manager->GetTextureViewUnit(texture_view_slot.index);
        //const impl::TextureViewSamplerManager::SamplerUnit *sampler_unit = texture_sampler_manager->GetSamplerUnit(sampler_slot.index);
        //return TextureSamplerHandle { .handle = *reinterpret_cast<const u32*>(texture_view_unit->descriptor_memory) | *reinterpret_cast<const u32*>(sampler_unit->descriptor_memory), };
    }

    ImageHandle GetImageHandle(DescriptorSlot texture_view_slot) {
        /* Ensure texture sampler manager */
        impl::TextureViewSamplerManager *texture_sampler_manager = impl::TextureViewSamplerManager::GetInstance();
        if (texture_sampler_manager == nullptr) { return cInvalidImageHandle; }
        /* Ensure valid descriptor slot */
        if (!texture_sampler_manager->IsTextureViewSlotValid(texture_view_slot)) { return cInvalidImageHandle; }
        /* Handle descriptor indexing vs nv bindless */
        //if (!gfx::IsOptionalFeature(OptionalFeature::SupportsNvBindlessTexture)) {
            /* Get index */
            return ImageHandle { .handle = texture_view_slot.index, };
        //}
        /* Get image handle */
        /* Note; this is an undocumented dependency */
        //const impl::TextureViewSamplerManager::TextureViewUnit *texture_view_unit = texture_sampler_manager->GetTextureViewUnit(texture_view_slot.index);
        //return ImageHandle { .handle = *reinterpret_cast<const u32*>(texture_view_unit->descriptor_memory), };
    }

    SeparateTextureHandle GetSeparateTextureHandle(DescriptorSlot texture_view_slot) {
        /* Ensure texture sampler manager */
        impl::TextureViewSamplerManager *texture_sampler_manager = impl::TextureViewSamplerManager::GetInstance();
        if (texture_sampler_manager == nullptr) { return cInvalidSeparateTextureHandle; }
        /* Ensure valid descriptor slot */
        if (!texture_sampler_manager->IsTextureViewSlotValid(texture_view_slot)) { return cInvalidSeparateTextureHandle; }
        /* Handle descriptor indexing vs nv bindless */
        //if (!gfx::IsOptionalFeature(OptionalFeature::SupportsNvBindlessTexture)) {
            /* Get index */
            return SeparateTextureHandle { .handle = texture_view_slot.index, };
        //}
        /* Get separate texture handle */
        /* Note; this is an undocumented dependency */
        //const impl::TextureViewSamplerManager::TextureViewUnit *texture_view_unit = texture_sampler_manager->GetTextureViewUnit(texture_view_slot.index);
        //return SeparateTextureHandle { .handle = *reinterpret_cast<const u32*>(texture_view_unit->descriptor_memory), };
    }

    SeparateSamplerHandle GetSeparateSamplerHandle(DescriptorSlot sampler_slot) {
        /* Ensure texture sampler manager */
        impl::TextureViewSamplerManager *texture_sampler_manager = impl::TextureViewSamplerManager::GetInstance();
        if (texture_sampler_manager == nullptr) { return cInvalidSeparateSamplerHandle; }
        /* Ensure valid descriptor slot */
        if (!texture_sampler_manager->IsSamplerSlotValid(sampler_slot)) { return cInvalidSeparateSamplerHandle; }
        /* Handle descriptor indexing vs nv bindless */
        //if (!gfx::IsOptionalFeature(OptionalFeature::SupportsNvBindlessTexture)) {
            /* Get index */
            return SeparateSamplerHandle { .handle = sampler_slot.index, };
        //}
        /* Get separate sampler handle */
        /* Note; this is an undocumented dependency */
        //const impl::TextureViewSamplerManager::SamplerUnit *sampler_unit = texture_sampler_manager->GetSamplerUnit(sampler_slot.index);
        //return SeparateSamplerHandle { .handle = *reinterpret_cast<const u32*>(sampler_unit->descriptor_memory), };
    }

    bool IsUnifiedMemory() {
        return impl::Context::GetInstance()->IsUnifiedMemory();
    }

    void FlushCpuCache(void *gpu_address, size_t size) {
        mem::FindGpuRootHeapContextFromAddress(gpu_address)->FlushCpuCache(gpu_address, size);
    }

    void InvalidateCpuCache(void *gpu_address, size_t size) {
        mem::FindGpuRootHeapContextFromAddress(gpu_address)->InvalidateCpuCache(gpu_address, size);
    }
}

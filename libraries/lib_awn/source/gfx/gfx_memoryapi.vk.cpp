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
        return TextureSamplerHandle { .handle = texture_view_slot.index | (static_cast<u64>(sampler_slot.index) << 0x20), };
    }

    ImageHandle GetImageHandle(DescriptorSlot texture_view_slot) {
        /* Ensure texture sampler manager */
        impl::TextureViewSamplerManager *texture_sampler_manager = impl::TextureViewSamplerManager::GetInstance();
        if (texture_sampler_manager == nullptr) { return cInvalidImageHandle; }
        /* Ensure valid descriptor slot */
        if (!texture_sampler_manager->IsTextureViewSlotValid(texture_view_slot)) { return cInvalidImageHandle; }
        return ImageHandle { .handle = texture_view_slot.index, };
    }

    SeparateTextureHandle GetSeparateTextureHandle(DescriptorSlot texture_view_slot) {
        /* Ensure texture sampler manager */
        impl::TextureViewSamplerManager *texture_sampler_manager = impl::TextureViewSamplerManager::GetInstance();
        if (texture_sampler_manager == nullptr) { return cInvalidSeparateTextureHandle; }
        /* Ensure valid descriptor slot */
        if (!texture_sampler_manager->IsTextureViewSlotValid(texture_view_slot)) { return cInvalidSeparateTextureHandle; }
        return SeparateTextureHandle { .handle = texture_view_slot.index, };
    }

    SeparateSamplerHandle GetSeparateSamplerHandle(DescriptorSlot sampler_slot) {
        /* Ensure texture sampler manager */
        impl::TextureViewSamplerManager *texture_sampler_manager = impl::TextureViewSamplerManager::GetInstance();
        if (texture_sampler_manager == nullptr) { return cInvalidSeparateSamplerHandle; }
        /* Ensure valid descriptor slot */
        if (!texture_sampler_manager->IsSamplerSlotValid(sampler_slot)) { return cInvalidSeparateSamplerHandle; }
        return SeparateSamplerHandle { .handle = sampler_slot.index, };
    }

    bool IsUnifiedMemory() {
        return impl::Context::GetInstance()->IsUnifiedMemory();
    }

    void FlushCpuCache(void *gpu_address, size_t size) {
        uintptr_t offset;
        MemoryPool *pool = FindMemoryPool(std::addressof(offset), gpu_address);
        if (pool == nullptr) { return; }
        pool->FlushMappedRange(offset, size);
    }

    void InvalidateCpuCache(void *gpu_address, size_t size) {
        uintptr_t offset;
        MemoryPool *pool = FindMemoryPool(std::addressof(offset), gpu_address);
        if (pool == nullptr) { return; }
        pool->InvalidateMappedRange(offset, size);
    }
}

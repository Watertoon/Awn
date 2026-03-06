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

    struct GpuMemoryRequirements {
        size_t size;
        s32 alignment;
    };

    constexpr GpuMemoryRequirements AppendMemoryRequirements(GpuMemoryRequirements base, GpuMemoryRequirements append) {
        const size_t align_size0 = vp::util::AlignUp(base.size, append.alignment);
        const size_t total_size = align_size0 + append.size;
        const s32 max_align = vp::util::Max(base.alignment, append.alignment);
        return { total_size, max_align, };
    }

    constexpr GpuMemoryRequirements MultiplyMemoryRequirements(GpuMemoryRequirements reqs, u32 count) {
        const size_t align_size = vp::util::AlignUp(reqs.size, reqs.alignment);
        return { align_size * count, reqs.alignment, };
    }

    static constexpr inline u32 cTargetMaxTextureDescriptorCount = 0x1'0000;
    static constexpr inline u32 cTargetMaxSamplerDescriptorCount = 0x1000;
    static constexpr inline u32 cMaxBindlessDescriptorCount = vp::util::Max(cTargetMaxSamplerDescriptorCount, cTargetMaxTextureDescriptorCount);
    static_assert(cMaxBindlessDescriptorCount <= 0xffff'ffff);

    using DescriptorSlotIndexType = u32;

    constexpr inline u32 cInvalidDescriptorSlotFreeCount = 0xffff'ffff;

    struct DescriptorSlot {
        union {
            struct {
                u32 index = cInvalidDescriptorSlotFreeCount;
                u32 free_count = cInvalidDescriptorSlotFreeCount;
            };
            u64 state;
        };
        
        constexpr bool operator==(const DescriptorSlot &rhs) const { return this->state == rhs.state; }
    };
    static_assert(sizeof(DescriptorSlot) == sizeof(u64));

    constexpr inline DescriptorSlot cInvalidDescriptorSlot = DescriptorSlot{ cInvalidDescriptorSlotFreeCount, cInvalidDescriptorSlotFreeCount, };

    struct TextureSamplerHandle {
        u64 handle = 0xffff'ffff'ffff'ffff;
    };
    static constexpr TextureSamplerHandle cInvalidTextureSamplerHandle = TextureSamplerHandle{};

    TextureSamplerHandle GetTextureSamplerHandle(DescriptorSlot texture_view_slot, DescriptorSlot sampler_slot);

    struct ImageHandle {
        u64 handle = 0xffff'ffff'ffff'ffff;
    };
    static constexpr ImageHandle cInvalidImageHandle = ImageHandle{};

    ImageHandle GetImageHandle(DescriptorSlot texture_view_slot);

    struct SeparateTextureHandle {
        u64 handle = 0xffff'ffff'ffff'ffff;
    };
    static constexpr SeparateTextureHandle cInvalidSeparateTextureHandle = SeparateTextureHandle{};

    SeparateTextureHandle GetSeparateTextureHandle(DescriptorSlot texture_view_slot);

    struct SeparateSamplerHandle {
        u64 handle = 0xffff'ffff'ffff'ffff;
    };
    static constexpr SeparateSamplerHandle cInvalidSeparateSamplerHandle = SeparateSamplerHandle{};

    SeparateSamplerHandle GetSeparateSamplerHandle(DescriptorSlot sampler_slot);

    bool IsUnifiedMemory();

    void FlushCpuCache(void *gpu_address, size_t size);
    void InvalidateCpuCache(void *gpu_address, size_t size);
}

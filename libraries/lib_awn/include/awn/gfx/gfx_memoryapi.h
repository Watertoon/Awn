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

    using GpuAddress = impl::GpuAddress;
    static constexpr GpuAddress cInvalidGpuAddress = impl::cInvalidGpuAddress;

    struct GpuAlloc {
        gfx::GpuAddress gpu_address;
        void *address;

        constexpr bool IsValid() const { return gpu_address.IsValid(); }
        constexpr bool IsAligned(size_t alignment) const { return gpu_address.IsAligned(alignment); }
        constexpr gfx::GpuAddress GetGpuAddress() const { return gpu_address; }
        constexpr void *GetAddress() const { return address; }
    };

    struct GpuMemoryRequirements {
        size_t size;
        size_t alignment;

        constexpr size_t GetTotalSize() const { return this->size; }
        constexpr size_t GetMaxAlignment() const { return this->alignment; }
    };

    constexpr GpuMemoryRequirements AppendMemoryRequirements(GpuMemoryRequirements base, GpuMemoryRequirements append) {
        const size_t align_size0 = vp::util::AlignUp(base.size, append.alignment);
        const size_t total_size = align_size0 + append.size;
        const size_t max_align = vp::util::Max(base.alignment, append.alignment);
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
        constexpr bool operator==(const TextureSamplerHandle &rhs) const { return this->handle == rhs.handle; }
    };
    static constexpr TextureSamplerHandle cInvalidTextureSamplerHandle = TextureSamplerHandle{};

    TextureSamplerHandle GetTextureSamplerHandle(DescriptorSlot texture_view_slot, DescriptorSlot sampler_slot);

    struct ImageHandle {
        u64 handle = 0xffff'ffff'ffff'ffff;
        constexpr bool operator==(const ImageHandle &rhs) const { return this->handle == rhs.handle; }
    };
    static constexpr ImageHandle cInvalidImageHandle = ImageHandle{};

    ImageHandle GetImageHandle(DescriptorSlot texture_view_slot);

    struct SeparateTextureHandle {
        u64 handle = 0xffff'ffff'ffff'ffff;
        constexpr bool operator==(const SeparateTextureHandle &rhs) const { return this->handle == rhs.handle; }
    };
    static constexpr SeparateTextureHandle cInvalidSeparateTextureHandle = SeparateTextureHandle{};

    SeparateTextureHandle GetSeparateTextureHandle(DescriptorSlot texture_view_slot);

    struct SeparateSamplerHandle {
        u64 handle = 0xffff'ffff'ffff'ffff;
        constexpr bool operator==(const SeparateSamplerHandle &rhs) const { return this->handle == rhs.handle; }
    };
    static constexpr SeparateSamplerHandle cInvalidSeparateSamplerHandle = SeparateSamplerHandle{};

    SeparateSamplerHandle GetSeparateSamplerHandle(DescriptorSlot sampler_slot);

    bool IsUnifiedMemory();

    void FlushCpuCache(void *gpu_address, size_t size);
    void InvalidateCpuCache(void *gpu_address, size_t size);
}

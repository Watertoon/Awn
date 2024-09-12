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
        size_t alignment;
    };

    static constexpr inline u32 cTargetMaxTextureDescriptorCount = 0xa000;
    static constexpr inline u32 cTargetMaxSamplerDescriptorCount = 4000;
    static constexpr inline u32 cMaxBindlessDescriptorCount = vp::util::Max(cTargetMaxSamplerDescriptorCount, cTargetMaxTextureDescriptorCount);

    using DescriptorSlotIndexType = vp::util::MinUIntType<cMaxBindlessDescriptorCount>;

    constexpr inline u16 cInvalidDescriptorSlotFreeCount = 0xffff;

    struct DescriptorSlot {
        DescriptorSlotIndexType index      = 0;
        u16                     free_count = cInvalidDescriptorSlotFreeCount;
    };
    static_assert(sizeof(DescriptorSlot) <= sizeof(u64));

    void FlushCpuCache(void *gpu_address, size_t size);
    void InvalidateCpuCache(void *gpu_address, size_t size);
}

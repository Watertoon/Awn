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

namespace awn::frm {

    static constexpr sys::CoreMask cAllCores = 0xffff'ffff'ffff'ffff;
    static constexpr sys::CoreMask cDefaultCoreMask = 0b111;
    static constexpr u32 cDefaultHeapSize = vp::util::c32MB;

    struct LibraryInfo {
        sys::CoreMask process_core_mask = cDefaultCoreMask;
        size_t        root_heap_size    = cDefaultHeapSize;
    };

    void InitializeLibraries(const LibraryInfo *lib_info);
    void FinalizeLibraries();

    struct GraphicsInfo {
        size_t cpu_uncached_memory_size         = vp::util::c16MB;
        size_t cpu_cached_memory_size           = vp::util::c16MB;
        size_t gpu_uncached_memory_size         = vp::util::c32MB;
        u32    per_stage_descriptor_memory_size = vp::util::c8MB;
        u32    max_texture_view_count           = 0x100;
        u32    max_sampler_count                = 0x40;
        u32    max_command_buffer_count         = 0x4;
        u32    max_command_list_count           = 0x20;
    };

    void InitializeGraphics(const GraphicsInfo *graphics_info);
    void FinalizeGraphics();
}

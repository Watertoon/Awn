#pragma once

namespace awn::gfx {
    using BufferInfo  = vp::res::ResGfxBufferInfo;
    using TextureInfo = vp::res::ResGfxTextureInfo;
}

namespace awn::mem {

    struct GpuRootHeapContext;

    struct GpuMemoryAddress {
        GpuRootHeapContext *gpu_root_heap_context;
        void               *address;

        VkBuffer CreateBuffer(VkBufferUsageFlags usage_flags, size_t size);
        ALWAYS_INLINE VkBuffer CreateBuffer(gfx::BufferInfo *buffer_info) {
            return this->CreateBuffer(vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(buffer_info->gpu_access_flags)), buffer_info->size);
        }

        VkImage CreateImage(VkImageUsageFlags image_usage_flags, gfx::TextureInfo *texture_info);
        ALWAYS_INLINE VkImage CreateImage(gfx::TextureInfo *texture_info) {
            return this->CreateImage(vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)), texture_info);
        }

        void FlushCpuCache(size_t size);
        void InvalidateCpuCache(size_t size);

        constexpr ALWAYS_INLINE GpuRootHeapContext *GetGpuRootHeap() const { return gpu_root_heap_context; }
        constexpr ALWAYS_INLINE void               *GetAddress()     const { return address; }
    };
}

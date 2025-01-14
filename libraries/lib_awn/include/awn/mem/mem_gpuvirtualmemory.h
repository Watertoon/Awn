#include <awn.hpp>

namespace awn::mem {

    Result AllocateGpuMemoryHandle(GpuMemoryHandle *out_memory_handle, mem::Heap *meta_data_heap, gfx::MemoryPropertyFlags gpu_memory_property_flags, size_t size);
    Result AllocateGpuMemoryHandle(GpuMemoryHandle *out_memory_handle, mem::Heap *meta_data_heap, gfx::MemoryPropertyFlags gpu_memory_property_flags, size_t size, const void *pnext);
    void FreeGpuMemoryHandle(GpuMemoryHandle handle);

    GpuMemoryHandle FindGpuMemoryHandle(size_t *out_offset, void *address);
}

#include <awn.hpp>

namespace awn::mem {

    GpuMemoryHandle AllocateGpuMemoryHandle(mem::Heap *meta_data_heap, gfx::MemoryPropertyFlags gpu_memory_property_flags, size_t size);
    GpuMemoryHandle AllocateGpuMemoryHandle(mem::Heap *meta_data_heap, gfx::MemoryPropertyFlags gpu_memory_property_flags, size_t size, const void *pnext);
    void FreeGpuMemoryHandle(GpuMemoryHandle handle);

    void *MapGpuMemoryHandle(GpuMemoryHandle handle);
}

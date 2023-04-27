#pragma once

namespace awn::mem {

    class Heap;

    struct OutOfMemoryInfo {
        Heap   *out_of_memory_heap;
        size_t  allocation_size;
        size_t  aligned_allocation_size;
        size_t  alignment;
    };

    using OutOfMemoryCallback = void (*)(OutOfMemoryInfo*);

    struct RootHeapInfo {
        void       *arena;
        size_t      arena_size;
    };

    struct HeapManager {
        using OutOfMemoryDelegate = vp::util::DelegateFunction<OutOfMemoryInfo*>;

        static constexpr u32 cMaxRootHeaps = 3;

        Heap                *root_heap_array[cMaxRootHeaps];
        OutOfMemoryDelegate  out_of_memory_delegate;
    };

    struct HeapManagerInfo {
        u32                  root_heap_count;
        RootHeapInfo        *root_heap_info_array;
        OutOfMemoryCallback  out_of_memory_callback;
    };
    bool InitializeHeapManager(HeapManagerInfo *heap_manager_info);
    void FinalizeHeapManager();

    HeapManager *GetHeapManager();
    Heap        *GetRootHeap();

    bool IsHeapManagerInitialized();

    mem::Heap *GetCurrentThreadHeap();
    void       SetCurrentThreadHeap(mem::Heap *heap);

    mem::Heap *FindHeapByName(const char *heap_name);
    mem::Heap *FindHeapFromAddress(void *address);
    bool       IsAddressFromAnyHeap(void *address);
}

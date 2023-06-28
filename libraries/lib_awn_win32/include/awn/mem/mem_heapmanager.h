#pragma once

namespace awn::sys {
    class ServiceCriticalSection;
}

namespace awn::mem {

    class Heap;

    struct OutOfMemoryInfo {
        Heap   *out_of_memory_heap;
        size_t  allocation_size;
        size_t  aligned_allocation_size;
        s32     alignment;
    };

    using OutOfMemoryCallback = bool (*)(OutOfMemoryInfo*);

    struct RootHeapInfo {
        void       *arena;
        size_t      arena_size;
    };

    struct HeapManager {
        using OutOfMemoryDelegate = vp::util::DelegateReturnFunction<bool, OutOfMemoryInfo*>;

        static constexpr u32 cMaxRootHeaps = 3;

        Heap                *root_heap_array[cMaxRootHeaps];
        size_t               out_of_memory_resize_alignment;
        OutOfMemoryDelegate  out_of_memory_delegate;
    };

    struct HeapManagerInfo {
        u32                  root_heap_count;
        RootHeapInfo        *root_heap_info_array;
        size_t               out_of_memory_resize_alignment;
        OutOfMemoryCallback  out_of_memory_callback;
    };
    bool InitializeHeapManager(HeapManagerInfo *heap_manager_info);
    void FinalizeHeapManager();

    HeapManager *GetHeapManager();
    Heap        *GetRootHeap(u32 index);
    size_t       GetRootHeapTotalSize(u32 index);

    bool IsHeapManagerInitialized();

    bool   OutOfMemoryImpl(OutOfMemoryInfo *out_of_memory_info);
    size_t GetOutOfMemoryResizeAlignment();

    sys::ServiceCriticalSection *GetHeapManagerLock();

    mem::Heap *FindHeapByName(const char *heap_name);
    mem::Heap *FindHeapFromAddress(void *address);
    bool       IsAddressFromAnyHeap(void *address);

    mem::Heap *GetCurrentThreadHeap();
    void       SetCurrentThreadHeap(mem::Heap *heap);

    class ScopedCurrentThreadHeap {
        private:
            mem::Heap *m_last_heap;
        public:
            explicit ALWAYS_INLINE ScopedCurrentThreadHeap(mem::Heap *current_heap) : m_last_heap(GetCurrentThreadHeap()) { VP_ASSERT(current_heap != nullptr); SetCurrentThreadHeap(current_heap); }
            ALWAYS_INLINE ~ScopedCurrentThreadHeap() { SetCurrentThreadHeap(m_last_heap); }
    };
}

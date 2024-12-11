#include <awn.hpp>

namespace awn::frm {

    void InitializeLibraries(const LibraryInfo *lib_info) {
        VP_ASSERT(lib_info != nullptr);

        /* Initialize timestamp */
        vp::util::InitializeTimeStamp();

        /* Initialize system manager */
        sys::InitializeSystem();
        const sys::CoreMask core_mask = (lib_info->process_core_mask == cAllCores) ? ~((~0ull) << (sys::GetProcessCoreCount() & (BITSIZEOF(sys::CoreMask) - 1))) : lib_info->process_core_mask;
        RESULT_ABORT_UNLESS(sys::SetProcessCoreMask(core_mask));

        /* Initialize heap manager */
        void *root_heap_memory = nullptr;
        if (sys::IsEnableLargePages()) {
            const size_t root_size = vp::util::AlignUp(lib_info->root_heap_size, sys::GetLargePageSize());
            RESULT_ABORT_UNLESS(mem::AllocateLargeAddressRegionWithMemoryPages(std::addressof(root_heap_memory), root_size));
        } else {
            const size_t root_size = vp::util::AlignUp(lib_info->root_heap_size, mem::cPageSize);
            RESULT_ABORT_UNLESS(mem::AllocateAddressRegion(std::addressof(root_heap_memory), root_size));
            RESULT_ABORT_UNLESS(mem::AllocateMemoryPages(root_heap_memory, root_size));
        }
        mem::RootHeapInfo root_heap_info_win32 = {
            .arena      = root_heap_memory,
            .arena_size = lib_info->root_heap_size
        };
        mem::HeapManagerInfo heap_mgr_info = {
            .root_heap_count      = 1,
            .root_heap_info_array = std::addressof(root_heap_info_win32),
        };
        const bool result1 = mem::InitializeHeapManager(std::addressof(heap_mgr_info));
        VP_ASSERT(result1);

        /* Create library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        mem::Heap *awn_library_heap = mem::ExpHeap::TryCreate("awn::frm::Library", root_heap, mem::Heap::cWholeSize, 8, false);
        mem::ScopedCurrentThreadHeap heap_scope(awn_library_heap);

        /* Initialize thread mgr */
        sys::ThreadManager *thread_mgr = sys::ThreadManager::CreateInstance(awn_library_heap);
        thread_mgr->Initialize(awn_library_heap);

        /* Initialize file device mgr */
        res::impl::FileDeviceManager *file_device_mgr = res::impl::FileDeviceManager::CreateInstance(awn_library_heap);
        file_device_mgr->Initialize(awn_library_heap);

        /* Initialize resource factory mgr */
        res::impl::ResourceFactoryManager::CreateInstance(awn_library_heap);

        /* Create virtual address heap */
        mem::VirtualAddressHeap::Create("VirtualAddressHeap");

        awn_library_heap->AdjustHeap();

        return;
    }

    void FinalizeLibraries() {

        /* Finalize library */
        mem::Heap *lib_heap = mem::FindHeapByName("awn::frm::Library");
        VP_ASSERT(lib_heap != nullptr);
        {
            mem::ScopedCurrentThreadHeap heap_scope(lib_heap);

            res::impl::ResourceFactoryManager::DeleteInstance();

            res::impl::FileDeviceManager::GetInstance()->Finalize();
            res::impl::FileDeviceManager::DeleteInstance();

            sys::ThreadManager::DeleteInstance();
        }

        /* Free library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        root_heap->Free(lib_heap);

        /* Free root heap */
        mem::FinalizeHeapManager();
        if (sys::IsEnableLargePages()) {
            mem::FreeLargeAddressRegionWithMemoryPages(root_heap);
        } else {
            mem::FreeAddressRegion(root_heap);
        }

        /* Free virtual address heap */
        mem::VirtualAddressHeap::Delete();

        return;
    }

    mem::GpuMemoryHandle sGpuMemoryHandleCpuUncached;
    mem::GpuMemoryHandle sGpuMemoryHandleCpuCached;    
    mem::GpuMemoryHandle sGpuMemoryHandleGpuUncached;

    void InitializeGraphics(const GraphicsInfo *graphics_info) {

        /* Create library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        mem::Heap *awn_graphics_heap = mem::ExpHeap::TryCreate("awn::frm::Graphics", root_heap, mem::Heap::cWholeSize, 8, false);

        /* Initialize graphics context */
#ifdef VP_TARGET_GRAPHICS_API_vk
        gfx::impl::Context *context = gfx::impl::Context::CreateInstance(awn_graphics_heap);
        gfx::impl::ContextInfo context_info = {};
        context->Initialize(std::addressof(context_info));
#elif VP_TARGET_GRAPHICS_API_nvn
        gfx::impl::Context *context = gfx::impl::Context::CreateInstance(awn_graphics_heap);
        gfx::impl::ContextInfo context_info = {};
        context->Initialize(awn_graphics_heap, std::addressof(context_info));
#endif

        /* Initialize gpu heap manager */
        mem::GpuHeapManager *gpu_heap_mgr = mem::GpuHeapManager::CreateInstance(awn_graphics_heap);
        sGpuMemoryHandleCpuUncached = mem::AllocateGpuMemoryHandle(awn_graphics_heap, gfx::MemoryPropertyFlags::CpuUncached, graphics_info->cpu_uncached_memory_size);
        sGpuMemoryHandleCpuCached = mem::AllocateGpuMemoryHandle(awn_graphics_heap, gfx::MemoryPropertyFlags::CpuCached, graphics_info->cpu_cached_memory_size);
        sGpuMemoryHandleGpuUncached = mem::AllocateGpuMemoryHandle(awn_graphics_heap, gfx::MemoryPropertyFlags::GpuUncached | gfx::MemoryPropertyFlags::CpuUncached, graphics_info->gpu_uncached_memory_size);
        const mem::GpuHeapManagerInfo gpu_heap_mgr_info = {
            .cpu_uncached_root_heap_count = 1,
            .cpu_cached_root_heap_count   = 1,
            .gpu_uncached_root_heap_count = 1,
            .cpu_uncached_info_array = {
                {                    
                    .handle = sGpuMemoryHandleCpuUncached,
                    .size = graphics_info->cpu_uncached_memory_size,
                },
            },
            .cpu_cached_info_array = {
                {                    
                    .handle = sGpuMemoryHandleCpuCached,
                    .size = graphics_info->cpu_cached_memory_size, 
                },
            },
            .gpu_uncached_info_array = {
                {                    
                    .handle = sGpuMemoryHandleGpuUncached,
                    .size = graphics_info->gpu_uncached_memory_size,
                },
            },
        };
        gpu_heap_mgr->Initialize(awn_graphics_heap, std::addressof(gpu_heap_mgr_info));
        mem::Heap *gpu_heap_gpu_host_uncached = gpu_heap_mgr->GetGpuRootHeapGpuUncached(0);

#ifdef VP_TARGET_GRAPHICS_API_vk
        /* Initialize command pool manager */
        gfx::impl::CommandPoolManager *cmd_pool_mgr = gfx::impl::CommandPoolManager::CreateInstance(awn_graphics_heap);
        cmd_pool_mgr->Initialize(awn_graphics_heap, graphics_info->max_command_list_count);

        /* Initialize descriptor memory manager */
        const gfx::impl::DescriptorMemoryManagerInfo desc_mgr_info = {
            .max_command_buffer_count = graphics_info->max_command_buffer_count,
            .per_stage_memory_size = graphics_info->per_stage_descriptor_memory_size,
        };
        gfx::impl::DescriptorMemoryManager *desc_memory_mgr = gfx::impl::DescriptorMemoryManager::CreateInstance(awn_graphics_heap);
        desc_memory_mgr->Initialize(awn_graphics_heap, gpu_heap_gpu_host_uncached, std::addressof(desc_mgr_info));
#endif

        /* Initialize texture view sampler manager */
        gfx::impl::TextureViewSamplerManager *tex_samp_mgr = gfx::impl::TextureViewSamplerManager::CreateInstance(awn_graphics_heap);
        const gfx::impl::TextureViewSamplerManagerInfo texture_sampler_mgr_info = {
            .max_texture_view_unit_count = graphics_info->max_texture_view_count,
            .max_sampler_unit_count      = graphics_info->max_sampler_count,
        };
        tex_samp_mgr->Initialize(awn_graphics_heap, gpu_heap_gpu_host_uncached, std::addressof(texture_sampler_mgr_info));

        awn_graphics_heap->AdjustHeap();

        return;
    }

    void FinalizeGraphics() {

        /* Finalize graphics */
        mem::Heap *gfx_heap = mem::FindHeapByName("awn::frm::Graphics");
        VP_ASSERT(gfx_heap != nullptr);
        {
            mem::ScopedCurrentThreadHeap heap_scope(gfx_heap);

            gfx::impl::TextureViewSamplerManager::GetInstance()->Finalize();
            gfx::impl::TextureViewSamplerManager::DeleteInstance();

#ifdef VP_TARGET_GRAPHICS_API_vk
            gfx::impl::DescriptorMemoryManager::GetInstance()->Finalize();
            gfx::impl::DescriptorMemoryManager::DeleteInstance();

            gfx::impl::CommandPoolManager::GetInstance()->Finalize();
            gfx::impl::CommandPoolManager::DeleteInstance();
#endif

            mem::GpuHeapManager::GetInstance()->Finalize();
            mem::GpuHeapManager::DeleteInstance();

            if (sGpuMemoryHandleCpuUncached.IsValid()) {
                mem::FreeGpuMemoryHandle(sGpuMemoryHandleCpuUncached);
                sGpuMemoryHandleCpuUncached = mem::cInvalidGpuMemoryHandle;
            }
            if (sGpuMemoryHandleCpuCached.IsValid()) {
                mem::FreeGpuMemoryHandle(sGpuMemoryHandleCpuCached);
                sGpuMemoryHandleCpuCached = mem::cInvalidGpuMemoryHandle;
            }
            if (sGpuMemoryHandleGpuUncached.IsValid()) {
                mem::FreeGpuMemoryHandle(sGpuMemoryHandleGpuUncached);
                sGpuMemoryHandleGpuUncached = mem::cInvalidGpuMemoryHandle;
            }

            gfx::impl::Context::GetInstance()->Finalize();
            gfx::impl::Context::DeleteInstance();
        }

        /* Free library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        root_heap->Free(gfx_heap);

        return;
    }
}

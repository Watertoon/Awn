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
        mem::RootHeapInfo root_heap_info = {
            .arena = root_heap_memory,
            .arena_size = lib_info->root_heap_size
        };
        mem::HeapManagerInfo heap_mgr_info = {
            .root_heap_count = 1,
            .root_heap_info_array = std::addressof(root_heap_info),
        };
        const bool result1 = mem::InitializeHeapManager(std::addressof(heap_mgr_info));
        VP_ASSERT(result1);

        /* Create library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        mem::Heap *awn_library_heap = mem::ExpHeap::TryCreate("awn::frm::Library", root_heap, mem::Heap::cWholeSize, alignof(size_t), false);
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

        sys::FinalizeSystem();

        return;
    }

    gfx::MemoryPool sMemoryPoolCpuUncached;
    gfx::MemoryPool sMemoryPoolCpuCached;    
    gfx::MemoryPool sMemoryPoolGpuUncached;

    void InitializeGraphics(const GraphicsInfo *graphics_info) {

        /* Create library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        mem::Heap *awn_graphics_heap = mem::ExpHeap::TryCreate("awn::frm::Graphics", root_heap, mem::Heap::cWholeSize, alignof(size_t), false);

        /* Initialize graphics context */
#ifdef VP_TARGET_GRAPHICS_API_vk
        gfx::impl::Context *context = gfx::impl::Context::CreateInstance(awn_graphics_heap);
        gfx::impl::ContextInfo context_info = {
            .texture_view_descriptor_count = graphics_info->max_texture_view_count,
            .sampler_descriptor_count = graphics_info->max_sampler_count,
        };
        context->Initialize(std::addressof(context_info));
#elif VP_TARGET_GRAPHICS_API_nvn
        gfx::impl::Context *context = gfx::impl::Context::CreateInstance(awn_graphics_heap);
        gfx::impl::ContextInfo context_info = {};
        context->Initialize(awn_graphics_heap, std::addressof(context_info));
#endif

        /* Initialize gpu heap manager */
        mem::GpuHeapManager *gpu_heap_mgr = mem::GpuHeapManager::CreateInstance(awn_graphics_heap);
        gfx::MemoryPoolInfo pool_info;
        pool_info.SetDefaults();
        const bool is_cpu_uncached = (0 < graphics_info->cpu_uncached_memory_size);
        const bool is_cpu_cached = (0 < graphics_info->cpu_cached_memory_size);
        const bool is_gpu_uncached = (0 < graphics_info->gpu_uncached_memory_size);
        if (is_cpu_uncached) {
            pool_info.SetMemoryPropertyFlags(gfx::MemoryPropertyFlags::CpuUncached);
            pool_info.SetSize(graphics_info->cpu_uncached_memory_size);
            RESULT_ABORT_UNLESS(sMemoryPoolCpuUncached.Initialize(std::addressof(pool_info)));
        }
        if (is_cpu_cached) {
            pool_info.SetMemoryPropertyFlags(gfx::MemoryPropertyFlags::CpuCached);
            pool_info.SetSize(graphics_info->cpu_uncached_memory_size);
            RESULT_ABORT_UNLESS(sMemoryPoolCpuCached.Initialize(std::addressof(pool_info)));
        }
        if (is_gpu_uncached) {
            pool_info.SetMemoryPropertyFlags(gfx::MemoryPropertyFlags::GpuUncached | gfx::MemoryPropertyFlags::CpuUncached);
            pool_info.SetSize(graphics_info->gpu_uncached_memory_size);
            RESULT_ABORT_UNLESS(sMemoryPoolGpuUncached.Initialize(std::addressof(pool_info)));
        }
        const mem::GpuHeapManagerInfo gpu_heap_mgr_info = {
            .cpu_uncached_root_heap_count = is_cpu_uncached,
            .cpu_cached_root_heap_count   = is_cpu_cached,
            .gpu_uncached_root_heap_count = is_gpu_uncached,
            .cpu_uncached_info_array = {
                {
                    .memory_pool = std::addressof(sMemoryPoolCpuUncached),
                    .size = graphics_info->cpu_uncached_memory_size,
                },
            },
            .cpu_cached_info_array = {
                {
                    .memory_pool = std::addressof(sMemoryPoolCpuCached),
                    .size = graphics_info->cpu_cached_memory_size, 
                },
            },
            .gpu_uncached_info_array = {
                {
                    .memory_pool = std::addressof(sMemoryPoolGpuUncached),
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
            .max_command_list_count = graphics_info->max_command_list_count,
            .per_stage_memory_size = graphics_info->per_stage_descriptor_memory_size,
        };
        gfx::impl::DescriptorMemoryManager *desc_memory_mgr = gfx::impl::DescriptorMemoryManager::CreateInstance(awn_graphics_heap);
        desc_memory_mgr->Initialize(awn_graphics_heap, gpu_heap_gpu_host_uncached, std::addressof(desc_mgr_info));
#endif

        /* Initialize texture view sampler manager */
        gfx::impl::TextureViewSamplerManager *tex_samp_mgr = gfx::impl::TextureViewSamplerManager::CreateInstance(awn_graphics_heap);
        const gfx::impl::TextureViewSamplerManagerInfo texture_sampler_mgr_info = {
            .max_texture_view_unit_count = graphics_info->max_texture_view_count,
            .max_sampler_unit_count = graphics_info->max_sampler_count,
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

            sMemoryPoolCpuUncached.Finalize();
            sMemoryPoolCpuCached.Finalize();
            sMemoryPoolGpuUncached.Finalize();

            gfx::impl::Context::GetInstance()->Finalize();
            gfx::impl::Context::DeleteInstance();
        }

        /* Free library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        root_heap->Free(gfx_heap);

        return;
    }
}

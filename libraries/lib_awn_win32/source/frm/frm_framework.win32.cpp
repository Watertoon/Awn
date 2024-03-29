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
#include <awn.hpp>

namespace awn::frm {

    bool Framework::OutOfMemoryCallback(mem::OutOfMemoryInfo *out_of_mem_info) {

        /* Integrity checks */
        VP_ASSERT(out_of_mem_info->allocation_size != mem::Heap::cWholeSize && out_of_mem_info->out_of_memory_heap->IsThreadSafe() != true);

        /* Calculate new target size */
        const size_t new_size = vp::util::AlignUp(out_of_mem_info->out_of_memory_heap->GetTotalSize() + out_of_mem_info->aligned_allocation_size, mem::GetOutOfMemoryResizeAlignment());

        /* Allocate more system memory if it's the root heap */
        if (out_of_mem_info->out_of_memory_heap == mem::GetRootHeap(0)) {
            const size_t size = out_of_mem_info->out_of_memory_heap->GetTotalSize();
            const void *new_address = ::VirtualAlloc(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(out_of_mem_info->out_of_memory_heap) + size), new_size - size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            VP_ASSERT(new_address != nullptr);
        }

        /* Try expand heap */
        const size_t adjust_size = out_of_mem_info->out_of_memory_heap->ResizeHeapBack(new_size);
        if (new_size != adjust_size) { return false; }

        /* Ensure we have enough memory */
        const size_t allocatable_size = out_of_mem_info->out_of_memory_heap->GetMaximumAllocatableSize(out_of_mem_info->alignment);
        if (allocatable_size < out_of_mem_info->aligned_allocation_size) { return false; }

        return true;
    }

    Result Framework::InitializeLibraries(FrameworkLibraryInfo *framework_lib_info) {

        /* Integrity check */
        VP_ASSERT(framework_lib_info != nullptr);

        /* Initialize time */
        vp::util::InitializeTimeStamp();

        /* Initialize System Manager */
        sys::InitializeSystemManager();

        /* Initialize ukern */
        ukern::InitializeUKern(framework_lib_info->process_core_count);

        /* Allocate root heap memory */
        void *root_heap_start = ::VirtualAlloc(nullptr, framework_lib_info->root_heap_initial_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        RESULT_RETURN_UNLESS(root_heap_start != nullptr, ResultFailedToAllocateRootHeap);

        /* Initialize heap mgr */
        mem::RootHeapInfo root_heap_info_win32 = {
            .arena      = root_heap_start,
            .arena_size = framework_lib_info->root_heap_initial_size
        };
        mem::HeapManagerInfo heap_mgr_info = {
            .root_heap_count                = 1,
            .root_heap_info_array           = std::addressof(root_heap_info_win32),
            .out_of_memory_resize_alignment = framework_lib_info->out_of_memory_resize_alignment,
            .out_of_memory_callback         = OutOfMemoryCallback
        };
        const bool result1 = mem::InitializeHeapManager(std::addressof(heap_mgr_info));
        RESULT_RETURN_UNLESS(result1 == true, ResultFailedToInitializeMemHeapManager);

        /* Get root heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);

        /* Create library heap */
        mem::Heap *awn_library_heap = mem::ExpHeap::TryCreate("awn::frm::Framework Libs", root_heap, mem::Heap::cWholeSize, 8, false);

        /* Set current heap */
        mem::ScopedCurrentThreadHeap heap_scope(awn_library_heap);

        /* Initialize thread mgr */
        sys::ThreadManager *thread_mgr = sys::ThreadManager::CreateInstance(awn_library_heap);
        thread_mgr->Initialize(awn_library_heap);

        /* Initialize file device mgr */
        res::FileDeviceManager *file_device_mgr = res::FileDeviceManager::CreateInstance(awn_library_heap);
        file_device_mgr->Initialize(awn_library_heap);

        /* Initialize resource factory mgr */
        res::ResourceFactoryManager::CreateInstance(awn_library_heap);

        /* Initialize input */
        hid::InitializeRawInputThread(awn_library_heap);

        /* Adjust library heap size */
        awn_library_heap->AdjustHeap();

        RESULT_RETURN_SUCCESS;
    }

    void Framework::FinalizeLibraries() {

        /* Finalize library */
        mem::Heap *lib_heap = mem::FindHeapByName("awn::frm::Framework Libs");
        VP_ASSERT(lib_heap != nullptr);
        {
            mem::ScopedCurrentThreadHeap heap_scope(lib_heap);

            hid::FinalizeRawInputThread();

            res::ResourceFactoryManager::DeleteInstance();

            res::FileDeviceManager::GetInstance()->Finalize();
            res::FileDeviceManager::DeleteInstance();

            sys::ThreadManager::DeleteInstance();
        }

        /* Free library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        root_heap->Free(lib_heap);

        //size_t     total_heap_size = mem::GetRootHeapTotalSize(0);
        mem::FinalizeHeapManager();

        const bool result1 = ::VirtualFree(root_heap, 0, MEM_RELEASE);
        VP_ASSERT(result1 == true);

        return;
    }

    Result Framework::InitializeGraphics(FrameworkGraphicsInfo *graphics_info) {

        /* Create library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        mem::Heap *awn_graphics_heap = mem::ExpHeap::TryCreate("awn::frm::Framework Graphics", root_heap, mem::Heap::cWholeSize, 8, false);

        /* Initialize graphics context */
        gfx::Context *context = gfx::Context::CreateInstance(awn_graphics_heap);

        gfx::ContextInfo context_info = {};
        context_info.SetDefaults();

        context->Initialize(std::addressof(context_info));

        /* Initialize gpu heap mgr */
        mem::GpuHeapManager *gpu_heap_mgr = mem::GpuHeapManager::CreateInstance(awn_graphics_heap);
        const mem::GpuHeapManagerInfo gpu_heap_mgr_info = {
            .host_uncached_root_heap_count     = 1,
            .host_cached_root_heap_count       = 1,
            .gpu_host_uncached_root_heap_count = 1,
            .host_uncached_size_array          = { graphics_info->host_uncached_memory_size },
            .host_cached_size_array            = { graphics_info->host_cached_memory_size },
            .gpu_host_uncached_size_array      = { graphics_info->gpu_host_uncached_memory_size },
        };
        gpu_heap_mgr->Initialize(awn_graphics_heap, std::addressof(gpu_heap_mgr_info));

        /* Initialize command pool mgr */
        gfx::CommandPoolManager *cmd_pool_mgr = gfx::CommandPoolManager::CreateInstance(awn_graphics_heap);
        cmd_pool_mgr->Initialize();

        /* Initialize texture sampler mgr */
        mem::Heap *gpu_heap_gpu_host_uncached = gpu_heap_mgr->GetGpuRootHeapGpuHostUncached(0);
        gfx::TextureSamplerManager *tex_samp_mgr = gfx::TextureSamplerManager::CreateInstance(awn_graphics_heap);
        const gfx::TextureSamplerManagerInfo texture_sampler_mgr_info = {
            .max_texture_handles = graphics_info->texture_handle_count,
            .max_sampler_handles = graphics_info->sampler_handle_count,
            .texture_memory_size = graphics_info->texture_memory_size,
        };
        tex_samp_mgr->Initialize(awn_graphics_heap, gpu_heap_gpu_host_uncached, std::addressof(texture_sampler_mgr_info));

        /* Register framework window class */
        const WNDCLASSEX wnd_class = {
            .cbSize        = sizeof(WNDCLASSEX),
            .style         = CS_OWNDC,
            .lpfnWndProc   = FrameworkWindowFunction,
            .hInstance     = ::GetModuleHandle(nullptr),
            .lpszClassName = "AwnFramework"
        };
        u32 result7 = ::RegisterClassExA(std::addressof(wnd_class));
        RESULT_RETURN_UNLESS(result7 != 0, ResultFailedToInitializeWindow);

        /* Adjust graphics heap size */
        awn_graphics_heap->AdjustHeap();

        RESULT_RETURN_SUCCESS;
    }

    void Framework::FinalizeGraphics() {

        /* Finalize graphics */
        mem::Heap *gfx_heap = mem::FindHeapByName("awn::frm::Framework Graphics");
        VP_ASSERT(gfx_heap != nullptr);
        {
            mem::ScopedCurrentThreadHeap heap_scope(gfx_heap);

            gfx::TextureSamplerManager::GetInstance()->Finalize();
            gfx::TextureSamplerManager::DeleteInstance();

            gfx::CommandPoolManager::GetInstance()->Finalize();
            gfx::CommandPoolManager::DeleteInstance();

            mem::GpuHeapManager::GetInstance()->Finalize();
            mem::GpuHeapManager::DeleteInstance();

            gfx::Context::GetInstance()->Finalize();
            gfx::Context::DeleteInstance();
        }

        /* Free library heap */
        mem::Heap *root_heap = mem::GetRootHeap(0);
        root_heap->Free(gfx_heap);

        /* Unregister framework window class */
        const bool result0 = ::UnregisterClassA("AwnFramework", ::GetModuleHandle(nullptr));
        VP_ASSERT(result0 == true);

        return;
    }

    void Framework::InitializeRun(FrameworkRunInfo *framework_run_info) {

        /* Set cmd arguments */
        m_argc = framework_run_info->argc;
        m_argv = framework_run_info->argv;

        /* Allocate Window array */
        m_window_thread_array.Initialize(framework_run_info->heap, framework_run_info->window_count);

        /* Allocate Windows */
        for (u32 i = 0; i < framework_run_info->window_count; ++i) {
            WindowThread *wnd_thread = new (framework_run_info->heap, 8) WindowThread(framework_run_info->heap, std::addressof(framework_run_info->window_info_array[i]), framework_run_info->drag_drop_count);
            VP_ASSERT(wnd_thread != nullptr);
            wnd_thread->StartThread();
            m_window_thread_array.PushPointer(wnd_thread);
        }

        /* Initialize windows */
        for (u32 i = 0; i < framework_run_info->window_count; ++i) {

            /* Wait for initialization */
            m_window_thread_array[i]->WaitForInitialize();

            /* Auto show the window if required */
            if (framework_run_info->is_auto_show_windows == true ) {
                m_window_thread_array[i]->ShowWindow();
            }
        }

        return;
    }
    void Framework::FinalizeRun() {

        /* Free windows */
        for (u32 i = 0; i < m_window_thread_array.GetUsedCount(); ++i) {
            if (m_window_thread_array[i] != nullptr) {
                m_window_thread_array[i]->ExitWindowThread();
            }
        }
        m_window_thread_array.Finalize();

        return;
    }

    void JobListFramework::PresentAsync([[maybe_unused]] size_t message) {

        /* Early finish on paused draw */
        if (m_is_pause_draw != 0) {
            m_present_event.Signal();
            return; 
        }

        /* Build window metadata */
        VkSwapchainKHR  vk_swapchain_array[cMaxWindowCount]         = {};
        u32             present_image_indice_array[cMaxWindowCount] = {};
        VkResult        vk_result_array[cMaxWindowCount]            = {};
        u32             draw_count                                  = 0;
        gfx::Sync      *acquire_sync_array[cMaxWindowCount]         = {};
        u32             acquire_count                               = 0;
        for (u32 i = 0; i < m_window_thread_array.GetUsedCount(); ++i) {

            /* Lock window for present */
            m_window_thread_array[i]->m_window_cs.Enter();

            /* Set present info */
            if (m_window_thread_array[i]->IsSkipDraw() == false) { 
                vk_swapchain_array[draw_count]         = m_window_thread_array[i]->GetVkSwapchain();
                present_image_indice_array[draw_count] = m_window_thread_array[i]->GetImageIndex();
                draw_count = draw_count + 1; 
            }

            /* Set acquire info */
            if (m_window_thread_array[i]->IsSkipAcquireSync() == false) {
                acquire_sync_array[acquire_count] = m_window_thread_array[i]->GetAcquireSync(m_current_command_list_index);
                acquire_count = acquire_count + 1; 
            }
        }

        /* Submit primary command buffer to graphics queue */
        gfx::Sync *sync_array[2] = {};
        sync_array[0] = std::addressof(m_present_sync_array[m_current_command_list_index]);
        sync_array[1] = std::addressof(m_graphics_queue_submit_sync_array[m_current_command_list_index]);
        gfx::Context::GetInstance()->SubmitCommandList(m_primary_graphics_command_list_array[m_current_command_list_index], acquire_sync_array, acquire_count, sync_array, 2);

        /* Present */
        VkSemaphore vk_semaphore = m_present_sync_array[m_current_command_list_index].GetVkSemaphore();
        const VkPresentInfoKHR present_info = {
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = std::addressof(vk_semaphore),
            .swapchainCount     = draw_count,
            .pSwapchains        = vk_swapchain_array,
            .pImageIndices      = present_image_indice_array,
            .pResults           = vk_result_array,
        };
        const u32 result = ::pfn_vkQueuePresentKHR(gfx::Context::GetInstance()->GetVkQueue(m_primary_graphics_command_list_array[m_current_command_list_index].queue_type), std::addressof(present_info));
        VP_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

        /* Wait for gpu to process */
        m_graphics_queue_submit_sync_array[m_current_command_list_index].Wait();
        m_graphics_queue_submit_sync_array[m_current_command_list_index].IncrementExpectedValue();

        /* Acquire next swapchain image for each window, recreate the swapchain if necessary */
        for (u32 i = 0; i < m_window_thread_array.GetUsedCount(); ++i) {

            /* Respect skip draw */
            if (m_window_thread_array[i]->IsSkipDraw() == true) { continue; }

            /* Try to acquire the next image as necessary */
            do {
                m_window_thread_array[i]->AcquireNextImage(m_current_command_list_index);
            } while (m_window_thread_array[i]->RecreateSwapchainIfNecessaryUnsafe() == true);

            /* Unlock window post acquire */
            m_window_thread_array[i]->m_window_cs.Leave();
        }

        /* Signal main loop */
        m_is_present = false;
        m_present_event.Signal();

        return;
    }

    void JobListFramework::MainLoop() {

        /* Start presentation thread */
        m_present_thread.StartThread();

        /* Initialize present event */
        m_present_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Manual);

        /* Initialize sync */
        for (u32 i = 0; i < WindowThread::cMaxRenderTargetCount; ++i) {
            m_graphics_queue_submit_sync_array[i].Initialize();
            m_present_sync_array[i].Initialize(true);
        }

        /* Acquire first images for rendering */
        for (u32 i = 0; i < m_window_thread_array.GetUsedCount(); ++i) {
            m_window_thread_array[i]->AcquireNextImage(m_current_command_list_index);
        }

        /* Mainloop */
        m_is_ready_to_exit = false;
        m_is_pause_calc    = false;
        while (m_is_ready_to_exit == false) {
            this->Draw();
            this->Calc();
            this->WaitForGpu();
        }

        /* Finalize presentation thread */
        m_present_thread.SendMessage(0);
        m_present_thread.WaitForThreadExit();

        /* Finalize present event */
        m_present_event.Finalize();

        /* Idle graphics queue for exit */
        ::pfn_vkQueueWaitIdle(gfx::Context::GetInstance()->GetVkQueueGraphics());

        /* Finalize sync */
        for (u32 i = 0; i < WindowThread::cMaxRenderTargetCount; ++i) {
            m_graphics_queue_submit_sync_array[i].Finalize();
            m_present_sync_array[i].Finalize();
        }

        /* Finalize any command lists */
        for (u32 i = 0; i < WindowThread::cMaxRenderTargetCount; ++i) {                
            if (m_primary_graphics_command_list_array[i].vk_command_buffer != VK_NULL_HANDLE) {
                gfx::CommandPoolManager::GetInstance()->FinalizeThreadLocalCommandList(m_primary_graphics_command_list_array[i]);
            }
            m_primary_graphics_command_list_array[i] = gfx::CommandList{};
        }

        return;
    }

    void JobListFramework::Calc() {

        /* Check pause state */
        if (m_is_pause_calc != 0) { return; }

        /* Run calc jobs */
        for (ListNodeJob &job : m_calc_job_list) {
            job.Invoke();
        }

        return;
    }

    void JobListFramework::Draw() {

        /* Check pause state */
        if (m_is_pause_draw != 0) { return; }

        /* Increment command list index */
        ++m_current_command_list_index;
        if (WindowThread::cMaxRenderTargetCount <= m_current_command_list_index) {
            m_current_command_list_index = 0;
        }

        /* Try delete the previous command list */
        if (m_primary_graphics_command_list_array[m_current_command_list_index].vk_command_buffer != VK_NULL_HANDLE) {
            gfx::CommandPoolManager::GetInstance()->FinalizeThreadLocalCommandList(m_primary_graphics_command_list_array[m_current_command_list_index]);
        }
        m_primary_graphics_command_list_array[m_current_command_list_index] = gfx::CommandList{};

        /* Setup primary command buffer */
        m_primary_graphics_command_buffer.Begin(gfx::QueueType::Graphics, true);

        /* Build current color target list */
        gfx::RenderTargetColor *color_target_array[cMaxWindowCount] = {};
        for (u32 i = 0; i < m_window_thread_array.GetUsedCount(); ++i) {
            color_target_array[i] = m_window_thread_array[i]->GetCurrentRenderTarget();
        }

        /* Transition window color targets to attachment */
        m_primary_graphics_command_buffer.TransitionRenderTargetsToAttachment(color_target_array, m_window_thread_array.GetUsedCount(), nullptr);

        /* Run draw jobs */
        for (ListNodeJob &job : m_draw_job_list) {
            job.Invoke();
        }

        /* Transition window color targets to present */
        m_primary_graphics_command_buffer.TransitionRenderTargetsToPresent(color_target_array, m_window_thread_array.GetUsedCount(), nullptr);

        /* End primary command buffer recording */
        m_primary_graphics_command_list_array[m_current_command_list_index] = m_primary_graphics_command_buffer.End();

        /* Clear present event */
        m_present_event.Clear();

        /* Signal present thread */
        m_is_present = true;
        m_present_thread.SendMessage(1);

        return;
    }

    void JobListFramework::WaitForGpu() {

        if (m_is_present == true) {

            /* Wait for window presentation to occur */
            m_present_event.Wait();
        }

        return;
    }

    AWN_SINGLETON_TRAITS_IMPL(JobListFramework);
}

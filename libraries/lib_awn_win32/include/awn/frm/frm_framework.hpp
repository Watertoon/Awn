#pragma once

namespace awn::frm {

    struct FrameworkLibraryInfo {
        u64    process_core_mask;
        size_t root_heap_initial_size;
        size_t out_of_memory_resize_alignment;

        constexpr void SetDefaults() {
            process_core_mask              = 1;
            root_heap_initial_size         = vp::util::c32MB;
            out_of_memory_resize_alignment = vp::util::c4KB;
        }
    };
    struct FrameworkRunInfo {
        mem::Heap   *heap;
        int          argc;
        char       **argv;
        u32          drag_drop_count;
        u32          window_count;
        WindowInfo *window_info_array;
    };

    long long int FrameworkWindowFunction(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam);

	class Framework {
        public:
            static constexpr size_t cMaxWindowCount = 8;
        public:
            using WindowArray = vp::util::PointerArray<WindowThread>;
        protected:
            int           m_argc;
            char        **m_argv;
            WindowArray   m_window_thread_array;
        public:
            VP_RTTI_BASE(Framework);
        protected:
            static bool OutOfMemoryCallback(mem::OutOfMemoryInfo *out_of_mem_info) {

                /* Integrity checks */
                VP_ASSERT(out_of_mem_info->allocation_size != mem::Heap::cWholeSize && out_of_mem_info->out_of_memory_heap->IsThreadSafe() != true);

                /* Calculate new target size */
                const size_t new_size = vp::util::AlignUp(out_of_mem_info->out_of_memory_heap->GetTotalSize() + out_of_mem_info->aligned_allocation_size, mem::GetOutOfMemoryResizeAlignment());

                /* Allocate more system memory if it's the root heap */
                if (out_of_mem_info->out_of_memory_heap == mem::GetRootHeap()) {
                    const void *new_address = ::VirtualAlloc(reinterpret_cast<void*>(out_of_mem_info->out_of_memory_heap), new_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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
        public:

            static Result InitializeLibraries(FrameworkLibraryInfo *framework_lib_info) {

                /* Integrity check */
                VP_ASSERT(framework_lib_info != nullptr);

                /* Initialize System Manager */
                sys::InitializeSystemManager();

                /* Initialize ukern */
                ukern::InitializeUKern(framework_lib_info->process_core_mask);

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
                mem::Heap *root_heap = mem::GetRootHeap();
                mem::Heap *awn_library_heap = mem::ExpHeap::TryCreate("awn::frm::Framework Libs", root_heap, mem::Heap::cWholeSize, 8, false);

                /* Initialize thread mgr */
                sys::ThreadManager *thread_mgr = sys::ThreadManager::CreateInstance(awn_library_heap);
                thread_mgr->Initialize(awn_library_heap);

                /* Initialize file device mgr */
                res::FileDeviceManager *file_device_mgr = res::FileDeviceManager::CreateInstance(awn_library_heap);
                file_device_mgr->Initialize(awn_library_heap);

                /* Initialize resource factory mgr */
                res::ResourceFactoryManager::CreateInstance(awn_library_heap);

                /* Initialize graphics context */
                gfx::Context *context = gfx::Context::CreateInstance(awn_library_heap);

                gfx::ContextInfo context_info = {};
                context_info.SetDefaults();

                context->Initialize(std::addressof(context_info));

                /* Register framework window class */
                const WNDCLASSEX wnd_class = {
                    .cbSize        = sizeof(WNDCLASSEX),
                    .style         = CS_OWNDC,
                    .lpfnWndProc   = FrameworkWindowFunction,
                    .hInstance     = ::GetModuleHandle(nullptr),
                    .lpszClassName = "AwnFramework"
                };
                u32 result7 = ::RegisterClassExA(std::addressof(wnd_class));
                RESULT_RETURN_UNLESS(result7 == 0, ResultFailedToInitializeWindow);

                /* Adjust library heap size */
                awn_library_heap->AdjustHeap();

                RESULT_RETURN_SUCCESS;
            }
        protected:
            void InitializeRun(FrameworkRunInfo *framework_run_info) {

                /* Set cmd arguments */
                m_argc = framework_run_info->argc;
                m_argv = framework_run_info->argv;

                /* Allocate Window array */
                m_window_thread_array.Initialize(framework_run_info->heap, framework_run_info->window_count);

                /* Allocate Windows */
                for (u32 i = 0; i < framework_run_info->window_count; ++i) {
                    WindowThread *wnd_thread = new (framework_run_info->heap, 8) WindowThread(framework_run_info->heap, std::addressof(framework_run_info->window_info_array[i]), framework_run_info->drag_drop_count);
                    VP_ASSERT(wnd_thread != nullptr);
                    m_window_thread_array.PushPointer(wnd_thread);
                }

                return;
            }
        public:
            constexpr ALWAYS_INLINE Framework() {/*...*/}

            virtual void MainLoop();

            virtual void Run(FrameworkRunInfo *framework_run_info) {

                /* Initialize state for run */
                this->InitializeRun(framework_run_info);
                
                /* Use the application's main loop */
                this->MainLoop();

                return;
            }

            constexpr WindowThread *GetWindowThread(u32 window_index) { return m_window_thread_array[window_index]; }

            constexpr void ShowWindow(u32 window_index) const { m_window_thread_array[window_index]->ShowWindow(); }
            constexpr void HideWindow(u32 window_index) const { m_window_thread_array[window_index]->HideWindow(); }
	};

    class ListNodeJob : public vp::util::Job {
        public:
            vp::util::IntrusiveListNode m_list_node;
        public:
            constexpr ALWAYS_INLINE ListNodeJob(const char *name) : Job(name), m_list_node() {/*...*/}
            constexpr ALWAYS_INLINE ~ListNodeJob() { m_list_node.Unlink(); }
    };

	class JobListFramework : public Framework {
        public:
            using JobList         = vp::util::IntrusiveListTraits<ListNodeJob, &ListNodeJob::m_list_node>::List;
            using PresentThread   = sys::DelegateServiceThread<JobListFramework>;
            using PresentDelegate = PresentThread::ThreadDelegate;
        protected:
            JobList                       m_calc_job_list;
            JobList                       m_draw_job_list;
            gfx::Sync                     m_graphics_queue_sync;
            gfx::ThreadLocalCommandBuffer m_primary_graphics_command_buffer;
            gfx::CommandList              m_last_primary_graphics_command_list;
            PresentDelegate               m_present_delegate;
            PresentThread                 m_present_thread;
            sys::ServiceEvent             m_present_event;
            u8                            m_is_pause_calc;
            u8                            m_is_pause_draw;
            u8                            m_is_ready_to_exit;
        public:
            VP_RTTI_DERIVED(JobListFramework, Framework);
        public:
            AWN_SINGLETON_TRAITS(JobListFramework);
        protected:
            void PresentAsync(size_t message) {

                if (m_is_pause_draw != 0) {
                    m_present_event.Signal();
                    return; 
                }

                /* Submit primary command buffer to graphics queue */
                gfx::Sync *sync = std::addressof(m_graphics_queue_sync);
                gfx::Context::GetInstance()->SubmitCommandList(m_last_primary_graphics_command_list, nullptr, 0, std::addressof(sync), 1);

                /* Wait for processing */
                m_graphics_queue_sync.Wait();

                /* Build swapchain metadata */
                VkSwapchainKHR vk_swapchain_array[cMaxWindowCount]         = {};
                u32            present_image_indice_array[cMaxWindowCount] = {};
                VkResult       vk_result_array[cMaxWindowCount]            = {};
                u32            draw_count                                  = 0;
                for (u32 i = 0; i < m_window_thread_array.GetUsedCount(); ++i) {

                    /* Lock window for present */
                    m_window_thread_array[i]->m_window_cs.Enter();

                    /* Respect skip draw */
                    if (m_window_thread_array[i]->IsSkipDraw() == true) { continue; }

                    /* Set present info */
                    vk_swapchain_array[draw_count]         = m_window_thread_array[i]->GetVkSwapchain();
                    present_image_indice_array[draw_count] = m_window_thread_array[i]->GetImageIndex();
                    draw_count = draw_count + 1;
                }

                /* Present */
                VkSemaphore vk_semaphore = m_graphics_queue_sync.GetVkSemaphore();
                const VkPresentInfoKHR present_info = {
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores    = std::addressof(vk_semaphore),
                    .swapchainCount     = draw_count,
                    .pSwapchains        = vk_swapchain_array,
                    .pImageIndices      = present_image_indice_array,
                    .pResults           = vk_result_array,
                };
                const u32 result = ::vkQueuePresentKHR(gfx::Context::GetInstance()->GetVkQueue(m_last_primary_graphics_command_list.queue_type), std::addressof(present_info));

                /* Acquire next swapchain image for each window, recreate the swapchain if necessary */
                for (u32 i = 0; i < m_window_thread_array.GetUsedCount(); ++i) {

                    /* Respect skip draw */
                    if (m_window_thread_array[i]->IsSkipDraw() == true) { continue; }

                    /* Try to acquire the next image as necessary */
                    do {
                        m_window_thread_array[i]->AcquireNextImage();
                    } while (m_window_thread_array[i]->RecreateSwapchainIfNecessary() == true);

                    /* Unlock window post acquire */
                    m_window_thread_array[i]->m_window_cs.Leave();
                }

                /* Signal main loop */
                m_present_event.Signal();

                return;
            }
        public:
            constexpr ALWAYS_INLINE JobListFramework() : Framework(), m_calc_job_list(), m_draw_job_list(), m_graphics_queue_sync(), m_primary_graphics_command_buffer(), m_last_primary_graphics_command_list(), m_present_delegate(this, PresentAsync), m_present_thread(std::addressof(m_present_delegate), "AwnFramework Present Thread", nullptr, sys::ThreadRunMode::Looping, 0, 8, 0x1000, sys::cHighPriority), m_present_event(), m_is_pause_calc(true), m_is_pause_draw(true), m_is_ready_to_exit(false) {/*...*/}

            virtual void MainLoop() override {

                /* Mainloop */
                m_is_ready_to_exit = false;
                m_is_pause_calc    = false;
                m_is_pause_draw    = false;
                while (m_is_ready_to_exit == false) {
                    this->Draw();
                    this->Calc();
                    this->WaitForGpu();
                }

                return;
            }

            void Calc() {

                /* Check pause state */
                if (m_is_pause_calc != 0) { return; }

                /* Run calc jobs */
                for (ListNodeJob &job : m_calc_job_list) {
                    job.Invoke();
                }

                return;
            }

            void Draw() {

                /* Check pause state */
                if (m_is_pause_draw != 0) { return; }

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
                m_last_primary_graphics_command_list = m_primary_graphics_command_buffer.End();

                /* Signal present thread */
                m_present_thread.SendMessage(1);

                return;
            }

            void WaitForGpu() {

                /* Wait for window presentation to occur */
                m_present_event.Wait();

                return;
            }

            void AddCalcJob(ListNodeJob *calc_job) { VP_ASSERT(calc_job != nullptr); m_calc_job_list.PushBack(*calc_job); }
            void AddDrawJob(ListNodeJob *draw_job) { VP_ASSERT(draw_job != nullptr); m_draw_job_list.PushBack(*draw_job); }
            
            constexpr void SetCalcPauseState(bool is_pause) { m_is_pause_calc = is_pause; }
            constexpr void SetDrawPauseState(bool is_pause) { m_is_pause_draw = is_pause; }
            constexpr void SetIsReadyToExit()               { m_is_ready_to_exit = true; }
	};
}

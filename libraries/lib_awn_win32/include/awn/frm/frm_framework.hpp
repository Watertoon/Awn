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

    struct FrameworkLibraryInfo {
        u32    process_core_count;
        size_t root_heap_initial_size;
        size_t out_of_memory_resize_alignment;

        constexpr void SetDefaults() {
            process_core_count             = 1;
            root_heap_initial_size         = vp::util::c32MB;
            out_of_memory_resize_alignment = vp::util::c4KB;
        }
    };

    struct FrameworkGraphicsInfo {
        size_t host_uncached_memory_size;
        size_t host_cached_memory_size;
        size_t gpu_host_uncached_memory_size;
        size_t texture_memory_size;
        u32    texture_handle_count;
        u32    sampler_handle_count;

        constexpr void SetDefaults() {
            host_uncached_memory_size     = vp::util::c32MB;
            host_cached_memory_size       = vp::util::c32MB;
            gpu_host_uncached_memory_size = vp::util::c32MB;
            texture_memory_size           = vp::util::c32MB;
            texture_handle_count          = 0x100;
            sampler_handle_count          = 0x100;
        }
    };

    struct FrameworkRunInfo {
        mem::Heap   *heap;
        int          argc;
        char       **argv;
        u32          drag_drop_count;
        bool         is_auto_show_windows;
        u32          window_count;
        WindowInfo  *window_info_array;
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
            static bool OutOfMemoryCallback(mem::OutOfMemoryInfo *out_of_mem_info);
        public:
            static Result InitializeLibraries(FrameworkLibraryInfo *framework_lib_info);
            static void FinalizeLibraries();

            static Result InitializeGraphics(FrameworkGraphicsInfo *graphics_info);
            static void FinalizeGraphics();
        protected:
            void InitializeRun(FrameworkRunInfo *framework_run_info);
            void FinalizeRun();
        public:
            constexpr ALWAYS_INLINE Framework() : m_argc(0), m_argv(nullptr), m_window_thread_array() {/*...*/}
            virtual ~Framework() {/*...*/}

            virtual void MainLoop() {/*...*/}

            virtual void Run(FrameworkRunInfo *framework_run_info) {

                /* Initialize state for run */
                this->InitializeRun(framework_run_info);
                
                /* Use the application's main loop */
                this->MainLoop();

                /* Finalize run state */
                this->FinalizeRun();

                return;
            }

            constexpr WindowThread *GetWindowThread(u32 window_index) { return m_window_thread_array[window_index]; }

            void ShowWindow(u32 window_index) const { m_window_thread_array[window_index]->ShowWindow(); }
            void HideWindow(u32 window_index) const { m_window_thread_array[window_index]->HideWindow(); }
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
            using PresentThread   = sys::DelegateServiceThread;
            using PresentDelegate = sys::ThreadDelegate<JobListFramework>;
        protected:
            JobList            m_calc_job_list;
            JobList            m_draw_job_list;
            gfx::Sync          m_present_sync_array[WindowThread::cMaxRenderTargetCount];
            gfx::Sync          m_graphics_queue_submit_sync_array[WindowThread::cMaxRenderTargetCount];
            gfx::CommandBuffer m_primary_graphics_command_buffer;
            u32                m_current_command_list_index;
            gfx::CommandList   m_primary_graphics_command_list_array[WindowThread::cMaxRenderTargetCount];
            PresentDelegate    m_present_delegate;
            PresentThread      m_present_thread;
            sys::ServiceEvent  m_present_event;
            u8                 m_is_pause_calc;
            u8                 m_is_pause_draw;
            u8                 m_is_ready_to_exit;
            u8                 m_is_present;
        public:
            VP_RTTI_DERIVED(JobListFramework, Framework);
        public:
            AWN_SINGLETON_TRAITS(JobListFramework);
        protected:
            void PresentAsync([[maybe_unused]] size_t message);

            virtual void MainLoop() override;

            void Draw();
            void Calc();
            void WaitForGpu();
        public:
            ALWAYS_INLINE JobListFramework() : Framework(), m_calc_job_list(), m_draw_job_list(), m_present_sync_array{}, m_graphics_queue_submit_sync_array{}, m_primary_graphics_command_buffer(), m_current_command_list_index(0), m_primary_graphics_command_list_array{}, m_present_delegate(this, PresentAsync), m_present_thread(std::addressof(m_present_delegate), "AwnFramework Present Thread", nullptr, sys::ThreadRunMode::WaitForMessage, 0, 8, 0x1000, sys::cPriorityHigh), m_present_event(), m_is_pause_calc(false), m_is_pause_draw(false), m_is_ready_to_exit(false), m_is_present(false) {/*...*/}
            virtual ~JobListFramework() override {/*...*/}

            void AddCalcJob(ListNodeJob *calc_job) { VP_ASSERT(calc_job != nullptr); m_calc_job_list.PushBack(*calc_job); }
            void AddDrawJob(ListNodeJob *draw_job) { VP_ASSERT(draw_job != nullptr); m_draw_job_list.PushBack(*draw_job); }
            
            constexpr void SetCalcPauseState(bool is_pause) { m_is_pause_calc = is_pause; }
            constexpr void SetDrawPauseState(bool is_pause) { m_is_pause_draw = is_pause; }
            constexpr void SetIsReadyToExit()               { m_is_ready_to_exit = true; }
	};
}

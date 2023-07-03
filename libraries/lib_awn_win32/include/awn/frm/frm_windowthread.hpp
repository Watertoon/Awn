#pragma once

namespace awn::frm {

    enum class DragDropStatus : u32 {
        Clear            = 0,
        Pending          = 1,
        TooLongPathError = 2,
        PathCopyError    = 3,
    };

    enum class PresentMode : u32 {
        Immediate    = 0,
        Mailbox      = 1,
        VSync        = 2,
        VSyncRelaxed = 3,
    };
    
    constexpr ALWAYS_INLINE VkPresentModeKHR ConvertAwnPresentModeToVkPresentMode(PresentMode present_mode) {
        return static_cast<VkPresentModeKHR>(present_mode);
    }

    struct WindowInfo {
        const char *window_name;
        const char *class_name;
        s32         width;
        s32         height;
        u32         x;
        u32         y;
        u32         view_count;
        PresentMode present_mode;
        bool        enable_window_resizing;
        bool        enable_clipping;
        bool        enable_triple_buffer;
        bool        is_lock_aspect_ratio;

        constexpr void SetDefaults() {
            window_name            = "AwnFramework";
            class_name             = "AwnFramework";
            width                  = 640;
            height                 = 480;
            x                      = 0;
            y                      = 0;
            view_count             = 1;
            present_mode           = PresentMode::VSync;
            enable_window_resizing = false;
            enable_clipping        = false;
            enable_triple_buffer   = false;
            is_lock_aspect_ratio   = false;
        }
    };

    class Framework;
    class JobListFramework;

    class WindowThread : public sys::ServiceThread {
        public:
            friend class Framework;
            friend class JobListFramework;
            friend long long int FrameworkWindowFunction(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam);
        public:
            static constexpr size_t cMaxRenderTargetCount    = 3;
            static constexpr size_t cInvalidAcquireSyncIndex = 0xffff'ffff;
        public:
            using DragDropArray = vp::util::HeapArray<vp::util::FixedString<vp::util::cMaxPath>>;
        protected:
            HWND                        m_hwnd;
            VkSurfaceKHR                m_vk_surface;
            VkSwapchainKHR              m_vk_swapchain;
            VkImage                     m_vk_image_array[cMaxRenderTargetCount];
            gfx::RenderTargetColor      m_render_target_color_array[cMaxRenderTargetCount];
            u32                         m_image_index;
            u32                         m_current_acquire_sync_index;
            u32                         m_wait_acquire_sync_index;
            gfx::Sync                   m_acquire_sync[cMaxRenderTargetCount];
            sys::ServiceCriticalSection m_window_cs;
            sys::ServiceEvent           m_window_event;
            WindowInfo                  m_window_info;
            DragDropArray               m_drag_drop_array;
            u32                         m_drag_drop_count;
            DragDropStatus              m_drag_drop_status;
            bool                        m_require_swapchain_refresh;
            bool                        m_skip_draw;
        public:
            struct WindowMessage {
                MSG    win32_msg;
                size_t awn_message;
            };
        public:
            WindowThread(mem::Heap *heap, WindowInfo *window_info, u32 max_file_drag_drop) : ServiceThread("WindowThread", heap, sys::ThreadRunMode::Looping, 0, 8, 0x1000, sys::cNormalPriority), m_hwnd(0), m_vk_surface(VK_NULL_HANDLE), m_vk_swapchain(VK_NULL_HANDLE), m_vk_image_array{}, m_render_target_color_array{}, m_image_index(-1), m_acquire_sync(), m_window_cs(), m_window_event(), m_window_info(), m_drag_drop_array(), m_drag_drop_count(), m_drag_drop_status(), m_require_swapchain_refresh(false), m_skip_draw(false) {

                /* Allocate drag drop array */
                m_drag_drop_array.Initialize(heap, max_file_drag_drop);

                /* Store window info */
                m_window_info = *window_info;
            }

            virtual void Run() override {

                {
                    /* Calculate window style */
                    const u32 drag_drop_style = (0 < m_drag_drop_array.GetCount()) ? WS_EX_ACCEPTFILES : 0;
                    const u32 window_style = WS_OVERLAPPEDWINDOW | drag_drop_style;

                    /* Calculate adjusted window size for desired client area */
                    RECT wnd_rect = {};
                    const bool rect_result = ::AdjustWindowRect(std::addressof(wnd_rect), window_style, false);
                    VP_ASSERT(rect_result == true);

                    /* Create window */
                    m_hwnd = ::CreateWindowA(m_window_info.class_name, m_window_info.window_name, window_style, m_window_info.x, m_window_info.y, m_window_info.width + (wnd_rect.right - wnd_rect.left), m_window_info.height + (wnd_rect.bottom - wnd_rect.top), nullptr, nullptr, nullptr, this);
                    VP_ASSERT(m_hwnd != nullptr);
                    ::SetWindowLongPtrA(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

                    /* Initialize surface */
                    const VkWin32SurfaceCreateInfoKHR win32_surface_info = {
                        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                        .hinstance = ::GetModuleHandle(nullptr),
                        .hwnd      = m_hwnd
                    };
                    const u32 result0 = ::pfn_vkCreateWin32SurfaceKHR(gfx::Context::GetInstance()->GetVkInstance(), std::addressof(win32_surface_info), gfx::Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_surface));
                    VP_ASSERT(result0 == VK_SUCCESS);

                    /* Initialize swapchain */
                    const u32 image_count = (m_window_info.enable_triple_buffer) ? 3 : 2;
                    const VkSwapchainCreateInfoKHR swapchain_info = {
                        .sType                = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                        .surface              = m_vk_surface,
                        .minImageCount        = image_count,
                        .imageFormat          = VK_FORMAT_R8G8B8A8_UNORM,
                        .imageColorSpace      = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                        .imageExtent          = {
                            .width  = static_cast<u32>(m_window_info.width),
                            .height = static_cast<u32>(m_window_info.height)
                        },
                        .imageArrayLayers      = m_window_info.view_count,
                        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                        .imageSharingMode      = VK_SHARING_MODE_CONCURRENT,
                        .queueFamilyIndexCount = gfx::Context::GetInstance()->GetQueueFamilyCount(),
                        .pQueueFamilyIndices   = gfx::Context::GetInstance()->GetQueueFamilyIndiceArray(),
                        .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                        .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                        .presentMode           = ConvertAwnPresentModeToVkPresentMode(m_window_info.present_mode),
                        .clipped               = m_window_info.enable_clipping,
                    };
                    const u32 result1 = ::pfn_vkCreateSwapchainKHR(gfx::Context::GetInstance()->GetVkDevice(), std::addressof(swapchain_info), gfx::Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_swapchain));
                    VP_ASSERT(result1 == VK_SUCCESS);

                    /* Get Swapchain images */
                    u32 swapchain_image_count = 0;
                    const u32 result2 = ::pfn_vkGetSwapchainImagesKHR(gfx::Context::GetInstance()->GetVkDevice(), m_vk_swapchain, std::addressof(swapchain_image_count), nullptr);
                    VP_ASSERT(result2 == VK_SUCCESS);
                    VP_ASSERT(swapchain_image_count == image_count);
                    const u32 result3 = ::pfn_vkGetSwapchainImagesKHR(gfx::Context::GetInstance()->GetVkDevice(), m_vk_swapchain, std::addressof(swapchain_image_count), m_vk_image_array);
                    VP_ASSERT(result3 == VK_SUCCESS);

                    /* Initialize render targets */
                    gfx::RenderTargetImportInfo import_info = {};
                    import_info.SetDefaults();
                    import_info.image_format  = static_cast<u32>(gfx::ImageFormat::R8G8B8A8_Unorm);
                    import_info.array_layers  = m_window_info.view_count;
                    import_info.render_width  = m_window_info.width;
                    import_info.render_height = m_window_info.height;

                    for (u32 i = 0; i < image_count; ++i) {
                        import_info.vk_image = m_vk_image_array[i];
                        m_render_target_color_array[i].Initialize(std::addressof(import_info));
                    }

                    /* Create fence */
                    /*const VkFenceCreateInfo fence_info = {
                        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    };
                    const u32 result4 = ::pfn_vkCreateFence(gfx::Context::GetInstance()->GetVkDevice(), std::addressof(fence_info), gfx::Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_acquire_vk_fence));
                    VP_ASSERT(result4 == VK_SUCCESS);/*

                    /* Initialize acquire sync */
                    m_acquire_sync.Initialize(true);

                    /* Acquire first image */
                    this->AcquireNextImage();

                    /* Signal setup has complete */
                    m_window_event.Signal();
                }

                /* Special thread mainloop for Window thread */
                WindowMessage window_message = {};
                while (::GetMessage(std::addressof(window_message.win32_msg), m_hwnd, 0, 0) > 0  && (m_message_queue.TryReceiveMessage(std::addressof(window_message.awn_message)) == false || window_message.awn_message != m_exit_message)) {
                    this->ThreadCalc(reinterpret_cast<size_t>(std::addressof(window_message)));
                }

                {
                    /* Set skip draw */
                    m_skip_draw = true;

                    /* Wait for idle */
                    ::pfn_vkQueueWaitIdle(gfx::Context::GetInstance()->GetVkQueueGraphics());

                    /* Finalize render targets */
                    const u32 image_count = (m_window_info.enable_triple_buffer) ? 3 : 2;
                    for (u32 i = 0; i < image_count; ++i) {
                        m_render_target_color_array[i].Finalize();
                    }

                    /* Destroy fence */
                    //::pfn_vkDestroyFence(gfx::Context::GetInstance()->GetVkDevice(), m_acquire_vk_fence, gfx::Context::GetInstance()->GetVkAllocationCallbacks());

                    /* Finalize acquire sync */
                    m_acquire_sync.Finalize();

                    /* Destroy swapchain */
                    ::pfn_vkDestroySwapchainKHR(gfx::Context::GetInstance()->GetVkDevice(), m_vk_swapchain, gfx::Context::GetInstance()->GetVkAllocationCallbacks());

                    /* Destroy surface */
                    ::pfn_vkDestroySurfaceKHR(gfx::Context::GetInstance()->GetVkInstance(), m_vk_surface, gfx::Context::GetInstance()->GetVkAllocationCallbacks());
                }

                return;
            }

            virtual void ThreadCalc(size_t arg) override {
                WindowMessage *window_message = reinterpret_cast<WindowMessage*>(arg);
                ::TranslateMessage(std::addressof(window_message->win32_msg));
                ::DispatchMessage(std::addressof(window_message->win32_msg));
            }

            bool RecreateSwapchainIfNecessaryUnsafe() {

                /* Lock Window */
                //std::scoped_lock lock(m_window_cs);

                /* Bail if the swapchain is fine */
                if (m_require_swapchain_refresh == false || m_skip_draw == true) { return false; }

                /* Finalize old render targets */
                const u32 image_count = (m_window_info.enable_triple_buffer) ? 3 : 2;
                for (u32 i = 0; i < image_count; ++i) {
                    m_render_target_color_array[i].Finalize();
                }

                /* Recreate swapchain */
                VkSwapchainKHR old_swapchain = m_vk_swapchain;
                const VkSwapchainCreateInfoKHR swapchain_info = {
                    .sType                = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .surface              = m_vk_surface,
                    .minImageCount        = image_count,
                    .imageFormat          = VK_FORMAT_R8G8B8A8_UNORM,
                    .imageColorSpace      = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                    .imageExtent          = {
                        .width  = static_cast<u32>(m_window_info.width),
                        .height = static_cast<u32>(m_window_info.height)
                    },
                    .imageArrayLayers      = m_window_info.view_count,
                    .imageUsage            = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                    .imageSharingMode      = VK_SHARING_MODE_CONCURRENT,
                    .queueFamilyIndexCount = gfx::Context::GetInstance()->GetQueueFamilyCount(),
                    .pQueueFamilyIndices   = gfx::Context::GetInstance()->GetQueueFamilyIndiceArray(),
                    .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                    .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                    .presentMode           = ConvertAwnPresentModeToVkPresentMode(m_window_info.present_mode),
                    .clipped               = m_window_info.enable_clipping,
                    .oldSwapchain          = old_swapchain
                };
                const u32 result0 = ::pfn_vkCreateSwapchainKHR(gfx::Context::GetInstance()->GetVkDevice(), std::addressof(swapchain_info), gfx::Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_swapchain));
                VP_ASSERT(result0 == VK_SUCCESS);

                /* Destroy old swapchain */
                ::pfn_vkDestroySwapchainKHR(gfx::Context::GetInstance()->GetVkDevice(), old_swapchain, gfx::Context::GetInstance()->GetVkAllocationCallbacks());

                /* Get Swapchain images */
                u32 swapchain_image_count = 0;
                const u32 result2 = ::pfn_vkGetSwapchainImagesKHR(gfx::Context::GetInstance()->GetVkDevice(), m_vk_swapchain, std::addressof(swapchain_image_count), nullptr);
                VP_ASSERT(result2 == VK_SUCCESS);
                VP_ASSERT(swapchain_image_count == image_count);
                const u32 result3 = ::pfn_vkGetSwapchainImagesKHR(gfx::Context::GetInstance()->GetVkDevice(), m_vk_swapchain, std::addressof(swapchain_image_count), m_vk_image_array);
                VP_ASSERT(result3 == VK_SUCCESS);

                /* Initialize render targets */
                gfx::RenderTargetImportInfo import_info = {};
                import_info.SetDefaults();
                import_info.image_format  = static_cast<u32>(gfx::ImageFormat::R8G8B8A8_Unorm);
                import_info.array_layers  = m_window_info.view_count;
                import_info.render_width  = m_window_info.width;
                import_info.render_height = m_window_info.height;

                for (u32 i = 0; i < image_count; ++i) {
                    import_info.vk_image = m_vk_image_array[i];
                    m_render_target_color_array[i].Initialize(std::addressof(import_info));
                }

                return true;
            }

            ALWAYS_INLINE void ShowWindow() const { ::ShowWindow(m_hwnd, SW_SHOW); }
            ALWAYS_INLINE void HideWindow() const { ::ShowWindow(m_hwnd, SW_HIDE); }

                      ALWAYS_INLINE void           SetDragDropEnable(bool enable_drag_drop) const { ::DragAcceptFiles(m_hwnd, enable_drag_drop); }
                                    void           ClearDragDropStatus()                          { m_drag_drop_status = DragDropStatus::Clear; }
            constexpr ALWAYS_INLINE DragDropStatus GetDragDropStatus()                      const { return m_drag_drop_status; }

            constexpr ALWAYS_INLINE void SetSkipDraw(bool is_skip_draw)       { m_skip_draw = is_skip_draw; }
            constexpr ALWAYS_INLINE bool IsSkipDraw()                   const { return m_skip_draw; }

            constexpr ALWAYS_INLINE void SetSkipAcquireSync(bool is_skip_acquire_sync)       { m_skip_acquire_sync = is_skip_acquire_sync; }
            constexpr ALWAYS_INLINE bool IsSkipAcquireSync()                           const { return m_skip_acquire_sync; }

            void SignalWindowChange() { m_require_swapchain_refresh = true; }

            void AcquireNextImage() {

                /* Acquire full */
                const u32 result0 = ::pfn_vkAcquireNextImageKHR(gfx::Context::GetInstance()->GetVkDevice(), m_vk_swapchain, 0xffff'ffff'ffff'ffff, m_acquire_sync.GetVkSemaphore(), VK_NULL_HANDLE, std::addressof(m_image_index));
                /*const u32 result1 = ::pfn_vkWaitForFences(gfx::Context::GetInstance()->GetVkDevice(), 1, std::addressof(m_acquire_vk_fence), true, 0xffff'ffff'ffff'ffff);
                VP_ASSERT(result1 == VK_SUCCESS);
                const u32 result2 = ::pfn_vkResetFences(gfx::Context::GetInstance()->GetVkDevice(), 1, std::addressof(m_acquire_vk_fence));
                VP_ASSERT(result2 == VK_SUCCESS);*/

                /* Update results */
                if (result0 == VK_SUBOPTIMAL_KHR) { m_require_swapchain_refresh = true; }
                else { VP_ASSERT(result0 == VK_SUCCESS); }
            }

            void WaitForInitialize() { 
                m_window_event.Wait(); 
            }

            void ExitWindowThread() { 
                this->SendMessage(0);
                this->WaitForThreadExit();
            }

            constexpr ALWAYS_INLINE gfx::Sync              *GetAcquireSync()         const { return std::addressof(m_acquire_sync); }
            constexpr ALWAYS_INLINE VkSurfaceKHR            GetVkSurface()           const { return m_vk_surface; }
            constexpr ALWAYS_INLINE VkSwapchainKHR          GetVkSwapchain()         const { return m_vk_swapchain; }
            constexpr ALWAYS_INLINE VkImage                 GetCurrentVkImage()      const { return m_vk_image_array[m_image_index]; }
            constexpr ALWAYS_INLINE u32                     GetImageIndex()          const { return m_image_index; }
            constexpr ALWAYS_INLINE gfx::RenderTargetColor *GetCurrentRenderTarget()       { return std::addressof(m_render_target_color_array[m_image_index]); }
    };
}

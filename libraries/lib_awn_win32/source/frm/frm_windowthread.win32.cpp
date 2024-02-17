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

    long long int FrameworkWindowFunction(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam) {

        /* Get thread */
        WindowThread *window_thread = reinterpret_cast<WindowThread*>(::GetWindowLongPtrA(hwnd, GWLP_USERDATA));

        switch (msg) {
            case WM_CREATE:
                break;
            case WM_DESTROY:
                ::PostQuitMessage(0);
                return 0;
            case WM_PAINT:
                return 0;
            case WM_SIZE:
            {
                /* Parse new window size */
                const s32 width  = LOWORD(lparam);
                const s32 height = HIWORD(lparam);

                /* Lock window */
                std::scoped_lock lock(window_thread->m_window_cs);
                
                /* Get previous size */
                const s32 prev_width  = window_thread->m_window_info.width;
                const s32 prev_height = window_thread->m_window_info.height;

                /* Check window size changed */
                if (window_thread->m_window_info.width == width && window_thread->m_window_info.height == height) { return ::DefWindowProc(hwnd, msg, wparam, lparam); }

                /* Set resize state */
                window_thread->m_window_info.width         = width;
                window_thread->m_window_info.height        = height;
                window_thread->m_require_swapchain_refresh = true;

                /* Handle minimization */
                if (width == 0 || height == 0) {
                    
                    /* Skip draw if minimized/surfaceless window */
                    window_thread->m_skip_draw = true;
                } else if (prev_width == 0 || prev_height == 0) {

                    /* Create a fresh swapchain on window unminimize */
                    window_thread->m_skip_draw = false;
                    window_thread->RecreateSwapchainIfNecessaryUnsafe();
                    //window_thread->AcquireNextImage();
                }
            }
                break;
            case WM_DROPFILES:
            {
                /* Drop the input if we are still pending from the last drag drop */
                if (window_thread->m_drag_drop_status == DragDropStatus::Pending) { return ::DefWindowProc(hwnd, msg, wparam, lparam); }

                /* Iterate drag drop files */
                HDROP hdrop = reinterpret_cast<HDROP>(wparam);
                const u32 file_count = ::DragQueryFileA(hdrop, 0xffff'ffff, nullptr, 0);
                for (u32 i = 0; i < file_count; ++i) {

                    /* Verify path length */
                    const u32 message_size = ::DragQueryFileA(hdrop, i, nullptr, 0);
                    if (vp::util::cMaxPath <= message_size) { window_thread->m_drag_drop_status = DragDropStatus::TooLongPathError; return 0; }

                    /* Copy string */
                    const u32 result = ::DragQueryFileA(hdrop, i, window_thread->m_drag_drop_array[i].GetString(), vp::util::cMaxPath);
                    if (result == 0) { window_thread->m_drag_drop_status = DragDropStatus::PathCopyError; return ::DefWindowProc(hwnd, msg, wparam, lparam); }
                }

                /* Set drag drop state */
                window_thread->m_drag_drop_status = DragDropStatus::Pending;
                window_thread->m_drag_drop_count  = file_count;
            }
                break;
            default:
                break;
        };
        return ::DefWindowProc(hwnd, msg, wparam, lparam);
    }

    void WindowThread::Run() {

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
            import_info.image_format      = static_cast<u32>(gfx::ImageFormat::R8G8B8A8_Unorm);
            import_info.array_layer_count = m_window_info.view_count;
            import_info.render_width      = m_window_info.width;
            import_info.render_height     = m_window_info.height;

            for (u32 i = 0; i < image_count; ++i) {
                import_info.vk_image = m_vk_image_array[i];
                m_render_target_color_array[i].Initialize(std::addressof(import_info));
            }

            /* Initialize acquire syncs */
            for (u32 i = 0; i < cMaxRenderTargetCount; ++i) {
                m_acquire_sync_array[i].Initialize(true);
            }
            
            /* Set initial acquire skip */
            m_skip_acquire_sync = false;

            /* Signal setup has complete */
            m_window_event.Signal();
        }

        /* Special thread mainloop for Window thread */
        WindowMessage window_message = {};
        while (::GetMessage(std::addressof(window_message.win32_msg), m_hwnd, 0, 0) > 0  && (m_message_queue.TryReceiveMessage(std::addressof(window_message.awn_message)) == false || window_message.awn_message != m_exit_message)) {
            this->ThreadMain(reinterpret_cast<size_t>(std::addressof(window_message)));
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

            /* Finalize acquire syncs */
            for (u32 i = 0; i < cMaxRenderTargetCount; ++i) {
                m_acquire_sync_array[i].Finalize();
            }

            /* Destroy swapchain */
            ::pfn_vkDestroySwapchainKHR(gfx::Context::GetInstance()->GetVkDevice(), m_vk_swapchain, gfx::Context::GetInstance()->GetVkAllocationCallbacks());

            /* Destroy surface */
            ::pfn_vkDestroySurfaceKHR(gfx::Context::GetInstance()->GetVkInstance(), m_vk_surface, gfx::Context::GetInstance()->GetVkAllocationCallbacks());
        }

        return;
    }

    void WindowThread::ThreadMain(size_t arg) {
        WindowMessage *window_message = reinterpret_cast<WindowMessage*>(arg);
        ::TranslateMessage(std::addressof(window_message->win32_msg));
        ::DispatchMessage(std::addressof(window_message->win32_msg));
    }

    bool WindowThread::RecreateSwapchainIfNecessaryUnsafe() {

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
        import_info.image_format      = static_cast<u32>(gfx::ImageFormat::R8G8B8A8_Unorm);
        import_info.array_layer_count = m_window_info.view_count;
        import_info.render_width      = m_window_info.width;
        import_info.render_height     = m_window_info.height;

        for (u32 i = 0; i < image_count; ++i) {
            import_info.vk_image = m_vk_image_array[i];
            m_render_target_color_array[i].Initialize(std::addressof(import_info));
        }

        return true;
    }

    void WindowThread::AcquireNextImage(u32 acquire_index) {

        /* Acquire full */
        const u32 image_count        = (m_window_info.enable_triple_buffer) ? 3 : 2;
        const u32 next_acquire_index = (image_count < acquire_index + 1) ? 0 : acquire_index + 1;
        const u32 result0 = ::pfn_vkAcquireNextImageKHR(gfx::Context::GetInstance()->GetVkDevice(), m_vk_swapchain, 0xffff'ffff'ffff'ffff, m_acquire_sync_array[next_acquire_index].GetVkSemaphore(), VK_NULL_HANDLE, std::addressof(m_image_index));

        /* Update results */
        if (result0 == VK_SUBOPTIMAL_KHR) { m_require_swapchain_refresh = true; }
        else { VP_ASSERT(result0 == VK_SUCCESS); }
    }

    void WindowThread::WaitForInitialize() { 
        m_window_event.Wait(); 
    }

    void WindowThread::ExitWindowThread() { 
        this->SendMessage(0);
        this->WaitForThreadExit();
    }
}

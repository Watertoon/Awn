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
            gfx::Sync                   m_acquire_sync_array[cMaxRenderTargetCount];
            sys::ServiceCriticalSection m_window_cs;
            sys::ServiceEvent           m_window_event;
            WindowInfo                  m_window_info;
            DragDropArray               m_drag_drop_array;
            u32                         m_drag_drop_count;
            DragDropStatus              m_drag_drop_status;
            u16                         m_image_count;
            bool                        m_require_swapchain_refresh;
            bool                        m_skip_draw;
            bool                        m_skip_acquire_sync;
        public:
            struct WindowMessage {
                MSG    win32_msg;
                size_t awn_message;
            };
        public:
            WindowThread(mem::Heap *heap, WindowInfo *window_info, u32 max_file_drag_drop) : ServiceThread("WindowThread", heap, sys::ThreadRunMode::Looping, 0, 8, 0x1000, sys::cPriorityNormal), m_hwnd(0), m_vk_surface(VK_NULL_HANDLE), m_vk_swapchain(VK_NULL_HANDLE), m_vk_image_array{}, m_render_target_color_array{}, m_image_index(-1), m_acquire_sync_array(), m_window_cs(), m_window_event(), m_window_info(), m_drag_drop_array(), m_drag_drop_count(), m_drag_drop_status(), m_require_swapchain_refresh(false), m_skip_draw(false) {

                /* Allocate drag drop array */
                m_drag_drop_array.Initialize(heap, max_file_drag_drop);

                /* Store window info */
                m_window_info = *window_info;
            }
            virtual ~WindowThread() override {/*...*/}

            virtual void Run() override;
            virtual void ThreadMain(size_t arg) override;

            bool RecreateSwapchainIfNecessaryUnsafe();

            ALWAYS_INLINE void ShowWindow() const { ::ShowWindow(m_hwnd, SW_SHOW); }
            ALWAYS_INLINE void HideWindow() const { ::ShowWindow(m_hwnd, SW_HIDE); }

                      ALWAYS_INLINE void           SetDragDropEnable(bool enable_drag_drop) const { ::DragAcceptFiles(m_hwnd, enable_drag_drop); }
                                    void           ClearDragDropStatus()                          { m_drag_drop_status = DragDropStatus::Clear; }
            constexpr ALWAYS_INLINE DragDropStatus GetDragDropStatus()                      const { return m_drag_drop_status; }

            constexpr ALWAYS_INLINE void SetSkipDraw(bool is_skip_draw)       { m_skip_draw = is_skip_draw; }
            constexpr ALWAYS_INLINE bool IsSkipDraw()                   const { return m_skip_draw; }

            constexpr ALWAYS_INLINE void SetSkipAcquireSync(bool is_skip_acquire_sync)       { m_skip_acquire_sync = is_skip_acquire_sync; }
            constexpr ALWAYS_INLINE bool IsSkipAcquireSync()                           const { return m_skip_acquire_sync; }

            constexpr void SignalWindowChange() { m_require_swapchain_refresh = true; }

            void AcquireNextImage(u32 acquire_index);

            void WaitForInitialize();
            void ExitWindowThread();

            constexpr ALWAYS_INLINE gfx::Sync              *GetAcquireSync(u32 acquire_index)       { return std::addressof(m_acquire_sync_array[acquire_index]); }
            constexpr ALWAYS_INLINE VkSurfaceKHR            GetVkSurface()                    const { return m_vk_surface; }
            constexpr ALWAYS_INLINE VkSwapchainKHR          GetVkSwapchain()                  const { return m_vk_swapchain; }
            constexpr ALWAYS_INLINE VkImage                 GetCurrentVkImage()               const { return m_vk_image_array[m_image_index]; }
            constexpr ALWAYS_INLINE u32                     GetImageIndex()                   const { return m_image_index; }
            constexpr ALWAYS_INLINE gfx::RenderTargetColor *GetCurrentRenderTarget()                { return std::addressof(m_render_target_color_array[m_image_index]); }
    };
}

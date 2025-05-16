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

namespace awn::grr::impl {

    AWN_SINGLETON_TRAITS_IMPL(DisplayManager);

    void DisplayManager::Present() {

        /* Submit user command lists */
        awn::gfx::Sync *submit_sync = std::addressof(m_submit_sync);
        awn::gfx::SubmitCommandLists(m_command_list_array, m_command_list_count, nullptr, 0, std::addressof(submit_sync), 1);

        /* Copy display to scanout */
        ID3D11DeviceContext4 *device_context = ContextDxgi11::GetInstance()->GetD3d11DeviceContext();
        const HRESULT hresult0 = device_context->Wait(m_submit_fence, m_submit_sync.GetExpectedValue());
        VP_ASSERT(hresult0 == S_OK);
        for (Display &display : m_display_list) {
            display.CopyScanOut();
        }
        device_context->Flush1(D3D11_CONTEXT_TYPE_ALL, m_submit_event);

        /* Present displays */
        for (Display &display : m_display_list) {
            display.Present();
        }

        /* Signal present complete */
        m_present_event.Signal();

        return;
    }

    void DisplayManager::PresentThreadMain(size_t message) {
        VP_UNUSED(message);
        this->Present();
    }

    void DisplayManager::AddNativeWindow(NativeWindow *native_window) {
        std::scoped_lock l(m_display_cs);
        m_native_window_list.PushBack(*native_window);
    }

    void DisplayManager::AddDisplay(Display *display) {
        std::scoped_lock l(m_display_cs);
        display->m_native_window->m_commit_settings.is_defer_pos = true;
        m_display_list.PushBack(*display);
    }

    void DisplayManager::RemoveNativeWindow(NativeWindow *native_window) {
        std::scoped_lock l(m_display_cs);
        m_native_window_list.Remove(*native_window);
    }

    void DisplayManager::RemoveDisplay(Display *display) {
        std::scoped_lock l(m_display_cs);
        display->m_native_window->m_commit_settings.is_defer_pos = false;
        m_display_list.Remove(*display);
    }

    void DisplayManager::Initialize(mem::Heap *heap) {

        /* Register window classes */
        const WNDCLASSEXW wnd_class_display = {
            .cbSize = sizeof(WNDCLASSEXW),
            .lpfnWndProc = NativeWindow::WindowProcedure,
            .hInstance = ::GetModuleHandle(nullptr),
            .lpszClassName = cNativeWindowClassName,
        };
        ATOM atom1 = ::RegisterClassExW(std::addressof(wnd_class_display));
        VP_ASSERT(atom1 != 0);

        /* Create present thread */
        sys::ThreadInfo thread_info = {
            .name = "awn::render::PresentThread",
            .exit_code = cMessageExitCode,
            .run_mode = sys::ThreadRunMode::WaitForMessage,
            .message_count = 8,
            .message_memory = m_message_memory,
            .stack_size = vp::util::c256KB,
            .priority = 0,
        };
        RESULT_ABORT_UNLESS(m_present_thread.Initialize(heap, std::addressof(thread_info), std::addressof(m_present_thread_function)));
        m_present_thread.StartThread();

        /* Create gfx sync for vulkan and direct3d 11 interoperation */
        m_submit_sync.Initialize();
        ContextDxgi11 *context_dxgi = ContextDxgi11::GetInstance();
        ID3D11Device5 *device_5 = context_dxgi->GetD3d11Device();
        const HRESULT hresult0 = device_5->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(std::addressof(m_submit_fence)));
        VP_ASSERT(hresult0 == S_OK);
        HANDLE semaphore_handle = nullptr;
        const HRESULT hresult1 = m_submit_fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, std::addressof(semaphore_handle));
        VP_ASSERT(hresult1 == S_OK);
        const VkImportSemaphoreWin32HandleInfoKHR import_info = {
            .sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR,
            .semaphore = m_submit_sync.GetVkSemaphore(),
            .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D11_FENCE_BIT,
            .handle = semaphore_handle,
        };
        const VkResult vk_result = ::pfn_vkImportSemaphoreWin32HandleKHR(gfx::impl::Context::GetInstance()->GetVkDevice(), std::addressof(import_info));
        RESULT_ABORT_UNLESS(gfx::impl::ConvertVkResultToResult(vk_result));
        ::CloseHandle(semaphore_handle);

        /* Create event to barrier d3d11 scanout */
        m_submit_event = ::CreateEventA(nullptr, false, false, nullptr);
        VP_ASSERT(m_submit_event != nullptr);

        /* Boost compositor clock */
        const HRESULT hresult2 = ::DCompositionBoostCompositorClock(true);
        VP_ASSERT(hresult2 == S_OK);

        return;
    }

    void DisplayManager::Finalize() {

        /* Force finalize object lists */
        auto display_iter = m_display_list.begin();
        while (display_iter != m_display_list.end()) {
            Display &display = *display_iter;
            ++display_iter;
            display.Finalize();
        }
        auto window_iter = m_native_window_list.begin();
        while (window_iter != m_native_window_list.end()) {
            NativeWindow &window = *window_iter;
            ++window_iter;
            window.Finalize();
        }

        /* Unregister window classes */
        const bool result1 = ::UnregisterClassW(cNativeWindowClassName, ::GetModuleHandle(nullptr));
        VP_ASSERT(result1);

        /* Exit present thread */
        m_present_thread.ExitThread(sys::ExitType::Jam);

        /* Destroy sync */
        if (m_submit_event != nullptr) {
            ::CloseHandle(m_submit_event);
            m_submit_event = nullptr;
        }
        if (m_submit_fence != nullptr) {
            m_submit_fence->Release();
            m_submit_fence = nullptr;
        }
        m_submit_sync.Finalize();

        /* Reset compositor clock boost */
        const HRESULT hresult0 = ::DCompositionBoostCompositorClock(false);
        VP_ASSERT(hresult0 == S_OK);

        return;
    }

    void DisplayManager::SubmitAndPresentAsync(gfx::CommandList *command_list_array, u32 command_list_count) {

        /* Set command lists */
        for (u32 i = 0; i < command_list_count; ++i) {
            m_command_list_array[i] = command_list_array[i];
        }
        m_command_list_count = command_list_count;

        /* Signal thread to present */
        m_is_present = true;
        m_present_event.Clear();
        m_present_thread.SendMessage(0);

        return;
    }

    void DisplayManager::WaitAndCompletePresent() {

        /* Check present completion */
        std::scoped_lock l(m_display_cs);
        if (m_is_present) {
            m_present_event.Wait();

            /* Ensure rendering completed */
            const u32 result = ::WaitForSingleObjectEx(m_submit_event, INFINITE, false);
            VP_ASSERT(result == WAIT_OBJECT_0);
            m_submit_sync.Wait();
            m_submit_sync.IncrementExpectedValue();

            /* Acquire next target id */
            for (Display &display : m_display_list) {
                display.AcquireNext();
            }
        }

        /* Apply window state */
        for (NativeWindow &window : m_native_window_list) {
            window.Calculate();
        }

        /* Create next target */
        if (m_is_present) {

            for (Display &display : m_display_list) {
                display.CreateTarget();
            }

            for (NativeWindow &window : m_native_window_list) {
                window.RequestWindowPos(false);
            }

            m_is_present = false;
        } else {
            /* Apply window position */
            for (NativeWindow &window : m_native_window_list) {
                window.RequestWindowPos(true);
            }
        }

        return;
    }
}

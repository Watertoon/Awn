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
#include <dwmapi.h>

namespace awn::render::impl {

    void Swapchain::PresentThreadMain(size_t message) {
        this->Present(message);
    }

    void Swapchain::Present(u64 message) {
        VP_UNUSED(message);

        std::scoped_lock l(m_present_cs);
        VP_ASSERT(m_present_stage == 1);

        /* Submit queue */
        gfx::Sync *submit_sync = std::addressof(m_submit_sync);
        gfx::impl::Context::GetInstance()->SubmitCommandList(m_command_list, nullptr, 0, std::addressof(submit_sync), 1);

        /* Present */
        u32 options = 0;
        u32 present_interval = m_present_interval;
        const bool is_post_resize = m_trigger_post_resize;
        if (is_post_resize) {
            m_trigger_post_resize = false;
            //DXGI_PRESENT_PARAMETERS present_params = {};
            //const HRESULT result0 = m_dxgi_swapchain->Present1(0, DXGI_PRESENT_RESTART, std::addressof(present_params));
            //VP_ASSERT(result0 == DXGI_STATUS_OCCLUDED || result0 == DXGI_STATUS_MODE_CHANGED || result0 == S_OK);
            //options = DXGI_PRESENT_DO_NOT_SEQUENCE;
        }
        DXGI_PRESENT_PARAMETERS present_params = {};
        const HRESULT result0 = m_dxgi_swapchain->Present1(present_interval, options, std::addressof(present_params));
        VP_ASSERT(result0 == DXGI_STATUS_OCCLUDED || result0 == DXGI_STATUS_MODE_CHANGED || result0 == S_OK);
        m_current_index = m_dxgi_swapchain->GetCurrentBackBufferIndex();

        /* Present sync */
        ID3D12CommandQueue *command_queue = impl::ContextDxgi::GetInstance()->GetD3d12CommandQueue();
        const size_t value = submit_sync->GetExpectedValue();
        command_queue->Signal(m_d3d12_fence, value);
        const HRESULT result1 = m_d3d12_fence->SetEventOnCompletion(value, m_fence_handle);
        VP_ASSERT(result1 == S_OK);

        /* Signal request completion */
        m_present_stage = 2;

        return;
    }

    bool Swapchain::ApplyUpdates() {

        std::scoped_lock l(m_settings_cs);

        if (m_is_request_recreate) {
            this->IdlePresent();
            this->CreateSwapchain();
            m_is_request_recreate = false;
            m_is_request_resize = false;
            m_trigger_post_resize = true;
            return true;
        }
        if (m_is_request_resize) {
            //this->IdlePresent();
            impl::ContextDxgi::GetInstance()->ReportDebug();
            this->DestroyImages();
            const u32 tearing_option = (impl::ContextDxgi::GetInstance()->IsTearing()) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
            const u32 swapchain_options = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | tearing_option;
            const HRESULT hresult = m_dxgi_swapchain->ResizeBuffers(m_update_settings.image_count, m_update_settings.size.x, m_update_settings.size.y, GfxImageFormatToDxgiFormat(m_update_settings.image_format), swapchain_options);
            VP_ASSERT(hresult == S_OK);
            this->CreateImages();
            m_current_index = m_dxgi_swapchain->GetCurrentBackBufferIndex();
            m_settings.size = m_update_settings.size;
            m_settings.image_format = m_update_settings.image_format;
            m_settings.image_count = m_update_settings.image_count;
            m_is_request_resize = false;
            m_trigger_post_resize = true;
            return true;
        }

        return false;
    }

    void Swapchain::CreateSwapchain() {

        /* Release previous swapchain */
        this->DestroySwapchain();

        /* Create swapchain */
        impl::ContextDxgi *context = impl::ContextDxgi::GetInstance();
		IDXGIFactory7 *factory_7 = context->GetDxgiFactory();
		ID3D12CommandQueue *command_queue = context->GetD3d12CommandQueue();
        const u32 tearing_option = (context->IsTearing()) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        const u32 swapchain_options = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | tearing_option;
		DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {
			.Width = static_cast<u32>(m_update_settings.size.x),
			.Height = static_cast<u32>(m_update_settings.size.y),
			.Format = impl::GfxImageFormatToDxgiFormat(m_update_settings.image_format),
			.Stereo = (m_update_settings.view_count == 2),
			.SampleDesc = {
				.Count = 1,
				.Quality = 0,
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = m_update_settings.image_count,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = (m_is_enable_framebuffer_alpha) ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE,
			.Flags = swapchain_options,
		};
		IDXGISwapChain1 *swapchain_1 = nullptr;
		const HRESULT hresult0 = factory_7->CreateSwapChainForComposition(command_queue, std::addressof(swapchain_desc), nullptr, std::addressof(swapchain_1));
		VP_ASSERT(hresult0 == S_OK);
		IDXGISwapChain4 *swapchain_4 = nullptr;
		const HRESULT hresult1 = swapchain_1->QueryInterface(IID_PPV_ARGS(std::addressof(swapchain_4)));
		VP_ASSERT(hresult1 == S_OK);
		swapchain_1->Release();
        m_dxgi_swapchain = swapchain_4;

		/* Set max latency */
		const HRESULT hresult2 = swapchain_4->SetMaximumFrameLatency(1);
		VP_ASSERT(hresult2 == S_OK);
        m_wait_handle = swapchain_4->GetFrameLatencyWaitableObject();

        /* Create swapchain images */
        this->CreateImages();

        m_current_index = m_dxgi_swapchain->GetCurrentBackBufferIndex();
        m_settings = m_update_settings;
    
        return;
    }

    void Swapchain::DestroySwapchain() {
        this->DestroyImages();
        if (m_wait_handle != nullptr) {
            ::CloseHandle(m_wait_handle);
            m_wait_handle = nullptr;
        }
        if (m_dxgi_swapchain != nullptr) {
            m_dxgi_swapchain->Release();
            m_dxgi_swapchain = nullptr;
        }
    }

    void Swapchain::CreateImages() {

        /* Get swapchain info */
		IDXGISwapChain4 *swapchain_4 = m_dxgi_swapchain;
		DXGI_SWAP_CHAIN_DESC1 out_swapchain_desc;
		const HRESULT hresult4 = swapchain_4->GetDesc1(std::addressof(out_swapchain_desc));
		VP_ASSERT(hresult4 == S_OK);

		/* Initialize resources */
        impl::ContextDxgi *context_dxgi = impl::ContextDxgi::GetInstance();
		ID3D12Device8 *device_8 = context_dxgi->GetD3d12Device();
		const VkExternalMemoryImageCreateInfo external_memory = {
			.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
			.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT,
		};
		const gfx::TextureInfo texture_info = {
			.image_storage_dimension = static_cast<u8>(gfx::ImageStorageDimension::Type2d),
			.tile_mode = static_cast<u16>(gfx::TileMode::Optimal),
			.mip_level_count = 1,
			.sample_count = 1,
			.image_format = static_cast<u32>(m_update_settings.image_format),
			.gpu_access_flags = static_cast<u32>(gfx::GpuAccessFlags::RenderTargetColor),
			.width = out_swapchain_desc.Width,
			.height = out_swapchain_desc.Height,
			.depth = 1,
			.array_layer_count = m_update_settings.view_count,
		};
		gfx::RenderTargetImportInfo render_target_info = {
			.image_format = static_cast<u32>(m_update_settings.image_format),
			.image_dimension = static_cast<u8>(gfx::ImageDimension::Type2d),
			.array_layer_count = m_update_settings.view_count,
			.render_width = out_swapchain_desc.Width,
			.render_height = out_swapchain_desc.Height,
		};
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
		for (u32 i = 0; i < m_update_settings.image_count; ++i) {

			/* Import dxgi resource as vulkan memory */
            ID3D12Resource2 *resource_2 = nullptr;
			const HRESULT result3 = swapchain_4->GetBuffer(i, IID_PPV_ARGS(std::addressof(resource_2)));
            VP_ASSERT(result3 == S_OK);
            HANDLE resource_handle = nullptr;
			const HRESULT result4 = device_8->CreateSharedHandle(resource_2, nullptr, GENERIC_ALL, nullptr, std::addressof(resource_handle));
			VP_ASSERT(result4 == S_OK);
            m_d3d12_resource_array[i] = resource_2;

			/* Create vk image and bind dxgi memory */
			VkImage vk_image = gfx::impl::CreateVkImage(std::addressof(texture_info), std::addressof(external_memory));
			VP_ASSERT(vk_image != VK_NULL_HANDLE);
            m_vk_image_array[i] = vk_image;
			gfx::GpuMemoryRequirements memory_requirements = gfx::impl::GetVkImageMemoryRequirements(vk_image);
			const VkImportMemoryWin32HandleInfoKHR import_win32_info = {
				.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
				.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT,
				.handle = resource_handle,
			};
			const VkMemoryDedicatedAllocateInfo dedicated_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
				.pNext = std::addressof(import_win32_info),
				.image = vk_image,
			};
			const VkMemoryAllocateInfo allocate_info = {
				.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext          = std::addressof(dedicated_info),
				.allocationSize = memory_requirements.size,
				.memoryTypeIndex = context->GetVkMemoryTypeIndex(gfx::MemoryPropertyFlags::GpuUncached),
			};
			const VkResult result5 = ::pfn_vkAllocateMemory(context->GetVkDevice(), std::addressof(allocate_info), context->GetVkAllocationCallbacks(), std::addressof(m_vk_device_memory_array[i]));
			VP_ASSERT(result5 == VK_SUCCESS);
            m_image_handle_array[i] = resource_handle;
            const VkResult result6 = ::pfn_vkBindImageMemory(context->GetVkDevice(), vk_image, m_vk_device_memory_array[i], 0);
            VP_ASSERT(result6 == VK_SUCCESS);

			/* Initialize render target */
			render_target_info.vk_image = vk_image;
			m_render_target_color_array[i].Initialize(std::addressof(render_target_info));
		}

        return;
    }

    void Swapchain::DestroyImages() {
        for (u32 i = 0; i < m_settings.image_count; ++i) {
            m_render_target_color_array[i].Finalize();
            gfx::impl::DestroyVkImage(m_vk_image_array[i]);
            m_vk_image_array[i] = VK_NULL_HANDLE;
            if (m_vk_device_memory_array[i] != nullptr) {
                gfx::impl::Context *context = gfx::impl::Context::GetInstance();
                ::pfn_vkFreeMemory(context->GetVkDevice(), m_vk_device_memory_array[i], context->GetVkAllocationCallbacks());
                m_vk_device_memory_array[i] = nullptr;
            }
            if (m_d3d12_resource_array[i] != nullptr) {
                m_d3d12_resource_array[i]->Release();
                m_d3d12_resource_array[i] = nullptr;
            }
            if (m_image_handle_array[i] != nullptr) {
                ::CloseHandle(m_image_handle_array[i]);
                m_image_handle_array[i] = nullptr;
            }
        }
        m_settings.image_count = 0;
    }

    void Swapchain::IdlePresent() {
        this->WaitForPresent();
        ::pfn_vkDeviceWaitIdle(gfx::impl::Context::GetInstance()->GetVkDevice());
    }

	void Swapchain::Initialize(const SwapchainInfo *swapchain_info, NativeWindowHandle handle) {

        VP_ASSERT(cTargetMinSwapchainImageCount <= swapchain_info->image_count && swapchain_info->image_count <= cTargetMaxSwapchainImageCount);
        VP_ASSERT(cTargetMinSwapchainViewCount <= swapchain_info->view_count && swapchain_info->view_count <= cTargetMaxSwapchainViewCount);

        m_update_settings.size = swapchain_info->size;
        m_update_settings.image_format = swapchain_info->image_format;
        m_update_settings.image_count = swapchain_info->image_count;
        m_update_settings.view_count = swapchain_info->view_count;
        m_present_interval = swapchain_info->present_interval;
        m_window_handle = handle;
        m_is_enable_framebuffer_alpha = true;

		/* Create swapchain */
        this->CreateSwapchain();

        vp::util::ConstructAt(m_present_thread, std::addressof(m_present_thread_function), "awn::render::PresentThread", nullptr, sys::ThreadRunMode::WaitForMessage, cMessageExitCode, 8, vp::util::c8KB, sys::cPriorityHigh);
        vp::util::GetReference(m_present_thread).StartThread();

        /* Initialize sync */
        m_submit_sync.Initialize();
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
        impl::ContextDxgi *context_dxgi = impl::ContextDxgi::GetInstance();
		ID3D12Device8 *device_8 = context_dxgi->GetD3d12Device();
        const HRESULT result7 = device_8->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(std::addressof(m_d3d12_fence)));
        VP_ASSERT(result7 == S_OK);
        const HRESULT result8 = device_8->CreateSharedHandle(m_d3d12_fence, nullptr, GENERIC_ALL, nullptr, std::addressof(m_sync_handle));
        VP_ASSERT(result8 == S_OK);
        const VkImportSemaphoreWin32HandleInfoKHR import_info = {
            .sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR,
            .semaphore = m_submit_sync.GetVkSemaphore(),
            .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT,
            .handle = m_sync_handle,
        };
        const VkResult result = ::pfn_vkImportSemaphoreWin32HandleKHR(context->GetVkDevice(), std::addressof(import_info));
        VP_ASSERT(result == VK_SUCCESS);
        m_fence_handle = ::CreateEventA(nullptr, false, false, nullptr);
        VP_ASSERT(m_fence_handle != nullptr);

		return;
	}

	void Swapchain::Finalize() {
        this->IdlePresent();
        vp::util::DestructAt(m_present_thread);
        this->DestroySwapchain();
        m_submit_sync.Finalize();
        if (m_d3d12_fence != nullptr) {
            m_d3d12_fence->Release();
            m_d3d12_fence = nullptr;
        }
        if (m_sync_handle != nullptr) {
            ::CloseHandle(m_sync_handle);
            m_sync_handle = nullptr;
        }
        if (m_fence_handle != nullptr) {
            ::CloseHandle(m_fence_handle);
            m_fence_handle = nullptr;
        }
	}

    void Swapchain::RequestPresent(gfx::CommandList command_list) {
        std::scoped_lock l(m_present_cs);
        const u32 present_id = m_current_index;
        m_command_list = command_list;
        m_present_stage = 1;
        vp::util::GetReference(m_present_thread).SendMessage(present_id);
        return;
    }

    void Swapchain::WaitForPresent() {

        /* Wait until async present occurs */     
        u8 *stage = std::addressof(m_present_stage);
        u8 last_stage = *stage;
        while (last_stage == 1) {
            sys::SleepThread(vp::TimeSpan::FromMicroSeconds(100));
            last_stage = *stage;
        }
        if (last_stage == 0) { return; }

        std::scoped_lock l(m_present_cs);

        /* Wait for queue and present */
        const HANDLE handle_array[2] = {
            m_fence_handle,
            m_wait_handle,
        };
        const u32 result1 = ::WaitForMultipleObjectsEx(2, handle_array, true, INFINITE, false);
        VP_ASSERT(result1 != WAIT_FAILED);

        /* Setup next sync value */
        gfx::Sync *submit_sync = std::addressof(m_submit_sync);
        submit_sync->IncrementExpectedValue();

        m_present_stage = 0;

        return;
    }

    void Swapchain::RequestSetSize(vp::util::Int2 size) {
        std::scoped_lock l(m_settings_cs);
        if (m_update_settings.size == size) { return; }
        m_update_settings.size = size;
        m_is_request_resize = true;
        return;
    }

    void Swapchain::RequestSetImageFormat(gfx::ImageFormat image_format) {
        std::scoped_lock l(m_settings_cs);
        if (m_update_settings.image_format == image_format) { return; }
        m_update_settings.image_format = image_format;
        m_is_request_resize = true;
        return;
    }

    void Swapchain::RequestSetImageCount(u32 image_count) {
        std::scoped_lock l(m_settings_cs);
        if (m_update_settings.image_count == image_count) { return; }
        m_update_settings.image_count = image_count;
        m_is_request_resize = true;
        return;
    }

    void Swapchain::RequestSetViewCount(u32 view_count) {
        std::scoped_lock l(m_settings_cs);
        if (m_update_settings.view_count == view_count) { return; }
        m_update_settings.view_count = view_count;
        m_is_request_recreate = true;
        return;
    }
}

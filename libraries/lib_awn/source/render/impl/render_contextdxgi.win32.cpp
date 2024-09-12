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

namespace awn::render::impl {

    AWN_SINGLETON_TRAITS_IMPL(ContextDxgi);

	void ContextDxgi::Initialize() {

		/* Ensure gfx exists */
		gfx::impl::Context *context = gfx::impl::Context::GetInstance();
		VP_ASSERT(context != nullptr);

#ifdef AWN_DXGI_DEBUG
        /* Create debug */
		const HRESULT hresult5 = ::D3D12GetDebugInterface(IID_PPV_ARGS(std::addressof(m_d3d12_debug)));
		VP_ASSERT(hresult5 == S_OK);
        m_d3d12_debug->EnableDebugLayer();
		const HRESULT hresult9 = ::DXGIGetDebugInterface1(0, IID_PPV_ARGS(std::addressof(m_dxgi_debug)));
		VP_ASSERT(hresult9 == S_OK);
#endif

		/* Create dxgi factory */
		IDXGIFactory7 *factory_7 = nullptr;
		const HRESULT hresult0 = ::CreateDXGIFactory2(0, IID_PPV_ARGS(std::addressof(factory_7)));
		VP_ASSERT(hresult0 == S_OK);

		/* Check tearing support */
		u32 is_tearing = false;
		const HRESULT hresult1 = factory_7->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, std::addressof(is_tearing), sizeof(u32));
		m_is_tearing = (hresult1 == S_OK) ? is_tearing : false;

		/* Use dxgi adapter from gfx context */
		const LUID *luid = reinterpret_cast<const LUID*>(context->GetPhysicalDeviceLuid());
		IDXGIAdapter4 *adapter_4 = nullptr;
		const HRESULT hresult2 = factory_7->EnumAdapterByLuid(*luid, IID_PPV_ARGS(std::addressof(adapter_4)));
		VP_ASSERT(hresult2 == S_OK);

		/* Create D3d12 device */
		ID3D12Device8 *d3d_device_8 = nullptr;
		const HRESULT hresult3 = ::D3D12CreateDevice(adapter_4, cDirectXFeatureLevel, IID_PPV_ARGS(std::addressof(d3d_device_8)));
		VP_ASSERT(hresult3 == S_OK);
#ifdef AWN_DXGI_DEBUG
        const HRESULT hresult7 = d3d_device_8->QueryInterface(IID_PPV_ARGS(std::addressof(m_d3d12_debug_device)));
        VP_ASSERT(hresult7 == S_OK);
#endif

		/* Create D3d12 command queue */
		const D3D12_COMMAND_QUEUE_DESC command_queue_desc = {
		    .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		    .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		};
		ID3D12CommandQueue *command_queue = nullptr;
		const HRESULT hresult4 = d3d_device_8->CreateCommandQueue(std::addressof(command_queue_desc), IID_PPV_ARGS(std::addressof(command_queue)));
		VP_ASSERT(hresult4 == S_OK);

        /* Create direct composition device */
        IDCompositionDesktopDevice *dcmp_device_desktop = nullptr;
        const HRESULT hresult6 = ::DCompositionCreateDevice3(nullptr, IID_PPV_ARGS(std::addressof(dcmp_device_desktop)));
        VP_ASSERT(hresult6 == S_OK);

        //DCOMPOSITION_FRAME_STATISTICS frame_stats = {};
        //dcmp_device_desktop->GetFrameStatistics(std::addressof(frame_stats));
        //::printf("FrameStatistics\n");
        //::printf("  lastFrameTime: %lld\n", frame_stats.lastFrameTime.QuadPart);
        //::printf("  currentCompositionRate: %d / %d\n", frame_stats.currentCompositionRate.Numerator, frame_stats.currentCompositionRate.Denominator);
        //::printf("  currentTime: %lld\n", frame_stats.currentTime.QuadPart);
        //::printf("  timeFrequency: %lld\n", frame_stats.timeFrequency.QuadPart);
        //::printf("  nextEstimatedFrameTime: %lld\n", frame_stats.nextEstimatedFrameTime.QuadPart);

		/* Set DirectX interfaces */
		m_dxgi_factory = factory_7;
		m_dxgi_adapter = adapter_4;
		m_d3d12_device = d3d_device_8;
		m_d3d12_command_queue = command_queue;
		m_dcmp_device = dcmp_device_desktop;

		return;
	}

	void ContextDxgi::Finalize() {
		if (m_dcmp_device != nullptr) {
			m_dcmp_device->Release();
			m_dcmp_device = nullptr;
		}
		if (m_d3d12_command_queue != nullptr) {
			m_d3d12_command_queue->Release();
			m_d3d12_command_queue = nullptr;
		}
		if (m_d3d12_device != nullptr) {
			m_d3d12_device->Release();
			m_d3d12_device = nullptr;
		}
		if (m_dxgi_adapter != nullptr) {
			m_dxgi_adapter->Release();
			m_dxgi_adapter = nullptr;
		}
		if (m_dxgi_factory != nullptr) {
			m_dxgi_factory->Release();
			m_dxgi_factory = nullptr;
		}
#ifdef AWN_DXGI_DEBUG
        this->ReportDebug();
		if (m_d3d12_debug_device != nullptr) {
			m_d3d12_debug_device->Release();
			m_d3d12_debug_device = nullptr;
		}
		if (m_dxgi_debug != nullptr) {
			m_dxgi_debug->Release();
			m_dxgi_debug = nullptr;
		}
		if (m_d3d12_debug != nullptr) {
			m_d3d12_debug->Release();
			m_d3d12_debug = nullptr;
		}
#endif
	}
}

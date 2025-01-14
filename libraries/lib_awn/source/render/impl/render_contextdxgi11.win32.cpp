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

    AWN_SINGLETON_TRAITS_IMPL(ContextDxgi11);

	void ContextDxgi11::Initialize() {

		/* Ensure gfx exists */
		gfx::impl::Context *context = gfx::impl::Context::GetInstance();
		VP_ASSERT(context != nullptr);

#ifdef AWN_RENDER_DIRECTX_DEBUG
        /* Create debug */
		const HRESULT hresult0 = ::DXGIGetDebugInterface1(0, IID_PPV_ARGS(std::addressof(m_dxgi_debug)));
		VP_ASSERT(hresult0 == S_OK);
#endif

		/* Create dxgi factory */
		IDXGIFactory7 *factory_7 = nullptr;
		const HRESULT hresult1 = ::CreateDXGIFactory2(0, IID_PPV_ARGS(std::addressof(factory_7)));
		VP_ASSERT(hresult1 == S_OK);

		/* Check tearing support */
		//u32 is_tearing = false;
		//const HRESULT hresult2 = factory_7->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, std::addressof(is_tearing), sizeof(u32));
		//m_is_tearing = (hresult2 == S_OK) ? is_tearing : false;

		/* Use dxgi adapter from gfx context */
		const LUID *luid = reinterpret_cast<const LUID*>(context->GetPhysicalDeviceLuid());
		IDXGIAdapter4 *adapter_4 = nullptr;
		const HRESULT hresult3 = factory_7->EnumAdapterByLuid(*luid, IID_PPV_ARGS(std::addressof(adapter_4)));
		VP_ASSERT(hresult3 == S_OK);

        /* Create d3d11 device */
        const D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_1;
#ifdef AWN_RENDER_DIRECTX_DEBUG
        const u32 debug_flags = D3D11_CREATE_DEVICE_DEBUG;
#else
        const u32 debug_flags = 0;
#endif
        const u32 d3d11_flags = D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS | D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT | debug_flags;
        const D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_UNKNOWN;
        ID3D11Device *d3d11_device = nullptr;
        ID3D11DeviceContext *d3d11_device_context = nullptr;
        const HRESULT hresult4 = ::D3D11CreateDevice(adapter_4, driver_type, nullptr, d3d11_flags, std::addressof(feature_level), 1, D3D11_SDK_VERSION, std::addressof(d3d11_device), nullptr, std::addressof(d3d11_device_context));
		VP_ASSERT(hresult4 == S_OK);
        const HRESULT hresult5 = d3d11_device->QueryInterface(IID_PPV_ARGS(std::addressof(m_d3d11_device)));
		VP_ASSERT(hresult5 == S_OK);
        d3d11_device->Release();
#ifdef AWN_RENDER_DIRECTX_DEBUG
        const HRESULT hresult6 = d3d11_device->QueryInterface(IID_PPV_ARGS(std::addressof(m_d3d11_debug)));
		VP_ASSERT(hresult6 == S_OK);
#endif
        const HRESULT hresult7 = d3d11_device_context->QueryInterface(IID_PPV_ARGS(std::addressof(m_d3d11_device_context)));
		VP_ASSERT(hresult7 == S_OK);
        d3d11_device_context->Release();

        /* Ensure display surface */
        D3D11_FEATURE_DATA_DISPLAYABLE displayable = {};
        const HRESULT hresult11 = m_d3d11_device->CheckFeatureSupport(static_cast<D3D11_FEATURE>(cD3d11FeatureDisplayable), std::addressof(displayable), sizeof(D3D11_FEATURE_DATA_DISPLAYABLE));
        VP_ASSERT(hresult11 == S_OK);
        VP_ASSERT(displayable.DisplayableTexture);

        //::printf("DisplayableTexture: %d\nSharedResourceTier: %d\n", displayable.DisplayableTexture, displayable.SharedResourceTier);

        /* Create direct composition device */
        IDCompositionDesktopDevice *dcmp_device_desktop = nullptr;
        const HRESULT hresult8 = ::DCompositionCreateDevice3(m_d3d11_device, IID_PPV_ARGS(std::addressof(dcmp_device_desktop)));
        VP_ASSERT(hresult8 == S_OK);

        IDCompositionDevice4 *dcmp_device4 = nullptr;
        const HRESULT hresult9 = dcmp_device_desktop->QueryInterface(IID_PPV_ARGS(std::addressof(dcmp_device4)));
        VP_ASSERT(hresult9 == S_OK);
        BOOL is_support = false;
        const HRESULT hresult10 = dcmp_device4->CheckCompositionTextureSupport(m_d3d11_device, std::addressof(is_support));
        VP_ASSERT(hresult10 == S_OK);
        VP_ASSERT(is_support);
        m_dcmp_device4 = dcmp_device4;

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
		m_dcmp_device = dcmp_device_desktop;

		return;
	}

	void ContextDxgi11::Finalize() {
		if (m_dcmp_device4 != nullptr) {
			m_dcmp_device4->Release();
			m_dcmp_device4 = nullptr;
		}
		if (m_dcmp_device != nullptr) {
			m_dcmp_device->Release();
			m_dcmp_device = nullptr;
		}
		if (m_d3d11_device_context != nullptr) {
			m_d3d11_device_context->Release();
			m_d3d11_device_context = nullptr;
		}
		if (m_d3d11_device != nullptr) {
			m_d3d11_device->Release();
			m_d3d11_device = nullptr;
		}
		if (m_dxgi_adapter != nullptr) {
			m_dxgi_adapter->Release();
			m_dxgi_adapter = nullptr;
		}
		if (m_dxgi_factory != nullptr) {
			m_dxgi_factory->Release();
			m_dxgi_factory = nullptr;
		}
#ifdef AWN_RENDER_DIRECTX_DEBUG
        //this->ReportDebug();
		//if (m_d3d11_debug_device != nullptr) {
		//	m_d3d11_debug_device->Release();
		//	m_d3d11_debug_device = nullptr;
		//}
		if (m_dxgi_debug != nullptr) {
			m_dxgi_debug->Release();
			m_dxgi_debug = nullptr;
		}
		//if (m_d3d11_info_queue != nullptr) {
		//	m_d3d11_info_queue->Release();
		//	m_d3d11_info_queue = nullptr;
		//}
		if (m_d3d11_debug != nullptr) {
			m_d3d11_debug->Release();
			m_d3d11_debug = nullptr;
		}
#endif
	}
}

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

namespace awn::render::impl {

//#define AWN_RENDER_DIRECTX_DEBUG

    constexpr DXGI_FORMAT GfxImageFormatToDxgiFormat(gfx::ImageFormat format) {
        switch(format) {
            case gfx::ImageFormat::B8G8R8A8_Unorm:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            case gfx::ImageFormat::R8G8B8A8_Unorm:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case gfx::ImageFormat::R10G10B10A2_Unorm:
                return DXGI_FORMAT_R10G10B10A2_UNORM;
            //case gfx::ImageFormat::R11G11B10F_Float:
            //    return DXGI_FORMAT_R11G11B10_FLOAT;
            case gfx::ImageFormat::R16G16B16A16_Float:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;
            default:
                break;
        }
        VP_UNEXPECTED_DEFAULT();
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

	class ContextDxgi11 {
		private:
            IDXGIFactory7              *m_dxgi_factory;
            IDXGIAdapter4              *m_dxgi_adapter;
            IDXGIDevice4               *m_dxgi_device;
            IDCompositionDesktopDevice *m_dcmp_device;
            IDCompositionDevice4       *m_dcmp_device4;
            ID3D11Device5              *m_d3d11_device;
            ID3D11DeviceContext4       *m_d3d11_device_context;
#ifdef AWN_RENDER_DIRECTX_DEBUG
            IDXGIDebug1                *m_dxgi_debug;
            ID3D11Debug                *m_d3d11_debug;
#endif
        public:
            AWN_SINGLETON_TRAITS(ContextDxgi11);
        public:
            constexpr ContextDxgi11() : m_dxgi_factory(), m_dxgi_adapter(), m_dcmp_device(), m_dcmp_device4(), m_d3d11_device()
#ifdef AWN_RENDER_DIRECTX_DEBUG
            , m_dxgi_debug(), m_d3d11_debug()
#endif
            {/*...*/}
            constexpr ~ContextDxgi11() {/*...*/}

            void Initialize();
            void Finalize();

            constexpr IDXGIFactory7 *GetDxgiFactory() const { return m_dxgi_factory; }
            constexpr IDXGIAdapter4 *GetDxgiAdapter() const { return m_dxgi_adapter; }
            constexpr ID3D11Device5 *GetD3d11Device() const { return m_d3d11_device; }
            constexpr ID3D11DeviceContext4 *GetD3d11DeviceContext() const { return m_d3d11_device_context; }
            constexpr IDCompositionDesktopDevice *GetDcmpDevice() const { return m_dcmp_device; }
            constexpr IDCompositionDevice4 *GetDcmpDevice4() const { return m_dcmp_device4; }

            void ReportDebug() {
#ifdef AWN_RENDER_DIRECTX_DEBUG
                m_dxgi_debug->ReportLiveObjects(DXGI_DEBUG_DXGI, DXGI_DEBUG_RLO_DETAIL);
                m_d3d11_debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
            }
	};
}

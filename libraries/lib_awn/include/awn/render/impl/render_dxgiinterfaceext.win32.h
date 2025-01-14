#pragma once

#undef INTERFACE
#define INTERFACE IDCompositionVisual3
DECLARE_INTERFACE_IID_(IDCompositionVisual3, IDCompositionVisualDebug, "2775F462-B6C1-4015-B0BE-B3E7D6A4976D")
{
STDMETHOD(SetDepthMode)(THIS_ DCOMPOSITION_DEPTH_MODE) PURE;
STDMETHOD(SetOffsetZ)(THIS_ IDCompositionAnimation*) PURE;
STDMETHOD(SetOffsetZ)(THIS_ float) PURE;
STDMETHOD(SetOpacity)(THIS_ IDCompositionAnimation*) PURE;
STDMETHOD(SetOpacity)(THIS_ float) PURE;
STDMETHOD(SetTransform)(THIS_ IDCompositionTransform3D*) PURE;
STDMETHOD(SetTransform)(THIS_ const D2D_MATRIX_4X4_F&) PURE;
STDMETHOD(SetVisible)(THIS_ BOOL) PURE;
};

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IDCompositionVisual3,0x2775f462,0xb6c1,0x4015,0xb0,0xbe,0xb3,0xe7,0xd6,0xa4,0x97,0x6d);
#endif

#undef INTERFACE
#define INTERFACE IDCompositionTexture
DECLARE_INTERFACE_IID_(IDCompositionTexture, IUnknown, "929BB1AA-725F-433B-ABD7-273075A835F2")
{
STDMETHOD(SetSourceRect)(THIS_ const D2D_RECT_U&) PURE;
STDMETHOD(SetColorSpace)(THIS_ DXGI_COLOR_SPACE_TYPE) PURE;
STDMETHOD(SetAlphaMode)(THIS_ DXGI_ALPHA_MODE) PURE;
STDMETHOD(GetAvailableFence)(THIS_ UINT64*, REFIID, void**) PURE;
};

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IDCompositionTexture,0x929bb1aa,0x725f,0x433b,0xab,0xd7,0x27,0x30,0x75,0xa8,0x35,0xf2);
#endif

#undef INTERFACE
#define INTERFACE IDCompositionDevice4
DECLARE_INTERFACE_IID_(IDCompositionDevice4, IDCompositionDevice3, "85FC5CCA-2DA6-494C-86B6-4A775C049B8A")
{
STDMETHOD(CheckCompositionTextureSupport)(THIS_ IUnknown*, BOOL*) PURE;
STDMETHOD(CreateCompositionTexture)(THIS_ IUnknown*, IDCompositionTexture**) PURE;
};

struct D3D11_FEATURE_DATA_DISPLAYABLE {
    BOOL                       DisplayableTexture;
    D3D11_SHARED_RESOURCE_TIER SharedResourceTier;
};

static constexpr u32 cD3d11FeatureDisplayable = 20;

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IDCompositionDevice4,0x85fc5cca,0x2da6,0x494c,0x86,0xb6,0x4a,0x77,0x5c,0x04,0x9b,0x8a);
#endif

STDAPI DCompositionBoostCompositorClock(BOOL);

STDAPI_(DWORD) DCompositionWaitForCompositorClock(UINT, const HANDLE*, DWORD);

#undef INTERFACE

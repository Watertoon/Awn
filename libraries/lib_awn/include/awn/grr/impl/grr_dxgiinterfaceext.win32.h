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

//#undef INTERFACE
//#define INTERFACE IDCompositionTexture
//DECLARE_INTERFACE_IID_(IDCompositionTexture, IUnknown, "929BB1AA-725F-433B-ABD7-273075A835F2")
//{
//STDMETHOD(SetSourceRect)(THIS_ const D2D_RECT_U&) PURE;
//STDMETHOD(SetColorSpace)(THIS_ DXGI_COLOR_SPACE_TYPE) PURE;
//STDMETHOD(SetAlphaMode)(THIS_ DXGI_ALPHA_MODE) PURE;
//STDMETHOD(GetAvailableFence)(THIS_ UINT64*, REFIID, void**) PURE;
//};
//
//#ifdef __CRT_UUID_DECL
//__CRT_UUID_DECL(IDCompositionTexture,0x929bb1aa,0x725f,0x433b,0xab,0xd7,0x27,0x30,0x75,0xa8,0x35,0xf2);
//#endif

//#undef INTERFACE
//#define INTERFACE IDCompositionDevice4
//DECLARE_INTERFACE_IID_(IDCompositionDevice4, IDCompositionDevice3, "85FC5CCA-2DA6-494C-86B6-4A775C049B8A")
//{
//STDMETHOD(CheckCompositionTextureSupport)(THIS_ IUnknown*, BOOL*) PURE;
//STDMETHOD(CreateCompositionTexture)(THIS_ IUnknown*, IDCompositionTexture**) PURE;
//};
//#ifdef __CRT_UUID_DECL
//__CRT_UUID_DECL(IDCompositionDevice4,0x85fc5cca,0x2da6,0x494c,0x86,0xb6,0x4a,0x77,0x5c,0x04,0x9b,0x8a);
//#endif

struct D3D11_FEATURE_DATA_DISPLAYABLE {
    BOOL                       DisplayableTexture;
    D3D11_SHARED_RESOURCE_TIER SharedResourceTier;
};

static constexpr u32 cD3d11FeatureDisplayable = 20;


STDAPI DCompositionBoostCompositorClock(BOOL);

STDAPI_(DWORD) DCompositionWaitForCompositorClock(UINT, const HANDLE*, DWORD);

/* composition swapchain api */
STDAPI CreatePresentationFactory(IUnknown*,REFIID,void**);

DEFINE_GUID(IID_IPresentationBuffer,0x2e217d3a,0x5abb,0x4138,0x9a,0x13,0xa7,0x75,0x59,0x3c,0x89,0xca);

MIDL_INTERFACE("2E217D3A-5ABB-4138-9A13-A775593C89CA")
IPresentationBuffer : public IUnknown
{
virtual HRESULT GetAvailableEvent(HANDLE*) = 0;
virtual HRESULT IsAvailable(boolean*) = 0;
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IPresentationBuffer,0x2e217d3a,0x5abb,0x4138,0x9a,0x13,0xa7,0x75,0x59,0x3c,0x89,0xca);
#endif

DEFINE_GUID(IID_IPresentationContent,0x5668bb79,0x3d8e,0x415c,0xb2,0x15,0xf3,0x80,0x20,0xf2,0xd2,0x52);

MIDL_INTERFACE("5668BB79-3D8E-415C-B215-F38020F2D252")
IPresentationContent : public IUnknown
{
virtual HRESULT SetTag(UINT_PTR*) = 0;
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IPresentationContent,0x5668bb79,0x3d8e,0x415c,0xb2,0x15,0xf3,0x80,0x20,0xf2,0xd2,0x52);
#endif

DEFINE_GUID(IID_IPresentationSurface,0x956710fb,0xea40,0x4eba,0xa3,0xeb,0x43,0x75,0xa0,0xeb,0x4e,0xdc);

struct PresentationTransform {
    float M11;
    float M12;
    float M21;
    float M22;
    float M31;
    float M32;
};

MIDL_INTERFACE("956710FB-EA40-4EBA-A3EB-4375A0EB4EDC")
IPresentationSurface : public IPresentationContent
{
virtual HRESULT SetBuffer(IPresentationBuffer*) = 0;
virtual HRESULT SetColorSpace(DXGI_COLOR_SPACE_TYPE) = 0;
virtual HRESULT SetAlphaMode(DXGI_ALPHA_MODE) = 0;
virtual HRESULT SetSourceRect(const RECT*) = 0;
virtual HRESULT SetTransform(PresentationTransform*) = 0;
virtual HRESULT RestrictToOutput(IUnknown*) = 0;
virtual HRESULT SetDisableReadback(boolean) = 0;
virtual HRESULT SetLetterboxingMargins(float,float,float,float) = 0;
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IPresentationSurface,0x956710fb,0xea40,0x4eba,0xa3,0xeb,0x43,0x75,0xa0,0xeb,0x4e,0xdc);
#endif

enum PresentStatisticsKind {
    PresentStatisticsKind_PresentStatus = 0,
    PresentStatisticsKind_CompositionFrame = 0,
    PresentStatisticsKind_IndependentFlipFrame = 0,
};

DEFINE_GUID(IID_IPresentStatistics,0xb44b8bda,0x7282,0x495d,0x9d,0xd7,0xce,0xad,0xd8,0xb4,0xbb,0x86);

MIDL_INTERFACE("B44B8BDA-7282-495D-9DD7-CEADD8B4BB86")
IPresentStatistics : public IUnknown
{
virtual UINT64 STDMETHODCALLTYPE GetPresentId() = 0;
virtual PresentStatisticsKind STDMETHODCALLTYPE GetKind() = 0;
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IPresentStatistics,0xb44b8bda,0x7282,0x495d,0x9d,0xd7,0xce,0xad,0xd8,0xb4,0xbb,0x86);
#endif

struct SystemInterruptTime {
    UINT64 value;
};

DEFINE_GUID(IID_IPresentationManager,0xfb562f82,0x6292,0x470a,0x88,0xb1,0x84,0x36,0x61,0xe7,0xf2,0x0c);

MIDL_INTERFACE("FB562F82-6292-470A-88B1-843661E7F20C")
IPresentationManager : public IUnknown
{
virtual HRESULT STDMETHODCALLTYPE AddBufferFromResource(IUnknown*, IPresentationBuffer**) = 0;
virtual HRESULT STDMETHODCALLTYPE CreatePresentationSurface(HANDLE,IPresentationSurface**) = 0;
virtual UINT64 STDMETHODCALLTYPE GetNextPresentId() = 0;
virtual HRESULT STDMETHODCALLTYPE SetTargetTime(SystemInterruptTime) = 0;
virtual HRESULT STDMETHODCALLTYPE SetPreferredPresentDuration(SystemInterruptTime,SystemInterruptTime) = 0;
virtual HRESULT STDMETHODCALLTYPE ForceVSyncInterrupt(boolean) = 0;
virtual HRESULT STDMETHODCALLTYPE Present() = 0;
virtual HRESULT STDMETHODCALLTYPE GetPresentRetiringFence(REFIID, void**) = 0;
virtual HRESULT STDMETHODCALLTYPE CancelPresentsFrom(UINT64) = 0;
virtual HRESULT STDMETHODCALLTYPE GetLostEvent(HANDLE*) = 0;
virtual HRESULT STDMETHODCALLTYPE GetPresentStatisticsAvailableEvent(HANDLE*) = 0;
virtual HRESULT STDMETHODCALLTYPE EnablePresentStatisticsKind(PresentStatisticsKind, boolean) = 0;
virtual HRESULT STDMETHODCALLTYPE GetNextPresentStatistics(IPresentStatistics**) = 0;
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IPresentationManager,0xfb562f82,0x6292,0x470a,0x88,0xb1,0x84,0x36,0x61,0xe7,0xf2,0x0c);
#endif

DEFINE_GUID(IID_IPresentationFactory,0x8fb37b58,0x1d74,0x4f64,0xa4,0x9c,0x1f,0x97,0xa8,0x0a,0x2e,0xc0);

MIDL_INTERFACE("8FB37B58-1D74-4F64-A49C-1F97A80A2EC0")
IPresentationFactory : public IUnknown
{
virtual boolean STDMETHODCALLTYPE IsPresentationSupported() = 0;
virtual boolean STDMETHODCALLTYPE IsPresentationSupportedWithIndependentFlip() = 0;
virtual HRESULT STDMETHODCALLTYPE CreatePresentationManager(IPresentationManager**) = 0;
};
#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IPresentationFactory,0x8fb37b58,0x1d74,0x4f64,0xa4,0x9c,0x1f,0x97,0xa8,0x0a,0x2e,0xc0);
#endif

#undef INTERFACE

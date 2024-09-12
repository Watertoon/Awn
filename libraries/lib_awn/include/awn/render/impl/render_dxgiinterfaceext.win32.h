#pragma once

#undef INTERFACE
#define INTERFACE IDCompositionVisual3
DECLARE_INTERFACE_IID_(IDCompositionVisual3, IDCompositionVisualDebug, "2775F462-B6C1-4015-B0BE-B3E7D6A4976D")
{
STDMETHOD(SetDepthMode)(THIS_ DCOMPOSITION_DEPTH_MODE) PURE;
STDMETHOD(SetOffsetZ)(THIS_ float) PURE;
STDMETHOD(SetOffsetZ)(THIS_ IDCompositionAnimation*) PURE;
STDMETHOD(SetOpacity)(THIS_ float) PURE;
STDMETHOD(SetOpacity)(THIS_ IDCompositionAnimation*) PURE;
STDMETHOD(SetTransform)(THIS_ const D2D_MATRIX_4X4_F&) PURE;
STDMETHOD(SetTransform)(THIS_ IDCompositionTransform3D*) PURE;
STDMETHOD(SetVisible)(THIS_ BOOL) PURE;
};

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL(IDCompositionVisual3,0x2775f462,0xb6c1,0x4015,0xb0,0xbe,0xb3,0xe7,0xd6,0xa4,0x97,0x6d);
#endif

#undef INTERFACE

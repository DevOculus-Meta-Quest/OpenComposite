#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("Overlay", "013") // R3E
GEN_INTERFACE("Overlay", "014") // RF2

#include "GVROverlay.gen.h"

vr::EVROverlayError CVROverlay_013::GetOverlayTexture(vr::VROverlayHandle_t ulOverlayHandle,
                                                      void** pNativeTextureHandle,
                                                      void* pNativeTextureRef,
                                                      uint32_t* pWidth,
                                                      uint32_t* pHeight,
                                                      uint32_t* pNativeFormat,
                                                      EGraphicsAPIConvention* pAPI,
                                                      vr::EColorSpace* pColorSpace)
{

  // It should be fairly simple, but it's unlikely to be used so I can't be bothered implementing it now
  STUBBED();
}

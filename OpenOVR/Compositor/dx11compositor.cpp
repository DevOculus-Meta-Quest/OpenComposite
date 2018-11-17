#include "stdafx.h"
#include "compositor.h"
#include "libovr_wrapper.h"

#include "OVR_CAPI_D3D.h"
#include <assert.h>
#include <algorithm>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Misc/ScopeGuard.h"

using namespace std;
#define OVSS (*ovr::session)

#undef min
#undef max

DX11Compositor::DX11Compositor(ID3D11Texture2D* initial)
{
  initial->GetDevice(&mDevice);
  mDevice->GetImmediateContext(&mContext);
}

DX11Compositor::~DX11Compositor()
{
  if (mOVRSwapChain != nullptr)
    ovr_DestroyTextureSwapChain(OVSS, mOVRSwapChain);
}

void DX11Compositor::Invoke(vr::Texture_t const* texture)
{
  ovrTextureSwapChainDesc &desc = mOVRSwapChainDesc;

  int currentIndex = 0;
  if (mOVRSwapChain != nullptr)
    ovr_GetTextureSwapChainCurrentIndex(OVSS, mOVRSwapChain, &currentIndex);

  auto* src = static_cast<ID3D11Texture2D*>(texture->handle);

  D3D11_TEXTURE2D_DESC srcDesc;
  src->GetDesc(&srcDesc);

  bool usable = mOVRSwapChain == nullptr ? false : CheckChainCompatible(srcDesc, desc, texture->eColorSpace);
  if (!usable) {
    OOVR_LOG("Generating new swap chain");

    // First, delete the old chain if necessary
    if (mOVRSwapChain != nullptr)
      ovr_DestroyTextureSwapChain(OVSS, mOVRSwapChain);

    // Make eye render buffer
    desc = {};
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Width = srcDesc.Width;
    desc.Height = srcDesc.Height;
    desc.Format = dxgiToOvrFormat(srcDesc.Format, texture->eColorSpace);
    desc.MipLevels = srcDesc.MipLevels;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;

    desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
    desc.BindFlags = ovrTextureBind_None;

    mInputSrcSize.w = srcDesc.Width;
    mInputSrcSize.h = srcDesc.Height;

    OOVR_FAILED_OVR_ABORT(ovr_CreateTextureSwapChainDX(OVSS, mDevice, &desc, &mOVRSwapChain));
  }

  CComPtr<ID3D11Texture2D> tex;
  ovr_GetTextureSwapChainBufferDX(OVSS, mOVRSwapChain, currentIndex, IID_PPV_ARGS(&tex));
  mContext->CopyResource(tex, src);
}

void DX11Compositor::Invoke(
  ovrEyeType eye, 
  vr::Texture_t const* texture, 
  vr::VRTextureBounds_t const* ptrBounds,
  vr::EVRSubmitFlags submitFlags, 
  ovrLayerEyeFov& layer)
{

  // Copy the texture across
  Invoke(texture);

  // Set the viewport up
  auto& viewport = layer.Viewport[eye];
  if (ptrBounds) {
    vr::VRTextureBounds_t bounds = *ptrBounds;

    if (bounds.vMin > bounds.vMax) {
      mSubmitVerticallyFlipped = true;
      float newMax = bounds.vMin;
      bounds.vMin = bounds.vMax;
      bounds.vMax = newMax;
    } else {
      mSubmitVerticallyFlipped = false;
    }

    viewport.Pos.x = (int) (bounds.uMin * mOVRSwapChainDesc.Width);
    viewport.Pos.y = (int) (bounds.vMin * mOVRSwapChainDesc.Height);
    viewport.Size.w = (int) ((bounds.uMax - bounds.uMin) * mOVRSwapChainDesc.Width);
    viewport.Size.h = (int) ((bounds.vMax - bounds.vMin) * mOVRSwapChainDesc.Height);
  } else {
    viewport.Pos.x = viewport.Pos.y = 0;
    viewport.Size.w = mOVRSwapChainDesc.Width;
    viewport.Size.h = mOVRSwapChainDesc.Height;

    mSubmitVerticallyFlipped = false;
  }
}

unsigned int DX11Compositor::GetFlags()
{
  return mSubmitVerticallyFlipped ? ovrLayerFlag_TextureOriginAtBottomLeft : 0;
}

bool DX11Compositor::CheckChainCompatible(
  D3D11_TEXTURE2D_DESC& inputDesc,
  ovrTextureSwapChainDesc& chainDsc,
  vr::EColorSpace colourSpace)
{
  bool usable = true;
#define FAIL(name) { \
	usable = false; \
	OOVR_LOG("Resource mismatch: " #name); \
}
#define CHECK(name) CHECK_ADV(name, name)
#define CHECK_ADV(name, chainName) \
if(static_cast<int>(inputDesc.name) != chainDsc.chainName) FAIL(name);

  CHECK(Width);
  CHECK(Height);
  CHECK(MipLevels);
  if (mOVRSwapChainDesc.Format != dxgiToOvrFormat(inputDesc.Format, colourSpace)) FAIL(Format);
  //CHECK_ADV(SampleDesc.Count, SampleCount);
  //CHECK_ADV(SampleDesc.Quality);
#undef CHECK
#undef CHECK_ADV
#undef FAIL

  return usable;
}

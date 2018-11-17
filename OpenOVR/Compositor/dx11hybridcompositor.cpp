#include "stdafx.h"
#include "compositor.h"
#include "libovr_wrapper.h"

using namespace std;
#define OVSS (*ovr::session)

DX11HybridCompositor::DX11HybridCompositor(ID3D10Texture2D* initial)
{
  HRESULT hr = S_OK;
  auto iu = static_cast<IUnknown*>(initial);
  OOVR_FALSE_ABORT(iu != nullptr);

  CComQIPtr<ID3D11Texture2D> dx11Texture = initial;
  OOVR_FALSE_ABORT(dx11Texture != nullptr);

  dx11Texture->GetDevice(&mDevice);
  OOVR_FALSE_ABORT(mDevice != nullptr);

  mDevice->GetImmediateContext(&mContext);
  OOVR_FALSE_ABORT(mContext != nullptr);

  mDevice1 = mDevice;
  OOVR_FALSE_ABORT(mDevice1 != nullptr);

  mContext1 = mContext;
  OOVR_FALSE_ABORT(mContext1 != nullptr);

  UINT deviceFlags = mDevice->GetCreationFlags();
  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

  // If the device is single threaded, the context state must be too
  UINT stateFlags = 0;
  if (deviceFlags & D3D11_CREATE_DEVICE_SINGLETHREADED) {
    stateFlags |= D3D11_1_CREATE_DEVICE_CONTEXT_STATE_SINGLETHREADED;
  }

  OOVR_FAILED_DX_LOG(mDevice1->CreateDeviceContextState(
    stateFlags,
    &featureLevel,
    1,
    D3D11_SDK_VERSION,
    __uuidof(ID3D11Device1),
    nullptr,
    &mDX11ContextState));
}

DX11HybridCompositor::~DX11HybridCompositor()
{
  if (mOVRSwapChain)
    ovr_DestroyTextureSwapChain(OVSS, mOVRSwapChain);
}

void DX11HybridCompositor::Invoke(vr::Texture_t const* texture)
{
  ovrTextureSwapChainDesc &desc = mOVRSwapChainDesc;

  ovrResult result = ovrSuccess;

  int currentIndex = 0;
  if (mOVRSwapChain != nullptr)
    OOVR_FAILED_OVR_LOG(ovr_GetTextureSwapChainCurrentIndex(OVSS, mOVRSwapChain, &currentIndex));

  auto iu = static_cast<IUnknown*>(texture->handle);

  CComQIPtr<ID3D11Texture2D> src = iu;
  OOVR_FALSE_ABORT(src != nullptr);

  D3D11_TEXTURE2D_DESC srcDesc;
  src->GetDesc(&srcDesc);

  bool usable = mOVRSwapChain == NULL ? false : CheckChainCompatible(srcDesc, desc, texture->eColorSpace);
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

    mInputSrcSize.w = srcDesc.Width;
    mInputSrcSize.h = srcDesc.Height;

    desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
    desc.BindFlags = ovrTextureBind_None; // ovrTextureBind_DX_RenderTarget;

    OOVR_FAILED_OVR_ABORT(ovr_CreateTextureSwapChainDX(OVSS, mDevice, &desc, &mOVRSwapChain));
  }

  CComPtr<ID3D11Texture2D> tex;
  OOVR_FAILED_OVR_ABORT(ovr_GetTextureSwapChainBufferDX(OVSS, mOVRSwapChain, currentIndex, IID_PPV_ARGS(&tex)));

  mContext->CopyResource(tex, src);
}

void DX11HybridCompositor::Invoke(
  ovrEyeType eye, 
  vr::Texture_t const* texture, 
  const vr::VRTextureBounds_t* ptrBounds,
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

unsigned int DX11HybridCompositor::GetFlags()
{
  return mSubmitVerticallyFlipped ? ovrLayerFlag_TextureOriginAtBottomLeft : 0;
}

void DX11HybridCompositor::SetSupportedContext()
{
  mContext1->SwapDeviceContextState(mDX11ContextState, &mOriginalContextState);
}

void DX11HybridCompositor::ResetSupportedContext()
{
  mContext1->SwapDeviceContextState(mOriginalContextState, nullptr);
  mOriginalContextState.Release();
}

bool DX11HybridCompositor::CheckChainCompatible(
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

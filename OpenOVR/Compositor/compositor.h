#pragma once

typedef unsigned int GLuint;

class Compositor
{
public:
  virtual ~Compositor();

  // Only copy a texture - this can be used for overlays and such
  virtual void Invoke(vr::Texture_t const* texture) = 0;

  virtual void Invoke(
    ovrEyeType eye,
    vr::Texture_t const* texture,
    vr::VRTextureBounds_t const* bounds,
    vr::EVRSubmitFlags submitFlags,
    ovrLayerEyeFov &layer) = 0;

  virtual ovrTextureSwapChain GetOVRSwapChain() { return mOVRSwapChain; };

  virtual unsigned int GetFlags() { return 0; }

  virtual OVR::Sizei GetInputSrcSize() { return mInputSrcSize; };

  virtual void SetSupportedContext() {};
  virtual void ResetSupportedContext() {};

protected:
  ovrTextureSwapChain mOVRSwapChain;
  OVR::Sizei mInputSrcSize;
  ovrTextureSwapChainDesc mOVRSwapChainDesc;
};

class DX11Compositor : public Compositor
{
public:
  DX11Compositor(ID3D11Texture2D* texture);

  virtual ~DX11Compositor() override;

  // Override
  virtual void Invoke(vr::Texture_t const* texture) override;

  virtual void Invoke(
    ovrEyeType eye,
    vr::Texture_t const* texture,
    vr::VRTextureBounds_t const* bounds,
    vr::EVRSubmitFlags submitFlags,
    ovrLayerEyeFov &layer) override;

  unsigned int GetFlags() override;

protected:
  bool CheckChainCompatible(
    D3D11_TEXTURE2D_DESC& inputDesc,
    ovrTextureSwapChainDesc& OVRChainDesc,
    vr::EColorSpace colourSpace);

protected:
  CComPtr<ID3D11Device> mDevice;
  CComPtr<ID3D11DeviceContext> mContext;

private:
  bool mSubmitVerticallyFlipped;
};

class DX11CubemapCompositor : public DX11Compositor
{
public:
  DX11CubemapCompositor(ID3D11Texture2D* firstTexture);

  virtual ~DX11CubemapCompositor() override;

  // Override
  virtual void Invoke(vr::Texture_t const* firstTexture) override;

  virtual void Invoke(
    ovrEyeType eye,
    vr::Texture_t const* texture,
    vr::VRTextureBounds_t const* bounds,
    vr::EVRSubmitFlags submitFlags,
    ovrLayerEyeFov &layer) override
  {}

private:
  void RenderSourceToCubemapChain(
    ID3D11Texture2D* faceSrc,
    D3D11_TEXTURE2D_DESC const& faceSrcDesc,
    ID3D11Texture2D* hmdTexture,
    int subResourceDestIndex);

private:
  CComPtr<ID3D11VertexShader> mVertexShader;
  CComPtr<ID3D11PixelShader> mPixelShader;
  CComPtr<ID3D11InputLayout> mShaderInputLayout;
  CComPtr<ID3D11Buffer> mQuadVertexBuffer;
  CComPtr<ID3D11SamplerState> mSampler;
  CComPtr<ID3DDeviceContextState> mCubemapTextureContextState;
  CComQIPtr<ID3D11Device1> mDevice1;
  CComQIPtr<ID3D11DeviceContext1> mContext1;
};

class DX11HybridCompositor : public Compositor
{
public:
  DX11HybridCompositor(ID3D10Texture2D* texture);

  virtual ~DX11HybridCompositor() override;

  // Override
  virtual void Invoke(vr::Texture_t const* texture) override;

  virtual void Invoke(
    ovrEyeType eye,
    vr::Texture_t const* texture,
    vr::VRTextureBounds_t const* bounds,
    vr::EVRSubmitFlags submitFlags,
    ovrLayerEyeFov &layer) override;

  unsigned int GetFlags() override;

  virtual void SetSupportedContext() override;
  virtual void ResetSupportedContext() override;

private:
  bool CheckChainCompatible(
    D3D11_TEXTURE2D_DESC& inputDesc,
    ovrTextureSwapChainDesc& OVRchainDesc,
    vr::EColorSpace colourSpace);

  // TODO: make sure all of below are indeed needed.
  CComPtr<ID3D11Device> mDevice;
  CComQIPtr<ID3D11Device1> mDevice1;
  CComQIPtr<ID3D11DeviceContext> mContext;
  CComQIPtr<ID3D11DeviceContext1> mContext1;
  CComPtr<ID3DDeviceContextState> mDX11ContextState;
  CComPtr<ID3DDeviceContextState> mOriginalContextState;

  bool mSubmitVerticallyFlipped;
};


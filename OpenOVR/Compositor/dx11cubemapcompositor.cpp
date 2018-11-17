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

struct TEXPOINT { FLOAT X, Y, Z; DirectX::XMFLOAT2 Color; };
struct CROP { FLOAT uMin, uMax, vMin, vMax; };

DX11CubemapCompositor::DX11CubemapCompositor(ID3D11Texture2D* initial) : DX11Compositor(initial)
{
  // Basic shader that simply renders texture on our custom geometry (quad) which allows flipping it to bottom-left origin.
  // Bottom-left is not supported by Oculus' ovrLayerCube.
  static char SHADER_STRING[] = R"HLSL(
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VShader( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = input.Pos;
    output.Tex = input.Tex;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PShader( PS_INPUT input) : SV_Target
{
	// To flip if needed down the road:
	// float2 uv = float2(input.Tex.x, 1.0f - input.Tex.y);
	// return txDiffuse.Sample(samLinear, uv);

	return txDiffuse.Sample(samLinear, input.Tex);
}

)HLSL";

      // load and compile the two shaders
  #if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
      UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
  #else
      UINT compileFlags = 0;
  #endif

      // Create Vertex and Pixel shaders:
      CComPtr<ID3DBlob> VS;
      CComPtr<ID3DBlob> PS;
      CComPtr<ID3DBlob> error;
      OOVR_FAILED_DX_ABORT(D3DCompile(SHADER_STRING, _countof(SHADER_STRING), "shader-src-embedded", nullptr, nullptr,
        "VShader", "vs_5_0", compileFlags, 0, &VS, &error));
      OOVR_FAILED_DX_ABORT(D3DCompile(SHADER_STRING, _countof(SHADER_STRING), "shader-src-embedded", nullptr, nullptr,
        "PShader", "ps_5_0", compileFlags, 0, &PS, &error));

      // Encapsulate both shaders into shader objects:
      OOVR_FAILED_DX_ABORT(mDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), nullptr, &mVertexShader));
      OOVR_FAILED_DX_ABORT(mDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), nullptr, &mPixelShader));

      // create the input layout object
      D3D11_INPUT_ELEMENT_DESC ied[] =
      {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      };

      OOVR_FAILED_DX_ABORT(mDevice->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &mShaderInputLayout));

      // Create the vertex buffer:
      D3D11_BUFFER_DESC bd{};
      bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
      bd.ByteWidth = sizeof(TEXPOINT) * 6;           // size is the VERTEX struct * 3
      bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
      bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer // TODO set up D3D11_USAGE_IMMUTABLE

      OOVR_FAILED_DX_ABORT(mDevice->CreateBuffer(&bd, NULL, &mQuadVertexBuffer));

      // Note: if we ever need to support multiple skybox texture orienations, it is just a matter
      // of adding new variety of vertices for the underlying quad.
      constexpr TEXPOINT OurVertices[] =
      {
        // Left-top triangle
        { -1.0f,  1.0f, 0.0f, DirectX::XMFLOAT2(0.0f, 1.0f) }, // top left
        {  1.0f, -1.0f, 0.0f, DirectX::XMFLOAT2(1.0f, 0.0f) }, // bottom right
        { -1.0f, -1.0f, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f) }, // bottom left

        // Lower-right triangle
        { -1.0f,  1.0f, 0.0f, DirectX::XMFLOAT2(0.0f, 1.0f) }, // top left
        {  1.0f,  1.0f, 0.0f, DirectX::XMFLOAT2(1.0f, 1.0f) }, // top right
        {  1.0f, -1.0f, 0.0f, DirectX::XMFLOAT2(1.0f, 0.0f) }, // bottom right
      };

      // Copy the vertices into the buffer:
      D3D11_MAPPED_SUBRESOURCE ms;
      OOVR_FAILED_DX_ABORT(mContext->Map(mQuadVertexBuffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &ms));
      memcpy(ms.pData, OurVertices, sizeof(OurVertices));
      mContext->Unmap(mQuadVertexBuffer, 0u);

      // Set up the sampler:
      D3D11_SAMPLER_DESC desc{};
      desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
      desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
      desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
      desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
      OOVR_FAILED_DX_ABORT(mDevice->CreateSamplerState(&desc, &mSampler));

      // Create our device context so that we can isolate callers from our rendering.
      UINT deviceFlags = mDevice->GetCreationFlags();
      D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

      // If the device is single threaded, the context state must be too.
      UINT stateFlags = 0u;
      if (deviceFlags & D3D11_CREATE_DEVICE_SINGLETHREADED)
        stateFlags |= D3D11_1_CREATE_DEVICE_CONTEXT_STATE_SINGLETHREADED;

      mDevice1 = mDevice;
      OOVR_FALSE_ABORT(mDevice1 != nullptr);

      mContext1 = mContext;
      OOVR_FALSE_ABORT(mContext1 != nullptr);

      OOVR_FAILED_DX_ABORT(mDevice1->CreateDeviceContextState(
        stateFlags,
        &featureLevel,
        1,
        D3D11_SDK_VERSION,
        __uuidof(ID3D11Device1),
        nullptr,
        &mCubemapTextureContextState));
}

DX11CubemapCompositor::~DX11CubemapCompositor()
{
  if (mOVRSwapChain != nullptr)
    ovr_DestroyTextureSwapChain(OVSS, mOVRSwapChain);
}

void DX11CubemapCompositor::Invoke(vr::Texture_t const* texture)
{
  auto& desc = mOVRSwapChainDesc;

  int currentIndex = 0;
  if (mOVRSwapChain != nullptr)
    ovr_GetTextureSwapChainCurrentIndex(OVSS, mOVRSwapChain, &currentIndex);

  auto* src = static_cast<ID3D11Texture2D*>(texture->handle);

  D3D11_TEXTURE2D_DESC srcDesc;
  src->GetDesc(&srcDesc);

  // In cubemap case, it is array.
  const auto* textures = texture;

  // LibOVR does not seem to like Height != Width.  Using min here produces results comparable to SVR.
  srcDesc.Height = srcDesc.Width = std::min(srcDesc.Height, srcDesc.Width);

  bool usable = mOVRSwapChain == nullptr ? false : CheckChainCompatible(srcDesc, desc, texture->eColorSpace);
  if (!usable) {
    OOVR_LOG("Generating new cubemap swap chain");

    // First, delete the old chain if necessary
    if (mOVRSwapChain != nullptr)
      ovr_DestroyTextureSwapChain(OVSS, mOVRSwapChain);

    // Make eye render buffer
    desc = {};
    desc.Type = ovrTexture_Cube;
    desc.ArraySize = 6;

    // Looks like width has to match height otherwise texture swap chain creation fails.
    desc.Width = desc.Height = std::min(srcDesc.Height, srcDesc.Width);
    desc.Format = dxgiToOvrFormat(srcDesc.Format, textures->eColorSpace);
    desc.MipLevels = srcDesc.MipLevels;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;

    desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
    desc.BindFlags = ovrTextureBind_DX_RenderTarget;  // We will be rendering directly into chain.

    mInputSrcSize.w = srcDesc.Width;
    mInputSrcSize.h = srcDesc.Height;

    OOVR_FAILED_OVR_ABORT(ovr_CreateTextureSwapChainDX(OVSS, mDevice, &desc, &mOVRSwapChain));
  }

  CComPtr<ID3D11Texture2D> tex;
  ovr_GetTextureSwapChainBufferDX(OVSS, mOVRSwapChain, currentIndex, IID_PPV_ARGS(&tex));

  constexpr int svrToOvrCubemapIndexMap[] = {
    5,  // Front
    4,  // Back
    0,  // Left
    1,  // Right
    2,  // Top
    3   // Bottom  NOTE: Bottom appears upside down still, but I am too tired to fix it and let it slide for now.
  };

  // Produce the final image:
  // Save device state.
  CComPtr<ID3DDeviceContextState> originalContextState;
  mContext1->SwapDeviceContextState(mCubemapTextureContextState, &originalContextState);
  auto revertToCallerContext = MakeScopeGuard([&]() {
    mContext1->SwapDeviceContextState(originalContextState, nullptr);
  });

  D3D11_VIEWPORT vp{};
  vp.Width = static_cast<FLOAT>(desc.Width);
  vp.Height = static_cast<FLOAT>(desc.Height);
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 0.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  mContext->RSSetViewports(1, &vp);

  // Set the shader objects
  mContext->IASetInputLayout(mShaderInputLayout);
  mContext->VSSetShader(mVertexShader, 0, 0);
  mContext->PSSetShader(mPixelShader, 0, 0);

  ID3D11SamplerState* samplerRaw = mSampler;
  mContext->PSSetSamplers(0, 1, &samplerRaw);

  for (int face = 0; face < 6; ++face) {
    ID3D11Texture2D* faceSrc = static_cast<ID3D11Texture2D*>(textures->handle);
    ++textures;
    RenderSourceToCubemapChain(faceSrc, srcDesc, tex, svrToOvrCubemapIndexMap[face]);
  }
}

void DX11CubemapCompositor::RenderSourceToCubemapChain(
  ID3D11Texture2D* faceSrc, 
  D3D11_TEXTURE2D_DESC const& faceSrcDesc,
  ID3D11Texture2D* hmdTexture,
  int subResourceDestIndex)
{
  D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
  renderTargetViewDesc.Format = faceSrcDesc.Format;
  renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
  renderTargetViewDesc.Texture2DArray.MipSlice = 0;
  renderTargetViewDesc.Texture2DArray.FirstArraySlice = subResourceDestIndex;
  renderTargetViewDesc.Texture2DArray.ArraySize = 1;

  CComPtr<ID3D11RenderTargetView> renderTargetView;
  OOVR_FAILED_DX_ABORT(mDevice->CreateRenderTargetView(hmdTexture, &renderTargetViewDesc, &renderTargetView));

  // Set the render target to be the render to texture.
  ID3D11RenderTargetView* renderTargetViewRaw = renderTargetView;
  mContext->OMSetRenderTargets(1, &renderTargetViewRaw, nullptr);

  // Create Source Shader Resource view for the input texture:
  CComPtr<ID3D11ShaderResourceView> srv;
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
  srvDesc.Format = faceSrcDesc.Format;
  srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = -1;
  OOVR_FAILED_DX_ABORT(mDevice->CreateShaderResourceView(faceSrc, &srvDesc, &srv));

  ID3D11ShaderResourceView* srvRaw = srv;
  mContext->PSSetShaderResources(0, 1, &srvRaw);

  // Clear the back buffer to a deep blue.
  constexpr FLOAT col[] = {0.0f, 0.2f, 0.4f, 1.0f};
  mContext->ClearRenderTargetView(renderTargetViewRaw, col);

  // Select which vertex buffer to display.
  UINT stride = sizeof(TEXPOINT);
  UINT offset = 0;
  ID3D11Buffer* quadVertexBufferRaw = mQuadVertexBuffer;
  mContext->IASetVertexBuffers(0, 1, &quadVertexBufferRaw, &stride, &offset);

  // Select which primtive type we are using
  mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  mContext->Draw(6, 0);
}
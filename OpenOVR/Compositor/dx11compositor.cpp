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

static void XTrace(LPCSTR lpszFormat, ...) {
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	char szBuffer[512]; // get rid of this hard-coded buffer
	nBuf = _vsnprintf_s(szBuffer, 511, lpszFormat, args);
	OutputDebugStringA(szBuffer);
	OOVR_LOG(szBuffer);
	va_end(args);
}

ovrTextureFormat dxgiToOvrFormat(DXGI_FORMAT dxgi, vr::EColorSpace colourSpace) {
	// TODO is this really how it should work?
	// No idea why or how or what, but for now just force SRGB on as otherwise
	// it causes trouble.
	bool useSrgb = true; // colourSpace != vr::ColorSpace_Auto;

	switch (dxgi) {
#define MAPPING(name) \
			case DXGI_ ## name: \
				return OVR_ ## name;

#define C_MAPPING(name) \
			case DXGI_ ## name: \
			case DXGI_ ## name ## _SRGB: \
				return useSrgb ? OVR_ ## name ## _SRGB : OVR_ ## name;

		MAPPING(FORMAT_B5G6R5_UNORM);
		MAPPING(FORMAT_B5G5R5A1_UNORM);
		MAPPING(FORMAT_B4G4R4A4_UNORM);
		C_MAPPING(FORMAT_R8G8B8A8_UNORM);
		C_MAPPING(FORMAT_B8G8R8A8_UNORM);
		C_MAPPING(FORMAT_B8G8R8X8_UNORM);
		MAPPING(FORMAT_R16G16B16A16_FLOAT);
		MAPPING(FORMAT_R11G11B10_FLOAT);

	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return useSrgb ? OVR_FORMAT_R8G8B8A8_UNORM_SRGB : OVR_FORMAT_R8G8B8A8_UNORM;

#undef C_MAPPING
#undef MAPPING
	}

	return OVR_FORMAT_UNKNOWN;
}

#define ERR(msg) { \
	std::string str = "Hit DX11-related error " + string(msg) + " at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
	OOVR_LOG(str.c_str()); \
	MessageBoxA(NULL, str.c_str(), "Errored func!", MB_OK); \
	/**((int*)NULL) = 0;*/\
	throw str; \
}

void DX11Compositor::ThrowIfFailed(HRESULT test) {
	if ((test) != S_OK) {
		HRESULT hr = S_OK;
		OOVR_FAILED_DX_LOG(device->GetDeviceRemovedReason());
		throw "ThrowIfFailed err";
	}
}

struct TEXPOINT { FLOAT X, Y, Z; DirectX::XMFLOAT2 Color; };
struct CROP { FLOAT uMin, uMax, vMin, vMax; };

DX11Compositor::DX11Compositor(ID3D11Texture2D *initial, bool skyboxMode) : cubemapMode(skyboxMode) {
	initial->GetDevice(&device);
	device->GetImmediateContext(&context);

	if (skyboxMode) {
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
		HRESULT hr = S_OK;
		CComPtr<ID3DBlob> VS;
		CComPtr<ID3DBlob> PS;
		CComPtr<ID3DBlob> error;
		OOVR_FAILED_DX_ABORT(D3DCompile(SHADER_STRING, _countof(SHADER_STRING), "shader-src-embedded", nullptr, nullptr,
			"VShader", "vs_5_0", compileFlags, 0, &VS, &error));
		OOVR_FAILED_DX_ABORT(D3DCompile(SHADER_STRING, _countof(SHADER_STRING), "shader-src-embedded", nullptr, nullptr,
			"PShader", "ps_5_0", compileFlags, 0, &PS, &error));

		// Encapsulate both shaders into shader objects:
		OOVR_FAILED_DX_ABORT(device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), nullptr, &vertexShader));
		OOVR_FAILED_DX_ABORT(device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), nullptr, &pixelShader));

		// create the input layout object
		D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		OOVR_FAILED_DX_ABORT(device->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &inputLayout));

		// Create the vertex buffer:
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
		bd.ByteWidth = sizeof(TEXPOINT) * 6;           // size is the VERTEX struct * 3
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer // TODO set up D3D11_USAGE_IMMUTABLE

		OOVR_FAILED_DX_ABORT(device->CreateBuffer(&bd, NULL, &quadVertexBuffer));

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
		OOVR_FAILED_DX_ABORT(context->Map(quadVertexBuffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &ms));
		memcpy(ms.pData, OurVertices, sizeof(OurVertices));
		context->Unmap(quadVertexBuffer, 0u);

		// Set up the sampler:
		D3D11_SAMPLER_DESC desc{};
		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		OOVR_FAILED_DX_ABORT(device->CreateSamplerState(&desc, &sampler));

		// Create our device context so that we can isolate callers from our rendering.
		UINT deviceFlags = device->GetCreationFlags();
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		// If the device is single threaded, the context state must be too.
		UINT stateFlags = 0u;
		if (deviceFlags & D3D11_CREATE_DEVICE_SINGLETHREADED)
			stateFlags |= D3D11_1_CREATE_DEVICE_CONTEXT_STATE_SINGLETHREADED;

		device1 = device;
		OOVR_FALSE_ABORT(device1 != nullptr);

		context1 = context;
		OOVR_FALSE_ABORT(context1 != nullptr);

		OOVR_FAILED_DX_ABORT(device1->CreateDeviceContextState(
			stateFlags,
			&featureLevel,
			1,
			D3D11_SDK_VERSION,
			__uuidof(ID3D11Device1),
			nullptr,
			&cubemapTextureContextState));
	}
}

DX11Compositor::~DX11Compositor() {
	if (chain)
		ovr_DestroyTextureSwapChain(OVSS, chain);
}

void DX11Compositor::Invoke(const vr::Texture_t * texture) {
	ovrTextureSwapChainDesc &desc = chainDesc;

	int currentIndex = 0;
	if (chain)
		ovr_GetTextureSwapChainCurrentIndex(OVSS, chain, &currentIndex);

	ID3D11Texture2D *src = (ID3D11Texture2D*)texture->handle;

	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);

	if (!cubemapMode) {
		bool usable = chain == nullptr ? false : CheckChainCompatible(srcDesc, desc, texture->eColorSpace);

		if (!usable) {
			OOVR_LOG("Generating new swap chain");

			// First, delete the old chain if necessary
			if (chain)
				ovr_DestroyTextureSwapChain(OVSS, chain);

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
			desc.BindFlags = ovrTextureBind_None; // ovrTextureBind_DX_RenderTarget;

			srcSize.w = srcDesc.Width;
			srcSize.h = srcDesc.Height;

			ovrResult result = ovr_CreateTextureSwapChainDX(OVSS, device, &desc, &chain);
			if (!OVR_SUCCESS(result))
				ERR("Cannot create DX texture swap chain " + to_string(result));
		}

		ID3D11Texture2D* tex = nullptr;
		ovr_GetTextureSwapChainBufferDX(OVSS, chain, currentIndex, IID_PPV_ARGS(&tex));
		context->CopyResource(tex, src);
	}
	else {  // Cubemap case
		HRESULT hr = S_OK;
		
		// In cubemap case, it is array.
		const auto* textures = texture;

		// LibOVR does not seem to like Height != Width.  Using min here produces results comparable to SVR.
		srcDesc.Height = srcDesc.Width = std::min(srcDesc.Height, srcDesc.Width);

		bool usable = chain == nullptr ? false : CheckChainCompatible(srcDesc, desc, texture->eColorSpace);

		if (!usable) {
			OOVR_LOG("Generating new swap chain");

			// First, delete the old chain if necessary
			if (chain != nullptr)
				ovr_DestroyTextureSwapChain(OVSS, chain);

			stagingTexture.Release();
			stagingRenderTargetView.Release();

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

			// VL: It would be great if we could avoid staging texture and render
			// directly into chain, but I dot undertand how to accomplish that.
			desc.BindFlags = ovrTextureBind_None; // ovrTextureBind_DX_RenderTarget;

			srcSize.w = srcDesc.Width;
			srcSize.h = srcDesc.Height;

			ovrResult result = ovr_CreateTextureSwapChainDX(OVSS, device, &desc, &chain);
			if (!OVR_SUCCESS(result))
				ERR("Cannot create DX texture swap chain " + to_string(result));

			// Create staging texture for image flipping:
			D3D11_TEXTURE2D_DESC targetDesc{};
			targetDesc.Width = desc.Width;
			targetDesc.Height = desc.Height;
			targetDesc.MipLevels = srcDesc.MipLevels;
			targetDesc.ArraySize = 1;
			targetDesc.Format = srcDesc.Format;
			targetDesc.SampleDesc.Count = srcDesc.SampleDesc.Count;
			targetDesc.SampleDesc.Quality = srcDesc.SampleDesc.Quality;
			targetDesc.Usage = D3D11_USAGE_DEFAULT;
			targetDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			targetDesc.CPUAccessFlags = 0;
			targetDesc.MiscFlags = 0;
			OOVR_FAILED_DX_ABORT(device->CreateTexture2D(&targetDesc, nullptr, &stagingTexture));

			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
			renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			OOVR_FAILED_DX_ABORT(device->CreateRenderTargetView(stagingTexture, &renderTargetViewDesc, &stagingRenderTargetView));
		}

		ID3D11Texture2D* tex = nullptr;
		ovr_GetTextureSwapChainBufferDX(OVSS, chain, currentIndex, IID_PPV_ARGS(&tex));

		constexpr int svrToOvrCubemapIndexMap[] = {
			5,  // Front
			4,  // Back
			0,  // Left
			1,  // Right
			2,  // Top
			3   // Bottom
		};

		// Produce the final image:
		// Save device state.
		CComPtr<ID3DDeviceContextState> originalContextState;
		context1->SwapDeviceContextState(cubemapTextureContextState, &originalContextState);
		auto revertToCallerContext = MakeScopeGuard([&]() {
			context1->SwapDeviceContextState(originalContextState, nullptr);
		});

		// Set the render target to be the render to texture.
		ID3D11RenderTargetView* stagingRenderTargetViewRaw = stagingRenderTargetView;
		context->OMSetRenderTargets(1, &stagingRenderTargetViewRaw, nullptr);

		D3D11_VIEWPORT vp{};
		vp.Width = static_cast<FLOAT>(desc.Width);
		vp.Height = static_cast<FLOAT>(desc.Height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 0.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		context->RSSetViewports(1, &vp);

		// Set the shader objects
		context->IASetInputLayout(inputLayout);
		context->VSSetShader(vertexShader, 0, 0);
		context->PSSetShader(pixelShader, 0, 0);

		ID3D11SamplerState* samplerRaw = sampler;
		context->PSSetSamplers(0, 1, &samplerRaw);

		for (int face = 0; face < 6; ++face) {
			ID3D11Texture2D* faceSrc = static_cast<ID3D11Texture2D*>(textures->handle);
			++textures;
			RenderSourceToCubemapChain(faceSrc, srcDesc, tex, svrToOvrCubemapIndexMap[face]);
		}
	}
}

void DX11Compositor::RenderSourceToCubemapChain(ID3D11Texture2D* faceSrc, const D3D11_TEXTURE2D_DESC& faceSrcDesc, ID3D11Texture2D* hmdTexture, int subResourceDestIndex)
{
	HRESULT hr = S_OK;

	// Create Source Shader Resource view for the input texture:
	CComPtr<ID3D11ShaderResourceView> srv;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = faceSrcDesc.Format;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	OOVR_FAILED_DX_ABORT(device->CreateShaderResourceView(faceSrc, &srvDesc, &srv));

	ID3D11ShaderResourceView* srvRaw = srv;
	context->PSSetShaderResources(0, 1, &srvRaw);

	// Clear the back buffer to a deep blue.
	constexpr FLOAT col[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	context->ClearRenderTargetView(stagingRenderTargetView, col);

	// Select which vertex buffer to display.
	UINT stride = sizeof(TEXPOINT);
	UINT offset = 0;
	ID3D11Buffer* quadVertexBufferRaw = quadVertexBuffer;
	context->IASetVertexBuffers(0, 1, &quadVertexBufferRaw, &stride, &offset);

	// Select which primtive type we are using
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->Draw(6, 0);

	context->CopySubresourceRegion(hmdTexture, subResourceDestIndex, 0, 0, 0, stagingTexture, 0, nullptr);
}

void DX11Compositor::Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * ptrBounds,
	vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) {

	// Copy the texture across
	Invoke(texture);

	// Set the viewport up
	ovrRecti &viewport = layer.Viewport[eye];
	if (ptrBounds) {
		vr::VRTextureBounds_t bounds = *ptrBounds;

		if (bounds.vMin > bounds.vMax) {
			submitVerticallyFlipped = true;
			float newMax = bounds.vMin;
			bounds.vMin = bounds.vMax;
			bounds.vMax = newMax;
		}
		else {
			submitVerticallyFlipped = false;
		}

		viewport.Pos.x = (int)(bounds.uMin * chainDesc.Width);
		viewport.Pos.y = (int)(bounds.vMin * chainDesc.Height);
		viewport.Size.w = (int)((bounds.uMax - bounds.uMin) * chainDesc.Width);
		viewport.Size.h = (int)((bounds.vMax - bounds.vMin) * chainDesc.Height);
	}
	else {
		viewport.Pos.x = viewport.Pos.y = 0;
		viewport.Size.w = chainDesc.Width;
		viewport.Size.h = chainDesc.Height;

		submitVerticallyFlipped = false;
	}
}

unsigned int DX11Compositor::GetFlags() {
	return submitVerticallyFlipped ? ovrLayerFlag_TextureOriginAtBottomLeft : 0;
}

bool DX11Compositor::CheckChainCompatible(D3D11_TEXTURE2D_DESC & inputDesc, ovrTextureSwapChainDesc &chainDsc, vr::EColorSpace colourSpace) {
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
	if(chainDesc.Format != dxgiToOvrFormat(inputDesc.Format, colourSpace)) FAIL(Format);
	//CHECK_ADV(SampleDesc.Count, SampleCount);
	//CHECK_ADV(SampleDesc.Quality);
#undef CHECK
#undef CHECK_ADV
#undef FAIL

	return usable;
}

#include "DrvOculusCommon.h"
#include "OculusBackend.h"

#include "OVR_CAPI.h"
#include "OVR_CAPI_Vk.h"
#include "../OpenOVR/libovr_wrapper.h"

void OculusBackend::GetDXGIOutputInfo(int32_t * adapterIndex) {
#ifdef SUPPORT_DX
#define VALIDATE(x, msg) if (!(x)) { MessageBoxA(nullptr, (msg), "CVRSystem", MB_ICONERROR | MB_OK); exit(-1); }

	LUID* luid = reinterpret_cast<LUID*>(ovr::luid);

	//IDXGIFactory * DXGIFactory = nullptr;
	//HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&DXGIFactory));
	IDXGIFactory* DXGIFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&DXGIFactory));
	VALIDATE((hr == ERROR_SUCCESS), "CreateDXGIFactory1 failed");

	bool match = false;
	IDXGIAdapter * Adapter = nullptr;
	for (UINT i = 0; DXGIFactory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC adapterDesc;
		Adapter->GetDesc(&adapterDesc);

		match = luid == nullptr || memcmp(&adapterDesc.AdapterLuid, luid, sizeof(LUID)) == 0;

		Adapter->Release();

		if (match) {
			*adapterIndex = i;
			ovr::dxDeviceId = i;
			break;
		}
	}

	DXGIFactory->Release();

	if (!match)
		OOVR_ABORT("Cannot find graphics card!");

#undef VALIDATE
#else
	OOVR_ABORT("DX not supported - build with SUPPORT_DX defined");
#endif
}

void OculusBackend::GetOutputDevice(uint64_t * pnDevice, vr::ETextureType textureType, VkInstance_T * pInstance) {
	switch (textureType) {
	case vr::TextureType_Vulkan: {
#if defined(SUPPORT_VK)
		ovrResult res = ovr_GetSessionPhysicalDeviceVk(*ovr::session, *ovr::luid, pInstance, (VkPhysicalDevice*)pnDevice);
		if (res) {
			OOVR_ABORT("Cannot get Vulkan session physical device!");
		}
#else
		OOVR_ABORT("OpenComposite was compiled with Vulkan support disabled, and app attempted to use Vulkan!");
#endif
	}
	default:
		OOVR_LOGF("Unsupported texture type for GetOutputDevice %d", textureType);
	}

}

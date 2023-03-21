// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "plat.h"

#include "Reimpl/Interfaces.h"

// Needed for the system-wide usage of this DLL (when renamed to vrclient[_x64].dll)
#include "generated/GVRClientCore.gen.h"

#include "Misc/Config.h"
#include "Misc/debug_helper.h"
#include "steamvr_abi.h"
#include <map>
#include <memory>

// Specific to OCOVR
#include "Drivers/Backend.h"
#include "Drivers/DriverManager.h"
#include "DrvOpenXR.h"
#include "Misc/AudioOverride.h"

HMODULE openovr_module_id;
HMODULE chainedImplementation;

static void init_audio();

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		openovr_module_id = hModule;
#if defined(_DEBUG)
		DbgSetModule(hModule);
#endif
		init_audio();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (chainedImplementation) {
			FreeLibrary(chainedImplementation);
		}
		break;
	}
	return TRUE;
}

// Returns the last Win32 error, in string format. Returns an empty string if there is no error.
static string GetLastErrorAsString()
{
	// Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return "<no error>"; // No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	string message(messageBuffer, size);

	// Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

alternativeCoreFactory_t PlatformGetAlternativeCoreFactory()
{
	string path = BaseClientCore::GetAlternativeRuntimePath();

	string dll = path + "\\bin\\vrclient";
#if defined(_WIN64)
	dll += "_x64";
#endif
	dll += ".dll";

	// Make SteamVR load the rest of it's stuff from the correct place
	bool success = SetEnvironmentVariableA("VR_OVERRIDE", path.c_str());
	if (!success) {
		string msg = "Failed to update SteamVR environment variable with error:\n" + GetLastErrorAsString();
		OOVR_ABORT(msg.c_str());
	}

	chainedImplementation = LoadLibraryA(dll.c_str());
	if (!chainedImplementation) {
		string msg = "Failed to load SteamVR DLL '" + dll + "' with error:\n" + GetLastErrorAsString();
		OOVR_ABORT(msg.c_str());
	}

	return (alternativeCoreFactory_t)GetProcAddress(chainedImplementation, "VRClientCoreFactory");
}

// Hook the audio thing as soon as possible. This will use the Rift device
//  as listed in the Windows audio settings.
void init_audio()
{
	if (!oovr_global_configuration.EnableAudioSwitch())
		return;

	OOVR_LOGF("Attempting to switch Audio.");

	std::wstring dev;
	HRESULT hr = find_output_device(dev, oovr_global_configuration.AudioDeviceName());

	if (SUCCEEDED(hr)) {
		OOVR_LOGF("Succeeded in getting audio device output: %s.  Setting app default audio output to device.", dev);
		set_app_default_audio_device(dev);
	} else {
		OOVR_LOGF("Failed to get audio device output");
	}		
}

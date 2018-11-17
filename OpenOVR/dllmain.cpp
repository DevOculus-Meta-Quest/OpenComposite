// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "Reimpl/Interfaces.h"

// Needed for the system-wide usage of this DLL (when renamed to vrclient[_x64].dll)
#include "Reimpl/GVRClientCore.gen.h"

#include "Misc/Config.h"
#include "Misc/ScopeGuard.h"
#include "Misc/audio_override.h"
#include "Misc/debug_helper.h"
#include "libovr_wrapper.h"
#include "steamvr_abi.h"
#include <map>
#include <memory>

#include "OVR_CAPI_Audio.h"

using namespace std;

HMODULE openovr_module_id;

#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <memory>
#include <stdexcept>

// could be made an option too.
// #define OOVR_REDIRECT_CONSOLE
#ifdef OOVR_REDIRECT_CONSOLE

#pragma region console_redirect_credits

//////////////////////////////////////////////////////////////
//                      -Creators- //
//   _________                       __ //
//  /   _____/_____ _____  ________/  |_ ____    ____ //
//  \_____  \ \____ \\__  \ \_  __ \ __\_/ __ \  /    \ //
//  /meerkat \|  |_> >/ __ \_|  | \/|  | \  ___/ |   |  \ //
// /_______  /|   __/(____  /|__|   |__| \___  >|___|  / //
//         \/ |__|        \/ \/      \/ //
//                            ////////////////////////////////
//    _____             .___  //
//   /  _  \   ____   __| _/  //
//  /  /_\  \ /    \ / __ |   //
// /    |    \   |  | /_/ |   //
// \____|__  /___|  |____ |   //
//         \/     \/     \/   //
//                            /////////////////
//   ________        .__   .__   .__ //
//  /  XPL0_/  ____  |  |  |  |  |__| ____ //
// /   \  ____/ __ \ |  |  |  |  |  |/ \  //
// \    \_\  \  ___/ |  |__|  |__|  | | \ //
//  \______  /\___ >|____/|____/|__|___| / //
//         \/     \/ \/  //
///////////////////////////////////////////////

#pragma endregion

class console_window
{
public:
  console_window(const console_window&) = delete;
  console_window()
  {

    if (AllocConsole() == FALSE)
      throw std::runtime_error("failed to AllocConsole");

    _stdHandle.reset(GetStdHandle(STD_OUTPUT_HANDLE), CloseHandle);

    if (!_stdHandle.get())
      throw std::runtime_error("failed to GetStdHandle");

    _conHandle.reset(new int(_open_osfhandle(PtrToUlong(_stdHandle.get()), _O_TEXT)));

    if (!_conHandle.get())
      throw std::runtime_error("failed to GetStdHandle");

    _fp.reset(_fdopen(*_conHandle.get(), "w"), fclose);

    if (!_fp.get())
      throw std::runtime_error("failed to _fdopen");

    _orgOut = *stdout;
    *stdout = *_fp;

    if (setvbuf(stdout, NULL, _IONBF, 0) != NULL)
      throw std::runtime_error("failed to setvbuf");
  }

  ~console_window()
  {
    if (_fp.get()) {
      *stdout = _orgOut;
      setvbuf(stdout, NULL, _IONBF, 0);
      FreeConsole();
    }
  }
  void __cdecl add_log(const char* fmt, ...)
  {
    if (!fmt || !strlen(fmt)) {
      return;
    }
    va_list va_alist;
    char logbuf[20000] = { 0 };
    va_start(va_alist, fmt);
    _vsnprintf_s(logbuf + strlen(logbuf), sizeof(logbuf) - strlen(logbuf), _TRUNCATE, fmt, va_alist);
    va_end(va_alist);
    printf(logbuf);
  }

protected:
private:
  std::shared_ptr<void> _stdHandle;
  std::shared_ptr<int> _conHandle;
  std::shared_ptr<FILE> _fp;
  FILE _orgOut;
  // HANDLE m_lStdHandle;
};

console_window c;
#endif // OOVR_REDIRECT_CONSOLE

void TraceStatrtupMsg()
{
  OOVR_LOGF("Wolf's Open Composite "
            "(TIWM fork) version: %s\n",
            OOVR_VERSION_STRING);
  OOVR_LOGF("Requested configuration:");
  OOVR_LOGF("supersampleRatio=%f", oovr_global_configuration.SupersampleRatio());
  OOVR_LOGF("dx10Mode=%s", oovr_global_configuration.DX10Mode() ? "true" : "false");
  OOVR_LOGF("enableLayers=%s", oovr_global_configuration.DX10Mode() ? "true" : "false");
  OOVR_LOGF("enableCubemap=%s", oovr_global_configuration.EnableCubemap() ? "true" : "false");
  OOVR_LOGF("threePartSubmit=%s", oovr_global_configuration.ThreePartSubmit() ? "true" : "false");
  OOVR_LOGF("useViewportStencil=%s", oovr_global_configuration.UseViewportStencil() ? "true" : "false");
  OOVR_LOGF("postInitCmd=\"%s\"\n", oovr_global_configuration.PostInitCmd().c_str());
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    openovr_module_id = hModule;
    TraceStatrtupMsg();
#if defined(_DEBUG)
    DbgSetModule(hModule);
#endif
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

// Binary-compatible openvr_api.dll implementation
static bool running;
static uint32_t current_init_token = 1;

void ERR(string msg)
{
  char buff[4096];
  snprintf(buff, sizeof(buff), "OpenComposite DLLMain ERROR: %s", msg.c_str());
  OOVR_ABORT_T(buff, "OpenComposite Error");
}

class _InheritCVRLayout
{
  virtual void _ignore() = 0;
};
class CVRCorrectLayout : public _InheritCVRLayout, public CVRCommon
{
};

static map<string, unique_ptr<CVRCorrectLayout>> interfaces;

VR_INTERFACE void* VR_CALLTYPE VR_GetGenericInterface(const char* interfaceVersion, EVRInitError* error)
{
  if (!running) {
    OOVR_LOGF("[INFO] VR_GetGenericInterface "
              "called while OOVR not "
              "running, setting "
              "error=NotInitialized, for "
              "interfaceVersion=%s",
              interfaceVersion);
    *error = VRInitError_Init_NotInitialized;
    return NULL;
  }

  // Unless we later change this otherwise, it was successful.
  *error = VRInitError_None;

  // First check if they're getting the 'FnTable' version of this interface.
  // This is a table of methods, but critically they *don't* take a 'this' pointer,
  // so we can't cheat and return the vtable.
  const char* fnTableStr = "FnTable:";
  if (!strncmp(fnTableStr, interfaceVersion, strlen(fnTableStr))) {
    const char* baseInterface = interfaceVersion + strlen(fnTableStr);

    // Get the C++ interface
    // Note we can't directly cast to CVRCommon, as we'll then be referring to the OpenVR interface
    // vtable - look up how vtables work with multiple inheritance if you're confused about this.
    CVRCorrectLayout* interfaceClass = (CVRCorrectLayout*)VR_GetGenericInterface(baseInterface, error);

    // If the interface is NULL, then error will have been set and we can return null too.
    if (!interfaceClass) {
      return NULL;
    }

    return interfaceClass->_GetStatFuncList();
  }

  if (interfaces.count(interfaceVersion)) {
    return interfaces[interfaceVersion].get();
  }

  CVRCorrectLayout* impl = (CVRCorrectLayout*)CreateInterfaceByName(interfaceVersion);
  if (impl) {
    unique_ptr<CVRCorrectLayout> ptr(impl);
    interfaces[interfaceVersion] = move(ptr);
    return impl;
  }

  OOVR_LOG(interfaceVersion);
  MessageBoxA(NULL, interfaceVersion, "Missing interface", MB_OK);
  ERR("unknown/unsupported interface " + string(interfaceVersion));
#undef INTERFACE
}

VR_INTERFACE uint32_t VR_CALLTYPE VR_GetInitToken() { return current_init_token; }

VR_INTERFACE char* VR_GetStringForHmdError(int err)
{
  OOVR_ABORT("Stub");
  return nullptr;
}

VR_INTERFACE const char* VR_CALLTYPE VR_GetVRInitErrorAsEnglishDescription(EVRInitError error)
{
  switch (error) {
  case VRInitError_None:
    return "None";
  }
  OOVR_ABORT(("Init desc: Unknown value " + to_string(error)).c_str());
}

VR_INTERFACE const char* VR_CALLTYPE VR_GetVRInitErrorAsSymbol(EVRInitError error)
{
  OOVR_ABORT("Stub");
  return nullptr;
}

VR_INTERFACE uint32_t VR_CALLTYPE VR_InitInternal(EVRInitError* peError, EVRApplicationType eApplicationType)
{
  return VR_InitInternal2(peError, eApplicationType, NULL);
}

void RunPostInitCommands()
{
  if (oovr_global_configuration.PostInitCmd().empty())
    return;

  STARTUPINFOA si{};
  PROCESS_INFORMATION pi{};
  auto cleanupOnExit = MakeScopeGuard([&]() {
    // Close process and thread handles.
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
  });

  if (!CreateProcessA(nullptr, /*lpApplicationName*/
                      const_cast<char*>(oovr_global_configuration.PostInitCmd().c_str()),
                      nullptr, /*lpProcessAttributes*/
                      nullptr, /*lpThreadAttributes*/
                      FALSE, /*bInheritHandles*/
                      0u, /*dwCreationFlags*/
                      nullptr, /*lpEnvironment*/
                      nullptr, /*lpCurrentDirectory*/
                      &si,
                      &pi)) {
    OOVR_LOGF("CreateProces for command \"%s\" "
              "failed (%d).\n",
              oovr_global_configuration.PostInitCmd().c_str(),
              ::GetLastError());
    return;
  }
}

VR_INTERFACE uint32_t VR_CALLTYPE VR_InitInternal2(EVRInitError* peError,
                                                   EVRApplicationType eApplicationType,
                                                   const char* pStartupInfo)
{
  // TODO use peError

  if (eApplicationType != VRApplication_Scene)
    ERR("Cannot init VR: unsuported "
        "apptype "
        + to_string(eApplicationType));

  if (running)
    ERR("Cannot init VR: Already running!");

  ovr::Setup();
  running = true;

  RunPostInitCommands();

  *peError = VRInitError_None;

  OOVR_LOG("Runtime initialized successfully.\n");
  return current_init_token;
}

VR_INTERFACE bool VR_CALLTYPE VR_IsHmdPresent() { return ovr::IsAvailable(); }

VR_INTERFACE bool VR_CALLTYPE VR_IsInterfaceVersionValid(const char* pchInterfaceVersion)
{
  return true; // Kinda dodgy
}

VR_INTERFACE bool VR_CALLTYPE VR_IsRuntimeInstalled()
{
  // TODO in future check that the Oculus Runtime is installed
  return true;
}

VR_INTERFACE const char* VR_CALLTYPE VR_RuntimePath()
{
  OOVR_ABORT("Stub");
  return nullptr;
}

VR_INTERFACE void VR_CALLTYPE VR_ShutdownInternal()
{
  // Reset interfaces
  // Do this first, while the OVR session is still available in case they
  // need to use it for cleanup.
  interfaces.clear();

  // Shut down LibOVR
  ovr::Shutdown();
  running = false;

  OOVR_LOG("Shutting down.");
}

VR_INTERFACE void* VRClientCoreFactory(const char* pInterfaceName, int* pReturnCode)
{
  *pReturnCode = VRInitError_None;

  string name = pInterfaceName;

#define CLIENT_VER(ver)                                                                            \
  if (IVRClientCore_##ver::IVRClientCore_Version == name) {                                        \
    static CVRClientCore_##ver inst;                                                               \
    return &inst;                                                                                  \
  }

  CLIENT_VER(002);
  CLIENT_VER(003);

#undef CLIENT_VER

  OOVR_LOG(pInterfaceName);
  MessageBoxA(NULL, pInterfaceName, "Missing client interface", MB_OK);
  ERR("unknown/unsupported interface " + name);
}
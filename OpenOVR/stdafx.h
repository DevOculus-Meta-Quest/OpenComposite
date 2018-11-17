// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define OOVR_VERSION_STRING "0.6.0.0"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#define VR_API_EXPORT 1

#include <string>
#include <d3d11.h>
#include <d3d11_1.h>

#include <vector>
#include <memory>

#pragma warning(push)
#pragma warning(disable : 4838)   // int to UINT truncation.
#include <atlbase.h>
#pragma warning(pop)

#include <Extras/OVR_Math.h>
#include <OVR_CAPI_D3D.h>
#include <OpenVR/interfaces/IVRCompositor_022.h>
#include <Extras/OVR_Math.h>

#include "OpenVR/interfaces/vrtypes.h"
#include "OpenVR/interfaces/vrannotation.h"
#include "custom_types.h"
#include "logging.h"
#include "Compositor/compositor.h"
#include "convert.h"

// This module's ID, from DLLMain
extern HMODULE openovr_module_id;

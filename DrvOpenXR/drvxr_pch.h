#pragma once
// Precompiled header for DrvOpenXR stuff - this MUST be the first include!

#include "pub/DrvOpenXR.h"

#include "../OpenOVR/Misc/xrutil.h"
#include "../OpenOVR/Misc/xr_ext.h"
#include "../OpenOVR/logging.h"

#ifndef _WIN32
#include "../OpenOVR/linux_funcs.h"
#endif

#ifndef STUBBED
#define STUBBED() \
	OOVR_ABORTF("DrvOpenXR: Hit stubbed file at %s:%d func %s", __FILE__, __LINE__, __func__)
#endif

namespace DrvOpenXR {
void SetupSession();
void ShutdownSession();
void FullShutdown();
} // namespace DrvOpenXR

#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("ClientCore", "003", CUSTOM)
GEN_INTERFACE("ClientCore", "002", CUSTOM)

// Unlike all the other interfaces, this interface is independent of the OpenXR session, and doesn't vanish on shutdown.
BASE_FLAG(CUSTOM_LIFECYCLE)

#include "generated/GVRClientCore.gen.h"

vr::EVRInitError CVRClientCore_002::Init(vr::EVRApplicationType eApplicationType)
{
	// This is how the DLL does it with the old InitInternal
	return base->Init(eApplicationType, NULL);
}

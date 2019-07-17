#pragma once
#include "OVR_CAPI.h"
#include "Extras/OVR_Math.h"

namespace ovr {
	extern ovrSession *session;
	extern ovrGraphicsLuid *luid;
	extern ovrHmdDesc hmdDesc;

	extern ovrEyeRenderDesc eyeRenderDesc[2];
	extern ovrPosef      hmdToEyeViewPose[2];
	extern OVR::Matrix4f    hmdToEyeMatrix[2];

	extern int dxDeviceId;

	void Setup();
	bool IsAvailable();
	void Shutdown();
};

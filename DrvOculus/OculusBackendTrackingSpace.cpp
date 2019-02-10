#include "DrvOculusCommon.h"
#include "OculusBackend.h"

#include "OVR_CAPI.h"
#include "../OpenOVR/libovr_wrapper.h"
#include "../OpenOVR/convert.h"

void OculusBackend::SetTrackingSpace(vr::ETrackingUniverseOrigin origin) {
	waitGetPosesOrigin = origin;

	// Set the origin in LibOVR
	ovrTrackingOrigin oOrigin = ovrTrackingOrigin_FloorLevel;
	if (origin == vr::TrackingUniverseSeated) {
		oOrigin = ovrTrackingOrigin_EyeLevel;
	}
	// When in dual-origin mode, always use the floor level as a base
	if (usingDualOriginMode) {
		oOrigin = ovrTrackingOrigin_FloorLevel;
	}
	OOVR_FAILED_OVR_ABORT(ovr_SetTrackingOriginType(*ovr::session, oOrigin));
}

vr::ETrackingUniverseOrigin OculusBackend::GetTrackingSpace() {
	return waitGetPosesOrigin;
}

vr::HmdMatrix34_t OculusBackend::PoseToTrackingSpace(vr::ETrackingUniverseOrigin toOrigin, ovrPosef pose) {
	// Standard path, most games only use the origin they have selected
	if (toOrigin == waitGetPosesOrigin && !usingDualOriginMode) {
		goto result;
	}

	if (!usingDualOriginMode) {
		// TODO if usingDualOriginMode is false, then do the initial stuff
		// otherwise the head position will jump around the first time this is used

		// Enable the dual origin mode
		usingDualOriginMode = true;

		// Reset the origin, so LibOVR is now working relative to the floor
		SetTrackingSpace(waitGetPosesOrigin);

		// Use the current height as the zero for the seated height
		ResetFakeSeatedHeight();
	}

	if (toOrigin == vr::TrackingUniverseSeated) {
		pose.Position.y -= fakeOriginHeight;
	}

	result:
	OVR::Posef thePose(pose);
	OVR::Matrix4f hmdTransform(thePose);

	vr::HmdMatrix34_t result = {0};
	O2S_om34(hmdTransform, result);
	return result;
}

void OculusBackend::ResetFakeSeatedHeight() {
	ovrTrackingState state = ovr_GetTrackingState(*ovr::session, ovr_GetTimeInSeconds(), false);

	fakeOriginHeight = state.HeadPose.ThePose.Position.y;
}

void OculusBackend::ResetSeatedZeroPose() {
	// TODO should this only work when seated or whatever?
	ovr_RecenterTrackingOrigin(*ovr::session);

	if (usingDualOriginMode)
		ResetFakeSeatedHeight();
}

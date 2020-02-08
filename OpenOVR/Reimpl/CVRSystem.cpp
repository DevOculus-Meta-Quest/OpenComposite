#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("System", "012") // R3E
GEN_INTERFACE("System", "021") // rF2

#include "GVRSystem.gen.h"

bool CVRSystem_012::GetControllerState(vr::TrackedDeviceIndex_t unControllerDeviceIndex,
                                       vr::VRControllerState_t* pControllerState)
{
  return base->GetControllerState(unControllerDeviceIndex, pControllerState, sizeof(vr::VRControllerState001_t));
}

bool CVRSystem_012::GetControllerStateWithPose(vr::ETrackingUniverseOrigin eOrigin,
                                               vr::TrackedDeviceIndex_t unControllerDeviceIndex,
                                               vr::VRControllerState_t* pControllerState,
                                               vr::TrackedDevicePose_t* pTrackedDevicePose)
{
  return base->GetControllerStateWithPose(
  eOrigin, unControllerDeviceIndex, pControllerState, sizeof(vr::VRControllerState001_t), pTrackedDevicePose);
}

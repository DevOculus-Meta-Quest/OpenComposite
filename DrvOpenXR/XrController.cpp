#include "XrController.h"

// properties
bool XrController::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	switch (prop) {
	case vr::Prop_DeviceProvidesBatteryStatus_Bool:
		return true;
	default:
		return XrTrackedDevice::GetBoolTrackedDeviceProperty(prop, pErrorL);
	}
}

int32_t XrController::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	// These are for the old input system, which we don't initially need
	switch (prop) {
	case vr::Prop_Axis0Type_Int32:
	case vr::Prop_Axis1Type_Int32:
	case vr::Prop_Axis2Type_Int32:
	case vr::Prop_Axis3Type_Int32:
	case vr::Prop_Axis4Type_Int32:
		XR_STUBBED();
	default:
		break;
	}

	/*
	// Don't apply the inputs to the tracking object
	if (IsTouchController()) {
		switch (prop) {
		case vr::Prop_Axis0Type_Int32:
			// TODO find out which of these SteamVR returns and do likewise
			// return vr::k_eControllerAxis_TrackPad;
			return vr::k_eControllerAxis_Joystick;

		case vr::Prop_Axis1Type_Int32:
		case vr::Prop_Axis2Type_Int32:
			return vr::k_eControllerAxis_Trigger;

		case vr::Prop_Axis3Type_Int32:
		case vr::Prop_Axis4Type_Int32:
			return vr::k_eControllerAxis_None;
		}
	}
	 */

	return XrTrackedDevice::GetInt32TrackedDeviceProperty(prop, pErrorL);
}

uint64_t XrController::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	// This is for the old input system, which we don't initially need
	if (prop == vr::Prop_SupportedButtons_Uint64) {
		XR_STUBBED();
	}

	// if (IsTouchController() && prop == vr::Prop_SupportedButtons_Uint64) {
	// 	return ButtonMaskFromId(k_EButton_ApplicationMenu) | ButtonMaskFromId(k_EButton_Grip) | ButtonMaskFromId(k_EButton_Axis2) | ButtonMaskFromId(k_EButton_DPad_Left)
	// 	| ButtonMaskFromId(k_EButton_DPad_Up) | ButtonMaskFromId(k_EButton_DPad_Down) | ButtonMaskFromId(k_EButton_DPad_Right) | ButtonMaskFromId(k_EButton_A)
	// 	| ButtonMaskFromId(k_EButton_SteamVR_Touchpad) | ButtonMaskFromId(k_EButton_SteamVR_Trigger);
	// }

	return XrTrackedDevice::GetUint64TrackedDeviceProperty(prop, pErrorL);
}

uint32_t XrController::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop,
    char* value, uint32_t bufferSize, vr::ETrackedPropertyError* pErrorL)
{

	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

#define PROP(in, out)                                                                                  \
	if (prop == in) {                                                                                  \
		if (value != NULL && bufferSize > 0) {                                                         \
			strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
		}                                                                                              \
		return (uint32_t)strlen(out) + 1;                                                              \
	}

	/*
	switch (device) {
	case EOculusTrackedObject::LTouch:
		PROP(vr::Prop_RenderModelName_String, "renderLeftHand");
		PROP(vr::Prop_ModelNumber_String, "Oculus Rift CV1 (Left Controller)");
		PROP(vr::Prop_RegisteredDeviceType_String, "oculus/F00BAAF00F_Controller_Left"); // TODO is this different CV1 vs S?
		break;
	case EOculusTrackedObject::RTouch:
		PROP(vr::Prop_RenderModelName_String, "renderRightHand");
		PROP(vr::Prop_ModelNumber_String, "Oculus Rift CV1 (Right Controller)");
		PROP(vr::Prop_RegisteredDeviceType_String, "oculus/F00BAAF00F_Controller_Right");
		break;
	case EOculusTrackedObject::Object0:
		PROP(vr::Prop_RenderModelName_String, "renderObject0");

		// This is made up, and not at all verified with SteamVR
		PROP(vr::Prop_ModelNumber_String, "Oculus Rift CV1 (Tracked Object 0)");
		break;
	}
	 */
	STUBBED();

	PROP(vr::Prop_ControllerType_String, "oculus_touch");

	return XrTrackedDevice::GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
}

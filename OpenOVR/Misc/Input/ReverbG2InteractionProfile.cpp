#include "ReverbG2InteractionProfile.h"

#include "stdafx.h"

#include <glm/gtc/matrix_inverse.hpp>

ReverbG2InteractionProfile::ReverbG2InteractionProfile()
{
	std::string left_hand_paths[] = {
		"/input/x/click",
		"/input/y/click"
	};

	std::string right_hand_paths[] = {
		"/input/a/click",
		"/input/b/click"
	};

	std::string paths[] = {
		"/input/menu/click",
		"/input/squeeze/value",
		"/input/trigger/value",
		"/input/thumbstick",
		"/input/thumbstick/x",
		"/input/thumbstick/y",
		"/input/thumbstick/click",
		"/input/grip/pose",
		"/input/aim/pose",
		"/output/haptic",
	};

	for (const auto& path : left_hand_paths) {
		validInputPaths.insert("/user/hand/left" + path);
	}

	for (const auto& path : right_hand_paths) {
		validInputPaths.insert("/user/hand/right" + path);
	}

	for (const auto& path : paths) {
		validInputPaths.insert("/user/hand/left" + path);
		validInputPaths.insert("/user/hand/right" + path);
	}

	pathTranslationMap = {
		{ "grip", "squeeze" },
		{ "joystick", "thumbstick" },
		{ "pull", "value" },
		{ "grip/click", "squeeze/value" },
		{ "trigger/click", "trigger/value" },
		{ "application_menu", "menu" }
	};

	hmdPropertiesMap = {
		{ vr::Prop_ManufacturerName_String, "WindowsMR" },
	};

	propertiesMap = {
		{ vr::Prop_ModelNumber_String, { "WindowsMR" } },
		{ vr::Prop_ControllerType_String, { GetOpenVRName().value() } },
	};

	// Setup the grip-to-steamvr space matrices

	// This is the matrices pulled from GetComponentState("handgrip") with SteamVR - they're
	// the opposite of what we want, since they transform from the SteamVR space to the grip space.
	// Both hands have the same matrix.
	glm::mat4 inverseHandTransform = {
		{ 1.000000, 0.000000, 0.000000, 0.000000 }, // Column 0
		{ 0.000000, 0.996138, 0.087799, 0.000000 }, // Column 1
		{ 0.000000, -0.087799, 0.996138, 0.000000 }, // Column 2
		{ 0.000000, 0.003000, 0.097000, 1.000000 }, // Column 3 (translation)
	};
	handTransform = glm::affineInverse(inverseHandTransform);

	// Set up the component transforms
	leftComponentTransforms["base"] = {
		{ -1.000000, 0.000000, 0.000000, 0.000000 },
		{ 0.000000, 0.999976, 0.006981, 0.000000 },
		{ -0.000000, 0.006981, -0.999976, 0.000000 },
		{ -0.003400, -0.003400, 0.149100, 1.000000 },
	};
	rightComponentTransforms["base"] = {
		{ -1.000000, 0.000000, 0.000000, 0.000000 },
		{ 0.000000, 0.999976, 0.006981, 0.000000 },
		{ -0.000000, 0.006981, -0.999976, 0.000000 },
		{ 0.003400, -0.003400, 0.149100, 1.000000 },
	};
	leftComponentTransforms["tip"] = {
		{ 1.000000, 0.000000, 0.000000, 0.000000 },
		{ 0.000000, 0.794415, -0.607376, 0.000000 },
		{ 0.000000, 0.607376, 0.794415, 0.000000 },
		{ 0.016694, -0.025220, 0.024687, 1.000000 },
	};
	rightComponentTransforms["tip"] = {
		{ 1.000000, 0.000000, 0.000000, 0.000000 },
		{ 0.000000, 0.794415, -0.607376, 0.000000 },
		{ 0.000000, 0.607376, 0.794415, 0.000000 },
		{ -0.016694, -0.025220, 0.024687, 1.000000 },
	};
}

const std::string& ReverbG2InteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/hp/mixed_reality_controller";
	return path;
}

std::optional<const char*> ReverbG2InteractionProfile::GetOpenVRName() const
{
	return "hpmotioncontroller";
}

const InteractionProfile::LegacyBindings* ReverbG2InteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings allBindings[2] = { {}, {} };
	int hand = handPath == "/user/hand/left" ? vr::Eye_Left : vr::Eye_Right;
	LegacyBindings& bindings = allBindings[hand];

	if (!bindings.menu) {
		bindings.menu = "input/menu/click";
		bindings.stickX = "input/thumbstick/x";
		bindings.stickY = "input/thumbstick/y";
		bindings.stickBtn = "input/thumbstick/click";
		bindings.trigger = "input/trigger/value";
		bindings.grip = "input/squeeze/value";
		bindings.haptic = "output/haptic";
		bindings.gripPoseAction = "input/grip/pose";
		bindings.aimPoseAction = "input/aim/pose";

		// TODO: figure out mappings for buttons on controllers
		if (handPath == "/user/hand/left") {
			bindings.btnA = "input/x/click";
		} else {
			bindings.btnA = "input/a/click";
		}
	}

	return &bindings;
}

glm::mat4 ReverbG2InteractionProfile::GetGripToSteamVRTransform(ITrackedDevice::HandType hand) const
{
	return handTransform;
}

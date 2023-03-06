#include "ReverbG2InteractionProfile.h"

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
		bindings.system = "input/menu/click";
		bindings.stickX = "input/thumbstick/x";
		bindings.stickY = "input/thumbstick/y";
		bindings.stickBtn = "input/thumbstick/click";
		bindings.trigger = "input/trigger/value";
		bindings.triggerClick = "input/trigger/value";
		bindings.triggerTouch = "input/trigger/value";
		bindings.grip = "input/squeeze/value";
		bindings.haptic = "output/haptic";
		bindings.gripPoseAction = "input/grip/pose";
		bindings.aimPoseAction = "input/aim/pose";

		// TODO: figure out mappings for buttons on controllers
		if (handPath == "/user/hand/left") {
			bindings.btnA = "input/x/click";
			bindings.menu = "input/y/click";
		} else {
			bindings.btnA = "input/a/click";
			bindings.menu = "input/b/click";
		}
	}

	return &bindings;
}

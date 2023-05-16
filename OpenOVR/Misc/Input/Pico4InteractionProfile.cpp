#include "Pico4InteractionProfile.h"

Pico4InteractionProfile::Pico4InteractionProfile()
{
	std::string left_hand_paths[] = {
		"/input/x/click",
		"/input/x/touch",
		"/input/y/click",
		"/input/y/touch",
		"/input/menu/click"
	};

	std::string right_hand_paths[] = {
		"/input/a/click",
		"/input/a/touch",
		"/input/b/click",
		"/input/b/touch"
	};

	std::string paths[] = {
		"/input/system/click",
		"/input/trigger/click",
		"/input/trigger/value",
		"/input/trigger/touch",
		"/input/thumbstick/y",
		"/input/thumbstick/x",
		"/input/thumbstick/click",
		"/input/thumbstick/touch",
		"/input/squeeze/click",
		"/input/squeeze/value",
		"/input/grip/pose",
		"/input/aim/pose",
		"/output/haptic"
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
		{ "application_menu", "menu" }
	};

	hmdPropertiesMap = {
		{ vr::Prop_ManufacturerName_String, "Pico" },
	};

	propertiesMap = {
		{ vr::Prop_ModelNumber_String, { "Pico4" } },
		{ vr::Prop_ControllerType_String, { GetOpenVRName().value() } },
	};

	// Setup the grip-to-steamvr space matrices
	glm::mat4 inverseHandTransformLeft = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99614, -0.08780, 0.00000 },
		{ 0.00000, 0.08780, 0.99614, 0.00000 },
		{ 0.00000, 0.00300, 0.09700, 1.00000 }
	};

	glm::mat4 inverseHandTransformRight = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99614, -0.08780, 0.00000 },
		{ 0.00000, 0.08780, 0.99614, 0.00000 },
		{ 0.00000, 0.00300, 0.09700, 1.00000 }
	};

	leftHandGripTransform = glm::affineInverse(inverseHandTransformLeft);
	rightHandGripTransform = glm::affineInverse(inverseHandTransformRight);

	// Set up the component transforms

	glm::mat4 bodyLeft = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 1.00000, -0.00065, 0.00000 },
		{ 0.00000, 0.00065, 1.00000, 0.00000 },
		{ 0.00000, 0.00300, 0.09700, 1.00000 }
	};
	glm::mat4 bodyRight = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 1.00000, -0.00065, 0.00000 },
		{ 0.00000, 0.00065, 1.00000, 0.00000 },
		{ 0.00000, 0.00300, 0.09700, 1.00000 },
	};
	glm::mat4 tipLeft = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99975, 0.02237, 0.00000 },
		{ -0.00000, -0.02237, 0.99975, 0.00000 },
		{ -0.01200, -0.01000, -0.00700, 1.00000 }
	};
	glm::mat4 tipRight = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99975, 0.02237, 0.00000 },
		{ -0.00000, -0.02237, 0.99975, 0.00000 },
		{ 0.01200, -0.01000, -0.00700, 1.00000 }
	};

	leftComponentTransforms["body"] = glm::affineInverse(bodyLeft);
	rightComponentTransforms["body"] = glm::affineInverse(bodyRight);
	leftComponentTransforms["tip"] = glm::affineInverse(tipLeft);
	rightComponentTransforms["tip"] = glm::affineInverse(tipRight);
}

const std::string& Pico4InteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/bytedance/pico4_controller";
	return path;
}

std::optional<const char*> Pico4InteractionProfile::GetOpenVRName() const
{
	return "pico4controller";
}

const InteractionProfile::LegacyBindings* Pico4InteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings allBindings[2] = { {}, {} };
	int hand = handPath == "/user/hand/left" ? vr::Eye_Left : vr::Eye_Right;
	LegacyBindings& bindings = allBindings[hand];

	if (!bindings.menu) {
		
		bindings.stickX = "input/thumbstick/x";
		bindings.stickY = "input/thumbstick/y";
		bindings.stickBtn = "input/thumbstick/click";
		bindings.stickBtnTouch = "input/thumbstick/touch";
		bindings.trigger = "input/trigger/value";
		bindings.triggerClick = "input/trigger/click";
		bindings.triggerTouch = "input/trigger/touch";
		bindings.grip = "input/squeeze/value";
		bindings.haptic = "output/haptic";
		bindings.gripPoseAction = "input/grip/pose";
		bindings.aimPoseAction = "input/aim/pose";

		if (handPath == "/user/hand/left") {
			bindings.btnA = "input/x/click";
			bindings.menu = "input/y/click";
			bindings.system = "input/menu/click";
		} else {
			bindings.btnA = "input/a/click";
			bindings.menu = "input/b/click";
		}
	}

	return &bindings;
}

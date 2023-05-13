#pragma once

#include <array>

constexpr auto SMOOTHING_WINDOW_SIZE = 3;

class SmoothInput {
public:
	SmoothInput(size_t smoothingWindowSize);

	void updateTriggerValue(int hand, float newTriggerValue);
	float getSmoothedTriggerValue(int hand) const;

	void updateGripValue(int hand, float newGripValue);
	float getSmoothedGripValue(int hand) const;

	void updateJoystickXValue(int hand, float newJoystickValue);
	float getSmoothedJoystickXValue(int hand) const;

	void updateJoystickYValue(int hand, float newJoystickValue);
	float getSmoothedJoystickYValue(int hand) const;

private:
	class SmoothValue {
	public:
		SmoothValue(size_t windowSize);
		void update(float newValue);
		float getSmoothedValue() const;

	private:
		std::vector<float> values;
		size_t index = 0;
	};

	size_t smoothingWindowSize;
	SmoothValue leftGrip;
	SmoothValue leftTrigger;
	SmoothValue leftJoystickX;
	SmoothValue leftJoystickY;
	SmoothValue rightGrip;
	SmoothValue rightTrigger;
	SmoothValue rightJoystickX;
	SmoothValue rightJoystickY;
};
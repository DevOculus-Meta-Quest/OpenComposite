#pragma once

#include <array>

constexpr auto SMOOTHING_WINDOW_SIZE = 3;

class SmoothInput {
public:
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
		void update(float newValue);
		float getSmoothedValue() const;

	private:
		std::array<float, SMOOTHING_WINDOW_SIZE> values;
		int index = 0;
	};

	SmoothValue leftGrip;
	SmoothValue leftTrigger;	
	SmoothValue leftJoystickX;
	SmoothValue leftJoystickY;
	SmoothValue rightGrip;
	SmoothValue rightTrigger;	
	SmoothValue rightJoystickX;
	SmoothValue rightJoystickY;
};
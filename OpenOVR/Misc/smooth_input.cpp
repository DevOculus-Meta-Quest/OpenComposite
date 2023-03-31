#include "smooth_input.h"
#include <algorithm>

void SmoothInput::updateTriggerValue(int hand, float newTriggerValue)
{
	(hand == 0 ? leftTrigger : rightTrigger).update(newTriggerValue);
}

float SmoothInput::getSmoothedTriggerValue(int hand) const
{
	return (hand == 0 ? leftTrigger : rightTrigger).getSmoothedValue();
}

void SmoothInput::updateGripValue(int hand, float newGripValue)
{
	(hand == 0 ? leftGrip : rightGrip).update(newGripValue);
}

float SmoothInput::getSmoothedGripValue(int hand) const
{
	return (hand == 0 ? leftGrip : rightGrip).getSmoothedValue();
}

void SmoothInput::updateJoystickXValue(int hand, float joystickValue)
{
	(hand == 0 ? leftJoystickX : rightJoystickX).update(joystickValue);
}

float SmoothInput::getSmoothedJoystickXValue(int hand) const
{
	return (hand == 0 ? leftJoystickX : rightJoystickX).getSmoothedValue();
}

void SmoothInput::updateJoystickYValue(int hand, float joystickValue)
{
	(hand == 0 ? leftJoystickY : rightJoystickY).update(joystickValue);
}

float SmoothInput::getSmoothedJoystickYValue(int hand) const
{
	return (hand == 0 ? leftJoystickY : rightJoystickY).getSmoothedValue();
}

void SmoothInput::SmoothValue::update(float newValue)
{
	sum -= values[index];
	sum += newValue;
	values[index] = newValue;
	index = (index + 1) % SMOOTHING_WINDOW_SIZE;
}

float SmoothInput::SmoothValue::getSmoothedValue() const
{
	std::array<float, SMOOTHING_WINDOW_SIZE> sortedValues = values;
	std::sort(sortedValues.begin(), sortedValues.end(), [](float a, float b) {
		return std::abs(a) < std::abs(b);
	});
	float sumWithoutTwoClosestToZero = sum - sortedValues[0] - sortedValues[1];
	return sumWithoutTwoClosestToZero / (SMOOTHING_WINDOW_SIZE - 2);
}

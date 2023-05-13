#include "smooth_input.h"
#include <algorithm>

SmoothInput::SmoothInput(size_t smoothingWindowSize)
    : leftGrip(smoothingWindowSize), 
	leftTrigger(smoothingWindowSize), 
	leftJoystickX(smoothingWindowSize), 
	leftJoystickY(smoothingWindowSize), 
	rightGrip(smoothingWindowSize), 
	rightTrigger(smoothingWindowSize),
	rightJoystickX(smoothingWindowSize), 
	rightJoystickY(smoothingWindowSize)
{
}

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

SmoothInput::SmoothValue::SmoothValue(size_t windowSize)
    : values(windowSize)
{
}

void SmoothInput::SmoothValue::update(float newValue)
{
	values[index] = newValue;
	index = (index + 1) % values.size();
}

float SmoothInput::SmoothValue::getSmoothedValue() const
{
	std::vector<float> sortedValues = values;
	std::sort(sortedValues.begin(), sortedValues.end(), [](float a, float b) {
		return std::abs(a) > std::abs(b);
	});
	return sortedValues[0];
}

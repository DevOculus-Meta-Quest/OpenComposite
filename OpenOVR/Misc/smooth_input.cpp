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
	rightJoystickY(smoothingWindowSize), 
	leftAButtonPress(smoothingWindowSize), 
	leftBButtonPress(smoothingWindowSize), 
	leftMenuPress(smoothingWindowSize),
	rightAButtonPress(smoothingWindowSize), 
	rightBButtonPress(smoothingWindowSize), 
	rightMenuPress(smoothingWindowSize),
	leftAButtonTouch(smoothingWindowSize), 
	leftBButtonTouch(smoothingWindowSize), 
	leftMenuTouch(smoothingWindowSize), 
	rightAButtonTouch(smoothingWindowSize), 
	rightBButtonTouch(smoothingWindowSize), 
	rightMenuTouch(smoothingWindowSize), 
	leftThumbClick(smoothingWindowSize), 
	leftThumbTouch(smoothingWindowSize),
	leftTriggerClick(smoothingWindowSize), 
	leftTriggerTouch(smoothingWindowSize), 
	leftGripClick(smoothingWindowSize), 
	rightThumbClick(smoothingWindowSize), 
	rightThumbTouch(smoothingWindowSize), 
	rightTriggerClick(smoothingWindowSize), 
	rightTriggerTouch(smoothingWindowSize), 
	rightGripClick(smoothingWindowSize)
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

void SmoothInput::updateAButtonPressValue(int hand, int aButtonPressValue)
{
	(hand == 0 ? leftAButtonPress : rightAButtonPress).updateBoolean(aButtonPressValue);
}
int SmoothInput::getSmoothedAButtonPressValue(int hand) const
{
	return (hand == 0 ? leftAButtonPress : rightAButtonPress).getSmoothedBooleanValue();
}

void SmoothInput::updateBButtonPressValue(int hand, int bButtonPressValue)
{
	(hand == 0 ? leftBButtonPress : rightBButtonPress).updateBoolean(bButtonPressValue);
}
int SmoothInput::getSmoothedBButtonPressValue(int hand) const
{
	return (hand == 0 ? leftBButtonPress : rightBButtonPress).getSmoothedBooleanValue();
}

void SmoothInput::updateMenuPressValue(int hand, int menuPressValue)
{
	(hand == 0 ? leftMenuPress : rightMenuPress).updateBoolean(menuPressValue);
}
int SmoothInput::getSmoothedMenuPressValue(int hand) const
{
	return (hand == 0 ? leftMenuPress : rightMenuPress).getSmoothedBooleanValue();
}

void SmoothInput::updateAButtonTouchValue(int hand, int aButtonTouchValue)
{
	(hand == 0 ? leftAButtonTouch : rightAButtonTouch).updateBoolean(aButtonTouchValue);
}
int SmoothInput::getSmoothedAButtonTouchValue(int hand) const
{
	return (hand == 0 ? leftAButtonTouch : rightAButtonTouch).getSmoothedBooleanValue();
}

void SmoothInput::updateBButtonTouchValue(int hand, int bButtonTouchValue)
{
	(hand == 0 ? leftBButtonTouch : rightBButtonTouch).updateBoolean(bButtonTouchValue);
}
int SmoothInput::getSmoothedBButtonTouchValue(int hand) const
{
	return (hand == 0 ? leftBButtonTouch : rightBButtonTouch).getSmoothedBooleanValue();
}

void SmoothInput::updateMenuTouchValue(int hand, int menuTouchValue)
{
	(hand == 0 ? leftMenuTouch : rightMenuTouch).updateBoolean(menuTouchValue);
}
int SmoothInput::getSmoothedMenuTouchValue(int hand) const
{
	return (hand == 0 ? leftMenuTouch : rightMenuTouch).getSmoothedBooleanValue();
}

void SmoothInput::updateThumbClickValue(int hand, int thumbClickValue)
{
	(hand == 0 ? leftThumbClick : rightThumbClick).updateBoolean(thumbClickValue);
}
int SmoothInput::getSmoothedThumbClickValue(int hand) const
{
	return (hand == 0 ? leftThumbClick : rightThumbClick).getSmoothedBooleanValue();
}

void SmoothInput::updateThumbTouchValue(int hand, int thumbTouchValue)
{
	(hand == 0 ? leftThumbTouch : rightThumbTouch).updateBoolean(thumbTouchValue);
}
int SmoothInput::getSmoothedThumbTouchValue(int hand) const
{
	return (hand == 0 ? leftThumbTouch : rightThumbTouch).getSmoothedBooleanValue();
}

void SmoothInput::updateTriggerClickValue(int hand, int triggerClickValue)
{
	(hand == 0 ? leftTriggerClick : rightTriggerClick).updateBoolean(triggerClickValue);
}
int SmoothInput::getSmoothedTriggerClickValue(int hand) const
{
	return (hand == 0 ? leftTriggerClick : rightTriggerClick).getSmoothedBooleanValue();
}

void SmoothInput::updateTriggerTouchValue(int hand, int triggerTouchValue)
{
	(hand == 0 ? leftTriggerTouch : rightTriggerTouch).updateBoolean(triggerTouchValue);
}
int SmoothInput::getSmoothedTriggerTouchValue(int hand) const
{
	return (hand == 0 ? leftTriggerTouch : rightTriggerTouch).getSmoothedBooleanValue();
}

void SmoothInput::updateGripClickValue(int hand, int gripClickValue)
{
	(hand == 0 ? leftGripClick : rightGripClick).updateBoolean(gripClickValue);
}
int SmoothInput::getSmoothedGripClickValue(int hand) const
{
	return (hand == 0 ? leftGripClick : rightGripClick).getSmoothedBooleanValue();
}

SmoothInput::SmoothValue::SmoothValue(size_t windowSize)
    : values(windowSize), booleanValues(windowSize)
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

void SmoothInput::SmoothValue::updateBoolean(int newValue)
{
	booleanValues[booleanIndex] = newValue;
	booleanIndex = (booleanIndex + 1) % booleanValues.size();
}

int SmoothInput::SmoothValue::getSmoothedBooleanValue() const
{
	std::vector<int> sortedValues = booleanValues;
	std::sort(sortedValues.begin(), sortedValues.end(), [](int a, int b) {
		return a > b;
	});
	return sortedValues[0];
}

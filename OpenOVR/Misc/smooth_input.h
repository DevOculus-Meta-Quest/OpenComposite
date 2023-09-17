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

	void updateAButtonPressValue(int hand, int aButtonPressValue);
	int getSmoothedAButtonPressValue(int hand) const;

	void updateBButtonPressValue(int hand, int bButtonPressValue);
	int getSmoothedBButtonPressValue(int hand) const;

	void updateMenuPressValue(int hand, int menuPressValue);
	int getSmoothedMenuPressValue(int hand) const;

	void updateAButtonTouchValue(int hand, int aButtonTouchValue);
	int getSmoothedAButtonTouchValue(int hand) const;

	void updateBButtonTouchValue(int hand, int bButtonTouchValue);
	int getSmoothedBButtonTouchValue(int hand) const;

	void updateMenuTouchValue(int hand, int menuTouchValue);
	int getSmoothedMenuTouchValue(int hand) const;

	void updateThumbClickValue(int hand, int thumbClickValue);
	int getSmoothedThumbClickValue(int hand) const;

	void updateThumbTouchValue(int hand, int thumbTouchValue);
	int getSmoothedThumbTouchValue(int hand) const;

	void updateTriggerClickValue(int hand, int triggerClickValue);
	int getSmoothedTriggerClickValue(int hand) const;

	void updateTriggerTouchValue(int hand, int triggerTouchValue);
	int getSmoothedTriggerTouchValue(int hand) const;

	void updateGripClickValue(int hand, int gripClickValue);
	int getSmoothedGripClickValue(int hand) const;

private:
	class SmoothValue {
	public:
		SmoothValue(size_t windowSize);
		void update(float newValue);
		void updateBoolean(int newValue);
		float getSmoothedValue() const;
		int getSmoothedBooleanValue() const;

	private:
		std::vector<float> values;
		std::vector<int> booleanValues;
		size_t booleanIndex = 0;
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
	SmoothValue leftAButtonPress;
	SmoothValue leftBButtonPress;
	SmoothValue leftMenuPress;
	SmoothValue rightAButtonPress;
	SmoothValue rightBButtonPress;
	SmoothValue rightMenuPress;
	SmoothValue leftAButtonTouch;
	SmoothValue leftBButtonTouch;
	SmoothValue leftMenuTouch;
	SmoothValue rightAButtonTouch;
	SmoothValue rightBButtonTouch;
	SmoothValue rightMenuTouch;
	SmoothValue leftThumbClick;
	SmoothValue leftThumbTouch;
	SmoothValue leftTriggerClick;
	SmoothValue leftTriggerTouch;
	SmoothValue leftGripClick;
	SmoothValue rightThumbClick;
	SmoothValue rightThumbTouch;
	SmoothValue rightTriggerClick;
	SmoothValue rightTriggerTouch;
	SmoothValue rightGripClick;
};
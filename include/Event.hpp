#pragma once


namespace vr {
	

	class Event {
	public:
		enum EventType
		{
			Closed,
			Resized,
			LostFocus,
			GainedFocus,
			TextEntered,
			KeyPressed,
			KeyReleased,
			MouseWheelMoved,
			MouseWheelScrolled,
			MouseButtonPressed,
			MouseButtonReleased,
			MouseMoved,
			MouseEntered,
			MouseLeft,
			JoystickButtonPressed,
			JoystickButtonReleased,
			JoystickMoved,
			JoystickConnected,
			JoystickDisconnected,
			TouchBegan,
			TouchMoved,
			TouchEnded,
			SensorChanged,
			Count
		};

		EventType type;

	};

	Closed,
	Resized,
	LostFocus,
	GainedFocus,
	TextEntered,
	KeyPressed,
	KeyReleased,
	MouseWheelMoved,
	MouseWheelScrolled,
	MouseButtonPressed,
	MouseButtonReleased,
	MouseMoved,
	MouseEntered,
	MouseLeft,
	JoystickButtonPressed,
	JoystickButtonReleased,
	JoystickMoved,
	JoystickConnected,
	JoystickDisconnected,
	TouchBegan,
	TouchMoved,
	TouchEnded,
	SensorChanged,
	Count




	




}
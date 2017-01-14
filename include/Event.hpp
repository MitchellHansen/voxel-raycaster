#pragma once
#include <SFML/Config.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Sensor.hpp>



namespace vr {
	
	// The event class here will both abstract out
	// the SFML sf::Event class, but will also provide
	// the ability to easily extend the class to contain
	// even more event types.

	// A result of getting rid of the union and extracting
	// event types into individual classes is the fact that
	// there is going to be a lot of repeat code i.e 
	// KeyPressed, KeyHeld, and KeyReleased all hold the same
	// data, it's just their names that are different

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
			KeyHeld,
			KeyReleased,
			MouseWheelMoved,
			MouseWheelScrolled,
			MouseButtonPressed,
			MouseButtonHeld,
			MouseButtonReleased,
			MouseMoved,
			MouseEntered,
			MouseLeft,
			JoystickButtonPressed,
			JoystickButtonHeld,
			JoystickButtonReleased,
			JoystickMoved,
			JoystickConnected,
			JoystickDisconnected,
			TouchBegan,
			TouchMoved,
			TouchEnded,
			SensorChanged,
			NetworkJoystickButtonPressed,
			NetworkJoystickButtonHeld,
			NetworkJoystickButtonReleased,
			NetworkJoystickMoved,
			NetworkJoystickConnected,
			NetworkJoystickDisconnected,
			Count
		};

		Event(EventType type) : type(type) {
			
		};

		EventType type;

	};

	class Closed : public Event {
	public:
		Closed() : Event(vr::Event::EventType::Closed) {};
	};

	class Resized : public Event {
	public:
		Resized(unsigned int width, unsigned int height) : 
			width(width), height(height), Event(vr::Event::EventType::Resized) {};
		unsigned int		width;
		unsigned int		height;
	};

	class LostFocus : public Event {
	public:
		LostFocus() : Event(vr::Event::EventType::LostFocus) { };
	};
	class GainedFocus : public Event {
	public:
		GainedFocus() : Event(vr::Event::EventType::GainedFocus) {};
	};
	
	class TextEntered : public Event {
	public:
		TextEntered(sf::Uint32 unicode) : 
			unicode(unicode), Event(vr::Event::EventType::TextEntered) {};

		sf::Uint32 unicode;
	};

	class KeyPressed : public Event {
	public:
		KeyPressed(sf::Keyboard::Key code, bool alt, bool control, bool shift, bool system ) :
			code(code), alt(alt), control(control), shift(shift), system(system), Event(vr::Event::EventType::KeyPressed) {};

		sf::Keyboard::Key	code;
		bool				alt;
		bool				control;
		bool				shift;
		bool				system;
	};

	class KeyHeld : public Event {
	public:
		KeyHeld(sf::Keyboard::Key code, bool alt, bool control, bool shift, bool system) :
			code(code), alt(alt), control(control), shift(shift), system(system), Event(vr::Event::EventType::KeyHeld) {};

		sf::Keyboard::Key	code;
		bool				alt;
		bool				control;
		bool				shift;
		bool				system;
	};

	class KeyReleased : public Event {
	public:
		KeyReleased(sf::Keyboard::Key code, bool alt, bool control, bool shift, bool system) :
			code(code), alt(alt), control(control), shift(shift), system(system), Event(vr::Event::EventType::KeyReleased) {};

		sf::Keyboard::Key	code;
		bool				alt;
		bool				control;
		bool				shift;
		bool				system;
	};

	// This is depreciated, remove?
	class MouseWheelMoved : public Event {
	public:
		MouseWheelMoved() : Event(vr::Event::EventType::MouseWheelMoved) {};
	};

	class MouseWheelScrolled : public Event {
	public:
		MouseWheelScrolled(sf::Mouse::Wheel wheel, float delta, int x, int y) :
			wheel(wheel), delta(delta), x(x), y(y), Event(vr::Event::EventType::MouseWheelScrolled) {};

		sf::Mouse::Wheel	wheel;
		float				delta;
		int					x;
		int					y;
	};

	class MouseButtonPressed : public Event {
	public:
		MouseButtonPressed(sf::Mouse::Button button, int x, int y) :
			button(button), x(x), y(y), Event(vr::Event::EventType::MouseButtonPressed) {};

		sf::Mouse::Button	button;
		int					x;
		int					y;
	};

	class MouseButtonHeld : public Event {
	public:
		MouseButtonHeld(sf::Mouse::Button button, int x, int y) :
			button(button), x(x), y(y), Event(vr::Event::EventType::MouseButtonHeld) {};

		sf::Mouse::Button	button;
		int					x;
		int					y;
	};

	class MouseButtonReleased : public Event {
	public:
		MouseButtonReleased(sf::Mouse::Button button, int x, int y) :
			button(button), x(x), y(y), Event(vr::Event::EventType::MouseButtonReleased) {};

		sf::Mouse::Button	button;
		int					x;
		int					y;
	};

	class MouseMoved : public Event {
	public:
		MouseMoved(int x, int y) :
			x(x), y(y), Event(vr::Event::EventType::MouseMoved) {};

		int					x;
		int					y;
	};

	class MouseEntered : public Event {
	public:
		MouseEntered(int x, int y) :
			x(x), y(y), Event(vr::Event::EventType::MouseEntered) {};

		int					x;
		int					y;
	};

	class MouseLeft : public Event {
	public:
		MouseLeft(int x, int y) :
			x(x), y(y), Event(vr::Event::EventType::MouseLeft) {};

		int					x;
		int					y;
	};

	class JoystickButtonPressed : public Event {
	public:
		JoystickButtonPressed(unsigned int joystickId, unsigned int button) :
			joystickId(joystickId), button(button), Event(vr::Event::EventType::JoystickButtonPressed) {};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class JoystickButtonHeld : public Event {
	public:
		JoystickButtonHeld(unsigned int joystickId, unsigned int button) :
			joystickId(joystickId), button(button), Event(vr::Event::EventType::JoystickButtonHeld) {};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class JoystickButtonReleased : public Event {
	public:
		JoystickButtonReleased(unsigned int joystickId, unsigned int button) :
			joystickId(joystickId), button(button), Event(vr::Event::EventType::JoystickButtonReleased) {};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class JoystickMoved : public Event {
	public:
		JoystickMoved(sf::Joystick::Axis axis, unsigned int joystickId, float position) :
			axis(axis), joystickId(joystickId), position(position), Event(vr::Event::EventType::JoystickMoved) {};

		sf::Joystick::Axis	axis;
		unsigned int		joystickId;
		float				position;
	};

	class JoystickConnected : public Event {
	public:
		JoystickConnected(unsigned int joystickId) :
			joystickId(joystickId), Event(vr::Event::EventType::JoystickConnected) {};

		unsigned int		joystickId;
	};

	class JoystickDisconnected : public Event {
	public:
		JoystickDisconnected(unsigned int joystickId) :
			joystickId(joystickId), Event(vr::Event::EventType::JoystickDisconnected) {};

		unsigned int		joystickId;
	};

	class TouchBegan : public Event {
	public:
		TouchBegan(unsigned int finger, int x, int y) :
			finger(finger), x(x), y(y), Event(vr::Event::EventType::TouchBegan) {};
				
		unsigned int		finger;
		int					x;
		int					y;
	};

	class TouchMoved : public Event {
	public:
		TouchMoved(unsigned int finger, int x, int y) :
			finger(finger), x(x), y(y), Event(vr::Event::EventType::TouchMoved) {};

		unsigned int		finger;
		int					x;
		int					y;
	};

	class TouchEnded : public Event {
	public:
		TouchEnded(unsigned int finger, int x, int y) :
			finger(finger), x(x), y(y), Event(vr::Event::EventType::TouchEnded) {};

		unsigned int		finger;
		int					x;
		int					y;
	};

	class SensorChanged : public Event {
	public:
		SensorChanged(sf::Sensor::Type type, float x, float y, float z) :
			type(type), x(x), y(y), z(z), Event(vr::Event::EventType::SensorChanged) {};

		sf::Sensor::Type	type;
		float				x;
		float				y;
		float				z;
	};

	// I'm moving this to it's own event type because
	// I don't want to have to mess with the static
	// sf::Joystick class and how it hits a good half
	// dozen impl files. 

	class NetworkJoystickButtonPressed : public Event {
	public:
		NetworkJoystickButtonPressed(unsigned int joystickId, unsigned int button) :
			joystickId(joystickId), button(button), Event(vr::Event::EventType::NetworkJoystickButtonPressed) {};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class NetworkJoystickButtonHeld : public Event {
	public:
		NetworkJoystickButtonHeld(unsigned int joystickId, unsigned int button) :
			joystickId(joystickId), button(button), Event(vr::Event::EventType::NetworkJoystickButtonHeld) {};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class NetworkJoystickButtonReleased : public Event {
	public:
		NetworkJoystickButtonReleased(unsigned int joystickId, unsigned int button) :
			joystickId(joystickId), button(button), Event(vr::Event::EventType::NetworkJoystickButtonReleased) {};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class NetworkJoystickMoved : public Event {
	public:
		NetworkJoystickMoved(sf::Joystick::Axis axis, unsigned int joystickId, float position) :
			axis(axis), joystickId(joystickId), position(position), Event(vr::Event::EventType::NetworkJoystickMoved) {};

		sf::Joystick::Axis	axis;
		unsigned int		joystickId;
		float				position;
	};

	class NetworkJoystickConnected : public Event {
	public:
		NetworkJoystickConnected(unsigned int joystickId) :
			joystickId(joystickId), Event(vr::Event::EventType::NetworkJoystickConnected) {};

		unsigned int		joystickId;
	};

	class NetworkJoystickDisconnected : public Event {
	public:
		NetworkJoystickDisconnected(unsigned int joystickId) :
			joystickId(joystickId), Event(vr::Event::EventType::NetworkJoystickDisconnected) {};

		unsigned int		joystickId;
	};



}

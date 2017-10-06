#pragma once
#include <SFML/Config.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Sensor.hpp>
#include <memory>


namespace vr {
	
	// The event class here will both abstract out
	// the SFML sf::Event class, but will also provide
	// the ability to easily extend the class to contain
	// even more event types.

	// An annoying result of getting rid of the union and extracting
	// event types into individual classes is the fact that
	// there is going to be a lot of repeat code i.e 
	// KeyPressed, KeyHeld, and KeyReleased all hold the same
	// data, it's just their names that are different. 

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
		virtual ~Event() {};

		virtual std::unique_ptr<Event> clone() = 0;

		EventType type;
	private:
		
	};

	

	class Closed : public Event {
	public:
		Closed() : Event(vr::Event::EventType::Closed) {};
		~Closed() override {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::Closed>(vr::Closed())); 
		};
	};

	class Resized : public Event {
	public:
		Resized(unsigned int width, unsigned int height) : 
			Event(vr::Event::EventType::Resized), width(width), height(height) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::Resized>(vr::Resized(width, height))); 
		};
		unsigned int		width;
		unsigned int		height;
	};

	class LostFocus : public Event {
	public:
		LostFocus() : Event(vr::Event::EventType::LostFocus) { };
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::LostFocus>(vr::LostFocus())); 
		};
	};
	class GainedFocus : public Event {
	public:
		GainedFocus() : Event(vr::Event::EventType::GainedFocus) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::GainedFocus>(vr::GainedFocus())); 
		};
	};
	
	class TextEntered : public Event {
	public:
		TextEntered(sf::Uint32 unicode) : 
			Event(vr::Event::EventType::TextEntered), unicode(unicode) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::TextEntered>(vr::TextEntered(unicode))); 
		};

		sf::Uint32 unicode;
	};

	class KeyPressed : public Event {
	public:
		KeyPressed(sf::Keyboard::Key code, bool alt, bool control, bool shift, bool system ) :
			Event(vr::Event::EventType::KeyPressed), code(code), alt(alt), control(control), shift(shift), system(system) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::KeyPressed>(vr::KeyPressed(code, alt, control, shift, system)));
		};

		sf::Keyboard::Key	code;
		bool				alt;
		bool				control;
		bool				shift;
		bool				system;
	};

	class KeyHeld : public Event {
	public:
		KeyHeld(sf::Keyboard::Key code, bool alt, bool control, bool shift, bool system) :
			Event(vr::Event::EventType::KeyHeld), code(code), alt(alt), control(control), shift(shift), system(system) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::KeyHeld>(vr::KeyHeld(code, alt, control, shift, system)));
		};

		sf::Keyboard::Key	code;
		bool				alt;
		bool				control;
		bool				shift;
		bool				system;
	};

	class KeyReleased : public Event {
	public:
		KeyReleased(sf::Keyboard::Key code, bool alt, bool control, bool shift, bool system) :
			Event(vr::Event::EventType::KeyReleased), code(code), alt(alt), control(control), shift(shift), system(system) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::KeyReleased>(vr::KeyReleased(code, alt, control, shift, system)));
		};

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
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::MouseWheelMoved>(vr::MouseWheelMoved()));
		};
	};

	class MouseWheelScrolled : public Event {
	public:
		MouseWheelScrolled(sf::Mouse::Wheel wheel, float delta, int x, int y) :
			Event(vr::Event::EventType::MouseWheelScrolled), wheel(wheel), delta(delta), x(x), y(y) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::MouseWheelScrolled>(vr::MouseWheelScrolled(wheel, delta, x, y)));
		};

		sf::Mouse::Wheel	wheel;
		float				delta;
		int					x;
		int					y;
	};

	class MouseButtonPressed : public Event {
	public:
		MouseButtonPressed(sf::Mouse::Button button, int x, int y) :
			Event(vr::Event::EventType::MouseButtonPressed), button(button), x(x), y(y) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::MouseButtonPressed>(vr::MouseButtonPressed(button, x ,y)));
		};

		sf::Mouse::Button	button;
		int					x;
		int					y;
	};

	class MouseButtonHeld : public Event {
	public:
		MouseButtonHeld(sf::Mouse::Button button, int x, int y) :
			Event(vr::Event::EventType::MouseButtonHeld), button(button), x(x), y(y) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::MouseButtonHeld>(vr::MouseButtonHeld(button, x, y)));
		};

		sf::Mouse::Button	button;
		int					x;
		int					y;
	};

	class MouseButtonReleased : public Event {
	public:
		MouseButtonReleased(sf::Mouse::Button button, int x, int y) :
			Event(vr::Event::EventType::MouseButtonReleased), button(button), x(x), y(y) {};
		std::unique_ptr<Event> clone() override {
			return std::unique_ptr<vr::Event>(std::make_unique<vr::MouseButtonReleased>(vr::MouseButtonReleased(button, x, y)));
		};

		sf::Mouse::Button	button;
		int					x;
		int					y;
	};

	class MouseMoved : public Event {
	public:
		MouseMoved(int x, int y) :
			Event(vr::Event::EventType::MouseMoved), x(x), y(y) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::MouseMoved>(vr::MouseMoved(x, y))); 
		};

		int					x;
		int					y;
	};

	class MouseEntered : public Event {
	public:
		MouseEntered(int x, int y) :
			Event(vr::Event::EventType::MouseEntered), x(x), y(y) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::MouseEntered>(vr::MouseEntered(x, y))); 
		};

		int					x;
		int					y;
	};

	class MouseLeft : public Event {
	public:
		MouseLeft(int x, int y) :
			Event(vr::Event::EventType::MouseLeft), x(x), y(y) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::MouseLeft>(vr::MouseLeft(x, y))); 
		};

		int					x;
		int					y;
	};

	class JoystickButtonPressed : public Event {
	public:
		JoystickButtonPressed(unsigned int joystickId, unsigned int button) :
			Event(vr::Event::EventType::JoystickButtonPressed), joystickId(joystickId), button(button) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::JoystickButtonPressed>(vr::JoystickButtonPressed(joystickId, button))); 
		};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class JoystickButtonHeld : public Event {
	public:
		JoystickButtonHeld(unsigned int joystickId, unsigned int button) :
			Event(vr::Event::EventType::JoystickButtonHeld), joystickId(joystickId), button(button) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::JoystickButtonHeld>(vr::JoystickButtonHeld(joystickId, button)));
		};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class JoystickButtonReleased : public Event {
	public:
		JoystickButtonReleased(unsigned int joystickId, unsigned int button) :
			joystickId(joystickId), button(button), Event(vr::Event::EventType::JoystickButtonReleased) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::JoystickButtonReleased>(vr::JoystickButtonReleased(joystickId, button)));
		};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class JoystickMoved : public Event {
	public:
		JoystickMoved(sf::Joystick::Axis axis, unsigned int joystickId, float position) :
			axis(axis), joystickId(joystickId), position(position), Event(vr::Event::EventType::JoystickMoved) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::JoystickMoved>(vr::JoystickMoved(axis, joystickId, position))); 
		};

		sf::Joystick::Axis	axis;
		unsigned int		joystickId;
		float				position;
	};

	class JoystickConnected : public Event {
	public:
		JoystickConnected(unsigned int joystickId) :
			Event(vr::Event::EventType::JoystickConnected), joystickId(joystickId) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::JoystickConnected>(vr::JoystickConnected(joystickId))); 
		};

		unsigned int		joystickId;
	};

	class JoystickDisconnected : public Event {
	public:
		JoystickDisconnected(unsigned int joystickId) :
			Event(vr::Event::EventType::JoystickDisconnected), joystickId(joystickId) {};
		std::unique_ptr<Event> clone() override {
			return std::unique_ptr<vr::Event>(std::make_unique<vr::JoystickDisconnected>(vr::JoystickDisconnected(joystickId))); 
		};

		unsigned int		joystickId;
	};

	class TouchBegan : public Event {
	public:
		TouchBegan(unsigned int finger, int x, int y) :
			Event(vr::Event::EventType::TouchBegan), finger(finger), x(x), y(y) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::TouchBegan>(vr::TouchBegan(finger, x, y)));
		};

		unsigned int		finger;
		int					x;
		int					y;
	};

	class TouchMoved : public Event {
	public:
		TouchMoved(unsigned int finger, int x, int y) :
			Event(vr::Event::EventType::TouchMoved), finger(finger), x(x), y(y) {};
		std::unique_ptr<Event> clone() override {
			return std::unique_ptr<vr::Event>(std::make_unique<vr::TouchMoved>(vr::TouchMoved(finger, x, y))); 
		};

		unsigned int		finger;
		int					x;
		int					y;
	};

	class TouchEnded : public Event {
	public:
		TouchEnded(unsigned int finger, int x, int y) :
			Event(vr::Event::EventType::TouchEnded), finger(finger), x(x), y(y) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::TouchEnded>(vr::TouchEnded(finger, x, y))); 
		};

		unsigned int		finger;
		int					x;
		int					y;
	};

	class SensorChanged : public Event {
	public:
		SensorChanged(sf::Sensor::Type type, float x, float y, float z) :
			Event(vr::Event::EventType::SensorChanged), type(type), x(x), y(y), z(z){};
		std::unique_ptr<Event> clone() override {
			return std::unique_ptr<vr::Event>(std::make_unique<vr::SensorChanged>(vr::SensorChanged(type, x, y, z))); 
		};

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
			Event(vr::Event::EventType::NetworkJoystickButtonPressed), joystickId(joystickId), button(button) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::NetworkJoystickButtonPressed>(vr::NetworkJoystickButtonPressed(joystickId, button))); 
		};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class NetworkJoystickButtonHeld : public Event {
	public:
		NetworkJoystickButtonHeld(unsigned int joystickId, unsigned int button) :
			Event(vr::Event::EventType::NetworkJoystickButtonHeld), joystickId(joystickId), button(button) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::NetworkJoystickButtonHeld>(vr::NetworkJoystickButtonHeld(joystickId, button)));
		};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class NetworkJoystickButtonReleased : public Event {
	public:
		NetworkJoystickButtonReleased(unsigned int joystickId, unsigned int button) :
			Event(vr::Event::EventType::NetworkJoystickButtonReleased), joystickId(joystickId), button(button) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::NetworkJoystickButtonReleased>(vr::NetworkJoystickButtonReleased(joystickId, button)));
		};

		unsigned int		joystickId;
		unsigned int		button;
	};

	class NetworkJoystickMoved : public Event {
	public:
		NetworkJoystickMoved(sf::Joystick::Axis axis, unsigned int joystickId, float position) :
			Event(vr::Event::EventType::NetworkJoystickMoved), axis(axis), joystickId(joystickId), position(position){};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::NetworkJoystickMoved>(vr::NetworkJoystickMoved(axis, joystickId, position))); 
		};

		sf::Joystick::Axis	axis;
		unsigned int		joystickId;
		float				position;
	};

	class NetworkJoystickConnected : public Event {
	public:
		NetworkJoystickConnected(unsigned int joystickId) :
			Event(vr::Event::EventType::NetworkJoystickConnected), joystickId(joystickId){};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::NetworkJoystickConnected>(vr::NetworkJoystickConnected(joystickId))); 
		};

		unsigned int		joystickId;
	};

	class NetworkJoystickDisconnected : public Event {
	public:
		NetworkJoystickDisconnected(unsigned int joystickId) :
			Event(vr::Event::EventType::NetworkJoystickDisconnected), joystickId(joystickId) {};
		std::unique_ptr<Event> clone() override { 
			return std::unique_ptr<vr::Event>(std::make_unique<vr::NetworkJoystickDisconnected>(vr::NetworkJoystickDisconnected(joystickId))); 
		};

		unsigned int		joystickId;
	};



}

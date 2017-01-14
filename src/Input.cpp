#include "Input.h"


Input::Input() :
	keyboard_flags(sf::Keyboard::Key::KeyCount, false),
	mouse_flags(sf::Mouse::Button::ButtonCount, false)
{

}

Input::~Input()
{

}

void Input::consume_sf_events(sf::RenderWindow *window) {

	std::list<sf::Event> sf_event_queue;

	sf::Event e;
	while (window->pollEvent(e)) {
		sf_event_queue.push_back(e);
	}

	transpose_sf_events(sf_event_queue);

}

void Input::consume_vr_events() {

}

void Input::set_flags() {

	for (auto e: event_queue) {
		if (e.type == sf::Event::KeyPressed) {
			held_keys.push_back(e.key.code);
		}
		else if (e.type == sf::Event::KeyReleased) {
			std::remove(held_keys.begin(), held_keys.end(), e.key.code);
		}
	}
}

void Input::dispatch_events() {

	while (event_queue.size() != 0) {
		notify(event_queue.front());
		event_queue.pop_front();
	}
}

void Input::transpose_sf_events(std::list<sf::Event> sf_event_queue) {
	
	for (auto sf_event: sf_event_queue) {

		vr::Event vr_event;

		switch(sf_event.type) {
		
			case sf::Event::Closed : {
				event_queue.push_back(vr::Closed());
			};
			case sf::Event::Resized: {
				event_queue.push_back(vr::Resized());
			};
			case sf::Event::LostFocus: {
				vr_event = vr::Closed();
			};
			case sf::Event::GainedFocus: {
				vr_event = vr::Closed();
			};

			case sf::Event::TextEntered: {
				vr_event = vr::Closed();
			};
			case sf::Event::KeyPressed: {
				vr_event = vr::Closed();
			};
			case sf::Event::KeyReleased: {
				vr_event = vr::Closed();
			};
			case sf::Event::MouseWheelMoved: {
				vr_event = vr::Closed();
			};
			case sf::Event::Closed: {
				vr_event = vr::Closed();
			};
			case sf::Event::Closed: {
				vr_event = vr::Closed();
			};

		}
		if (e.type == sf::Event::Closed ||
			e.type == sf::Event::LostFocus ||
			e.type == sf::Event::GainedFocus) {

			event_class = Event_Class::WindowEvent;
		}

		// Preserve a little of sfml's default behavior and separate resized event
		else if (e.type == sf::Event::Resized) {
			event_class = Event_Class::SizeEvent;
		}

		else if (e.type == sf::Event::TextEntered) {
			event_class = Event_Class::TextEvent;
		}

		else if (e.type == sf::Event::KeyPressed ||
			e.type == sf::Event::KeyReleased) {

			event_class = Event_Class::KeyEvent;
		}

		else if (e.type == sf::Event::MouseWheelMoved ||
			e.type == sf::Event::MouseWheelScrolled) {

			event_class = Event_Class::MouseWheelScrollEvent;
		}

		else if (e.type == sf::Event::MouseButtonPressed ||
			e.type == sf::Event::MouseButtonReleased) {

			event_class = Event_Class::MouseButtonEvent;
		}

		// Is this a good idea, mixing events that contain data, and don't contain data?
		else if (e.type == sf::Event::MouseMoved ||
			e.type == sf::Event::MouseEntered ||
			e.type == sf::Event::MouseLeft) {

			event_class = Event_Class::MouseMoveEvent;
		}

		else if (e.type == sf::Event::JoystickButtonPressed ||
			e.type == sf::Event::JoystickButtonReleased) {

			event_class = Event_Class::JoystickButtonEvent;
		}

		else if (e.type == sf::Event::JoystickMoved) {
			event_class = Event_Class::JoystickMoveEvent;
		}

		else if (e.type == sf::Event::JoystickConnected ||
			e.type == sf::Event::JoystickDisconnected) {

			event_class = Event_Class::JoystickConnectEvent;
		}

		else if (e.type == sf::Event::TouchBegan ||
			e.type == sf::Event::TouchEnded ||
			e.type == sf::Event::TouchMoved) {

			event_class = Event_Class::TouchEvent;
		}

		else if (e.type == sf::Event::SensorChanged) {
			event_class = Event_Class::SensorEvent;
		}
	}


}

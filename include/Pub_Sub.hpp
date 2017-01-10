#pragma once
#include <SFML/Graphics.hpp>


class SfEventPublisher;

class SfEventSubscriber {
public:
	virtual ~SfEventSubscriber() {};
	virtual void update(SfEventPublisher* p, sf::Event e) = 0;
};

class SfEventPublisher {
public:

	// Allows the subscription to classes of events
	enum Event_Class {
		JoystickButtonEvent,
		JoystickConnectEvent,
		JoystickMoveEvent,
		KeyEvent,
		MouseButtonEvent,
		MouseMoveEvent,
		MouseWheelEvent,
		MouseWheelScrollEvent,
		SensorEvent,
		SizeEvent,
		TextEvent,
		TouchEvent,
		WindowEvent
	};

	virtual ~SfEventPublisher() {};
	virtual void subscribe(SfEventSubscriber *s, Event_Class c) {
		subscribers[c].push_back(s);
	};
	virtual void unsubscribe(SfEventSubscriber *s, Event_Class c) {
		std::remove(subscribers[c].begin(), subscribers[c].end(), s);
	};
	virtual void notify(sf::Event e) {

		// I don't quite like that some event classes contained multiple types of events:
		// KeyPressed, and KeyReleased under KeyEvent for example.
		// While others are not represented by classes:
		// Closed, LostFocus, and GainedFocus have no representing class.
		// So, we'll add another "class", WindowEvent which contains those previously mentioned,
		// and use those new identifiers to group our events

		// This will also make it a bit easier when subscribing to certain events

		Event_Class event_class;

		if (e.type == sf::Event::Closed      ||
			e.type == sf::Event::LostFocus   ||
			e.type == sf::Event::GainedFocus ){

			event_class = Event_Class::WindowEvent;
		}

		// Preserve a little of sfml's default behavior and separate resized event
		else if (e.type == sf::Event::Resized) {
			event_class = Event_Class::SizeEvent;
		}

		else if (e.type == sf::Event::TextEntered) {
			event_class = Event_Class::TextEvent;
		}

		else if (e.type == sf::Event::KeyPressed  ||
				 e.type == sf::Event::KeyReleased ){

			event_class = Event_Class::KeyEvent;
		}

		else if (e.type == sf::Event::MouseWheelMoved    ||
				 e.type == sf::Event::MouseWheelScrolled ){

			event_class = Event_Class::MouseWheelScrollEvent;
		}

		else if (e.type == sf::Event::MouseButtonPressed  ||
				 e.type == sf::Event::MouseButtonReleased ){

			event_class = Event_Class::MouseButtonEvent;
		}

		// Is this a good idea, mixing events that contain data, and don't contain data?
		else if (e.type == sf::Event::MouseMoved   ||
				 e.type == sf::Event::MouseEntered ||
				 e.type == sf::Event::MouseLeft    ){

			event_class = Event_Class::MouseMoveEvent;
		}

		else if (e.type == sf::Event::JoystickButtonPressed  ||
				 e.type == sf::Event::JoystickButtonReleased ){

			event_class = Event_Class::JoystickButtonEvent;
		}

		else if (e.type == sf::Event::JoystickMoved) {
			event_class = Event_Class::JoystickMoveEvent;
		}

		else if (e.type == sf::Event::JoystickConnected    ||
				 e.type == sf::Event::JoystickDisconnected ){

			event_class = Event_Class::JoystickConnectEvent;
		}

		else if (e.type == sf::Event::TouchBegan ||
				 e.type == sf::Event::TouchEnded ||
				 e.type == sf::Event::TouchMoved ){
			
			event_class = Event_Class::TouchEvent;
		}

		else if (e.type == sf::Event::SensorChanged) {
			event_class = Event_Class::SensorEvent;
		}


		std::vector<SfEventSubscriber*> *event_type_bucket = &subscribers[event_class];

		for (auto s : *event_type_bucket) {
			s->update(this, e);
		}
	};
private:
	std::map<Event_Class, std::vector<SfEventSubscriber*>> subscribers;

};


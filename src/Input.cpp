#include "Input.h"
#include <iostream>
#include <memory>
#include "imgui/imgui-SFML.h"


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
		
		ImGui::SFML::ProcessEvent(e);
		sf_event_queue.push_back(e);
	}

	transpose_sf_events(sf_event_queue);

	sf_event_queue.clear();

}

void Input::consume_vr_events() {

}

void Input::handle_held_keys() {

	// When keys and buttons are pressed, add them to the held list.
	// When they are depressed, remove them

	for (auto&& event: event_queue) {

		// Key
		if (event->type == vr::Event::KeyPressed) {
			vr::KeyPressed *e = static_cast<vr::KeyPressed*>(event.get());
			held_keys.push_back(e->code);
		}
		else if (event->type == vr::Event::KeyReleased) {
			vr::KeyReleased *e = static_cast<vr::KeyReleased*>(event.get());
			held_keys.erase(std::remove(held_keys.begin(), held_keys.end(), e->code), held_keys.end());
		}

		// Mouse Button
		else if (event->type == vr::Event::MouseButtonPressed) {
			vr::MouseButtonPressed *e = static_cast<vr::MouseButtonPressed*>(event.get());
			held_mouse_buttons.push_back(e->button);
		}
		else if (event->type == vr::Event::MouseButtonReleased) {
			vr::MouseButtonReleased *e = static_cast<vr::MouseButtonReleased*>(event.get());
			held_keys.erase(std::remove(held_keys.begin(), held_keys.end(), e->button), held_keys.end());
		}
	}

	// Generate Held events for each of the held buttons and keys

	for (auto key : held_keys) {

		// Not sure if this is a good idea, but I'm going to grab
		// the real-time values of the mod keys and add them to the event

		bool alt = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) || sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt)) {
			alt = true;
		}
		
		bool control = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			control = true;
		}

		bool shift = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
			shift = true;
		}

		bool system = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LSystem) || sf::Keyboard::isKeyPressed(sf::Keyboard::RSystem)) {
			system = true;
		}
		
		event_queue.push_back(std::make_unique<vr::KeyHeld>(vr::KeyHeld(key, alt, control, shift, system)));
	}


	for (auto mouse_button : held_mouse_buttons) {
		
		// Again, I'm going to poll the real-time status of this event
		// to fill in the X and Y values. I can do this either with screen
		// co-ords or with viewport co-ords. I don't have access to the window
		// from here so for now I'm going to do screen co-ords

		sf::Vector2i mouse_pos = sf::Mouse::getPosition();

		event_queue.push_back(std::make_unique<vr::MouseButtonHeld>(vr::MouseButtonHeld(mouse_button, mouse_pos.x, mouse_pos.y)));
	}

}

void Input::dispatch_events() {

	while (event_queue.size() != 0) {
		notify_subscribers(std::move(event_queue.front()));
		event_queue.pop_front();
	}

}

void Input::transpose_sf_events(std::list<sf::Event> sf_event_queue) {
	
	for (auto sf_event: sf_event_queue) {

		switch(sf_event.type) {
		
			case sf::Event::Closed : {
				event_queue.push_back(std::make_unique<vr::Closed>(vr::Closed()));
				break;
			};
			case sf::Event::Resized: {
				event_queue.push_back(std::make_unique<vr::Resized>(vr::Resized(sf_event.size.width, sf_event.size.height)));
				break;
			};
			case sf::Event::LostFocus: {
				event_queue.push_back(std::make_unique<vr::LostFocus>(vr::LostFocus()));
				break;
			};
			case sf::Event::GainedFocus: {
				event_queue.push_back(std::make_unique<vr::GainedFocus>(vr::GainedFocus()));
				break;
			};
			case sf::Event::TextEntered: {
				event_queue.push_back(std::make_unique<vr::TextEntered>(vr::TextEntered(sf_event.text.unicode)));
				break;
			};
			case sf::Event::KeyPressed: {
				event_queue.push_back(std::make_unique<vr::KeyPressed>(vr::KeyPressed(sf_event.key.code, sf_event.key.alt, sf_event.key.control, sf_event.key.shift, sf_event.key.system)));
				break;
			};
			case sf::Event::KeyReleased: {
				event_queue.push_back(std::make_unique<vr::KeyReleased>(vr::KeyReleased(sf_event.key.code, sf_event.key.alt, sf_event.key.control, sf_event.key.shift, sf_event.key.system)));
				break;
			};

			// Mouse wheel moved will generate a MouseWheelScrolled event with the defaul vertical wheel
			case sf::Event::MouseWheelMoved: {
				event_queue.push_back(std::make_unique<vr::MouseWheelScrolled>(vr::MouseWheelScrolled(sf::Mouse::VerticalWheel, sf_event.mouseWheelScroll.delta, sf_event.mouseWheelScroll.x, sf_event.mouseWheelScroll.y)));
				break;
			};
			case sf::Event::MouseWheelScrolled: {
				event_queue.push_back(std::make_unique<vr::MouseWheelScrolled>(vr::MouseWheelScrolled(sf_event.mouseWheelScroll.wheel, sf_event.mouseWheelScroll.delta, sf_event.mouseWheelScroll.x, sf_event.mouseWheelScroll.y)));
				break;
			};
			case sf::Event::MouseButtonPressed: {
				event_queue.push_back(std::make_unique<vr::MouseButtonPressed>(vr::MouseButtonPressed(sf_event.mouseButton.button, sf_event.mouseButton.x, sf_event.mouseButton.y)));
				break;
			};
			case sf::Event::MouseButtonReleased: {
				event_queue.push_back(std::make_unique<vr::MouseButtonReleased>(vr::MouseButtonReleased(sf_event.mouseButton.button, sf_event.mouseButton.x, sf_event.mouseButton.y)));
				break;
			};
			case sf::Event::MouseMoved: {
				event_queue.push_back(std::make_unique<vr::MouseMoved>(vr::MouseMoved(sf_event.mouseMove.x, sf_event.mouseMove.y)));
				break;
			};
			case sf::Event::MouseEntered: {
				event_queue.push_back(std::make_unique<vr::MouseEntered>(vr::MouseEntered(sf_event.mouseMove.x, sf_event.mouseMove.y)));
				break;
			};
			case sf::Event::MouseLeft: {
				event_queue.push_back(std::make_unique<vr::MouseLeft>(vr::MouseLeft(sf_event.mouseMove.x, sf_event.mouseMove.x)));
				break;
			};
			case sf::Event::JoystickButtonPressed: {
				event_queue.push_back(std::make_unique<vr::JoystickButtonPressed>(vr::JoystickButtonPressed(sf_event.joystickButton.joystickId, sf_event.joystickButton.button)));
				break;
			};
			case sf::Event::JoystickButtonReleased: {
				event_queue.push_back(std::make_unique<vr::JoystickButtonReleased>(vr::JoystickButtonReleased(sf_event.joystickButton.joystickId, sf_event.joystickButton.button)));
				break;
			};
			case sf::Event::JoystickMoved: {
				event_queue.push_back(std::make_unique<vr::JoystickMoved>(vr::JoystickMoved(sf_event.joystickMove.axis, sf_event.joystickMove.joystickId, sf_event.joystickMove.position)));
				break;
			};
			case sf::Event::JoystickConnected: {
				event_queue.push_back(std::make_unique<vr::JoystickConnected>(vr::JoystickConnected(sf_event.joystickConnect.joystickId)));
				break;
			};
			case sf::Event::JoystickDisconnected: {
				event_queue.push_back(std::make_unique<vr::JoystickDisconnected>(vr::JoystickDisconnected(sf_event.joystickConnect.joystickId)));
				break;
			};
			case sf::Event::TouchBegan: {
				event_queue.push_back(std::make_unique<vr::TouchBegan>(vr::TouchBegan(sf_event.touch.finger, sf_event.touch.x, sf_event.touch.y)));
				break;
			};
			case sf::Event::TouchMoved: {
				event_queue.push_back(std::make_unique<vr::TouchMoved>(vr::TouchMoved(sf_event.touch.finger, sf_event.touch.x, sf_event.touch.y)));
				break;
			};
			case sf::Event::TouchEnded: {
				event_queue.push_back(std::make_unique<vr::TouchEnded>(vr::TouchEnded(sf_event.touch.finger, sf_event.touch.x, sf_event.touch.y)));
				break;
			};
			case sf::Event::SensorChanged: {
				event_queue.push_back(std::make_unique<vr::SensorChanged>(vr::SensorChanged(sf_event.sensor.type, sf_event.sensor.x, sf_event.sensor.y, sf_event.sensor.z)));
				break;
			};
			default: {
				std::cout << "Event not recognized";
				abort();
				break;
			}
		}
	}
}


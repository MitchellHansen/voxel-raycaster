#include "Input.h"


Input::Input() :
	keyboard_flags(sf::Keyboard::Key::KeyCount, false),
	mouse_flags(sf::Mouse::Button::ButtonCount, false)
{

}

Input::~Input()
{

}

void Input::consume_events(sf::RenderWindow *window) {

	sf::Event e;
	while (window->pollEvent(e)) {
		event_queue.push_back(e);
	}

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

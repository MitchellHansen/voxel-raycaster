#pragma once
#include <SFML/Graphics.hpp>
#include <list>
#include "Event.hpp"
#include <memory>
#include "Pub_Sub.h"


class Input : public VrEventPublisher {
public:
	
	Input();
	~Input();

	// Keep track of keys that are not released
	// Keep track of mouse up and downs in conjunction with dragging
	// Keep track of joystick buttons
	void consume_sf_events(sf::RenderWindow *window);
	void consume_vr_events();
	
	void handle_held_keys();
	void dispatch_events();
	
private:

	void transpose_sf_events(std::list<sf::Event> event_queue);

	std::vector<sf::Keyboard::Key> held_keys;
	std::vector<sf::Mouse::Button> held_mouse_buttons;
	
	std::vector<bool> keyboard_flags;
	std::vector<bool> mouse_flags;

private:
	
	std::list<std::unique_ptr<vr::Event>> event_queue;
};

class WindowHandler : public VrEventSubscriber {
	
public:
	WindowHandler(sf::RenderWindow *window) : window_ref(window) { };

	virtual void recieve_event(VrEventPublisher* publisher, std::unique_ptr<vr::Event>(event)) override {
		if (event.get()->type == vr::Event::Closed) {
			window_ref->close();
		}
	};

private:
	sf::RenderWindow* window_ref;

};
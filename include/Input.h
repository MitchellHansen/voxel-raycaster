#pragma once
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <SFML/Graphics.hpp>
#include "Event.hpp"
#include "Gui.h"
#include "LightHandle.h"
#include "Logger.h"
#include "Pub_Sub.h"
#include <imgui/imgui-SFML.h>
/**
 *
 * Input
 *
 * For each frame the Application must call
 *
 * consume_sf_events(*window)
 * handle_held_keys()
 * dispatch_events()
 *
 * which will pull all the events from the sfml event queue, transpose them over
 * to vr:events, compare to the last frame and create held key events for keys held
 * for longer than one frame, and finally dispatch the events to the relevent VrEventListener's
 *
 */


class Input : public VrEventPublisher, private Gui{
public:
	
	Input();
	~Input();

	void consume_sf_events(sf::RenderWindow *window);
	void consume_vr_events();
	
	void handle_held_keys();
	void dispatch_events();
	

	virtual void render_gui() override;
	virtual void update_gui() override;

private:

	void transpose_sf_events(std::list<sf::Event> event_queue);

	std::vector<sf::Keyboard::Key> held_keys;
	std::vector<sf::Mouse::Button> held_mouse_buttons;


private:
	
	static const std::vector<std::string> key_strings;

	std::list<std::unique_ptr<vr::Event>> event_queue;
};

class WindowHandler : public VrEventSubscriber {
	
public:
	WindowHandler(sf::RenderWindow *window) : window_ref(window) { };

	virtual void event_handler(VrEventPublisher *publisher, std::unique_ptr<vr::Event>(event)) override {
		if (event.get()->type == vr::Event::Closed) {
			window_ref->close();

		} else if (event.get()->type == vr::Event::KeyPressed) {

			vr::KeyPressed *key_event = static_cast<vr::KeyPressed*>(event.get());

			if (key_event->code == sf::Keyboard::Escape) {
				window_ref->close();
			}
		}
	};

private:
	sf::RenderWindow* window_ref;

};
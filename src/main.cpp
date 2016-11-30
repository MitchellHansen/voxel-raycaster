#include "GL_Testing.h"

#ifdef linux
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined _WIN32
#include <windows.h>
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined TARGET_OS_MAC
#include <OpenGL/gl.h>
# include <OpenGL/OpenGL.h>
# include <OpenCL/opencl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenCL/cl_ext.h>

#endif

#pragma once
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>


#include <SFML/Graphics.hpp>
#include "Old_Map.h"
#include "util.hpp"
#include "RayCaster.h"
#include "Hardware_Caster.h"
#include "Vector4.hpp"
#include <Camera.h>
#include "Software_Caster.h"


const int WINDOW_X = 1920;
const int WINDOW_Y = 1080;
const int WORK_SIZE = WINDOW_X * WINDOW_Y;

const int MAP_X = 512;
const int MAP_Y = 512;
const int MAP_Z = 512;

float elap_time(){
	static std::chrono::time_point<std::chrono::system_clock> start;
	static bool started = false;

	if (!started){
		start = std::chrono::system_clock::now();
		started = true;
	}

	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_time = now - start;
	return static_cast<float>(elapsed_time.count());
}

sf::Sprite window_sprite;
sf::Texture window_texture;

// Y: -1.57 is straight up
// Y: 1.57 is straight down


int main() {

	sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "SFML");

	GL_Testing t;
	t.compile_shader("../shaders/passthrough.frag", GL_Testing::Shader_Type::FRAGMENT);
	t.compile_shader("../shaders/passthrough.vert", GL_Testing::Shader_Type::VERTEX);
	t.create_program();
	t.create_buffers();

	// Initialize the raycaster hardware, compat, or software
	RayCaster *rc = new Hardware_Caster();
	//RayCaster *rc = new Software_Caster();
	if (rc->init() != 1) {
		delete rc;
		// rc = new Hardware_Caster_Compat();
		// if (rc->init() != 0) {
		//		delete rc;
		//		rc = new Software_Caster();
		// }
	}

	// Set up the raycaster

	std::cout << "map...";
	sf::Vector3i map_dim(MAP_X, MAP_Y, MAP_Z);
	Old_Map* map = new Old_Map(map_dim);
	map->generate_terrain();

	rc->assign_map(map);

	Camera *camera = new Camera(
		sf::Vector3f(10, 11, 12),
		sf::Vector2f(0.1f, 1.00f)
	);

	rc->assign_camera(camera);

	rc->create_viewport(WINDOW_X, WINDOW_Y, 50.0f, 80.0f);

	Light l;
	l.direction_cartesian = sf::Vector3f(1.5f, 1.2f, 0.5f);
	l.position = sf::Vector3f(100.0f, 100.0f, 100.0f);
	l.rgbi = sf::Vector4f(0.3f, 0.4f, 0.3f, 1.0f);

	rc->assign_lights(std::vector<Light>{l});

	rc->validate();

	// Done setting up raycaster

	// ========== DEBUG ==========
    fps_counter fps;

	sf::Vector2f *dp = camera->get_direction_pointer();
	debug_text cam_text_x(1, 30, &dp->x, "incli: ");
	debug_text cam_text_y(2, 30, &dp->y, "asmth: ");
	// ===========================


	// 16.6 milliseconds (60FPS)
	float step_size = 0.0166f;
	double  frame_time = 0.0,
			elapsed_time = 0.0,
			delta_time = 0.0,
			accumulator_time = 0.0,
			current_time = 0.0;

	// Mouse capture
	sf::Vector2i deltas;
	sf::Vector2i fixed(window.getSize());
	sf::Vector2i prev_pos;
	bool mouse_enabled = true;
	bool reset = false;

	while (window.isOpen()) {

		// Poll for events from the user
		sf::Event event;
		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Space) {
					if (mouse_enabled)
						mouse_enabled = false;
					else
						mouse_enabled = true;
				} if (event.key.code == sf::Keyboard::R) {
					reset = true;
				}
			}
		}

		float speed = 1.0f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
			speed = 0.2f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
			camera->add_relative_impulse(Camera::DIRECTION::DOWN, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
			camera->add_relative_impulse(Camera::DIRECTION::UP, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			camera->add_relative_impulse(Camera::DIRECTION::FORWARD, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			camera->add_relative_impulse(Camera::DIRECTION::REARWARD, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			camera->add_relative_impulse(Camera::DIRECTION::LEFT, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			camera->add_relative_impulse(Camera::DIRECTION::RIGHT, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
			camera->set_position(sf::Vector3f(50, 50, 50));
		}

		if (mouse_enabled) {
			if (reset) {
				reset = false;
				sf::Mouse::setPosition(sf::Vector2i(2560/2, 1080/2));
				prev_pos = sf::Vector2i(2560 / 2, 1080 / 2);
			}

			deltas = prev_pos - sf::Mouse::getPosition();
			if (deltas != sf::Vector2i(0, 0) && mouse_enabled == true) {

				// Mouse movement
				sf::Mouse::setPosition(fixed);
				prev_pos = sf::Mouse::getPosition();
				camera->slew_camera(sf::Vector2f(
					deltas.y / 300.0f,
					deltas.x / 300.0f
				));
			}
		}

        // Time keeping
		elapsed_time = elap_time();
		delta_time = elapsed_time - current_time;
		current_time = elapsed_time;
		if (delta_time > 0.2f)
			delta_time = 0.2f;
		accumulator_time += delta_time;
		while ((accumulator_time - step_size) >= step_size) {
            accumulator_time -= step_size;

            // ==== DELTA TIME LOCKED ====
        }

        // ==== FPS LOCKED ====
		camera->update(delta_time);

		window.clear(sf::Color::Black);

		// Run the raycast
		rc->compute();
		rc->draw(&window);
		
		window.popGLStates();

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//t.rotate(delta_time);
		//t.transform();
		//t.draw();
		
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		window.pushGLStates();

		// Give the frame counter the frame time and draw the average frame time
		fps.frame(delta_time);
		fps.draw(&window);

		cam_text_x.draw(&window);
		cam_text_y.draw(&window);

		window.display();

	}
	return 0;
}

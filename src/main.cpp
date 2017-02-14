
// This has to be up here or else glew will complain
#include "GL_Testing.h"

#ifdef linux
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined _WIN32
#include <windows.h>

// As if hardware is ever going to move away from 1.2
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
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
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "Old_Map.h"
#include "util.hpp"
#include "raycaster/RayCaster.h"
#include "raycaster/Hardware_Caster.h"
#include "Vector4.hpp"
#include "Camera.h"
#include "Input.h"
#include "Pub_Sub.h"
#include "NetworkInput.h"
#include "LightController.h"
#include "LightHandle.h"

const int WINDOW_X = 1440;
const int WINDOW_Y = 900;
const int WORK_SIZE = WINDOW_X * WINDOW_Y;

const int MAP_X = 256;
const int MAP_Y = 256;
const int MAP_Z = 256;

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

	// Keep at this at the top of main. I think it has to do with it and
	// sf::RenderWindow stepping on each others feet
	glewInit();

	// The socket listener for interacting with the TCP streaming android controller
	NetworkInput ni;
	ni.listen_for_clients(5000);

	// Currently just close it right away for debug
	ni.stop_listening_for_clients();

	sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "SFML");
	window.setMouseCursorVisible(false);

	// =============================
	Map _map(sf::Vector3i(0, 0, 0));
	_map.generate_octree();
	// =============================



	/*GL_Testing t;
	t.compile_shader("../shaders/passthrough.frag", GL_Testing::Shader_Type::FRAGMENT);
	t.compile_shader("../shaders/passthrough.vert", GL_Testing::Shader_Type::VERTEX);
	t.create_program();
	t.create_buffers();*/

	// Start up the raycaster
	std::shared_ptr<Hardware_Caster> raycaster(new Hardware_Caster());
	
	if (raycaster->init() != 1) {
		abort();
	}

	// Create and generate the old 3d array style map
	Old_Map* map = new Old_Map(sf::Vector3i(MAP_X, MAP_Y, MAP_Z));
	map->generate_terrain();

	// Send the data to the GPU
	raycaster->assign_map(map);

	// Create a new camera with (starting position, direction)
	Camera *camera = new Camera(
		sf::Vector3f(50, 50, 50),
		sf::Vector2f(1.5f, 0.0f),
		&window
	);

	// *link* the camera to the GPU
	raycaster->assign_camera(camera);

	// Generate and send the viewport to the GPU. Also creates the viewport texture
	raycaster->create_viewport(WINDOW_X, WINDOW_Y, 0.625f * 90.0f, 90.0f);

	float w = 60.0;
	float h = 90.0;


	LightController light_controller(raycaster);
	LightPrototype prototype(
		sf::Vector3f(100.0f, 100.0f, 30.0f),
		sf::Vector3f(-1.0f, -1.0f, -1.5f),
		sf::Vector4f(1.0f, 1.0f, 1.0f, 1.0f)
	);
	
	std::shared_ptr<LightHandle> handle(light_controller.create_light(prototype));


	// Load in the spritesheet texture
	sf::Texture spritesheet;
	spritesheet.loadFromFile("../assets/textures/minecraft_tiles.png");
	spritesheet.getNativeHandle();
	raycaster->create_texture_atlas(&spritesheet, sf::Vector2i(16, 16));


	// Checks to see if proper data was uploaded, then sets the kernel args
	// ALL DATA LOADING MUST BE FINISHED
	raycaster->validate();

	// ========== DEBUG ==========
    fps_counter fps;

	sf::Vector2f *dp = camera->get_direction_pointer();
	debug_text cam_text_x(1, 30, &dp->x, "incli: ");
	debug_text cam_text_y(2, 30, &dp->y, "asmth: ");
	debug_text cam_text_pos_x(3, 30, &camera->get_position_pointer()->x, "x: ");
	debug_text cam_text_pos_y(4, 30, &camera->get_position_pointer()->y, "y: ");
	debug_text cam_text_pos_z(5, 30, &camera->get_position_pointer()->z, "z: ");
	// ===========================


	// 16.6 milliseconds (60FPS)
	float step_size = 0.0166f;
	double  frame_time = 0.0,
			elapsed_time = 0.0,
			delta_time = 0.0,
			accumulator_time = 0.0,
			current_time = 0.0;


	Input input_handler;

	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::KeyHeld);
	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::KeyPressed);
	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::MouseMoved);
	handle->subscribe_to_publisher(&ni, vr::Event::EventType::JoystickMoved);

	WindowHandler win_hand(&window);
	win_hand.subscribe_to_publisher(&input_handler, vr::Event::EventType::Closed);

	window.setKeyRepeatEnabled(false);

	while (window.isOpen()) {

		input_handler.consume_sf_events(&window);
		input_handler.handle_held_keys();
		input_handler.dispatch_events();
		ni.dispatch_events();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F11)) {
			while (raycaster->debug_quick_recompile() != 0);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0)) {
			std::string path = "../assets/";
			std::string filename;
			std::getline(std::cin, filename);
			filename += ".png";

			sf::Texture window_texture;
			window_texture.create(window.getSize().x, window.getSize().y);
			window_texture.update(window);

			sf::Image image = window_texture.copyToImage();
			image.saveToFile(path + filename);
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
		handle->update(delta_time);

		window.clear(sf::Color::Black);

		// Run the raycast
		raycaster->compute();
		raycaster->draw(&window);
		
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

		cam_text_pos_x.draw(&window);
		cam_text_pos_y.draw(&window);
		cam_text_pos_z.draw(&window);

		window.display();

	}
	return 0;
}

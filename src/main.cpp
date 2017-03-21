
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
#include "util.hpp"
#include <iostream>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "Old_Map.h"
#include "raycaster/RayCaster.h"
#include "raycaster/Hardware_Caster.h"
#include "Vector4.hpp"
#include "Camera.h"
#include "Input.h"
#include "Pub_Sub.h"
#include "LightController.h"
#include "LightHandle.h"
#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"


const int WINDOW_X = 1440;
const int WINDOW_Y = 900;
const int WORK_SIZE = WINDOW_X * WINDOW_Y;

const int MAP_X = 512;
const int MAP_Y = 512;
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


// TODO: 
// - Texture axis sign flipping issue
// - Diffuse fog hard cut off
// - Infinite light distance, no inverse square
// - Inconsistent lighting constants. GUI manipulation

int main() {

	// Keep at this at the top of main. I think it has to do with it and
	// sf::RenderWindow stepping on each others feet
	#ifdef linux
	glewInit();
	#elif defined _WIN32
	glewInit();
	#elif defined TARGET_OS_MAC
	#endif 

	// The socket listener for interacting with the TCP streaming android controller
	// NetworkInput ni;
	// ni.listen_for_clients(5000);
	// ni.stop_listening_for_clients();

	// =============================
	// Map _map(sf::Vector3i(0, 0, 0));
	// _map.generate_octree();
	// _map.a.get_voxel(sf::Vector3i(5, 5, 0));
	// return 0;
	// =============================

    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "SFML");
    window.setMouseCursorVisible(false);
	window.setKeyRepeatEnabled(false);
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(false);
	
	ImGui::SFML::Init(window);
	window.resetGLStates();

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
		sf::Vector3f(100.0f, 100.0f, 75.0f),
		sf::Vector3f(-1.0f, -1.0f, -1.5f),
		sf::Vector4f(0.2f, 0.9f, 0.0f, 1.0f)
	);
	
	std::shared_ptr<LightHandle> handle(light_controller.create_light(prototype));

	// Load in the spritesheet texture
	sf::Texture spritesheet;
	spritesheet.loadFromFile("../assets/textures/minecraft_tiles.png");
	raycaster->create_texture_atlas(&spritesheet, sf::Vector2i(16, 16));

	// Checks to see if proper data was uploaded, then sets the kernel args
	// ALL DATA LOADING MUST BE FINISHED
	raycaster->validate();

	Input input_handler;
	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::KeyHeld);
	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::KeyPressed);
	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::MouseMoved);

	WindowHandler win_hand(&window);
	win_hand.subscribe_to_publisher(&input_handler, vr::Event::EventType::Closed);


	float step_size = 0.0166f;
	double  frame_time = 0.0,
		elapsed_time = 0.0,
		delta_time = 0.0,
		accumulator_time = 0.0,
		current_time = 0.0;

	// The sfml imgui wrapper I'm using requires Update be called with sf::Time
	// Might modify it to also accept seconds
	sf::Clock sf_delta_clock;
	fps_counter fps;

	float light_color[4] = { 0, 0, 0, 0 };
	float light_pos[4] = { 100, 100, 30 };
	char screenshot_buf[128]{0};

	while (window.isOpen()) {

		input_handler.consume_sf_events(&window);
		input_handler.handle_held_keys();
		input_handler.dispatch_events();

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

		ImGui::SFML::Update(window, sf_delta_clock.restart());
		camera->update(delta_time);
		handle->update(delta_time);

		// Run the raycast
		raycaster->compute();

		window.clear(sf::Color::Black);
		raycaster->draw(&window);

		// Give the frame counter the frame time and draw the average frame time
		fps.frame(delta_time);
		fps.draw();

		ImGui::Begin("Camera");
		
		ImGui::Columns(2);
		
		ImGui::Text("Camera Inclination");
		ImGui::Text("Camera Azimuth");
		ImGui::Text("Camera Pos_X");
		ImGui::Text("Camera Poz_Y");
		ImGui::Text("Camera Poz_Z");

		ImGui::NextColumn();

		sf::Vector2f dir = camera->get_direction();
		sf::Vector3f pos = camera->get_position();

		ImGui::Text(std::to_string(dir.x).c_str());
		ImGui::Text(std::to_string(dir.y).c_str());
		ImGui::Text(std::to_string(pos.x).c_str());
		ImGui::Text(std::to_string(pos.y).c_str());
		ImGui::Text(std::to_string(pos.z).c_str());

		ImGui::NextColumn();

		ImGui::InputText("filename", screenshot_buf, 128);
		if (ImGui::Button("Take Screen shot")) {
			
			std::string path = "../assets/";
			std::string filename(screenshot_buf);
			filename += ".png";

			sf::Texture window_texture;
			window_texture.create(window.getSize().x, window.getSize().y);
			window_texture.update(window);

			sf::Image image = window_texture.copyToImage();
			image.saveToFile(path + filename);
			
		}

		ImGui::NextColumn();

		if (ImGui::Button("Recompile kernel")) {
			while (raycaster->debug_quick_recompile() != 0);
		}
		ImGui::End();

		ImGui::Begin("Lights");

		if (ImGui::SliderFloat4("Color", light_color, 0, 1)) {
			sf::Vector4f light(light_color[0], light_color[1], light_color[2], light_color[3]);
			handle->set_rgbi(light);
		}

		if (ImGui::SliderFloat3("Position", light_pos, 0, MAP_X)) {
			sf::Vector3f light(light_pos[0], light_pos[1], light_pos[2]);
			handle->set_position(light);
		}

		light_pos[0] = sin(elapsed_time) * 100.0f + 300.0f;
		light_pos[1] = sin(elapsed_time) * 100.0f + 300.0f;

		sf::Vector3f light(light_pos[0], light_pos[1], light_pos[2]);
		handle->set_position(light);


		ImGui::End();


		ImGui::Render();


		window.draw(sf::CircleShape(0));
		window.display();

	}
	return 0;
}

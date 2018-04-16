#pragma once

// As if hardware is ever going to move away from 1.2
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#ifdef linux
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined _WIN32
// Good lord, windows.h overwrote the std::min() max() definitions
#define NOMINMAX
#include <windows.h>
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined TARGET_OS_MAC
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenCL/cl_ext.h>
#endif
#pragma once
#include <chrono>
#include <SFML/Graphics.hpp>
#include "Camera.h"
#include "Input.h"
#include "LightController.h"
#include "LightHandle.h"
#include "map/Map.h"
#include "util.hpp"
#include "GraphTimer.h"

// Srsly people who macro error codes are the devil
#undef ERROR
#include "Logger.h"

class CLCaster;

class Application {

public:

// 	static const int WINDOW_X = 1366;
// 	static const int WINDOW_Y = 768;
//	static const int WINDOW_X = 400;
//	static const int WINDOW_Y = 400;
    static const int WINDOW_X = 5;
    static const int WINDOW_Y = 5;
	static const int MAP_X;
	static const int MAP_Y;
	static const int MAP_Z;

	Application();
	~Application();
	
	bool init_clcaster();
	bool init_events();

	bool game_loop();

private:

	static float elap_time();

	sf::Sprite window_sprite;
	sf::Texture window_texture;
	sf::Texture spritesheet;

	std::shared_ptr<sf::RenderWindow> window;
	std::shared_ptr<Map> map;
	std::shared_ptr<Camera> camera;
	std::shared_ptr<CLCaster> raycaster;
	std::shared_ptr<LightHandle> light_handle;
	std::shared_ptr<LightController> light_controller;
    GraphTimer fps;
	Input input_handler;
	std::shared_ptr<WindowHandler> window_handler;

	// The sfml imgui wrapper I'm using requires Update be called with sf::Time
	// Might modify it to also accept seconds
	sf::Clock sf_delta_clock;


	// vars for us to use with ImGUI
	float light_color[4] = { 0, 0, 0, 0 };
	float light_pos[4] = { 100, 100, 30 };
	char screenshot_buf[128]{ 0 };
	bool paused = false;
	float camera_speed = 1.0;

	// Game loop values
	float step_size = 0.0166f;
	double frame_time = 0.0,
		elapsed_time = 0.0,
		delta_time = 0.0,
		accumulator_time = 0.0,
		current_time = 0.0;
};

#pragma once

// As if hardware is ever going to move away from 1.2
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#ifdef linux

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
//#include <SFML/Graphics.hpp>
#include "../include-legacy/Camera.h"
#include "../include-legacy/Input.h"
#include "../include-legacy/LightController.h"
#include "../include-legacy/LightHandle.h"
#include "map/Map.h"
#include "util.hpp"
#include "../include-legacy/GraphTimer.h"

// Srsly people who macro error codes are the devil
#undef ERROR
#include "Logger.h"

class CLCaster;

class Application {

public:

    static const int WINDOW_X = 50;
    static const int WINDOW_Y = 50;

	Application();
	~Application();
	
	bool init_clcaster();
	bool init_events();
	bool game_loop();

private:

	static float elap_time();

	float camera_speed = 1.0;

	// Game loop values
	float step_size 		= 0.0166f;
	double frame_time 		= 0.0;
	double elapsed_time 	= 0.0;
	double delta_time 		= 0.0;
	double accumulator_time = 0.0;
	double current_time 	= 0.0;
};

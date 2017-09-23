#pragma once
#ifdef linux
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined _WIN32
// Good lord, windows.h overwrote the std::min() max() definitions
#define NOMINMAX
#include <windows.h>

// As if hardware is ever going to move away from 1.2
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined TARGET_OS_MAC
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenCL/cl_ext.h>
#endif

#include "util.hpp"
#include <iostream>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "map/Old_Map.h"
#include "CLCaster.h"
#include "Vector4.hpp"
#include "Camera.h"
#include "Input.h"
#include "Pub_Sub.h"
#include "LightController.h"
#include "LightHandle.h"
#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"
#include "map/Map.h"

// Srsly people who macro error codes are the devil
#undef ERROR
#include "Logger.h"

class Application {

public:	
	const int WINDOW_X = 1536;
	const int WINDOW_Y = 1024;

	const int MAP_X = 256;
	const int MAP_Y = 256;
	const int MAP_Z = 256;



private:

	static float elap_time();
};
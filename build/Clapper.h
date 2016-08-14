#pragma once
#include <GL/glew.h>
#include <vector>
#include <iostream>

#ifdef linux
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined _WIN32
#include <CL/cl_gl.h>
#include <CL/cl.h>
#include <CL/opencl.h>
#include <windows.h>

#elif defined TARGET_OS_MAC
# include <OpenGL/OpenGL.h>
# include <OpenCL/opencl.h>

#endif

struct platform_tracker {
	
};

class Clapper {
public:

	enum PLATFORM_TYPE { GPU, CPU, ALL };

	Clapper();
	~Clapper();

	void init();
	void print_sys_info();


private:

	bool initialized = false;

	cl_uint platform_count;
	std::vector<cl_platform_id> platformIds;

	cl_uint deviceIdCount = 0;
	std::vector<cl_device_id> deviceIds;
};


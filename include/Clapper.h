#include <vector>
#include <iostream>
#include "util.hpp"
#include <map>

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

struct device {
    cl_device_id id;
    cl_device_type type;
    cl_uint clock_frequency;
    char version[128];
    cl_platform_id platform;
};

class Clapper {
public:

	enum PLATFORM_TYPE { GPU, CPU, ALL };

	Clapper();
	~Clapper();

    int acquire_platform_and_device();
    int create_shared_context();
    int create_command_queue();
    int compile_kernel(std::string kernel_source, bool is_path, std::string kernel_name);

    cl_device_id getDeviceID();
    cl_platform_id getPlatformID();
    cl_context getContext();

private:

    int error = 0;
	bool initialized = false;

    bool assert(int error_code, std::string function_name);
    cl_platform_id platform_id;
	cl_device_id device_id;
    cl_context context;

    std::map<std::string, cl_kernel> kernel_map;
};


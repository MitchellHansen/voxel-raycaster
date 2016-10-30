#include <vector>
#include <iostream>
#include "util.hpp"
#include <map>
#include <string.h>

#ifdef linux
#include <CL/cl.h>
#include <CL/opencl.h>
#include <GL/glx.h>

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
	cl_uint comp_units;
};

class CL_Wrapper {
public:

	CL_Wrapper();
	~CL_Wrapper();

    int acquire_platform_and_device();

	int create_shared_context();
	int create_cl_context();

    int create_command_queue();

	int check_cl_khr_gl_sharing();

    int compile_kernel(std::string kernel_source, bool is_path, std::string kernel_name);

	int create_buffer(std::string buffer_name, cl_uint size, void* data);
	int create_buffer(std::string buffer_name, cl_uint size, void* data, cl_mem_flags flags);
	int store_buffer(cl_mem, std::string buffer_name);

	int set_kernel_arg(std::string kernel_name, int index, std::string buffer_name);
	int run_kernel(std::string kernel_name, const int work_size);

	bool assert(int error_code, std::string function_name);

    cl_device_id getDeviceID();
    cl_platform_id getPlatformID();
    cl_context getContext();
    cl_kernel getKernel(std::string kernel_name);
	cl_command_queue getCommandQueue();

	bool was_init_valid();

private:

    int error = 0;
	bool initialized = false;
	bool cl_khr_gl_sharing_fallback = false;
	bool cl_supported = false;

    cl_platform_id platform_id;
	cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;

    std::map<std::string, cl_kernel> kernel_map;
    std::map<std::string, cl_mem> buffer_map;
};


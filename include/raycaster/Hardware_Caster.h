#pragma once
#include <raycaster/RayCaster.h>
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

// Note: windows.h must be included before Gl/GL.h
#include <windows.h>
#include <GL/GL.h>

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

struct PackedData;

class Hardware_Caster : public RayCaster
{
public:
	Hardware_Caster();

	virtual ~Hardware_Caster();

	int init() override;

	// In interop mode, this will create a GL texture that we share
	// Otherwise, it will create the pixel buffer and pass that in as an image, retrieving it each draw
	// Both will create the view matrix, view res buffer
	void create_viewport(int width, int height, float v_fov, float h_fov) override;
	
	void assign_lights(std::vector<PackedData> *data) override;
	void assign_map(Old_Map *map) override;
	void assign_camera(Camera *camera) override;
	void validate() override;

	// TODO: Hoist this to the base class
	void create_texture_atlas(sf::Texture *t, sf::Vector2i tile_dim);


	// draw will abstract the gl sharing and software rendering
	// methods of retrieving the screen buffer
	void compute() override;
	void draw(sf::RenderWindow* window) override;


	int debug_quick_recompile();
	void test_edit_viewport(int width, int height, float v_fov, float h_fov);
private:


	int acquire_platform_and_device();

	int create_shared_context();

	int create_command_queue();

	int check_cl_khr_gl_sharing();

	int create_image_buffer(std::string buffer_name, cl_uint size, sf::Texture* texture);
	int create_buffer(std::string buffer_name, cl_uint size, void* data);
	int create_buffer(std::string buffer_name, cl_uint size, void* data, cl_mem_flags flags);
	int store_buffer(cl_mem, std::string buffer_name);
	int release_buffer(std::string buffer_name);
	
	int compile_kernel(std::string kernel_source, bool is_path, std::string kernel_name);

	int set_kernel_arg(std::string kernel_name, int index, std::string buffer_name);

	int run_kernel(std::string kernel_name, const int work_size);

	void print_kernel_arguments();

	bool assert(int error_code, std::string function_name);

	cl_device_id getDeviceID();
	cl_platform_id getPlatformID();
	cl_context getContext();
	cl_kernel getKernel(std::string kernel_name);
	cl_command_queue getCommandQueue();

	cl_platform_id platform_id;
	cl_device_id device_id;
	cl_context context;
	cl_command_queue command_queue;

	std::map<std::string, cl_kernel> kernel_map;
	std::map<std::string, cl_mem> buffer_map;

};


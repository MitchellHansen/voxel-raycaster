#pragma once
#include <raycaster/RayCaster.h>
#include <Vector4.hpp>
#include <vector>
#include <iostream>
#include <map>
#include <string.h>
#include "LightController.h"
#include "map/Old_Map.h"
#include "Camera.h"

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
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>
#include <OpenGL/gl.h>

#endif

struct device {
	cl_device_id id;
	cl_device_type type;
	cl_uint clock_frequency;
	char version[128];
	cl_platform_id platform;
	cl_uint comp_units;
	char extensions[1024];
	char name[256];
	cl_bool is_little_endian = false;
	bool cl_gl_sharing = false;
};

struct device_info {
	cl_uint cl_device_address_bits;
	cl_bool cl_device_available;
	cl_bool cl_device_compiler_available;
	cl_bool cl_device_endian_little;
	cl_bool cl_device_error_correction_support;
	char cl_device_extensions[1024];
	cl_ulong cl_device_global_mem_cache_size;
	cl_uint cl_device_global_mem_cacheline_size;
	cl_ulong cl_device_global_mem_size;
	cl_bool cl_device_image_support;
	size_t cl_device_image2d_max_height;
	size_t cl_device_image2d_max_width;
	size_t cl_device_image3d_max_depth;
	size_t cl_device_image3d_max_height;
	size_t cl_device_image3d_max_width;
	cl_ulong cl_device_local_mem_size;
	cl_uint cl_device_max_clock_frequency;
	cl_uint cl_device_max_compute_units;
	cl_uint cl_device_max_constant_args;
	cl_ulong cl_device_max_constant_buffer_size;
	cl_ulong cl_device_max_mem_alloc_size;
	size_t cl_device_max_parameter_size;
	cl_uint cl_device_max_read_image_args;
	cl_uint cl_device_max_samplers;
	size_t cl_device_max_work_group_size;
	cl_uint cl_device_max_work_item_dimensions;
	size_t cl_device_max_work_item_sizes[3];
	cl_uint cl_device_max_write_image_args;
	cl_uint cl_device_mem_base_addr_align;
	cl_uint cl_device_min_data_type_align_size;
	char cl_device_name[128];
	cl_platform_id cl_device_platform;
	cl_uint cl_device_preferred_vector_width_char;
	cl_uint cl_device_preferred_vector_width_short;
	cl_uint cl_device_preferred_vector_width_int;
	cl_uint cl_device_preferred_vector_width_long;
	cl_uint cl_device_preferred_vector_width_float;
	cl_uint cl_device_preferred_vector_width_double;
	char cl_device_profile[256];
	size_t cl_device_profiling_timer_resolution;
	cl_device_type cl_device_type;
	char cl_device_vendor[128];
	cl_uint cl_device_vendor_id;
	char cl_device_version[128];
	char cl_driver_version[128];
};

struct raycaster_settings {
	
};


struct PackedData;

class Hardware_Caster : public RayCaster
{
public:
	Hardware_Caster();

	virtual ~Hardware_Caster();

	
	// Queries hardware, creates the command queue and context, and compiles kernel
	int init() override;

	// Creates a texture to send to the GPU via height and width
	// Creates a viewport vector array via vertical and horizontal fov
	void create_viewport(int width, int height, float v_fov, float h_fov) override;
	
	// Light controllers own the copy of the PackedData array.
	// We receive a pointer to the array and USE_HOST_POINTER to map the memory to the GPU
	void assign_lights(std::vector<PackedData> *data) override;

	// We take a ptr to the map and create the map, and map_dimensions buffer for the GPU
	void assign_map(Old_Map *map) override;

	// We take a ptr to the camera and create a camera direction and position buffer
	void assign_camera(Camera *camera) override;

	// TODO: Hoist this to the base class
	// Creates 3 buffers relating to the texture atlas: texture_atlas, atlas_dim, and tile_dim
	// With these on the GPU we can texture any quad with an atlas tile
	void create_texture_atlas(sf::Texture *t, sf::Vector2i tile_dim);
	
	// Check to make sure that the buffers have been initiated and set them as kernel args
	void validate() override;

	// Aquires the GL objects, runs the kernel, releases back the GL objects
	void compute() override;

	// Take the viewport sprite and draw it to the screen
	void draw(sf::RenderWindow* window) override;


	// ================================== DEBUG =======================================
	
	// Re compile the kernel and revalidate the args
	int debug_quick_recompile();

	// Modify the viewport matrix
	void test_edit_viewport(int width, int height, float v_fov, float h_fov);

	void gui();

private:

	// Iterate the devices available and choose the best one
	// Also checks for the sharing extension
	int acquire_platform_and_device();

	int query_hardware();

	// With respect to the individual platforms implementation of sharing
	// create a shared cl_gl context
	int create_shared_context();

	// Using the context and the device create a command queue for them
	int create_command_queue();

	// Buffer operations
	// All of these functions create and store a buffer in a map with the key representing their name
	
	// Create an image buffer from an SF texture. Access Type is the read/write specifier required by OpenCL
	int create_image_buffer(std::string buffer_name, cl_uint size, sf::Texture* texture, cl_int access_type);

	// Create a buffer with CL_MEM_READ_ONLY and CL_MEM_COPY_HOST_PTR
	int create_buffer(std::string buffer_name, cl_uint size, void* data);

	// Create a buffer with user defined data flags
	int create_buffer(std::string buffer_name, cl_uint size, void* data, cl_mem_flags flags);
	
	// Store a cl_mem object in the buffer map <string:name, cl_mem:buffer>
	int store_buffer(cl_mem buffer, std::string buffer_name);

	// Using CL release the memory object and remove the KVP associated with the buffer name
	int release_buffer(std::string buffer_name);
	
	// Compile the kernel with either a full src string or by is_path=true and kernel_source = a valid path
	int compile_kernel(std::string kernel_source, bool is_path, std::string kernel_name);

	// Set the arg index for the specified kernel and buffer
	int set_kernel_arg(std::string kernel_name, int index, std::string buffer_name);

	// Run the kernel using a 1d work size
	// TODO: Test 2d worksize
	int run_kernel(std::string kernel_name, const int work_dim_x, const int work_dim_y);

	// Run a test kernel that prints out the kernel args
	void print_kernel_arguments();

	// CL error code handler. ImGui overlaps the assert() function annoyingly so I had to rename it
	bool vr_assert(int error_code, std::string function_name);

	cl_device_id getDeviceID();
	cl_platform_id getPlatformID();
	cl_context getContext();
	cl_kernel getKernel(std::string kernel_name);
	cl_command_queue getCommandQueue();

	// Our device data
	cl_platform_id platform_id;
	cl_device_id device_id;

	// And state
	cl_context context;
	cl_command_queue command_queue;

	// Containers holding the kernels and buffers
	std::map<std::string, cl_kernel> kernel_map;
	std::map<std::string, cl_mem> buffer_map;

};


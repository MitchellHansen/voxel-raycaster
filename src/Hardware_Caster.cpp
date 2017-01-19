#include "Hardware_Caster.h"



Hardware_Caster::Hardware_Caster() {

}


Hardware_Caster::~Hardware_Caster() {
}

int Hardware_Caster::init() {

	// Initialize opencl up to the point where we start assigning buffers

	error = acquire_platform_and_device();
	if(assert(error, "aquire_platform_and_device"))
		return error;

	error = check_cl_khr_gl_sharing();
	if(assert(error, "check_cl_khr_gl_sharing"))
		return error;

	error = create_shared_context();
	if (assert(error, "create_shared_context"))
		return error;

	error = create_command_queue();
	if (assert(error, "create_command_queue"))
		return error;

	error = compile_kernel("../kernels/ray_caster_kernel.cl", true, "raycaster");
	if (assert(error, "compile_kernel")) {
		std::cin.get(); // hang the output window so we can read the error
		return error;
	}

	srand(NULL);

	int *seed_memory = new int[1920*1080];

	create_buffer("seed", sizeof(int) * 1920 * 1080, seed_memory);

	return 1;

}

void Hardware_Caster::assign_map(Old_Map *map) {

	this->map = map;
	auto dimensions = map->getDimensions();
	
	create_buffer("map", sizeof(char) * dimensions.x * dimensions.y * dimensions.z, map->get_voxel_data());
	create_buffer("map_dimensions", sizeof(int) * 3, &dimensions);

}

void Hardware_Caster::assign_camera(Camera *camera) {

	this->camera = camera;

	create_buffer("camera_direction", sizeof(float) * 4, (void*)camera->get_direction_pointer(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	create_buffer("camera_position", sizeof(float) * 4, (void*)camera->get_position_pointer(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
}

void Hardware_Caster::validate()
{
	// Check to make sure everything has been entered;
	if (camera == nullptr ||
		map == nullptr ||
		viewport_image == nullptr ||
		viewport_matrix == nullptr) {
		
		std::cout << "Raycaster.validate() failed, camera, map, or viewport not initialized";
	
	} else {
	
		// Set all the kernel args
		set_kernel_arg("raycaster", 0, "map");
		set_kernel_arg("raycaster", 1, "map_dimensions");
		set_kernel_arg("raycaster", 2, "viewport_resolution");
		set_kernel_arg("raycaster", 3, "viewport_matrix");
		set_kernel_arg("raycaster", 4, "camera_direction");
		set_kernel_arg("raycaster", 5, "camera_position");
		set_kernel_arg("raycaster", 6, "lights");
		set_kernel_arg("raycaster", 7, "light_count");
		set_kernel_arg("raycaster", 8, "image");
		set_kernel_arg("raycaster", 9, "seed");

		//print_kernel_arguments();
	}

	
}

void Hardware_Caster::compute()
{
	// correlating work size with texture size? good, bad?
	run_kernel("raycaster", viewport_texture.getSize().x * viewport_texture.getSize().y);
}

// There is a possibility that I would want to move this over to be all inside it's own
// container to make it so it can be changed via CL_MEM_USE_HOST_PTR. But I doubt it
// would ever be called enough to warrent that
void Hardware_Caster::create_viewport(int width, int height, float v_fov, float h_fov) {
	
	// CL needs the screen resolution
	sf::Vector2i view_res(width, height);
	create_buffer("viewport_resolution", sizeof(int) * 2, &view_res);

	// And an array of vectors describing the way the "lens" of our
	// camera works

	// This could be modified to make some odd looking camera lenses

	double y_increment_radians = DegreesToRadians(v_fov / view_res.y);
	double x_increment_radians = DegreesToRadians(h_fov / view_res.x);

	viewport_matrix = new sf::Vector4f[width * height * 4];

	for (int y = -view_res.y / 2; y < view_res.y / 2; y++) {
		for (int x = -view_res.x / 2; x < view_res.x / 2; x++) {

			// The base ray direction to slew from
			sf::Vector3f ray(1, 0, 0);

			// Y axis, pitch
			ray = sf::Vector3f(
				static_cast<float>(ray.z * sin(y_increment_radians * y) + ray.x * cos(y_increment_radians * y)),
				static_cast<float>(ray.y),
				static_cast<float>(ray.z * cos(y_increment_radians * y) - ray.x * sin(y_increment_radians * y))
			);

			// Z axis, yaw
			ray = sf::Vector3f(
				static_cast<float>(ray.x * cos(x_increment_radians * x) - ray.y * sin(x_increment_radians * x)),
				static_cast<float>(ray.x * sin(x_increment_radians * x) + ray.y * cos(x_increment_radians * x)),
				static_cast<float>(ray.z)
			);

			// correct for the base ray pointing to (1, 0, 0) as (0, 0). Should equal (1.57, 0)
			ray = sf::Vector3f(
				static_cast<float>(ray.z * sin(-1.57) + ray.x * cos(-1.57)),
				static_cast<float>(ray.y),
				static_cast<float>(ray.z * cos(-1.57) - ray.x * sin(-1.57))
			);

			int index = (x + view_res.x / 2) + view_res.x * (y + view_res.y / 2);
			ray = Normalize(ray);

			viewport_matrix[index] = sf::Vector4f(
				ray.x,
				ray.y,
				ray.z,
				0
			);
		}
	}

	create_buffer("viewport_matrix", sizeof(float) * 4 * view_res.x * view_res.y, viewport_matrix, CL_MEM_USE_HOST_PTR);

	// Create the image that opencl's rays write to
	viewport_image = new sf::Uint8[width * height * 4];

	for (int i = 0; i < width * height * 4; i += 4) {

		viewport_image[i] = 255;     // R
		viewport_image[i + 1] = 255; // G
		viewport_image[i + 2] = 255; // B
		viewport_image[i + 3] = 100; // A
	}

	// Interop lets us keep a reference to it as a texture
	viewport_texture.create(width, height);
	viewport_texture.update(viewport_image);
	viewport_sprite.setTexture(viewport_texture);

	// Pass the buffer to opencl
	create_image_buffer("image", sizeof(sf::Uint8) * width * height * 4, viewport_image);

}

void Hardware_Caster::assign_lights(std::vector<Light> *lights) {
	
	this->lights = lights;

	light_count = static_cast<int>(lights->size());

	create_buffer("lights", sizeof(float) * 10 * light_count, this->lights->data(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	create_buffer("light_count", sizeof(int), &light_count);

}

void Hardware_Caster::draw(sf::RenderWindow* window) {
	window->draw(viewport_sprite);
}

int Hardware_Caster::debug_quick_recompile()
{
	int error = compile_kernel("../kernels/ray_caster_kernel.cl", true, "raycaster");
	if (assert(error, "compile_kernel")) {
		std::cin.get(); // hang the output window so we can read the error
		return error;
	}
	validate();

}

void Hardware_Caster::test_edit_viewport(int width, int height, float v_fov, float h_fov)
{
	sf::Vector2i view_res(width, height);

	double y_increment_radians = DegreesToRadians(v_fov / view_res.y);
	double x_increment_radians = DegreesToRadians(h_fov / view_res.x);

	for (int y = -view_res.y / 2; y < view_res.y / 2; y++) {
		for (int x = -view_res.x / 2; x < view_res.x / 2; x++) {

			// The base ray direction to slew from
			sf::Vector3f ray(1, 0, 0);

			// Y axis, pitch
			ray = sf::Vector3f(
				static_cast<float>(ray.z * sin(y_increment_radians * y) + ray.x * cos(y_increment_radians * y)),
				static_cast<float>(ray.y),
				static_cast<float>(ray.z * cos(y_increment_radians * y) - ray.x * sin(y_increment_radians * y))
				);

			// Z axis, yaw
			ray = sf::Vector3f(
				static_cast<float>(ray.x * cos(x_increment_radians * x) - ray.y * sin(x_increment_radians * x)),
				static_cast<float>(ray.x * sin(x_increment_radians * x) + ray.y * cos(x_increment_radians * x)),
				static_cast<float>(ray.z)
				);

			// correct for the base ray pointing to (1, 0, 0) as (0, 0). Should equal (1.57, 0)
			ray = sf::Vector3f(
				static_cast<float>(ray.z * sin(-1.57) + ray.x * cos(-1.57)),
				static_cast<float>(ray.y),
				static_cast<float>(ray.z * cos(-1.57) - ray.x * sin(-1.57))
				);

			int index = (x + view_res.x / 2) + view_res.x * (y + view_res.y / 2);
			ray = Normalize(ray);

			viewport_matrix[index] = sf::Vector4f(
				ray.x,
				ray.y,
				ray.z,
				0
				);
		}
	}
}

int Hardware_Caster::acquire_platform_and_device() {

	// Get the number of platforms
	cl_uint plt_cnt = 0;
	clGetPlatformIDs(0, nullptr, &plt_cnt);

	// Fetch the platforms
	std::map<cl_platform_id, std::vector<device>> plt_ids;

	// buffer before map init
	std::vector<cl_platform_id> plt_buf(plt_cnt);
	clGetPlatformIDs(plt_cnt, plt_buf.data(), nullptr);

	// Map init
	for (auto id : plt_buf) {
		plt_ids.emplace(std::make_pair(id, std::vector<device>()));
	}

	// For each platform, populate its devices
	for (unsigned int i = 0; i < plt_cnt; i++) {

		cl_uint deviceIdCount = 0;
		error = clGetDeviceIDs(plt_buf[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceIdCount);

		// Check to see if we even have opencl on this machine
		if (deviceIdCount == 0) {
			std::cout << "There appears to be no platforms supporting opencl" << std::endl;
			return OPENCL_NOT_SUPPORTED;
		}

		// Get the device ids
		std::vector<cl_device_id> deviceIds(deviceIdCount);
		error = clGetDeviceIDs(plt_buf[i], CL_DEVICE_TYPE_ALL, deviceIdCount, deviceIds.data(), NULL);

		if (assert(error, "clGetDeviceIDs"))
			return OPENCL_ERROR;

		for (unsigned int q = 0; q < deviceIdCount; q++) {

			device d;

			d.id = deviceIds[q];

			clGetDeviceInfo(d.id, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &d.platform, NULL);
			clGetDeviceInfo(d.id, CL_DEVICE_VERSION, sizeof(char) * 128, &d.version, NULL);
			clGetDeviceInfo(d.id, CL_DEVICE_TYPE, sizeof(cl_device_type), &d.type, NULL);
			clGetDeviceInfo(d.id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &d.clock_frequency, NULL);
			clGetDeviceInfo(d.id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &d.comp_units, NULL);

			plt_ids.at(d.platform).push_back(d);
		}
	}


	// The devices how now been queried we want to shoot for a gpu with the fastest clock,
	// falling back to the cpu with the fastest clock if we weren't able to find one

	device current_best_device;
	current_best_device.type = 0; // Set this to 0 so the first run always selects a new device
	current_best_device.clock_frequency = 0;
	current_best_device.comp_units = 0;


	for (auto kvp : plt_ids) {

		for (auto device : kvp.second) {

			// Gonna just split this up into cases. There are so many devices I cant test with
			// that opencl supports. I'm not going to waste my time making a generic implimentation

			// Upon success of a condition, set the current best device values

			if (device.type == CL_DEVICE_TYPE_GPU && current_best_device.type != CL_DEVICE_TYPE_GPU) {
				current_best_device = device;
			}
			else if (device.comp_units > current_best_device.comp_units) {
				current_best_device = device;
			}
			else if (current_best_device.type != CL_DEVICE_TYPE_GPU && device.clock_frequency > current_best_device.clock_frequency) {
				current_best_device = device;
			}
		}
	}

	platform_id = current_best_device.platform;
	device_id = current_best_device.id;

	return 1;
};

int Hardware_Caster::create_shared_context() {

	// Hurray for standards!
	// Setup the context properties to grab the current GL context

#ifdef linux

	cl_context_properties context_properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
		0
	};

#elif defined _WIN32

	HGLRC hGLRC = wglGetCurrentContext();
	HDC hDC = wglGetCurrentDC();
	cl_context_properties context_properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
		CL_GL_CONTEXT_KHR, (cl_context_properties)hGLRC,
		CL_WGL_HDC_KHR, (cl_context_properties)hDC,
		0
	};


#elif defined TARGET_OS_MAC

	CGLContextObj glContext = CGLGetCurrentContext();
	CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);
	cl_context_properties context_properties[] = {
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
		(cl_context_properties)shareGroup,
		0
	};

#endif

	// Create our shared context
	context = clCreateContext(
		context_properties,
		1,
		&device_id,
		nullptr, nullptr,
		&error
		);

	if (assert(error, "clCreateContext"))
		return OPENCL_ERROR;

	return 1;
}

int Hardware_Caster::create_command_queue() {

	// If context and device_id have initialized
	if (context && device_id) {
		
		command_queue = clCreateCommandQueue(context, device_id, 0, &error);

		if (assert(error, "clCreateCommandQueue"))
			return OPENCL_ERROR;

		return 1;
	}
	else {
		std::cout << "Failed creating the command queue. Context or device_id not initialized";
		return OPENCL_ERROR;
	}
}

int Hardware_Caster::check_cl_khr_gl_sharing() {

	// Test for sharing
	size_t ext_str_size = 1024;
	char *ext_str = new char[ext_str_size];
	clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, ext_str_size, ext_str, &ext_str_size);

	if (std::string(ext_str).find("cl_khr_gl_sharing") == std::string::npos) {
		std::cout << "No support for the cl_khr_gl_sharing extension";
		delete ext_str;
		return RayCaster::SHARING_NOT_SUPPORTED;
	}

	delete ext_str;
	return 1;
}

int Hardware_Caster::compile_kernel(std::string kernel_source, bool is_path, std::string kernel_name) {

	const char* source;
	std::string tmp;

	if (is_path) {
		//Load in the kernel, and c stringify it
		tmp = read_file(kernel_source);
		source = tmp.c_str();
	}
	else {
		source = kernel_source.c_str();
	}

	size_t kernel_source_size = strlen(source);

	// Load the source into CL's data structure

	cl_program program = clCreateProgramWithSource(
		context, 1,
		&source,
		&kernel_source_size, &error
		);

	// This is not for compilation, it only loads the source
	if (assert(error, "clCreateProgramWithSource"))
		return OPENCL_ERROR;


	// Try and build the program
	error = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	// Check to see if it errored out
	if (assert(error, "clBuildProgram")) {

		// Get the size of the queued log
		size_t log_size;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = new char[log_size];

		// Grab the log
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		std::cout << log;
		return OPENCL_ERROR;
	}

	// Done initializing the kernel
	cl_kernel kernel = clCreateKernel(program, kernel_name.c_str(), &error);

	if (assert(error, "clCreateKernel"))
		return OPENCL_ERROR;

	// Do I want these to overlap when repeated??
	kernel_map[kernel_name] = kernel;
	//kernel_map.emplace(std::make_pair(kernel_name, kernel));

	return 1;
}

int Hardware_Caster::set_kernel_arg(
	std::string kernel_name,
	int index,
	std::string buffer_name) {

	error = clSetKernelArg(
		kernel_map.at(kernel_name),
		index,
		sizeof(cl_mem),
		(void *)&buffer_map.at(buffer_name));

	if (assert(error, "clSetKernelArg"))
		return OPENCL_ERROR;

	return 0;

}

int Hardware_Caster::create_image_buffer(std::string buffer_name, cl_uint size, void* data) {

	// I can imagine overwriting buffers will be common, so I think
	// this is safe to overwrite / release old buffers quietly
	if (buffer_map.count(buffer_name) > 0) {
		release_buffer(buffer_name);
	}

	int error;
	cl_mem buff = clCreateFromGLTexture(
		getContext(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D,
		0, viewport_texture.getNativeHandle(), &error);

	if (assert(error, "clCreateFromGLTexture"))
		return OPENCL_ERROR;

	store_buffer(buff, buffer_name);

	return 1;
}

int Hardware_Caster::create_buffer(std::string buffer_name, cl_uint size, void* data, cl_mem_flags flags) {

	// I can imagine overwriting buffers will be common, so I think
	// this is safe to overwrite / release old buffers quietly
	if (buffer_map.count(buffer_name) > 0) {
		release_buffer(buffer_name);
	}

	cl_mem buff = clCreateBuffer(
		getContext(), flags,
		size, data, &error
		);

	if (assert(error, "clCreateBuffer"))
		return OPENCL_ERROR;

	store_buffer(buff, buffer_name);

	return 1;

}

int Hardware_Caster::create_buffer(std::string buffer_name, cl_uint size, void* data) {
	
	// I can imagine overwriting buffers will be common, so I think
	// this is safe to overwrite / release old buffers quietly
	if (buffer_map.count(buffer_name) > 0) {
		release_buffer(buffer_name);
	}

	cl_mem buff = clCreateBuffer(
		getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		size, data, &error
		);

	if (assert(error, "clCreateBuffer"))
		return OPENCL_ERROR;

	store_buffer(buff, buffer_name);

	return 1;

}

int Hardware_Caster::release_buffer(std::string buffer_name) {

	if (buffer_map.count(buffer_name) > 0) {
		
		int error = clReleaseMemObject(buffer_map.at(buffer_name));
		
		if (assert(error, "clReleaseMemObject")) {
			std::cout << "Error releasing buffer : " << buffer_name;
			std::cout << "Buffer not removed";
			return -1;

		} else {
			buffer_map.erase(buffer_name);
		}

	} else {
		std::cout << "Error releasing buffer : " << buffer_name;
		std::cout << "Buffer not found";
		return -1;
	}

	return 1;

}

int Hardware_Caster::store_buffer(cl_mem buffer, std::string buffer_name) {
	buffer_map.emplace(std::make_pair(buffer_name, buffer));
	return 1;
}

int Hardware_Caster::run_kernel(std::string kernel_name, const int work_size) {

	size_t global_work_size[1] = { static_cast<size_t>(work_size) };

	cl_kernel kernel = kernel_map.at(kernel_name);

	error = clEnqueueAcquireGLObjects(getCommandQueue(), 1, &buffer_map.at("image"), 0, 0, 0);
	if (assert(error, "clEnqueueAcquireGLObjects"))
		return OPENCL_ERROR;

	//error = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
	error = clEnqueueNDRangeKernel(
		command_queue, kernel,
		1, NULL, global_work_size,
		NULL, 0, NULL, NULL);

	if (assert(error, "clEnqueueNDRangeKernel"))
		return OPENCL_ERROR;

	clFinish(getCommandQueue());

	// What if errors out and gl objects are never released?
	error = clEnqueueReleaseGLObjects(getCommandQueue(), 1, &buffer_map.at("image"), 0, NULL, NULL);
	if (assert(error, "clEnqueueReleaseGLObjects"))
		return OPENCL_ERROR;

	return 1;
}

void Hardware_Caster::print_kernel_arguments()
{
	compile_kernel("../kernels/print_arguments.cl", true, "printer");
	set_kernel_arg("printer", 0, "map");
	set_kernel_arg("printer", 1, "map_dimensions");
	set_kernel_arg("printer", 2, "viewport_resolution");
	set_kernel_arg("printer", 3, "viewport_matrix");
	set_kernel_arg("printer", 4, "camera_direction");
	set_kernel_arg("printer", 5, "camera_position");
	set_kernel_arg("printer", 6, "lights");
	set_kernel_arg("printer", 7, "light_count");
	set_kernel_arg("printer", 8, "image");

	run_kernel("printer", 1);
}

cl_device_id Hardware_Caster::getDeviceID() { return device_id; };
cl_platform_id Hardware_Caster::getPlatformID() { return platform_id; };
cl_context Hardware_Caster::getContext() { return context; };
cl_kernel Hardware_Caster::getKernel(std::string kernel_name) { return kernel_map.at(kernel_name); };
cl_command_queue Hardware_Caster::getCommandQueue() { return command_queue; };

bool Hardware_Caster::assert(int error_code, std::string function_name) {

	// Just gonna do a little jump table here, just error codes so who cares
	std::string err_msg = "Error : ";

	switch (error_code) {

	case CL_SUCCESS:
		return false;

	case 1:
		return false;

	case CL_DEVICE_NOT_FOUND:
		err_msg += "CL_DEVICE_NOT_FOUND";
		break;
	case CL_DEVICE_NOT_AVAILABLE:
		err_msg = "CL_DEVICE_NOT_AVAILABLE";
		break;
	case CL_COMPILER_NOT_AVAILABLE:
		err_msg = "CL_COMPILER_NOT_AVAILABLE";
		break;
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:
		err_msg = "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		break;
	case CL_OUT_OF_RESOURCES:
		err_msg = "CL_OUT_OF_RESOURCES";
		break;
	case CL_OUT_OF_HOST_MEMORY:
		err_msg = "CL_OUT_OF_HOST_MEMORY";
		break;
	case CL_PROFILING_INFO_NOT_AVAILABLE:
		err_msg = "CL_PROFILING_INFO_NOT_AVAILABLE";
		break;
	case CL_MEM_COPY_OVERLAP:
		err_msg = "CL_MEM_COPY_OVERLAP";
		break;
	case CL_IMAGE_FORMAT_MISMATCH:
		err_msg = "CL_IMAGE_FORMAT_MISMATCH";
		break;
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:
		err_msg = "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		break;
	case CL_BUILD_PROGRAM_FAILURE:
		err_msg = "CL_BUILD_PROGRAM_FAILURE";
		break;
	case CL_MAP_FAILURE:
		err_msg = "CL_MAP_FAILURE";
		break;
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:
		err_msg = "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		break;
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
		err_msg = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		break;
	case CL_COMPILE_PROGRAM_FAILURE:
		err_msg = "CL_COMPILE_PROGRAM_FAILURE";
		break;
	case CL_LINKER_NOT_AVAILABLE:
		err_msg = "CL_LINKER_NOT_AVAILABLE";
		break;
	case CL_LINK_PROGRAM_FAILURE:
		err_msg = "CL_LINK_PROGRAM_FAILURE";
		break;
	case CL_DEVICE_PARTITION_FAILED:
		err_msg = "CL_DEVICE_PARTITION_FAILED";
		break;
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
		err_msg = "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
		break;
	case CL_INVALID_VALUE:
		err_msg = "CL_INVALID_VALUE";
		break;
	case CL_INVALID_DEVICE_TYPE:
		err_msg = "CL_INVALID_DEVICE_TYPE";
		break;
	case CL_INVALID_PLATFORM:
		err_msg = "CL_INVALID_PLATFORM";
		break;
	case CL_INVALID_DEVICE:
		err_msg = "CL_INVALID_DEVICE";
		break;
	case CL_INVALID_CONTEXT:
		err_msg = "CL_INVALID_CONTEXT";
		break;
	case CL_INVALID_QUEUE_PROPERTIES:
		err_msg = "CL_INVALID_QUEUE_PROPERTIES";
		break;
	case CL_INVALID_COMMAND_QUEUE:
		err_msg = "CL_INVALID_COMMAND_QUEUE";
		break;
	case CL_INVALID_HOST_PTR:
		err_msg = "CL_INVALID_HOST_PTR";
		break;
	case CL_INVALID_MEM_OBJECT:
		err_msg = "CL_INVALID_MEM_OBJECT";
		break;
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
		err_msg = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		break;
	case CL_INVALID_IMAGE_SIZE:
		err_msg = "CL_INVALID_IMAGE_SIZE";
		break;
	case CL_INVALID_SAMPLER:
		err_msg = "CL_INVALID_SAMPLER";
		break;
	case CL_INVALID_BINARY:
		err_msg = "CL_INVALID_BINARY";
		break;
	case CL_INVALID_BUILD_OPTIONS:
		err_msg = "CL_INVALID_BUILD_OPTIONS";
		break;
	case CL_INVALID_PROGRAM:
		err_msg = "CL_INVALID_PROGRAM";
		break;
	case CL_INVALID_PROGRAM_EXECUTABLE:
		err_msg = "CL_INVALID_PROGRAM_EXECUTABLE";
		break;
	case CL_INVALID_KERNEL_NAME:
		err_msg = "CL_INVALID_KERNEL_NAME";
		break;
	case CL_INVALID_KERNEL_DEFINITION:
		err_msg = "CL_INVALID_KERNEL_DEFINITION";
		break;
	case CL_INVALID_KERNEL:
		err_msg = "CL_INVALID_KERNEL";
		break;
	case CL_INVALID_ARG_INDEX:
		err_msg = "CL_INVALID_ARG_INDEX";
		break;
	case CL_INVALID_ARG_VALUE:
		err_msg = "CL_INVALID_ARG_VALUE";
		break;
	case CL_INVALID_ARG_SIZE:
		err_msg = "CL_INVALID_ARG_SIZE";
		break;
	case CL_INVALID_KERNEL_ARGS:
		err_msg = "CL_INVALID_KERNEL_ARGS";
		break;
	case CL_INVALID_WORK_DIMENSION:
		err_msg = "CL_INVALID_WORK_DIMENSION";
		break;
	case CL_INVALID_WORK_GROUP_SIZE:
		err_msg = "CL_INVALID_WORK_GROUP_SIZE";
		break;
	case CL_INVALID_WORK_ITEM_SIZE:
		err_msg = "CL_INVALID_WORK_ITEM_SIZE";
		break;
	case CL_INVALID_GLOBAL_OFFSET:
		err_msg = "CL_INVALID_GLOBAL_OFFSET";
		break;
	case CL_INVALID_EVENT_WAIT_LIST:
		err_msg = "CL_INVALID_EVENT_WAIT_LIST";
		break;
	case CL_INVALID_EVENT:
		err_msg = "CL_INVALID_EVENT";
		break;
	case CL_INVALID_OPERATION:
		err_msg = "CL_INVALID_OPERATION";
		break;
	case CL_INVALID_GL_OBJECT:
		err_msg = "CL_INVALID_GL_OBJECT";
		break;
	case CL_INVALID_BUFFER_SIZE:
		err_msg = "CL_INVALID_BUFFER_SIZE";
		break;
	case CL_INVALID_MIP_LEVEL:
		err_msg = "CL_INVALID_MIP_LEVEL";
		break;
	case CL_INVALID_GLOBAL_WORK_SIZE:
		err_msg = "CL_INVALID_GLOBAL_WORK_SIZE";
		break;
	case CL_INVALID_PROPERTY:
		err_msg = "CL_INVALID_PROPERTY";
		break;
	case CL_INVALID_IMAGE_DESCRIPTOR:
		err_msg = "CL_INVALID_IMAGE_DESCRIPTOR";
		break;
	case CL_INVALID_COMPILER_OPTIONS:
		err_msg = "CL_INVALID_COMPILER_OPTIONS";
		break;
	case CL_INVALID_LINKER_OPTIONS:
		err_msg = "CL_INVALID_LINKER_OPTIONS";
		break;
	case CL_INVALID_DEVICE_PARTITION_COUNT:
		err_msg = "CL_INVALID_DEVICE_PARTITION_COUNT";
		break;
	case RayCaster::SHARING_NOT_SUPPORTED:
		err_msg = "SHARING_NOT_SUPPORTED";
		break;
	case RayCaster::OPENCL_NOT_SUPPORTED:
		err_msg = "OPENCL_NOT_SUPPORTED";
		break;
	case RayCaster::OPENCL_ERROR:
		err_msg = "OPENCL_ERROR";
		break;
	case RayCaster::ERR:
		err_msg = "ERROR";
		break;
	}

	std::cout << err_msg << "  =at=  " << function_name << std::endl;
	return true;
}
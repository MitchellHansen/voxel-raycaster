#include "CLCaster.h"

CLCaster::CLCaster() {}
CLCaster::~CLCaster() {

    delete[] viewport_matrix;
    delete[] viewport_image;

	camera.reset();
	map.reset();
}

bool CLCaster::init() {

	Logger::log("Initializing the Hardware Caster", Logger::LogLevel::INFO);

	if (!aquire_hardware()) {
		Logger::log("Failed to acquire OpenCL hardware", Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	if (!load_config()) {

		std::cout << "Select a device number which you wish to use" << std::endl;

		for (int i = 0; i < device_list.size(); i++) {

			std::cout << "\n-----------------------------------------------------------------" << std::endl;
			std::cout << "\tDevice Number : " << i << std::endl;
			std::cout << "-----------------------------------------------------------------" << std::endl;

			device_list.at(i).print(std::cout);
		}

		int selection = -1;

		while (selection < 0 && selection >= device_list.size()) {

			std::cout << "Device which you wish to use : ";
			std::cin >> selection;
		}

		device_id = device_list.at(selection).getDeviceId();
		platform_id = device_list.at(selection).getPlatformId();

		save_config();
	}

	if (!create_shared_context()) {
		Logger::log("Failed to create shared CL GL context", Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	if (!create_command_queue()) {
		Logger::log("Failed to create a OpenCL command queue", Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

    if (!create_settings_buffer()) {
        Logger::log("Failed to create settings buffer", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

	srand(time(nullptr));

	int *seed_memory = new int[1920*1080];

	if (!create_buffer("seed", sizeof(int) * 1920 * 1080, seed_memory))
		return false;

	return true;

}

bool CLCaster::assign_map(std::shared_ptr<Map> map) {
	
	this->map = map;
	auto dimensions = map->array_map.getDimensions();

	if (!create_buffer("map", sizeof(char) * dimensions.x * dimensions.y * dimensions.z, map->array_map.getDataPtr()))
		return false;
	if (!create_buffer("map_dimensions", sizeof(int) * 3, &dimensions))
		return false;

	return true;
}

bool CLCaster::release_map() {
	
	this->map = nullptr;

	if (!release_buffer("map"))
		return false;
	if (!release_buffer("map_dimensions"))
		return false;

	return true;
}


bool CLCaster::assign_octree(std::shared_ptr<Map> map) {

	this->map = map;

	if (!create_buffer("octree_descriptor_buffer", map->octree.buffer_size * sizeof(uint64_t), map->octree.descriptor_buffer))
		return false;
	if (!create_buffer("octree_attachment_lookup_buffer", map->octree.buffer_size * sizeof(uint32_t), map->octree.attachment_lookup))
		return false;
	if (!create_buffer("octree_attachment_buffer", map->octree.buffer_size * sizeof(uint64_t), map->octree.attachment_buffer))
		return false;

    add_to_settings_buffer("octree_root_index", "OCTREE_ROOT_INDEX", (int64_t*)&map->octree.root_index);

	return true;
}


bool CLCaster::release_octree()
{
	this->map = nullptr;

	if (!release_buffer("octree_descriptor_buffer"))
		return false;
	if (!release_buffer("octree_attachment_lookup_buffer"))
		return false;
	if (!release_buffer("octree_attachment_buffer"))
		return false;

	return true;
}

bool CLCaster::assign_camera(std::shared_ptr<Camera> camera) {
	
	this->camera = camera;

	if (!create_buffer("camera_direction", sizeof(float) * 4, (void*)camera->get_direction_pointer(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR))
		return false;
	if (!create_buffer("camera_position", sizeof(float) * 4, (void*)camera->get_position_pointer(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR))
		return false;

	return true;
}

bool CLCaster::release_camera() {

	this->camera = nullptr;

	if (!release_buffer("camera_direction"))
		return false;
	if (!release_buffer("camera_position"))
		return false;

	return true;
}

bool CLCaster::validate() {

    if (!compile_kernel("../kernels/ray_caster_kernel.cl", true, "raycaster")) {
        Logger::log("Failed to compile the kernel", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        std::cin.get(); // hang the output window so we can read the error
        return false;
    }

	Logger::log("Validating OpenCL kernel args", Logger::LogLevel::INFO);

	// Check to make sure everything has been entered
	if (!camera.get()) {
		Logger::log("Raycaster.validate() failed, camera not initialized", Logger::LogLevel::WARN);
		return false;
	}
	if (!map.get()) {
		Logger::log("Raycaster.validate() failed, map not initialized", Logger::LogLevel::WARN);
		return false;
	}
	if (!viewport_image) {
		Logger::log("Raycaster.validate() failed, viewport_image not initialized", Logger::LogLevel::WARN);
		return false;
	}
	if (!viewport_matrix) {
		Logger::log("Raycaster.validate() failed, viewport_matrix not initialized", Logger::LogLevel::WARN);
		return false;
	}

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
	//set_kernel_arg("raycaster", 9, "seed");
	set_kernel_arg("raycaster", 9, "texture_atlas");
	set_kernel_arg("raycaster", 10, "atlas_dim");
	set_kernel_arg("raycaster", 11, "tile_dim");
	set_kernel_arg("raycaster", 12, "octree_descriptor_buffer");
	set_kernel_arg("raycaster", 13, "octree_attachment_lookup_buffer");
	set_kernel_arg("raycaster", 14, "octree_attachment_buffer");
	set_kernel_arg("raycaster", 15, "settings_buffer");

	return true;

}

bool CLCaster::create_texture_atlas(sf::Texture *t, sf::Vector2i tile_dim) {
	
	if (!create_image_buffer("texture_atlas", t->getSize().x * t->getSize().x * 4 * sizeof(float), t, CL_MEM_READ_ONLY))
		return false;
	
	// create_buffer observes arg 3's
	sf::Vector2u v = t->getSize();
	
	if (!create_buffer("atlas_dim", sizeof(sf::Vector2u) , &v))
		return false;
	if (!create_buffer("tile_dim", sizeof(sf::Vector2i), &tile_dim))
		return false;

	return true;
}

bool CLCaster::compute() {
	
	// correlating work size with texture size? good, bad?
	return run_kernel("raycaster", viewport_texture.getSize().x, viewport_texture.getSize().y);
}

// There is a possibility that I would want to move this over to be all inside it's own
// container to make it so it can be changed via CL_MEM_USE_HOST_PTR. But I doubt it
// would ever be called enough to warrant that
bool CLCaster::create_viewport(int width, int height, float v_fov, float h_fov) {
	
	// CL needs the screen resolution
	sf::Vector2i view_res(width, height);
	if (!create_buffer("viewport_resolution", sizeof(int) * 2, &view_res))
		return false;

	// And an array of vectors describing the way the "lens" of our
	// camera works

	// This could be modified to make some odd looking camera lenses

	viewport_matrix = new sf::Vector4f[width * height * 4];

	for (int y = -view_res.y / 2; y < view_res.y / 2; y++) {

		for (int x = -view_res.x / 2; x < view_res.x / 2; x++) {

			// The base ray direction to slew from
			sf::Vector3f ray(-800, x, y);

			// correct for the base ray pointing to (1, 0, 0) as (0, 0). Should equal (1.57, 0)
			ray = sf::Vector3f(
				static_cast<float>(ray.z * sin(1.57) + ray.x * cos(1.57)),
				static_cast<float>(ray.y),
				static_cast<float>(ray.z * cos(1.57) - ray.x * sin(1.57))
			);

            ray.y += (rand() % 1000) / 100000.0;
            ray.x += (rand() % 1000) / 100000.0;
            ray.z += (rand() % 1000) / 100000.0;

			ray = Normalize(ray);
			int index = (x + view_res.x / 2) + view_res.x * (y + view_res.y / 2);

			viewport_matrix[index] = sf::Vector4f(
					ray.x,
					ray.y,
					ray.z,
					0
			);
		}
	}

	if (!create_buffer("viewport_matrix", sizeof(float) * 4 * view_res.x * view_res.y, viewport_matrix))
		return false;

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
	if (!create_image_buffer("image", sizeof(sf::Uint8) * width * height * 4, &viewport_texture, CL_MEM_WRITE_ONLY))
		return false;

	return true;

}

bool CLCaster::release_viewport() {

    bool success = true;
    if (!release_buffer("viewport_resolution"))
        success = false;
    if (!release_buffer("viewport_matrix"))
        success = false;
    if (!release_buffer("image"))
        success = false;
    return success;
}

bool CLCaster::assign_lights(std::vector<PackedData> *data) {

	// Get a pointer to the packed light data
	this->lights = data;

	light_count = static_cast<int>(lights->size());

	cl_uint packed_size = sizeof(PackedData);

	if (!create_buffer("lights", packed_size * light_count, lights->data(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR))
		return false;
	if (!create_buffer("light_count", 8, &light_count))
		return false;

	return true;
}

void CLCaster::draw(sf::RenderWindow* window) {
	window->draw(viewport_sprite);
}

bool CLCaster::debug_quick_recompile() {
	
	if (!compile_kernel("../kernels/ray_caster_kernel.cl", true, "raycaster")) {
		Logger::log("Failed to recompile kernel", Logger::LogLevel::WARN, __LINE__, __FILE__);
		std::cin.get(); // hang the output window so we can read the error
		return false;
	}
	
	return validate();

}


void CLCaster::render_gui() {
	
	ImGui::Begin("CLCaster");

	if (ImGui::Button("Recompile Kernel")) {
		while (!debug_quick_recompile()) {
			std::cin.get();
		};
	}

	ImGui::End();
}


void CLCaster::update_gui() {
	rendering = true;
}


void CLCaster::event_handler(VrEventPublisher *publisher, std::unique_ptr<vr::Event> event) {

	if (event->type == vr::Event::KeyPressed) {

		vr::KeyPressed *key_event = static_cast<vr::KeyPressed*>(event.get());

		if (key_event->code == sf::Keyboard::T) {
			debug_quick_recompile();
		}
}
}

bool CLCaster::aquire_hardware() {

	Logger::log("Acquiring OpenCL Hardware", Logger::LogLevel::INFO);

	// Get the number of platforms
	cl_uint platform_count = 0;

	error = clGetPlatformIDs(0, nullptr, &platform_count);

	if (cl_assert(error)) {
		Logger::log("Failed at clGetPlatformIDs() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	if (platform_count == 0) {
		Logger::log("There appears to be no OpenCL platforms on this machine", Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	// Get the ID's for those platforms
	std::vector<cl_platform_id> plt_buf(platform_count);

	error = clGetPlatformIDs(platform_count, plt_buf.data(), nullptr);

	if (cl_assert(error)) {
		Logger::log("Failed at clGetPlatformIDs() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	// Cycle through the platform ID's
	for (unsigned int i = 0; i < platform_count; i++) {

		// And get their device count
		cl_uint deviceIdCount = 0;
		error = clGetDeviceIDs(plt_buf[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceIdCount);

		if (cl_assert(error)) {
			Logger::log("Failed at clGetDeviceIDs() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
			return false;
		}

		if (deviceIdCount == 0) {
			Logger::log("There appears to be no OpenCL platforms on this platform", Logger::LogLevel::INFO, __LINE__, __FILE__);
		}
		else {

			// Get the device ids and place them in the device list
			std::vector<cl_device_id> deviceIds(deviceIdCount);

			error = clGetDeviceIDs(plt_buf[i], CL_DEVICE_TYPE_ALL, deviceIdCount, deviceIds.data(), NULL);

			if (cl_assert(error)) {
				Logger::log("Failed at clGetDeviceIDs() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
				return false;
			}

			for (int d = 0; d < deviceIds.size(); d++) {
				device_list.emplace_back(device(deviceIds[d], plt_buf.at(i)));
			}
		}
	}

	return true;
}

void CLCaster::save_config() {

	Logger::log("Saving OpenCL hardware config", Logger::LogLevel::INFO);

	std::ofstream output_file;
	output_file.open("device_config.bin", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);

	device d(device_id, platform_id);
	d.print_packed_data(output_file);

	output_file.close();
}

bool CLCaster::load_config() {

	Logger::log("Loading hardware config", Logger::LogLevel::INFO);

	std::ifstream input_file("device_config.bin", std::ios::binary | std::ios::in);

	if (!input_file.is_open()) {
		Logger::log("No device_config.bin file found", Logger::LogLevel::WARN);
		return false;
	}

	device::packed_data data;
	input_file.read(reinterpret_cast<char*>(&data), sizeof(data));
	input_file.close();

	bool found = false;

	for (auto d : device_list) {

		if (memcmp(&d, &data, sizeof(device::packed_data)) == 0) {
			Logger::log("Found saved hardware config", Logger::LogLevel::INFO);
			found = true;
			device_id = d.getDeviceId();
			platform_id = d.getPlatformId();
			break;
		}
	}

	if (!found) {
		Logger::log("No hardware matching the saved device in device_config.bin found", Logger::LogLevel::WARN);
		return false;
	}

	return true;
}

bool CLCaster::query_hardware()
{

	Logger::log("Querying OpenCL hardware", Logger::LogLevel::INFO);

	// Get the number of platforms
	cl_uint plt_cnt = 0;
	error = clGetPlatformIDs(0, nullptr, &plt_cnt);

	if (cl_assert(error)) {
		Logger::log("Failed at clGetPlatformIDs() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	// Fetch the platforms
	std::map<cl_platform_id, std::vector<device_info>> plt_ids;

	// buffer before map init
	std::vector<cl_platform_id> plt_buf(plt_cnt);
	error = clGetPlatformIDs(plt_cnt, plt_buf.data(), nullptr);

	if (cl_assert(error)) {
		Logger::log("Failed at clGetPlatformIDs() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	// Map init
	for (auto id : plt_buf) {
		plt_ids.emplace(std::make_pair(id, std::vector<device_info>()));
	}

	// For each platform, populate its devices
	for (unsigned int i = 0; i < plt_cnt; i++) {

		cl_uint deviceIdCount = 0;
		error = clGetDeviceIDs(plt_buf[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceIdCount);

		// Check to see if we even have OpenCL on this machine
		if (deviceIdCount == 0) {
			Logger::log("No devices supporting OpenCL found", Logger::LogLevel::ERROR, __LINE__, __FILE__);
			return false;
		}

		// Get the device ids
		std::vector<cl_device_id> deviceIds(deviceIdCount);
		error = clGetDeviceIDs(plt_buf[i], CL_DEVICE_TYPE_ALL, deviceIdCount, deviceIds.data(), NULL);

		if (cl_assert(error)) {
			Logger::log("Failed at clGetDeviceIDs() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
			return false;
		}

		for (unsigned int q = 0; q < deviceIdCount; q++) {

			device_info d;

			cl_device_id id = deviceIds[q];
			
			clGetDeviceInfo(id, CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &d.cl_device_address_bits, NULL);			
			clGetDeviceInfo(id, CL_DEVICE_AVAILABLE, sizeof(cl_bool), &d.cl_device_available, NULL);
			clGetDeviceInfo(id, CL_DEVICE_COMPILER_AVAILABLE, sizeof(cl_bool), &d.cl_device_compiler_available, NULL);
			clGetDeviceInfo(id, CL_DEVICE_ENDIAN_LITTLE, sizeof(cl_bool), &d.cl_device_endian_little, NULL);
			clGetDeviceInfo(id, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(cl_bool), &d.cl_device_error_correction_support, NULL);
			clGetDeviceInfo(id, CL_DEVICE_EXTENSIONS, sizeof(char)*1024, &d.cl_device_extensions, NULL);
			clGetDeviceInfo(id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &d.cl_device_global_mem_cache_size, NULL);
			clGetDeviceInfo(id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cl_uint), &d.cl_device_global_mem_cacheline_size, NULL);
			clGetDeviceInfo(id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &d.cl_device_global_mem_size, NULL);
			clGetDeviceInfo(id, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &d.cl_device_image_support, NULL);
			clGetDeviceInfo(id, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &d.cl_device_image2d_max_height, NULL);
			clGetDeviceInfo(id, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &d.cl_device_image2d_max_width, NULL);
			clGetDeviceInfo(id, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &d.cl_device_image3d_max_depth, NULL);
			clGetDeviceInfo(id, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &d.cl_device_image3d_max_height, NULL);
			clGetDeviceInfo(id, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &d.cl_device_image3d_max_width, NULL);
			clGetDeviceInfo(id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &d.cl_device_local_mem_size, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(size_t), &d.cl_device_max_clock_frequency, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(size_t), &d.cl_device_max_compute_units, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(size_t), &d.cl_device_max_constant_args, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &d.cl_device_max_constant_buffer_size, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &d.cl_device_max_mem_alloc_size, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(size_t), &d.cl_device_max_parameter_size, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(cl_uint), &d.cl_device_max_read_image_args, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_SAMPLERS, sizeof(cl_uint), &d.cl_device_max_samplers, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), &d.cl_device_max_work_group_size, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_ulong), &d.cl_device_max_work_item_dimensions, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*3, &d.cl_device_max_work_item_sizes, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(cl_uint), &d.cl_device_max_write_image_args, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(cl_uint), &d.cl_device_mem_base_addr_align, NULL);
			clGetDeviceInfo(id, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, sizeof(cl_uint), &d.cl_device_min_data_type_align_size, NULL);
			clGetDeviceInfo(id, CL_DEVICE_NAME, sizeof(char)*128, &d.cl_device_name, NULL);
			clGetDeviceInfo(id, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &d.cl_device_platform, NULL);
			clGetDeviceInfo(id, CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT, sizeof(cl_uint), &d.cl_device_preferred_vector_width_char, NULL);
			clGetDeviceInfo(id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &d.cl_device_preferred_vector_width_short, NULL);
			clGetDeviceInfo(id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &d.cl_device_preferred_vector_width_int, NULL);
			clGetDeviceInfo(id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &d.cl_device_preferred_vector_width_long, NULL);
			clGetDeviceInfo(id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &d.cl_device_preferred_vector_width_float, NULL);
			clGetDeviceInfo(id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &d.cl_device_preferred_vector_width_double, NULL);
			clGetDeviceInfo(id, CL_DEVICE_PROFILE, sizeof(char) * 256, &d.cl_device_profile, NULL);
			clGetDeviceInfo(id, CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(size_t), &d.cl_device_profiling_timer_resolution, NULL);
			clGetDeviceInfo(id, CL_DEVICE_TYPE, sizeof(cl_device_type), &d.device_type, NULL);
			clGetDeviceInfo(id, CL_DEVICE_VENDOR, sizeof(char)*128, &d.cl_device_vendor, NULL);
			clGetDeviceInfo(id, CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &d.cl_device_vendor_id, NULL);
			clGetDeviceInfo(id, CL_DEVICE_VERSION, sizeof(char)*128, &d.cl_device_version, NULL);
			clGetDeviceInfo(id, CL_DRIVER_VERSION, sizeof(char)*128, &d.cl_driver_version, NULL);

			plt_ids.at(d.cl_device_platform).push_back(d);
		}
	}

	return true;
}

bool CLCaster::create_shared_context()
{

	// Hurray for standards!
	// Setup the context properties to grab the current GL context

#ifdef linux

	cl_context_properties context_properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
        //CL_GL_CONTEXT_KHR, (cl_context_properties)eglGetCurrentContext(),
        //CL_EGL_DISPLAY_KHR, (cl_context_properties)eglGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
		0
	};

#elif defined _WIN32

	HDC hDC = wglGetCurrentDC();
	HGLRC hGLRC = wglGetCurrentContext();
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

	if (cl_assert(error)) {
		Logger::log("Failed at clCreateContext() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool CLCaster::create_command_queue() {

	// If context and device_id have initialized
	if (context && device_id) {
		
		command_queue = clCreateCommandQueue(context, device_id, 0, &error);

		if (cl_assert(error)) {
			Logger::log("Failed at clCreateCommandQueue() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
			return false;
		}

	} else {

		Logger::log("Failed creating the command queue. Context or device_id not initialized", Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool CLCaster::compile_kernel(std::string kernel_source, bool is_path, std::string kernel_name) {
	
	Logger::log("Compiling OpenCL Kernel", Logger::LogLevel::INFO);
	
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
	if (cl_assert(error)) {
		Logger::log("Failed at clCreateProgramWithSource() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

    std::stringstream build_string_stream;

    // walk the settings index's and add them to the defines
    for (auto const& define : defines_map){
        build_string_stream << " -D" << define.first << "=" << define.second;
    }

	build_string_stream << " -cl-finite-math-only -cl-fast-relaxed-math -cl-unsafe-math-optimizations";

    std::string build_string = build_string_stream.str();

	error = clBuildProgram(program, 1, &device_id, build_string.c_str(), NULL, NULL);

	// Check to see if it error'd out
	if (cl_assert(error)) {

		// Get the size of the queued log
		size_t log_size;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = new char[log_size];

		// Grab the log
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		Logger::log("Failed at clBuildProgram() with " + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		Logger::log("CL_ERROR -->" + std::string(log), Logger::LogLevel::ERROR, __LINE__, __FILE__);

		return false;
	}

	// Done initializing the kernel
	cl_kernel kernel = clCreateKernel(program, kernel_name.c_str(), &error);

	if (cl_assert(error)) {
		Logger::log("Failed at clCreateKernel() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	// Do I want these to overlap when repeated??
	kernel_map[kernel_name] = kernel;
	//kernel_map.emplace(std::make_pair(kernel_name, kernel));

	return true;
}

bool CLCaster::set_kernel_arg(
	std::string kernel_name,
	int index,
	std::string buffer_name) {

	error = clSetKernelArg(
		kernel_map.at(kernel_name),
		index,
		sizeof(cl_mem),
		(void *)&buffer_map.at(buffer_name));

	if (cl_assert(error)) {
		Logger::log("Failed at clSetKernelArg() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		Logger::log("Buffer name : " + buffer_name, Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	return true;

}

bool CLCaster::create_image_buffer(std::string buffer_name, cl_uint size, sf::Texture* texture, cl_int access_type) {

	// I can imagine overwriting buffers will be common, so I think
	// this is safe to overwrite / release old buffers quietly
	if (buffer_map.count(buffer_name) > 0) {
		Logger::log("buffer_map already contains buffer of the same name, releasing conflicting buffer : " + buffer_name, Logger::LogLevel::INFO);
		if (!release_buffer(buffer_name))
			return false;
	}

	cl_mem buff = clCreateFromGLTexture(
		getContext(), access_type, GL_TEXTURE_2D,
		0, texture->getNativeHandle(), &error);

	if (cl_assert(error)) {
		Logger::log("Failed at clCreateFromGLTexture() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	if (!store_buffer(buff, buffer_name))
		return false;

	return true;
}

bool CLCaster::create_buffer(std::string buffer_name, cl_uint size, void* data, cl_mem_flags flags) {

	// I can imagine overwriting buffers will be common, so I think
	// this is safe to overwrite / release old buffers quietly
	if (buffer_map.count(buffer_name) > 0) {
		Logger::log("buffer_map already contains buffer of the same name, releasing conflicting buffer : " + buffer_name, Logger::LogLevel::INFO);
		if (!release_buffer(buffer_name))
			return false;
	}

	cl_mem buff = clCreateBuffer(
		getContext(), flags,
		size, data, &error
		);

	if (cl_assert(error)) {
		Logger::log("Failed at clCreateBuffer() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		Logger::log("Buffer name : " + buffer_name, Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	if (!store_buffer(buff, buffer_name))
		return false;

	return true;

}

bool CLCaster::create_buffer(std::string buffer_name, cl_uint size, void* data) {
	
	// I can imagine overwriting buffers will be common, so I think
	// this is safe to overwrite / release old buffers quietly
	if (buffer_map.count(buffer_name) > 0) {
		Logger::log("buffer_map already contains buffer of the same name, releasing conflicting buffer : " + buffer_name, Logger::LogLevel::INFO);
		if (!release_buffer(buffer_name))
			return false;
	}

	cl_mem buff = clCreateBuffer(
		getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		size, data, &error
		);

	if (cl_assert(error)) {
		Logger::log("Failed at clCreateBuffer() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		Logger::log("Buffer name : " + buffer_name, Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	if (!store_buffer(buff, buffer_name))
		return false;

	return true;

}

bool CLCaster::release_buffer(std::string buffer_name) {

	if (buffer_map.count(buffer_name) > 0) {

		int error = clReleaseMemObject(buffer_map.at(buffer_name));
		
		if (cl_assert(error)) {
			Logger::log("Error releasing buffer at clReleaseMemObject()" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
			Logger::log("buffer not removed : " + buffer_name, Logger::LogLevel::WARN, __LINE__, __FILE__);
			return false;
		} 
	
		buffer_map.erase(buffer_name);
		
	} else {
		Logger::log("Error releasing buffer, buffer not found : " + buffer_name , Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	return true;

}

bool CLCaster::store_buffer(cl_mem buffer, std::string buffer_name) {
	
	if (buffer_map.count(buffer_name) == 0) {
		buffer_map.emplace(std::make_pair(buffer_name, buffer));
		return true;
	}

	Logger::log("Failed to store buffer : " + buffer_name + " , name already taken", Logger::LogLevel::ERROR, __LINE__, __FILE__);
	return false;

}

bool CLCaster::run_kernel(std::string kernel_name, const int work_dim_x, const int work_dim_y) {

	size_t global_work_size[2] = { static_cast<size_t>(work_dim_x), static_cast<size_t>(work_dim_y)};

	cl_kernel kernel = kernel_map.at(kernel_name);

	error = clEnqueueAcquireGLObjects(getCommandQueue(), 1, &buffer_map.at("image"), 0, 0, 0);

	if (cl_assert(error)) {
		Logger::log("Failed at clEnqueueAcquireGLObjects() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	//error = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
	error = clEnqueueNDRangeKernel(
		command_queue, kernel,
		2, NULL, global_work_size,
		NULL, 0, NULL, NULL);

	if (cl_assert(error)) {
		Logger::log("Failed at clEnqueueNDRangeKernel() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	error = clFinish(getCommandQueue());

	if (cl_assert(error)) {
		Logger::log("Failed at clFinish() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}

	// What if errors out and gl objects are never released?
	error = clEnqueueReleaseGLObjects(getCommandQueue(), 1, &buffer_map.at("image"), 0, NULL, NULL);

	if (cl_assert(error)) {
		Logger::log("Failed at clEnqueueReleaseGLObjects() :" + cl_err_lookup(error), Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return false;
	}


	return true;
}

void CLCaster::print_kernel_arguments()
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

	run_kernel("printer", 1, 1);
}

cl_device_id CLCaster::getDeviceID() { return device_id; };
cl_platform_id CLCaster::getPlatformID() { return platform_id; };
cl_context CLCaster::getContext() { return context; };
cl_kernel CLCaster::getKernel(std::string kernel_name) { return kernel_map.at(kernel_name); };
cl_command_queue CLCaster::getCommandQueue() { return command_queue; };

bool CLCaster::cl_assert(int error_code) {
	
	if (error_code == CL_SUCCESS || error_code == 1)
		return false;
	else
		return true;
}


std::string CLCaster::cl_err_lookup(int error_code) {
	
	std::string err_msg = "";

	switch (error_code) {

		case CL_SUCCESS:
			err_msg += "CL_SUCCESS";
			break;
		case 1:
			err_msg += "CL_SUCCESS";
			break;
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
		case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
			err_msg = "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
			break;
		case CL_PLATFORM_NOT_FOUND_KHR:
			err_msg = "CL_PLATFORM_NOT_FOUND_KHR";
			break;
		default:
			err_msg = "UNKNOWN_ERROR";
	}

	return err_msg;

}

void CLCaster::set_define(std::string name, std::string value) {
	defines_map[name] = value;
}

void CLCaster::remove_define(std::string name) {
    defines_map.erase(name);
}

bool CLCaster::add_to_settings_buffer(std::string setting_name, std::string define_accessor_name, int64_t *value) {

    bool success = true;

    if (settings_buffer == nullptr){

        Logger::log("Trying to push settings to an uninitialized settings buffer", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        success = false;

    } else if (defines_map.count(define_accessor_name)) {

        Logger::log("Define name already present in the defines map", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        success = false;

    } else {

        if (settings_buffer_position < SETTINGS_BUFFER_SIZE) {
            defines_map[define_accessor_name] = std::to_string(settings_buffer_position);
            settings_buffer[settings_buffer_position] = *value;
            settings_buffer_position++;
        } else {
            Logger::log("Settings buffer has reached the maximum size of " + std::to_string(SETTINGS_BUFFER_SIZE) + " elements", Logger::LogLevel::ERROR, __LINE__, __FILE__);
            success = false;
        }
    }

    return success;
}

bool CLCaster::create_settings_buffer() {

    settings_buffer = new int64_t[SETTINGS_BUFFER_SIZE];
    if (!create_buffer("settings_buffer", sizeof(int64_t) * SETTINGS_BUFFER_SIZE, settings_buffer, CL_MEM_USE_HOST_PTR))
        return false;
    return true;
}

bool CLCaster::remove_from_settings_buffer(std::string setting_name) {

    Logger::log("remove_from_settings_buffer() not implimented", Logger::LogLevel::WARN, __LINE__, __FILE__);
    return false;
}

bool CLCaster::release_settings_buffer() {

    if (!release_buffer("settings_buffer"))
        return false;
    return true;
}

bool CLCaster::overwrite_setting(std::string settings_name, int64_t *value) {

    bool success = true;

    if (settings_buffer == nullptr){

        Logger::log("Trying to push settings to an uninitialized settings buffer", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        success = false;

    } else {

        if (settings_buffer_indices.count(settings_name)) {

            unsigned int postion = settings_buffer_indices[settings_name];
            settings_buffer[postion] = *value;
        } else {

            Logger::log("No setting matching [" + settings_name +"]", Logger::LogLevel::ERROR, __LINE__, __FILE__);
            success = false;
        }
    }
    return success;
}


CLCaster::device::device(cl_device_id device_id, cl_platform_id platform_id) {

	this->device_id = device_id;
	this->platform_id = platform_id;

	int error = 0;
	error = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 128, (void*)&data.platform_name, nullptr);
	if (cl_assert(error)) {
		Logger::log("Failed at function clGetPlatformInfo", Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return;
	}
		
	error = clGetDeviceInfo(device_id, CL_DEVICE_VERSION, sizeof(char) * 128, &data.opencl_version, NULL);

	// Just check for error on the first call
	if (cl_assert(error)) {
		Logger::log("Failed at function clGetDeviceInfo", Logger::LogLevel::ERROR, __LINE__, __FILE__);
		return;
	}

	error = clGetDeviceInfo(device_id, CL_DEVICE_TYPE, sizeof(cl_device_type), &data.device_type, NULL);
	error = clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &data.clock_frequency, NULL);
	error = clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &data.compute_units, NULL);
	error = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 1024, &data.device_extensions, NULL);
	error = clGetDeviceInfo(device_id, CL_DEVICE_NAME, 256, &data.device_name, NULL);
	error = clGetDeviceInfo(device_id, CL_DEVICE_ENDIAN_LITTLE, sizeof(cl_bool), &is_little_endian, NULL);

	// Check for the sharing extension
	if (std::string(data.device_extensions).find("cl_khr_gl_sharing") != std::string::npos ||
		std::string(data.device_extensions).find("cl_APPLE_gl_sharing") != std::string::npos) {
		cl_gl_sharing = true;
	}
}


CLCaster::device::device(const device& d) {

	// member values, copy individually
	device_id = d.device_id;
	platform_id = d.platform_id;
	is_little_endian = d.is_little_endian;
	cl_gl_sharing = d.cl_gl_sharing;

	// struct so it copies by value
	data = d.data;

}

void CLCaster::device::print(std::ostream& stream) const {

	stream << "\n\tDevice ID        : " << device_id << std::endl;
    stream <<   "\tPlatform ID      : " << platform_id << std::endl;

    stream <<   "\tDevice Name      : " << data.device_name << std::endl;
	stream <<   "\tPlatform Name    : " << data.platform_name << std::endl;
    stream <<   "\tDevice Type      : ";

    if (data.device_type == CL_DEVICE_TYPE_CPU)
        stream << "CPU" << std::endl;

    else if (data.device_type == CL_DEVICE_TYPE_GPU)
        stream << "GPU" << std::endl;

    else if (data.device_type == CL_DEVICE_TYPE_ACCELERATOR)
        stream << "Accelerator" << std::endl;

	stream <<   "\tOpenCL Version   : " << data.opencl_version << std::endl;
	stream <<   "\tSupports sharing : " << std::boolalpha << cl_gl_sharing << std::endl;

	stream << "\tIs Little Endian : " << std::boolalpha << is_little_endian << std::endl;
	stream << "\tClock Frequency  : " << data.clock_frequency << std::endl;
	stream << "\tCompute Units    : " << data.compute_units << std::endl;

	stream << "\n*Extensions*" << std::endl;
	stream << data.device_extensions << std::endl;
	stream << "\n";

}

void CLCaster::device::print_packed_data(std::ostream& stream) {
	stream.write(reinterpret_cast<char*>(&data), sizeof(data));
}

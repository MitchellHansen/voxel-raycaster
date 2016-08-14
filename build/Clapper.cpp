#include "Clapper.h"



Clapper::Clapper() {
}


Clapper::~Clapper() {
}

void Clapper::init() {

	int error = 0;

	// ===================================================================== //
	// ==== Opencl setup

	// Get the number of platforms
	cl_uint platform_count = 0;
	clGetPlatformIDs(0, nullptr, &platform_count);

	// Fetch the platforms
	std::vector<cl_platform_id> platformIds(platform_count);
	clGetPlatformIDs(platform_count, platformIds.data(), nullptr);



	// get the number of devices, fetch them, choose the first one
	cl_uint deviceIdCount = 0;
	error = clGetDeviceIDs(platformIds[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceIdCount);

	std::vector<cl_device_id> deviceIds(deviceIdCount);

	for (int q = 0; q < deviceIdCount; q++) {

		std::cout << "++++Device " << q << std::endl;
		error = clGetDeviceIDs(platformIds[i], CL_DEVICE_TYPE_ALL, deviceIdCount, deviceIds.data(), NULL);

		clGetDeviceInfo(deviceIds[q], CL_DEVICE_NAME, 128, platform, NULL);
		clGetDeviceInfo(deviceIds[q], CL_DEVICE_VERSION, 128, version, NULL);

		std::cout << platform << "\n";
		std::cout << version << "\n\n";

	}


	std::cout << "============================================" << std::endl;

	cl_uint deviceIdCount = 0;
	std::vector<cl_device_id> deviceIds;

	// Try to get a GPU first
	error = clGetDeviceIDs(platformIds[1], CL_DEVICE_TYPE_GPU, 0, nullptr,
		&deviceIdCount);


	if (deviceIdCount == 0) {
		std::cout << "couldn't acquire a GPU, falling back to CPU" << std::endl;
		error = clGetDeviceIDs(platformIds[1], CL_DEVICE_TYPE_CPU, 0, nullptr, &deviceIdCount);
		deviceIds.resize(deviceIdCount);
		error = clGetDeviceIDs(platformIds[1], CL_DEVICE_TYPE_CPU, deviceIdCount, deviceIds.data(), NULL);
	}
	else {
		std::cout << "acquired GPU cl target" << std::endl;
		deviceIds.resize(deviceIdCount);
		clGetDeviceIDs(platformIds[1], CL_DEVICE_TYPE_GPU, deviceIdCount, deviceIds.data(), nullptr);
	}



	if (error != 0) {
		std::cout << "Err: clGetDeviceIDs returned: " << error << std::endl;
		return error;
	}

	// Hurray for standards!
	// Setup the context properties to grab the current GL context
#ifdef linux
	cl_context_properties context_properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
		0
	};

#elif defined _WIN32
	//cl_context_properties context_properties[] = {
	//    CL_CONTEXT_PLATFORM, (cl_context_properties) platformIds[0],
	//    CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(),
	//    CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
	//    0
	//};
	HGLRC hGLRC = wglGetCurrentContext();
	HDC hDC = wglGetCurrentDC();
	cl_context_properties context_properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformIds[1], CL_GL_CONTEXT_KHR, (cl_context_properties)hGLRC, CL_WGL_HDC_KHR, (cl_context_properties)hDC, 0 };


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
	auto context = clCreateContext(
		context_properties,
		1,
		&deviceIds[0],
		nullptr, nullptr,
		&error
		);


	//cl_device_id devices[32]; 
	//size_t size;
	//clGetGLContextInfoKHR(context_properties, CL_DEVICES_FOR_GL_CONTEXT_KHR, 
	//	32 * sizeof(cl_device_id), devices, &size);

	if (error != 0) {
		std::cout << "Err: clCreateContext returned: " << error << std::endl;
		return error;
	}

	// And the cl command queue
	auto commandQueue = clCreateCommandQueue(context, deviceIds[0], 0, &error);

	if (error != 0) {
		std::cout << "Err: clCreateCommandQueue returned: " << error << std::endl;
		return error;
	}

	// At this point the shared GL/CL context is up and running
}

void Clapper::print_sys_info() {

	int error = 0;

	// Get the number of platforms
	cl_uint plt_cnt = 0;
	clGetPlatformIDs(0, nullptr, &plt_cnt);

	// Fetch the platforms
	std::vector<cl_platform_id> plt_ids(plt_cnt);
	clGetPlatformIDs(plt_cnt, plt_ids.data(), nullptr);


	// Print out this machines info

	std::cout << "============ Hardware info =================" << std::endl;

	for (unsigned int i = 0; i < plt_cnt; i++) {

		std::cout << "--Platform: " << i << std::endl;

		char platform[128];
		char version[128];

		clGetPlatformInfo(plt_ids[i], CL_PLATFORM_NAME, 128, platform, NULL);
		clGetPlatformInfo(plt_ids[i], CL_PLATFORM_VERSION, 128, version, NULL);

		std::cout << platform << "\n";
		std::cout << version << "\n\n";

		// get the number of devices, fetch them, choose the first one
		cl_uint deviceIdCount = 0;
		error = clGetDeviceIDs(plt_ids[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceIdCount);

		std::vector<cl_device_id> deviceIds(deviceIdCount);

		for (int q = 0; q < deviceIdCount; q++) {

			std::cout << "++++Device " << q << std::endl;
			error = clGetDeviceIDs(plt_ids[i], CL_DEVICE_TYPE_ALL, deviceIdCount, deviceIds.data(), NULL);

			clGetDeviceInfo(deviceIds[q], CL_DEVICE_NAME, 128, platform, NULL);
			clGetDeviceInfo(deviceIds[q], CL_DEVICE_VERSION, 128, version, NULL);

			std::cout << platform << "\n";
			std::cout << version << "\n\n";

		}
	}

	std::cout << "============================================" << std::endl;

}

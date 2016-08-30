#include "CL_Wrapper.h"

CL_Wrapper::CL_Wrapper() {
}


CL_Wrapper::~CL_Wrapper() {
}

int CL_Wrapper::acquire_platform_and_device(){

    // Get the number of platforms
    cl_uint plt_cnt = 0;
    clGetPlatformIDs(0, nullptr, &plt_cnt);

    // Fetch the platforms
    std::map<cl_platform_id, std::vector<device>> plt_ids;

    // buffer before map init
    std::vector<cl_platform_id> plt_buf(plt_cnt);
    clGetPlatformIDs(plt_cnt, plt_buf.data(), nullptr);

    // Map init
    for (auto id: plt_buf){
        plt_ids.emplace(std::make_pair(id, std::vector<device>()));
    }

    // For each platform, populate its devices
    for (unsigned int i = 0; i < plt_cnt; i++) {

        cl_uint deviceIdCount = 0;
        error = clGetDeviceIDs(plt_buf[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceIdCount);

        // Get the device ids
        std::vector<cl_device_id> deviceIds(deviceIdCount);
        error = clGetDeviceIDs(plt_buf[i], CL_DEVICE_TYPE_ALL, deviceIdCount, deviceIds.data(), NULL);

        if (assert(error, "clGetDeviceIDs"))
            return -1;

        for (int q = 0; q < deviceIdCount; q++) {

            device d;

            d.id = deviceIds[q];

            clGetDeviceInfo(d.id, CL_DEVICE_PLATFORM, 128, &d.platform, NULL);
            clGetDeviceInfo(d.id, CL_DEVICE_VERSION, 128, &d.version, NULL);
            clGetDeviceInfo(d.id, CL_DEVICE_TYPE, 128, &d.type, NULL);
            clGetDeviceInfo(d.id, CL_DEVICE_MAX_CLOCK_FREQUENCY, 128, &d.clock_frequency, NULL);

            plt_ids.at(d.platform).push_back(d);
        }
    }


    // The devices how now been queried we want to shoot for a gpu with the fastest clock,
    // falling back to the cpu with the fastest clock if we weren't able to find one

    device current_best_device;
    current_best_device.clock_frequency = 0; // Set this to 0 so the first run always selects a new device

    for (auto kvp: plt_ids){

        for (auto device: kvp.second){

            // Gonna just split this up into cases. There are so many devices I cant test with
            // that opencl supports. I'm not going to waste my time making a generic implimentation

            // Upon success of a condition, set the current best device values

            if (device.type == CL_DEVICE_TYPE_GPU && current_best_device.type != CL_DEVICE_TYPE_GPU){
                current_best_device = device;
            }
            else if (device.clock_frequency > current_best_device.clock_frequency){
                current_best_device = device;
            }
        }
    }

    platform_id = current_best_device.platform;
    device_id = current_best_device.id;

    return 0;
};

int CL_Wrapper::create_shared_context() {

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

    // TODO: Clean this up next time I'm on a windows machine
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
    context = clCreateContext(
            context_properties,
            1,
            &device_id,
            nullptr, nullptr,
            &error
    );

    if (assert(error, "clCreateContext"))
        return -1;

    return 0;
}

int CL_Wrapper::create_command_queue(){

    if (context && device_id) {
        // And the cl command queue
        command_queue = clCreateCommandQueue(context, device_id, 0, &error);

        if (assert(error, "clCreateCommandQueue"))
            return -1;

        return 0;
    }
    else {
        std::cout << "Failed creating the command queue, context or device_id not initialized";
        return -1;
    }
}

int CL_Wrapper::compile_kernel(std::string kernel_source, bool is_path, std::string kernel_name) {

    const char* source;
    std::string tmp;

    if (is_path){
        //Load in the kernel, and c stringify it
        tmp = read_file(kernel_source);
        source = tmp.c_str();
    } else {
        source = kernel_source.c_str();
    }

    size_t kernel_source_size = strlen(source);


    // Load the source into CL's data structure
    cl_program program = clCreateProgramWithSource(
            context, 1,
            &source,
            &kernel_source_size, &error
    );

    if (assert(error, "clCreateProgramWithSource"))
        return -1;


    // Try and build the program
    error = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    // Check to see if it errored out
    if (assert(error, "clBuildProgram")){

        // Get the size of the queued log
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = new char[log_size];

        // Grab the log
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        std::cout << log;
        return -1;
    }

    // Done initializing the kernel
    cl_kernel kernel = clCreateKernel(program, kernel_name.c_str(), &error);

    if (assert(error, "clCreateKernel"))
        return -1;

    kernel_map.emplace(std::make_pair(kernel_name, kernel));
}

int CL_Wrapper::set_kernel_arg(
        std::string kernel_name,
        int index,
        std::string buffer_name){

    error = clSetKernelArg(
            kernel_map.at(kernel_name),
            index,
            sizeof(cl_mem),
            (void *)&buffer_map.at(buffer_name));

    if (assert(error, "clSetKernelArg"))
        return -1;

    return 0;

}

int CL_Wrapper::store_buffer(cl_mem buffer, std::string buffer_name){
    buffer_map.emplace(std::make_pair(buffer_name, buffer));
}

int CL_Wrapper::run_kernel(std::string kernel_name){

    const int WORKER_SIZE = 1;
    size_t global_work_size[1] = { WORKER_SIZE };

    cl_kernel kernel = kernel_map.at(kernel_name);

    error = clEnqueueNDRangeKernel(
            command_queue, kernel,
            1, NULL, global_work_size,
            NULL, 0, NULL, NULL);

    if (assert(error, "clEnqueueNDRangeKernel"))
        return -1;


}



cl_device_id CL_Wrapper::getDeviceID(){ return device_id; };
cl_platform_id CL_Wrapper::getPlatformID(){ return platform_id; };
cl_context CL_Wrapper::getContext(){ return context; };
cl_kernel CL_Wrapper::getKernel(std::string kernel_name ){ return kernel_map.at(kernel_name); }

bool CL_Wrapper::assert(int error_code, std::string function_name){

    // Just gonna do a little jump table here, just error codes so who cares
    std::string err_msg = "Error : ";

    switch (error_code) {


        case CL_SUCCESS:
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
    }

    std::cout << err_msg << "  =at=  " << function_name << std::endl;
    return true;
}
#ifdef linux

#elif defined _WIN32

#elif defined TARGET_OS_MAC
# include <GL/glew.h>
# include <OpenGL/OpenGL.h>
# include <OpenCL/opencl.h>
#endif


int IsExtensionSupported(
        const char* support_str,
        const char* ext_string,
        size_t ext_buffer_size) {

    size_t offset = 0;

    const char* space_substr = strnstr(ext_string + offset, " ", ext_buffer_size - offset);

    size_t space_pos = space_substr ? space_substr - ext_string : 0;

    while (space_pos < ext_buffer_size) {

        if( strncmp(support_str, ext_string + offset, space_pos) == 0 ) {
            // Device supports requested extension!
            printf("Info: Found extension support ‘%s’!\n", support_str);
            return 1;
        }

        // Keep searching -- skip to next token string
        offset = space_pos + 1;
        space_substr = strnstr(ext_string + offset, " ", ext_buffer_size - offset);
        space_pos = space_substr ? space_substr - ext_string : 0;
    }

    printf("Warning: Extension not supported ‘%s’!\n", support_str);
    return 0;
}

int test_for_gl_cl_sharing() {


    int err = 0;
#if defined (__APPLE__) || defined(MACOSX)
    static const char *CL_GL_SHARING_EXT = "cl_APPLE_gl_sharing";
#else
    static const char* CL_GL_SHARING_EXT = "cl_khr_gl_sharing";
#endif

    cl_uint num_devices, i;
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

    cl_device_id *devices = (cl_device_id *) calloc(sizeof(cl_device_id), num_devices);
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

    // Get string containing supported device extensions
    size_t ext_size = 1024;
    char *ext_string = (char *) malloc(ext_size);
    err = clGetDeviceInfo(devices[0], CL_DEVICE_EXTENSIONS, ext_size, ext_string, &ext_size);

    free(devices);

    // Search for GL support in extension string (space delimited)
    int supported = IsExtensionSupported(CL_GL_SHARING_EXT, ext_string, ext_size);
    if (supported) {
        // Device supports context sharing with OpenGL
        printf("Found GL Sharing Support!\n");
        return 1;
    }
    return -1;
}


int query_platform_devices() {
    // From stackoverflow, gets and lists the compute devices
    cl_uint num_devices, i;
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

    cl_device_id *devices = (cl_device_id *) calloc(sizeof(cl_device_id), num_devices);
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

    char buf[128];
    for (i = 0; i < num_devices; i++) {
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 128, buf, NULL);
        fprintf(stdout, "Device %s supports ", buf);

        clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, 128, buf, NULL);
        fprintf(stdout, "%s\n", buf);
    }

    free(devices);

    return 1;
}
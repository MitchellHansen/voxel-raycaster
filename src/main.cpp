
/**
 *
 * Title:     Voxel Raycaster
 * Author:    Mitchell Hansen
 * License:   Apache 2.0
 * 
 * A little help from:
 *		John Amanatides & Andrew Woo: A Fast Voxel Traversal Algorithm forRay Tracing
 *		Samuli Laine & Tero Karras: Efficient Sparse Voxel Octrees
 *		
 * This project should not be viewed as a product or really anything other than
 * a cool little example on how real time volumetric ray marching is becoming 
 * very possible on todays hardware. Don't expect this program to work on your hardware,
 * perform as expected, have documentation, walk your dog, etc. 
 * 
 */

/**
 * TODO:
 *   + REWRITE!
 */

//#include "Application.h"


#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <Logger.h>
#include <VKaster.h>

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f) 											    			   \
{																					   \
    VkResult res = (f);																   \
    if (res != VK_SUCCESS)															   \
    {																				   \
        printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__); \
        assert(res == VK_SUCCESS);													   \
    }																				   \
}

std::string vk_error_lookup(int error_code) {

    std::string err_msg;

    switch (error_code) {
        case VK_NOT_READY:
            err_msg += "VK_NOT_READY";
            break;
        case VK_TIMEOUT:
            err_msg += "VK_TIMEOUT";
            break;
        case VK_EVENT_SET:
            err_msg += "VK_EVENT_SET";
            break;
        case VK_EVENT_RESET:
            err_msg += "VK_EVENT_RESET";
            break;
        case VK_INCOMPLETE:
            err_msg += "VK_INCOMPLETE";
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            err_msg += "VK_ERROR_OUT_OF_HOST_MEMORY";
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            err_msg += "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            err_msg += "VK_ERROR_INITIALIZATION_FAILED";
            break;
        case VK_ERROR_DEVICE_LOST:
            err_msg += "VK_ERROR_DEVICE_LOST";
            break;
        case VK_ERROR_MEMORY_MAP_FAILED:
            err_msg += "VK_ERROR_MEMORY_MAP_FAILED";
            break;
        case VK_ERROR_LAYER_NOT_PRESENT:
            err_msg += "VK_ERROR_LAYER_NOT_PRESENT";
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            err_msg += "VK_ERROR_EXTENSION_NOT_PRESENT";
            break;
        case VK_ERROR_FEATURE_NOT_PRESENT:
            err_msg += "VK_ERROR_FEATURE_NOT_PRESENT";
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            err_msg += "VK_ERROR_INCOMPATIBLE_DRIVER";
            break;
        case VK_ERROR_TOO_MANY_OBJECTS:
            err_msg += "VK_ERROR_TOO_MANY_OBJECTS";
            break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            err_msg += "VK_ERROR_FORMAT_NOT_SUPPORTED";
            break;
        case VK_ERROR_FRAGMENTED_POOL:
            err_msg += "VK_ERROR_FRAGMENTED_POOL";
            break;
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            err_msg += "VK_ERROR_OUT_OF_POOL_MEMORY";
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            err_msg += "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            err_msg += "VK_ERROR_SURFACE_LOST_KHR";
            break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            err_msg += "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            break;
        case VK_SUBOPTIMAL_KHR:
            err_msg += "VK_SUBOPTIMAL_KHR";
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            err_msg += "VK_ERROR_OUT_OF_DATE_KHR";
            break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            err_msg += "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            break;
        case VK_ERROR_VALIDATION_FAILED_EXT:
            err_msg += "VK_ERROR_VALIDATION_FAILED_EXT";
            break;
        case VK_ERROR_INVALID_SHADER_NV:
            err_msg += "VK_ERROR_INVALID_SHADER_NV";
            break;
        case VK_ERROR_FRAGMENTATION_EXT:
            err_msg += "VK_ERROR_FRAGMENTATION_EXT";
            break;
        case VK_ERROR_NOT_PERMITTED_EXT:
            err_msg += "VK_ERROR_NOT_PERMITTED_EXT";
            break;
        default:
            err_msg = "UNKNOWN_ERROR";
    }

    return err_msg;
}

bool vk_assert(int error_code) {

    if (error_code == VK_SUCCESS || error_code == 1)
        return false;
    else
        return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(
        VkDebugReportFlagsEXT                       flags,
        VkDebugReportObjectTypeEXT                  objectType,
        uint64_t                                    object,
        size_t                                      location,
        int32_t                                     messageCode,
        const char*                                 pLayerPrefix,
        const char*                                 pMessage,
        void*                                       pUserData) {

    printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);

    return VK_FALSE;
}


VkInstance                instance;
VkPhysicalDevice          physicalDevice;
VkDevice                  device;
VkPipeline                pipeline;
VkPipelineLayout          pipelineLayout;
VkShaderModule            computeShaderModule;
VkCommandPool             commandPool;
VkCommandBuffer           commandBuffer;
VkDescriptorPool          descriptorPool;
VkDescriptorSet           descriptorSet;
VkDescriptorSetLayout     descriptorSetLayout;
VkBuffer                  buffer;
VkDeviceMemory            bufferMemory;
uint32_t                  bufferSize;
VkQueue                   queue;
uint32_t                  queueFamilyIndex;

int main() {

//	Application application;
//	application.init_clcaster();
//	application.init_events();
//	application.game_loop();


    // Buffer size of the storage buffer that will contain the rendered mandelbrot set.

    VKaster cast;
    cast.createInstance();
    cast.findPhysicalDevice();
    cast.createDevice();



//    VKaster app;
//
//    app.createInstance();
//    app.findPhysicalDevice();

//    // Initialize vulkan:
    Logger::log("Creating Instance", Logger::LogLevel::INFO, __LINE__, __FILE__);




//    createDevice();
//    createBuffer();
//    createDescriptorSetLayout();
//    createDescriptorSet();
//    createComputePipeline();
//    createCommandBuffer();
//
//    // Finally, init the recorded command buffer.
//    runCommandBuffer();
//
//    // The former command rendered a mandelbrot set to a buffer.
//    // Save that buffer as a png on disk.
//    saveRenderedImage();
//
//    // Clean up all vulkan resources.
//    cleanup();



    return 0;
}

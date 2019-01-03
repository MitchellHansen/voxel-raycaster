#pragma once
#include <vulkan/vulkan.h>

#include <vector>
#include <string.h>
#include <assert.h>
#include <stdexcept>
#include <cmath>
#include "Logger.h"

const int WIDTH = 3200; // Size of rendered mandelbrot set.
const int HEIGHT = 2400; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32; // Workgroup size in compute shader.

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f) 																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__); \
        assert(res == VK_SUCCESS);																		\
    }																									\
}


class VKaster {

public:
    bool init();

    bool saveRenderedImage();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(
            VkDebugReportFlagsEXT                       flags,
            VkDebugReportObjectTypeEXT                  objectType,
            uint64_t                                    object,
            size_t                                      location,
            int32_t                                     messageCode,
            const char*                                 pLayerPrefix,
            const char*                                 pMessage,
            void*                                       pUserData) ;

    /**
     * Create the Vulkan instance and check for enabled layers
     * @return Success or Failure
     */
    bool createInstance() ;

    /**
     * Look for the correct physical device to use
     * TODO: User interaction and print out of available hardware devices
     * @return Success or Failure
     */
    bool findPhysicalDevice() ;

    /**
     * Create a device and associated device queue belonging to the device found in findPhysicalDevice
     * @return Success or Failure
     */
    bool createDevice() ;

    // find memory type with desired properties.
    uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) ;

    bool createBuffer();

    bool createDescriptorSetLayout() ;

    bool createDescriptorSet() ;

    // Read file into array of bytes, and cast to uint32_t*, then return.
    // The data has been padded, so that it fits into an array uint32_t.
    uint32_t* readFile(uint32_t& length, const char* filename) ;

    bool createComputePipeline() ;

    bool createCommandBuffer() ;

    bool runCommandBuffer() ;

    bool cleanup() ;

    bool vk_assert(int error_code);
    std::string vk_error_lookup(int error_code);

private:

    const std::string APPLICATION_NAME = {"VULKAN APPLICATION"};
    const std::string ENGINE_NAME      = {"VULKAN ENGINE"};

    /**
     * Walk through the queue families supported by the device found by findPhysicalDevice.
     * @return The index of a queue family that supports COMPUTE
     */
    uint32_t getComputeQueueFamilyIndex() ;

    struct Pixel {
        float r, g, b, a;
    };

    VkInstance instance;
    VkDebugReportCallbackEXT debugReportCallback;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkShaderModule computeShaderModule;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    uint32_t bufferSize;
    VkQueue queue;
    uint32_t queueFamilyIndex;
    VkResult err;
    std::vector<const char *> enabledLayers;

};


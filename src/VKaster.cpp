
#include <VKaster.h>

#include "VKaster.h"

bool VKaster::init()  {

    Logger::log("Initializing the Hardware Caster", Logger::LogLevel::INFO);

    if (!createInstance()) {
        Logger::log("Failed to createInstance", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    if (!findPhysicalDevice()) {
        Logger::log("Failed to findPhysicalDevice", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    if (!createDevice()) {
        Logger::log("Failed to createDevice", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    if (!createBuffer()) {
        Logger::log("Failed to createBuffer", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    if (!createDescriptorSetLayout()) {
        Logger::log("Failed to createDescriptorSetLayout", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    if (!createDescriptorSet()) {
        Logger::log("Failed to createDescriptorSet", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    if (!createComputePipeline()) {
        Logger::log("Failed to createComputePipeline", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    if (!createCommandBuffer()) {
        Logger::log("Failed to createCommandBuffer", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    if (!runCommandBuffer()) {
        Logger::log("Failed to runCommandBuffer", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    return true;

    // The former command rendered a mandelbrot set to a buffer.
    // Save that buffer as a png on disk.
    saveRenderedImage();

    // Clean up all vulkan resources.
    cleanup();
}

bool VKaster::saveRenderedImage()  {
    void* mappedMemory = NULL;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &mappedMemory);
    Pixel* pmappedMemory = (Pixel *)mappedMemory;

    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    std::vector<unsigned char> image;
    image.reserve(WIDTH * HEIGHT * 4);
    for (int i = 0; i < WIDTH*HEIGHT; i += 1) {
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].r)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].g)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].b)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].a)));
    }
    // Done reading, so unmap.
    vkUnmapMemory(device, bufferMemory);

}

VkBool32 VKaster::debugReportCallbackFn(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                                   uint64_t object, size_t location, int32_t messageCode,
                                                   const char *pLayerPrefix, const char *pMessage, void *pUserData) {

    printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);

    return VK_FALSE;
}

bool VKaster::createInstance() {

    std::vector<const char *> enabledExtensions;

    /*
    By enabling validation layers, Vulkan will emit warnings if the API
    is used incorrectly. We shall enable the layer VK_LAYER_LUNARG_standard_validation,
    which is basically a collection of several useful validation layers.
    */
    bool enableValidationLayers = false;
    if (enableValidationLayers) {
        /*
        We get all supported layers with vkEnumerateInstanceLayerProperties.
        */
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, NULL);

        std::vector<VkLayerProperties> layerProperties(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

        /*
        And then we simply check if VK_LAYER_LUNARG_standard_validation is among the supported layers.
        */
        bool foundLayer = false;
        for (VkLayerProperties prop : layerProperties) {

            if (strcmp("VK_LAYER_LUNARG_standard_validation", prop.layerName) == 0) {
                foundLayer = true;
                break;
            }

        }

        if (!foundLayer) {
            throw std::runtime_error("Layer VK_LAYER_LUNARG_standard_validation not supported\n");
        }
        enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation"); // Alright, we can use this layer.

        /*
        We need to enable an extension named VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        in order to be able to print the warnings emitted by the validation layer.

        So again, we just check if the extension is among the supported extensions.
        */

        uint32_t extensionCount;

        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties.data());

        bool foundExtension = false;
        for (VkExtensionProperties prop : extensionProperties) {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0) {
                foundExtension = true;
                break;
            }

        }

        if (!foundExtension) {
            throw std::runtime_error("Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
        }
        enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // Fill in the application info, pay attention to API VERSION
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = APPLICATION_NAME.c_str();
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = ENGINE_NAME.c_str();
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_API_VERSION_1_0;;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;

    // Give our desired layers and extensions to vulkan.
    createInfo.enabledLayerCount       = enabledLayers.size();
    createInfo.ppEnabledLayerNames     = enabledLayers.data();

    createInfo.enabledExtensionCount   = enabledExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VkResult err = vkCreateInstance(&createInfo,NULL,&instance);
    if (vk_assert(err)){
        Logger::log("Error at vkCreateInstance() : " + vk_error_lookup(err), Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    /*
    Register a callback function for the extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME, so that warnings emitted from the validation
    layer are actually printed.
    */
    if (enableValidationLayers) {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        createInfo.pfnCallback = &debugReportCallbackFn;

        // We have to explicitly load this function.
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        if (vkCreateDebugReportCallbackEXT == nullptr) {
            throw std::runtime_error("Could not load vkCreateDebugReportCallbackEXT");
        }

        // Create and register callback.
        VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(instance, &createInfo, NULL, &debugReportCallback));
    }

}

bool VKaster::findPhysicalDevice() {

    Logger::log("Acquiring Graphics Hardware", Logger::LogLevel::INFO);

    // Get the device count
    uint32_t deviceCount;
    err = vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    if (vk_assert(err)) {
        Logger::log("vKEnumeratePhysicalDevices : " + vk_error_lookup(err), Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    if (deviceCount == 0) {
        Logger::log("Could not find a device with Vulkan support", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (VkPhysicalDevice device : devices) {
        physicalDevice = device;
        break;
    }

    return true;
}

uint32_t VKaster::getComputeQueueFamilyIndex() {
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    // Retrieve all queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    // Now find a family that supports compute.
    uint32_t i = 0;
    for (; i < queueFamilies.size(); ++i) {
        VkQueueFamilyProperties properties = queueFamilies[i];

        if (properties.queueCount > 0 && (properties.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            // found a queue with compute. We're done!
            break;
        }
    }

    if (i == queueFamilies.size()) {
        throw std::runtime_error("could not find a queue family that supports operations");
    }

    return i;
}

bool VKaster::createDevice() {

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamilyIndex = getComputeQueueFamilyIndex(); // find queue family with compute capability.
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1; // create one queue in this family. We don't need more.
    float queuePriorities = 1.0;    // we only have one queue, so this is not that imporant.
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo       deviceCreateInfo = {};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledLayerCount = enabledLayers.size();
    deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    err = vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);

    if (vk_assert(err)) {
        Logger::log("Could not create a device from physical device", Logger::LogLevel::ERROR, __LINE__, __FILE__);
        return false;
    }

    // Get a handle to the only member of the queue family.
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

uint32_t VKaster::findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }

    Logger::log("Could not determine memory type", Logger::LogLevel::ERROR, __LINE__, __FILE__);
    return -1;
}

bool VKaster::createBuffer() {
    /*
    We will now create a buffer. We will render the mandelbrot set into this buffer
    in a computer shade later.
    */

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = bufferSize; // buffer size in bytes.
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // buffer is used as a storage buffer.
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // buffer is exclusive to a single queue family at a time.

    VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer)); // create buffer.

    /*
    But the buffer doesn't allocate memory for itself, so we must do that manually.
    */

    /*
    First, we find the memory requirements for the buffer.
    */
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    /*
    Now use obtained memory requirements info to allocate the memory for the buffer.
    */
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size; // specify required memory.
    /*
    There are several types of memory that can be allocated, and we must choose a memory type that:

    1) Satisfies the memory requirements(memoryRequirements.memoryTypeBits).
    2) Satifies our own usage requirements. We want to be able to read the buffer memory from the GPU to the CPU
       with vkMapMemory, so we set VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
    Also, by setting VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory written by the device(GPU) will be easily
    visible to the host(CPU), without having to call any extra flushing commands. So mainly for convenience, we set
    this flag.
    */
    allocateInfo.memoryTypeIndex = findMemoryType(
            memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, &bufferMemory)); // allocate memory on device.

    // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory.
    VK_CHECK_RESULT(vkBindBufferMemory(device, buffer, bufferMemory, 0));
}

bool VKaster::createDescriptorSetLayout() {
    /*
    Here we specify a descriptor set layout. This allows us to bind our descriptors to
    resources in the shader.

    */

    /*
    Here we specify a binding of type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER to the binding point
    0. This binds to

      layout(std140, binding = 0) buffer buf

    in the compute shader.
    */
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
    descriptorSetLayoutBinding.binding = 0; // binding = 0
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1; // only a single binding in this descriptor set layout.
    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

    // Create the descriptor set layout.
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));
}

bool VKaster::createDescriptorSet() {
    /*
    So we will allocate a descriptor set here.
    But we need to first create a descriptor pool to do that.
    */

    /*
    Our descriptor pool can only allocate a single storage buffer.
    */
    VkDescriptorPoolSize descriptorPoolSize = {};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

    // create descriptor pool.
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));

    /*
    With the pool allocated, we can now allocate the descriptor set.
    */
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

    /*
    Next, we need to connect our actual storage buffer with the descrptor.
    We use vkUpdateDescriptorSets() to update the descriptor set.
    */

    // Specify the buffer to bind to the descriptor.
    VkDescriptorBufferInfo descriptorBufferInfo = {};
    descriptorBufferInfo.buffer = buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = bufferSize;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet; // write to this descriptor set.
    writeDescriptorSet.dstBinding = 0; // write to the first, and only binding.
    writeDescriptorSet.descriptorCount = 1; // update a single descriptor.
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

    // perform the update of the descriptor set.
    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
}

uint32_t *VKaster::readFile(uint32_t &length, const char *filename) {

    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not find or open file: %s\n", filename);
    }

    // get file size.
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    long filesizepadded = long(ceil(filesize / 4.0)) * 4;

    // read file contents.
    char *str = new char[filesizepadded];
    fread(str, filesize, sizeof(char), fp);
    fclose(fp);

    // data padding.
    for (int i = filesize; i < filesizepadded; i++) {
        str[i] = 0;
    }

    length = filesizepadded;
    return (uint32_t *)str;
}

bool VKaster::createComputePipeline() {
    /*
    We create a compute pipeline here.
    */

    /*
    Create a shader module. A shader module basically just encapsulates some shader code.
    */
    uint32_t filelength;
    // the code in comp.spv was created by running the command:
    // glslangValidator.exe -V shader.comp
    uint32_t* code = readFile(filelength, "../shaders/comp.spv");
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pCode = code;
    createInfo.codeSize = filelength;

    VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, NULL, &computeShaderModule));
    delete[] code;

    /*
    Now let us actually create the compute pipeline.
    A compute pipeline is very simple compared to a graphics pipeline.
    It only consists of a single stage with a compute shader.

    So first we specify the compute shader stage, and it's entry point(main).
    */
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.module = computeShaderModule;
    shaderStageCreateInfo.pName = "main";

    /*
    The pipeline layout allows the pipeline to access descriptor sets.
    So we just specify the descriptor set layout we created earlier.
    */
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));

    VkComputePipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;

    /*
    Now, we finally create the compute pipeline.
    */
    VK_CHECK_RESULT(vkCreateComputePipelines(
            device, VK_NULL_HANDLE,
            1, &pipelineCreateInfo,
            NULL, &pipeline));
}

bool VKaster::createCommandBuffer() {
    /*
    We are getting closer to the end. In order to send commands to the device(GPU),
    we must first record commands into a command buffer.
    To allocate a command buffer, we must first create a command pool. So let us do that.
    */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;
    // the queue family of this command pool. All command buffers allocated from this command pool,
    // must be submitted to queues of this family ONLY.
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool));

    /*
    Now allocate a command buffer from the command pool.
    */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool; // specify the command pool to allocate from.
    // if the command buffer is primary, it can be directly submitted to queues.
    // A secondary buffer has to be called from some primary command buffer, and cannot be directly
    // submitted to a queue. To keep things simple, we use a primary command buffer.
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1; // allocate a single command buffer.
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer)); // allocate command buffer.

    /*
    Now we shall start recording commands into the newly allocated command buffer.
    */
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // the buffer is only submitted and used once in this application.
    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo)); // start recording commands.

    /*
    We need to bind a pipeline, AND a descriptor set before we dispatch.

    The validation layer will NOT give warnings if you forget these, so be very careful not to forget them.
    */
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

    /*
    Calling vkCmdDispatch basically starts the compute pipeline, and executes the compute shader.
    The number of workgroups is specified in the arguments.
    If you are already familiar with compute shaders from OpenGL, this should be nothing new to you.
    */
    vkCmdDispatch(commandBuffer, (uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer)); // end recording commands.
}

bool VKaster::runCommandBuffer() {
    /*
    Now we shall finally submit the recorded command buffer to a queue.
    */

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1; // submit a single command buffer
    submitInfo.pCommandBuffers = &commandBuffer; // the command buffer to submit.

    /*
      We create a fence.
    */
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));

    /*
    We submit the command buffer on the queue, at the same time giving a fence.
    */
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
    /*
    The command will not have finished executing until the fence is signalled.
    So we wait here.
    We will directly after this read our buffer from the GPU,
    and we will not be sure that the command has finished executing unless we wait for the fence.
    Hence, we use a fence here.
    */
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

    vkDestroyFence(device, fence, NULL);
}

bool VKaster::cleanup() {
    /*
    Clean up all Vulkan Resources.
    */

    bool enableValidationLayers = false;
    if (enableValidationLayers) {
        // destroy callback.
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        if (func == nullptr) {
            throw std::runtime_error("Could not load vkDestroyDebugReportCallbackEXT");
        }
        func(instance, debugReportCallback, NULL);
    }

    vkFreeMemory(device, bufferMemory, NULL);
    vkDestroyBuffer(device, buffer, NULL);
    vkDestroyShaderModule(device, computeShaderModule, NULL);
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyCommandPool(device, commandPool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
}

bool VKaster::vk_assert(int error_code) {

    if (error_code == VK_SUCCESS || error_code == 1)
        return false;
    else
        return true;
}

std::string VKaster::vk_error_lookup(int error_code) {

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

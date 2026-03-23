// Vulkan Test
// By: Jupiter Sinclair Chong

#include "renderer.h"

namespace Renderer {

#pragma region Component Classes

    // Instance

    bool Instance::checkValidationLayerSupport(std::vector<const char*> validationLayers) {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const VkLayerProperties& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    Instance::Instance() {}

    Instance::Instance(bool enableValidationLayers, std::vector<const char*> validationLayers) {
        printf("[Vulkan]\t Creating Instance...\n");

        if (enableValidationLayers && !checkValidationLayerSupport(validationLayers))
            throw std::runtime_error("Validation layers requested, but not available");

        printf("[Vulkan]\t Setup validation layers\n");

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Determine the extensions needed to interface with GLFW
        uint32_t glfwExtensionCount = 0;
        uint32_t extensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;

        // Load validation layers into Create Info
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // List loaded extensions

        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "[Vulkan]\t Supported Extensions:\n";

        for (const VkExtensionProperties& extension : extensions)
            std::cout << '\t' << extension.extensionName << '\n';

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
        else {
            printf("[Vulkan]\t Created Instance\n");
        }

    }

    Instance::~Instance() {
        //vkDestroyInstance(instance, nullptr);
        //printf("[Cleanup]\t Destroyed Instance\n");
    }

    Instance& Instance::operator=(const Instance& other)
    {
        this->instance = other.instance;

        return *this;
    }

    void Instance::cleanup() {
        vkDestroyInstance(instance, nullptr);
        printf("[Cleanup]\t Destroyed Instance\n");
        instance = VK_NULL_HANDLE;
    }

    // Surface

    Surface::Surface() {}

    Surface::Surface(Instance* instance, GLFWwindow* window) {
        if (glfwCreateWindowSurface(instance->instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }

        this->instance = instance;

        printf("[Vulkan]\t Created Surface\n");
    }

    Surface::~Surface() {
        //vkDestroySurfaceKHR(instance->instance, surface, nullptr);
        //printf("[Cleanup]\t Destroyed Surface\n");
    }

    void Surface::cleanup() {
        vkDestroySurfaceKHR(instance->instance, surface, nullptr);
        printf("[Cleanup]\t Destroyed Surface\n");

        instance = nullptr;
    }

#pragma region Helper Functions

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            // TODO Problem, surface invalid for some reason
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const VkExtensionProperties& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // Query supported surface formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        // Queeery Presentation Modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    /// <summary>
    /// Params for how we determine whether to use a device
    /// TODO Only checking queue families RN. Update this to be more sophisticated
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    bool isDeviceSuitable(VkPhysicalDevice device, std::vector<const char*> deviceExtensions, VkSurfaceKHR surface) {
        QueueFamilyIndices indices = findQueueFamilies(device, surface);
        bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    /// <summary>
    /// Find first GPU that supports the features we want
    /// TODO Update this to be more sophisticated. EX: rate GPU suitablility and choose the highest, or just pick dedicated GPU
    /// </summary>
    void pickPhysicalDevice(VkPhysicalDevice& physicalDevice, Instance* instance, std::vector<const char*> deviceExtensions, VkSurfaceKHR surface) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance->instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance->instance, &deviceCount, devices.data());

        for (const VkPhysicalDevice& device : devices) {
            if (isDeviceSuitable(device, deviceExtensions, surface)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }

        printf("[Vulkan]\t Picked GPU\n");
    }

#pragma endregion

    // Logical Device

    LogicalDevice::LogicalDevice() {}

    LogicalDevice::LogicalDevice(Instance* instance, LogicalDeviceCreateInfo ci, Surface* surface) {
        this->instance = instance;
        this->surface = surface;

        pickPhysicalDevice(physicalDevice, instance, ci.deviceExtensions, surface->surface);

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface->surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value()
        };

        float queuePriority = 1.0f;

        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(ci.deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = ci.deviceExtensions.data();

        if (ci.enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(ci.validationLayers.size());
            createInfo.ppEnabledLayerNames = ci.validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

        printf("[Vulkan]\t Created Logical Device\n");
    }

    LogicalDevice::~LogicalDevice() {
        //vkDestroyDevice(device, nullptr);
        //printf("[Cleanup]\t Destroyed Logical Device\n");
    }

    void LogicalDevice::cleanup() {
        vkDestroyDevice(device, nullptr);
        printf("[Cleanup]\t Destroyed Logical Device\n");

        device = VK_NULL_HANDLE;
        physicalDevice = VK_NULL_HANDLE;
        surface = nullptr;
        instance = nullptr;
    }

    // Swap Chain

    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const VkSurfaceFormatKHR& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const VkPresentModeKHR& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    /// <summary>
    /// So we can actually use the images in or swap chain
    /// </summary>
    void SwapChain::createImageViews(VkDevice device) {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    SwapChain::SwapChain() {
        this->logicalDevice = nullptr;
    }

    SwapChain::SwapChain(LogicalDevice* logicalDevice, Surface* surface, GLFWwindow* window) {
        this->logicalDevice = logicalDevice;
        
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(logicalDevice->physicalDevice, surface->surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface->surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(logicalDevice->physicalDevice, surface->surface);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;       // Optional
            createInfo.pQueueFamilyIndices = nullptr;   // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(logicalDevice->device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(logicalDevice->device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(logicalDevice->device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;

        printf("[Vulkan]\t Created Swap Chain\n");

        createImageViews(logicalDevice->device);

        printf("[Vulkan]\t Created Image Views\n");
    }

    SwapChain::~SwapChain() {
        //for (VkImageView imageView : swapChainImageViews) {
        //    vkDestroyImageView(logicalDevice->device, imageView, nullptr);
        //}

        //vkDestroySwapchainKHR(logicalDevice->device, swapChain, nullptr);
    }

    void SwapChain::cleanup() {
        for (VkImageView imageView : swapChainImageViews) {
            vkDestroyImageView(logicalDevice->device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(logicalDevice->device, swapChain, nullptr);

        printf("[Cleanup]\t Destroyed Swap Chain\n");

        logicalDevice = nullptr;
        swapChain = VK_NULL_HANDLE;
    }

    // Render Pass
    // TODO Create info for this guy too

    RenderPass::RenderPass() {
        //this->logicalDevice == nullptr;
        //printf("[Bruh]\t Render Pass Default Constructor :(\n");
    }

    RenderPass::RenderPass(LogicalDevice* logicalDevice, VkFormat swapChainImageFormat) {
        this->logicalDevice = logicalDevice;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // Subpasses are later rendering operations that depend on the contents of framebuffers in previous passes
        // Subpasses can be used for stacking post processing effects more memory efficiently cuz Vulkan reorders them
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;    // yay
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(logicalDevice->device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
        else {
            std::cout << "[Vulkan]\t Created A Render Pass\n";
        }
    }

    RenderPass::~RenderPass() {
        //vkDestroyRenderPass(logicalDevice->device, renderPass, nullptr);
        //printf("[Cleanup]\t Destroyed Render Pass");
    }

    RenderPass::RenderPass(const RenderPass& other) : 
        renderPass(other.renderPass), logicalDevice(other.logicalDevice)
    {}

    RenderPass& RenderPass::operator=(const RenderPass& other)
    {
        this->logicalDevice = other.logicalDevice;
        this->renderPass = other.renderPass;

        return *this;
    }

    void RenderPass::cleanup() {
        vkDestroyRenderPass(logicalDevice->device, renderPass, nullptr);
        printf("[Cleanup]\t Destroyed Render Pass\n");

        logicalDevice = nullptr;
        renderPass = VK_NULL_HANDLE;
    }

    // Graphics Pipeline
    // TODO Make shader class
    // TODO Pass in create info on application layer

    /// <summary>
    /// Helper function for loading shaders
    /// </summary>
    std::vector<char> GraphicsPipeline::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        printf("[Vulkan]\t Shader Loaded: ");
        printf(filename.c_str());
        printf("\n");

        return buffer;
    }

    /// <summary>
    /// Byte code -> usable shader module
    /// </summary>
    VkShaderModule GraphicsPipeline::createShaderModule(VkDevice device, const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        // this is getting really familiar
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    GraphicsPipeline::GraphicsPipeline() {}

    /// <summary>
    /// The name describes it
    /// Can we load shaders later and insert them?
    /// </summary>
    GraphicsPipeline::GraphicsPipeline(LogicalDevice* device, VkExtent2D swapChainExtent, RenderPass* renderPass) {
        logicalDevice = device;

        std::vector<char> vertShaderCode = readFile("shaders/vert.spv");
        std::vector<char> fragShaderCode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(logicalDevice->device, vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(logicalDevice->device, fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        // What kind of primitive will be drawn
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Self explanitory
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // Fixed function
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(logicalDevice->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        else {
            printf("[Vulkan]\t Created Graphics Pipeline Layout\n");
        }

        // Finally The actual struct
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;

        // Fixed function stages
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;

        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass->renderPass;
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional


        if (vkCreateGraphicsPipelines(logicalDevice->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        else {
            printf("[Vulkan]\t Created Graphics Pipeline!\n");
        }

        vkDestroyShaderModule(logicalDevice->device, fragShaderModule, nullptr);
        vkDestroyShaderModule(logicalDevice->device, vertShaderModule, nullptr);
    }

    void GraphicsPipeline::cleanup() {
        vkDestroyPipeline(logicalDevice->device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(logicalDevice->device, pipelineLayout, nullptr);

        logicalDevice = nullptr;
        pipelineLayout = VK_NULL_HANDLE;
        graphicsPipeline = VK_NULL_HANDLE;
    }

    // Framebuffer

    Framebuffer::Framebuffer(LogicalDevice* logicalDevice, SwapChain* swapChain, RenderPass* renderPass) {
        this->logicalDevice = logicalDevice;

        swapChainFramebuffers.resize(swapChain->swapChainImageViews.size());

        for (size_t i = 0; i < swapChain->swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                swapChain->swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass->renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChain->swapChainExtent.width;
            framebufferInfo.height = swapChain->swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(logicalDevice->device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }

        printf("[Vulkan]\t Created Framebuffers\n");
    }

    void Framebuffer::cleanup() {
        for (VkFramebuffer framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(logicalDevice->device, framebuffer, nullptr);
        }

        logicalDevice = nullptr;
    }
    
    // Command Pool - Combine these?

    CommandPool::CommandPool(LogicalDevice* logicalDevice, Surface* surface) {
        this->logicalDevice = logicalDevice;

        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(logicalDevice->physicalDevice, surface->surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(logicalDevice->device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }

        printf("[Vulkan]\t Created Command Pool\n");
    }

    void CommandPool::cleanup() {
        vkDestroyCommandPool(logicalDevice->device, commandPool, nullptr);
        printf("[Cleanup]\t Destroyed Command Pool\n");

        logicalDevice = nullptr;
    }

    // Command Buffer

    CommandBuffer::CommandBuffer(LogicalDevice* logicalDevice, CommandPool* commandPool) {
        this->logicalDevice = logicalDevice;
        
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(logicalDevice->device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        } else {
            printf("[Vulkan]\t Created Command Buffer\n");
        }
    }

    void CommandBuffer::cleanup() {

        logicalDevice = nullptr;
    }

    // Sync Objects

    SyncObjects::SyncObjects(LogicalDevice* logicalDevice) {
        this->logicalDevice = logicalDevice;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;     // Starting off signaled to avoid infinite loop on first draw

        if (vkCreateSemaphore(logicalDevice->device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(logicalDevice->device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(logicalDevice->device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        } else {
            printf("[Vulkan]\t Created Sync Objects\n");
        }
    }

    void SyncObjects::cleanup() {
        vkDestroySemaphore(logicalDevice->device, imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(logicalDevice->device, renderFinishedSemaphore, nullptr);
        vkDestroyFence(logicalDevice->device, inFlightFence, nullptr);

        logicalDevice = nullptr;
    }

#pragma endregion

    // Renderer Layer

    RendererLayer::RendererLayer(RendererLayer& rh) {
        this->window = rh.window;
        //this->graphicsQueue = rh.graphicsQueue;
        //this->presentQueue = rh.presentQueue;
        this->validationLayers = rh.validationLayers;
        this->deviceExtensions = rh.deviceExtensions;

        this->instance = rh.instance;
        this->surface = rh.surface;
        this->logicalDevice = rh.logicalDevice;
        this->swapChain = rh.swapChain;
        this->renderPass = rh.renderPass;
        this->graphicsPipeline = rh.graphicsPipeline;
        this->framebuffers = rh.framebuffers;
        this->commandPool = rh.commandPool;
        this->commandBuffer = rh.commandBuffer;
        this->syncObjects= rh.syncObjects;
    }

    RendererLayer::RendererLayer(RendererLayerCreateInfo createInfo) {
        this->window = createInfo.window;

        printf("[Vulkan]\t Initting Vulkan\n");

        // ? Hmmm who should own these cuz they're being passed twice in 2 diff ways rn
        validationLayers = createInfo.LI_CreateInfo.validationLayers;
        deviceExtensions = createInfo.LI_CreateInfo.deviceExtensions;

        // ? Should I pass in wrapper objects or Vulkan objects if there's only 1 thing being used

        instance = Instance(enableValidationLayers, validationLayers);

        surface = Surface(&instance, createInfo.window);

        logicalDevice = LogicalDevice(&instance, createInfo.LI_CreateInfo, &surface);

        swapChain = SwapChain(&logicalDevice, &surface, window);

        renderPass = RenderPass(&logicalDevice, swapChain.swapChainImageFormat);

        graphicsPipeline = GraphicsPipeline(&logicalDevice, swapChain.swapChainExtent, &renderPass);

        framebuffers = Framebuffer(&logicalDevice, &swapChain, &renderPass);

        commandPool = CommandPool(&logicalDevice, &surface);

        commandBuffer = CommandBuffer(&logicalDevice, &commandPool);

        syncObjects = SyncObjects(&logicalDevice);

        printf("[Vulkan]\t Ready!\n");
    }

    RendererLayer::~RendererLayer() {
        printf("[Cleanup]\t Destroying Renderer Layer...\n");

        // ? Should each object hold device or should I just pass it in
        // ? Deconstructor no work like this cuz it runs on copy assignment
        
        syncObjects.cleanup();
        commandPool.cleanup();
        framebuffers.cleanup();
        graphicsPipeline.cleanup();
        renderPass.cleanup();
        swapChain.cleanup();
        surface.cleanup();
        logicalDevice.cleanup();
        instance.cleanup();
        
        //printf("[Cleanup]\t Destroyed Renderer Layer\n");
      }

    RendererLayer& RendererLayer::operator=(const RendererLayer& other) {
        this->window = other.window;
        //this->graphicsQueue = other.graphicsQueue;
        //this->presentQueue = other.presentQueue;
        this->validationLayers = other.validationLayers;
        this->deviceExtensions = other.deviceExtensions;

        this->instance = other.instance;
        this->surface = other.surface;
        this->logicalDevice = other.logicalDevice;
        this->renderPass = other.renderPass;
        this->graphicsPipeline = other.graphicsPipeline;
        this->framebuffers = other.framebuffers;
        this->commandPool = other.commandPool;
        this->commandBuffer = other.commandBuffer;
        this->syncObjects = other.syncObjects;

        return *this;
    }

    /// <summary>
    /// Actually adds the commands to the buffer
    /// This is the kool part
    /// </summary>
    void RendererLayer::recordCommandBuffer(uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer.commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass.renderPass;
        renderPassInfo.framebuffer = framebuffers.swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChain.swapChainExtent;

        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain.swapChainExtent.width);
        viewport.height = static_cast<float>(swapChain.swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChain.swapChainExtent;
        vkCmdSetScissor(commandBuffer.commandBuffer, 0, 1, &scissor);

        // juuuuust like opengl
        vkCmdDraw(commandBuffer.commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer.commandBuffer);

        if (vkEndCommandBuffer(commandBuffer.commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    // TODO Draw from a scene object
    void RendererLayer::drawFrame() {
        // Wait for previous frame to be finished
        vkWaitForFences(logicalDevice.device, 1, &syncObjects.inFlightFence, VK_TRUE, UINT64_MAX);

        vkResetFences(logicalDevice.device, 1, &syncObjects.inFlightFence);

        // Get image from swap chain
        uint32_t imageIndex;
        vkAcquireNextImageKHR(logicalDevice.device, swapChain.swapChain, UINT64_MAX, syncObjects.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(commandBuffer.commandBuffer, 0);

        // Write to our command buffer
        recordCommandBuffer(imageIndex);

        // Submit to the queues
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { syncObjects.imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // trying changing to VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer.commandBuffer;

        VkSemaphore signalSemaphores[] = { syncObjects.renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // TODO This is the problem. When it tries to submit the second command
        if (vkQueueSubmit(logicalDevice.graphicsQueue, 1, &submitInfo, syncObjects.inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // ? Reread this part
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain.swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        vkQueuePresentKHR(logicalDevice.presentQueue, &presentInfo);
    }
}

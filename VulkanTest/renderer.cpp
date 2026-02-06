// Vulkan Test
// By: Jupiter Sinclair Chong

#include "renderer.h"

namespace Renderer {

#pragma region Component Classes

    // Instance

    bool Instance::checkValidationLayerSupport(std::vector<const char*> validationLayers) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
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

    Instance::Instance() {

    }

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

        //auto extensions = getRequiredExtensions();
        //createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        //createInfo.ppEnabledExtensionNames = extensions.data();

        std::cout << "[Vulkan]\t Supported Extensions:\n";

        for (const auto& extension : extensions)
            std::cout << '\t' << extension.extensionName << '\n';

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
        else {
            printf("[Vulkan]\t Created Instance\n");
        }

    }

    Instance::~Instance() {
        printf("[Cleanup]\t Destroying Instance\n"),
            vkDestroyInstance(instance, nullptr);
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
        printf("[Cleanup]\t Destroying Surface\n");
        vkDestroySurfaceKHR(instance->instance, surface, nullptr);
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

        for (const auto& extension : availableExtensions) {
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

        for (const auto& device : devices) {
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

    LogicalDevice::LogicalDevice(Instance* instance, LogicalDeviceCreateInfo ci, VkQueue graphicsQueue, VkQueue presentQueue, Surface* surface) {
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
        printf("[Cleanup]\t Destroying Logical Device\n");
        vkDestroyDevice(device, nullptr);
    }

    // Swap Chain

    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
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
    }

    SwapChain::~SwapChain() {
        for (VkImageView imageView : swapChainImageViews) {
            vkDestroyImageView(logicalDevice->device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(logicalDevice->device, swapChain, nullptr);
    }

#pragma endregion

    // Renderer Layer

    RendererLayer::RendererLayer() {

    }

    RendererLayer::RendererLayer(RendererLayer& rh) {
        this->window = rh.window;
        this->graphicsQueue = rh.presentQueue;
        this->validationLayers = rh.validationLayers;
        this->deviceExtensions = rh.deviceExtensions;

        this->instance = rh.instance;
        this->surface = rh.surface;
        this->logicalDevice = rh.logicalDevice;
    }

    RendererLayer::RendererLayer(RendererLayerCreateInfo createInfo) {
        this->window = createInfo.window;

        printf("[Vulkan]\t Initting Vulkan\n");

        // Hmmm who should own these cuz they're being passed twice in 2 diff ways rn
        validationLayers = createInfo.LI_CreateInfo.validationLayers;
        deviceExtensions = createInfo.LI_CreateInfo.deviceExtensions;

        instance = new Instance(enableValidationLayers, validationLayers);

        surface = new Surface(instance, createInfo.window);

        // More elegant way to pass this to Logical Device?
        // Should logical device own Surface?
        //createInfo.LI_CreateInfo.surface = surface.surface;

        logicalDevice = new LogicalDevice(instance, createInfo.LI_CreateInfo, graphicsQueue, presentQueue, surface);

        swapChain = new SwapChain(logicalDevice, surface, window);

        //createRenderPass();
        //createGraphicsPipeline();   // It's really that easy
        //createFramebuffers();       // Back to more familiar territory
        //createCommandPool();
        //createCommandBuffer();
        //createSyncObjects();

        printf("[Vulkan]\t Ready!\n");
    }

    RendererLayer::~RendererLayer() {
        printf("[Cleanup]\t Destroying Renderer Layer\n");

        delete swapChain;
        delete surface;
        delete logicalDevice;
        delete instance;
    }

    RendererLayer& RendererLayer::operator=(const RendererLayer& other) {
        this->window = other.window;
        this->graphicsQueue = other.presentQueue;
        this->validationLayers = other.validationLayers;
        this->deviceExtensions = other.deviceExtensions;

        this->instance = other.instance;
        this->surface = other.surface;
        this->logicalDevice = other.logicalDevice;

        return *this;
    }

    // TODO Draw from a scene object
    void RendererLayer::drawFrame() {

    }

}

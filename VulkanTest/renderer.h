#pragma once

// Vulkan
#include <vulkan/vulkan.h>

// GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// C++
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>

// Create structs to pass in data to new objects do it like vulkan lol

struct LogicalDeviceCreateInfo {
    // Required Extensions
    std::vector<const char*> deviceExtensions;

    // Required Validation Layers
    std::vector<const char*> validationLayers;


#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

};

/// <summary>
/// Used to pass application specific information when setting up Renderer Layer
/// </summary>
struct RendererLayerCreateInfo {
    GLFWwindow* window = nullptr;

    LogicalDeviceCreateInfo LI_CreateInfo;
};

/// <summary>
/// Connection between this App and Vulkan library
/// </summary>
struct Instance {
    VkInstance instance = VK_NULL_HANDLE;

    /// <summary>
    /// Check if all layers in Validation Layers exist in Available Layers
    /// </summary>
    bool checkValidationLayerSupport(std::vector<const char*> validationLayers) {
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

    Instance() {

    }

    Instance(bool enableValidationLayers, std::vector<const char*> validationLayers) {
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

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    ~Instance() {
        vkDestroyInstance(instance, nullptr);
    }
};

/// <summary>
/// Surface to be rendered to. GLFW does this
/// </summary>
struct Surface {
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    Instance* instance = nullptr;

    Surface() {

    }

    /// <summary>
    /// Sets Surface using GLFW
    /// </summary>
    Surface(Instance* instance, GLFWwindow* window) {
        if (glfwCreateWindowSurface(instance->instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }

        this->instance = instance;
    }

    ~Surface() {
        vkDestroySurfaceKHR(instance->instance, surface, nullptr);
    }

};

/// <summary>
/// This is the actual thing we need
/// Logical Device: Describe the features you want to use. A representation of Physical Device
/// </summary>
class LogicalDevice {
    VkDevice device = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && \
                presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

        // Problem
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
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

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
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
    bool isDeviceSuitable(VkPhysicalDevice device, std::vector<const char*> deviceExtensions) {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    /// <summary>
    /// Find first GPU that supports the features we want
    /// TODO Update this to be more sophisticated. EX: rate GPU suitablility and choose the highest, or just pick dedicated GPU
    /// </summary>
    void pickPhysicalDevice(Instance* instance, std::vector<const char*> deviceExtensions) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance->instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance->instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device, deviceExtensions)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }

        printf("[Vulkan]\t Picked GPU\n");
    }

public:
    LogicalDevice() {

    }

    LogicalDevice(Instance* instance, LogicalDeviceCreateInfo ci, VkQueue graphicsQueue, VkQueue presentQueue) {
        pickPhysicalDevice(instance, ci.deviceExtensions);

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

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

    ~LogicalDevice() {
        vkDestroyDevice(device, nullptr);
    }

};

class RendererLayer {
private:

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    GLFWwindow* window = nullptr;
    VkQueue graphicsQueue = VK_NULL_HANDLE;     // 
    VkQueue presentQueue = VK_NULL_HANDLE;      // Images to be presented to the screen

    // Required Validation Layers
    std::vector<const char*> validationLayers;

    // Required Extensions
    std::vector<const char*> deviceExtensions;
public:

    RendererLayer(RendererLayerCreateInfo createInfo) {
        this->window = window;

        printf("[Vulkan]\t Initting Vulkan\n");

        instance = Instance(enableValidationLayers, validationLayers);

        // setupDebuggerMessenger();
        //createSurface();
        
        surface = Surface(&instance, createInfo.window);
        
        //pickPhysicalDevice();
        //createLogicalDevice();
        
        logicalDevice = LogicalDevice(&instance, createInfo.LI_CreateInfo, graphicsQueue, presentQueue);
        
        //createSwapChain();
        //createImageViews();

        //createRenderPass();
        //createGraphicsPipeline();   // It's really that easy
        //createFramebuffers();       // Back to more familiar territory
        //createCommandPool();
        //createCommandBuffer();
        //createSyncObjects();

        printf("[Vulkan]\t Ready!\n");
    }

    // TODO Draw from a scene object
    void drawFrame() {

    }

    ~RendererLayer() {

    }

private:

    Instance instance;
    
    Surface surface;

   LogicalDevice logicalDevice;
};
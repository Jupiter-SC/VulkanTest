// Vulkan Test
// By: Jupiter Sinclair Chong

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

namespace Renderer {

#pragma region Create Infos
    // Create structs to pass in data to new objects do it like vulkan lol
    // ? Not necessary now to have 2 create infos...
    struct LogicalDeviceCreateInfo {
        std::vector<const char*> deviceExtensions;
        std::vector<const char*> validationLayers;

#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif
        VkSurfaceKHR surface = VK_NULL_HANDLE;
    };

    /// <summary>
    /// Used to pass application specific information when setting up Renderer Layer
    /// </summary>
    struct RendererLayerCreateInfo {
        GLFWwindow* window = nullptr;

        LogicalDeviceCreateInfo LI_CreateInfo;
    };

#pragma endregion

#pragma region Helper Structs

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

#pragma endregion

#pragma region Component Structs
    /// <summary>
    /// Connection between this App and Vulkan library
    /// </summary>
    struct Instance {
        VkInstance instance = VK_NULL_HANDLE;

        /// <summary>
        /// Check if all layers in Validation Layers exist in Available Layers
        /// </summary>
        bool checkValidationLayerSupport(std::vector<const char*> validationLayers);

        Instance();

        Instance(bool enableValidationLayers, std::vector<const char*> validationLayers);

        ~Instance();

        Instance& operator=(const Instance& other);

        // This defeats like half the point of this but I have to cuz the destructors are called when using copy assignments
        void cleanup(){
            vkDestroyInstance(instance, nullptr);
            printf("[Cleanup]\t Destroyed Instance\n");
        }
    };

    /// <summary>
    /// Surface to be rendered to. GLFW handles this
    /// </summary>
    struct Surface {
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        // Needed for cleanup
        Instance* instance = nullptr;

        Surface();

        /// <summary>
        /// Sets Surface using GLFW
        /// </summary>
        Surface(Instance* instance, GLFWwindow* window);

        ~Surface();

        void cleanup() {        
            vkDestroySurfaceKHR(instance->instance, surface, nullptr);
            printf("[Cleanup]\t Destroyed Surface\n");
        }
    };

    /// <summary>
    /// Describes the features we want to use. A representation of Physical Device
    /// </summary>
    struct LogicalDevice {
        VkDevice device = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        Surface* surface = nullptr;
        Instance* instance = nullptr;

    public:
        LogicalDevice();

        LogicalDevice(Instance* instance, LogicalDeviceCreateInfo ci, VkQueue graphicsQueue, VkQueue presentQueue, Surface* surface);

        ~LogicalDevice();

        void cleanup() {
            vkDestroyDevice(device, nullptr);
            printf("[Cleanup]\t Destroyed Logical Device\n");
        }
    };

    /// <summary>
    /// 
    /// </summary>
    struct SwapChain {
        LogicalDevice* logicalDevice = nullptr;

        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;           // The images we're rendering
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;   // Accessing our images

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

        SwapChain();

        SwapChain(LogicalDevice* logicalDevice, Surface* surface, GLFWwindow* window);

        ~SwapChain();

        void cleanup() {
            for (VkImageView imageView : swapChainImageViews) {
                vkDestroyImageView(logicalDevice->device, imageView, nullptr);
            }

            vkDestroySwapchainKHR(logicalDevice->device, swapChain, nullptr);

            printf("[Cleanup] Destroyed Swap Chain");
        }
    };

    /// <summary>
    /// 
    /// </summary>
    struct RenderPass {
        VkRenderPass renderPass = VK_NULL_HANDLE;

        LogicalDevice* logicalDevice = nullptr;
        
        RenderPass();

        RenderPass(LogicalDevice* logicalDevice, VkFormat swapChainImageFormat);

        ~RenderPass();

        RenderPass(const RenderPass& other);

        RenderPass& operator=(const RenderPass& other);

        void cleanup() {
            vkDestroyRenderPass(logicalDevice->device, renderPass, nullptr);
            printf("[Cleanup]\t Destroyed Render Pass");
        }
    };

    /// <summary>
    /// 
    /// </summary>
    class RendererLayer {

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

        // These are only woking as pointers cuz otherwise they get destroyed too soon, but IDK why

        Instance instance;
        Surface surface;
        LogicalDevice logicalDevice;
        SwapChain swapChain;
        RenderPass renderPass;         // Ooo

    public:
        RendererLayer();

        RendererLayer(RendererLayer& rh);

        RendererLayer(RendererLayerCreateInfo createInfo);

        ~RendererLayer();

        RendererLayer& operator=(const RendererLayer& other);

        // TODO Draw from a scene object
        void drawFrame();

    };
#pragma endregion

}
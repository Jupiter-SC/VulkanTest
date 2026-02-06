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
        void cleanup();
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

        void cleanup();
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

        void cleanup();
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

        void cleanup();
    };

    /// <summary>
    /// 
    /// </summary>
    struct RenderPass {
        LogicalDevice* logicalDevice = nullptr;

        VkRenderPass renderPass = VK_NULL_HANDLE;

        RenderPass();

        RenderPass(LogicalDevice* logicalDevice, VkFormat swapChainImageFormat);

        ~RenderPass();

        RenderPass(const RenderPass& other);

        RenderPass& operator=(const RenderPass& other);

        void cleanup();
    };


    // TODO Shader struct eyes emoji

    /// <summary>
    /// 
    /// </summary>
    struct GraphicsPipeline {
        LogicalDevice* logicalDevice = nullptr;
        
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;

        /// <summary>
        /// Helper function for loading shaders
        /// </summary>
        static std::vector<char> readFile(const std::string& filename);

        /// <summary>
        /// Byte code -> usable shader module
        /// </summary>
        VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);

        GraphicsPipeline();

        /// <summary>
        /// The name describes it
        /// Can we load shaders later and insert them?
        /// </summary>
        GraphicsPipeline(LogicalDevice* device, VkExtent2D swapChainExtent, RenderPass* renderPass);

        void cleanup();
    };

    /// <summary>
    /// 
    /// </summary>
    struct Framebuffer {
        LogicalDevice* logicalDevice = nullptr;
        
        std::vector<VkFramebuffer> swapChainFramebuffers;

        Framebuffer() = default;

        Framebuffer(LogicalDevice* logicalDevice, SwapChain* swapChain, RenderPass* renderPass);

        void cleanup();
    };

    struct CommandPool {
        LogicalDevice* logicalDevice = nullptr;
        
        VkCommandPool commandPool = VK_NULL_HANDLE;

        CommandPool() = default;

        CommandPool(LogicalDevice* logicalDevice, Surface* surface);

        void cleanup();
    };

    struct CommandBuffer {
        LogicalDevice* logicalDevice = nullptr;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        
        CommandBuffer() = default;
        
        CommandBuffer(LogicalDevice* logicalDevice, CommandPool* commandPool);

        void cleanup();
    };

    /// <summary>
    /// Sync objects are Semaphores and Fences, in order to synchronize command calls
    /// Semaphores halt queue submission until the previous one has been received, but doesn't stop all code. 
    /// Used for swapchain operations so the GPUs doesn't have to wait
    /// Fences to stop all code. Used for waiting for the prev frame, so we don't draw more than one frame at a time
    /// </summary>
    struct SyncObjects {
        LogicalDevice* logicalDevice = nullptr;

        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;

        SyncObjects() = default;

        SyncObjects(LogicalDevice* logicalDevice);

        void cleanup();
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
        RenderPass renderPass;
        GraphicsPipeline graphicsPipeline;
        Framebuffer framebuffers;
        CommandPool commandPool;
        CommandBuffer commandBuffer;
        SyncObjects syncObjects;

    public:
        RendererLayer() = default;

        RendererLayer(RendererLayer& rh);

        RendererLayer(RendererLayerCreateInfo createInfo);

        ~RendererLayer();

        RendererLayer& operator=(const RendererLayer& other);

        // TODO Draw from a scene object
        void drawFrame();

    };
#pragma endregion

}
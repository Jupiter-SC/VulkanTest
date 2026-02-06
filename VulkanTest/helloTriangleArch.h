// Vulkan Test
// By: Jupiter Sinclair Chong

#pragma once 

// Renderer Layer
#include "renderer.h"

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class HelloArchitecture {
public:
    void run() {
        initWindow();

        // Application specific setup
        Renderer::RendererLayerCreateInfo RL_CreateInfo;
        RL_CreateInfo.window = window;

        RL_CreateInfo.LI_CreateInfo.deviceExtensions = deviceExtensions;
        RL_CreateInfo.LI_CreateInfo.validationLayers = validationLayers;

        // What's the deal with copy contructors ?

        /*
        LogicalDeviceCreateInfo LI_CreateInfo;
        LI_CreateInfo.deviceExtensions = deviceExtensions;
        LI_CreateInfo.validationLayers = validationLayers;
        */

        //rendererNotPointer = RendererLayer(RL_CreateInfo);

        renderer = new Renderer::RendererLayer(RL_CreateInfo);

        mainLoop();
        cleanup();
    }

private:
    Renderer::RendererLayer* renderer;
    //RendererLayer rendererNotPointer;

    // Window vars
    GLFWwindow* window = nullptr;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    // Required Validation Layers
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    // Required Extensions
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    //
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void initWindow() {
        printf("[Window]\t Initting Window\n");

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "But Vulkan is the hardz: Architecture - Fuck Header Files", nullptr, nullptr);

        printf("[Window]\t Ready!\n");
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            renderer->drawFrame();
        }

        //vkDeviceWaitIdle(device);
    }

    void cleanup() {
        printf("[Cleanup]\t Started\n");

        // Vulkan

        delete renderer;

        glfwDestroyWindow(window);
        glfwTerminate();

        printf("[Cleanup]\t Finished\n");
    }
};
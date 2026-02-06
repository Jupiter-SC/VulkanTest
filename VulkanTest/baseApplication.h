#pragma once
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

struct ApplicationInfo {
    // Window Info
    uint32_t WIDTH = 800;
    uint32_t HEIGHT = 600;
    std::string windowName = "Base Application";

    ApplicationInfo() = default;
    ApplicationInfo(const uint32_t& WIDTH, const uint32_t& HEIGHT, const std::string& windowName)
        : WIDTH(WIDTH), HEIGHT(HEIGHT), windowName(windowName)
    {}

};

class Application {
public:
    void run() {
        initWindow();
        initGraphicsLayer();
        mainLoop();
        cleanup();
    }

protected:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    GLFWwindow* window = nullptr;
    ApplicationInfo info;
    Renderer::RendererLayer* renderer;

    void initWindow() {
        printf("[Window]\t Initting Window\n");
        glfwInit();


        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        info = createApplicationInfo();
        window = glfwCreateWindow(info.WIDTH, info.HEIGHT, info.windowName.c_str(), nullptr, nullptr);

        printf("[Window]\t Ready!\n");
    }

    void initGraphicsLayer() {
        renderer = new Renderer::RendererLayer(createRenderLayerInfo());
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            renderer->drawFrame();
        }

        vkDeviceWaitIdle(renderer->getDevice());
    }

    void cleanup() {
        printf("[Cleanup]\t Started\n");

        // Vulkan

        delete renderer;
        renderer = nullptr;

        glfwDestroyWindow(window);
        glfwTerminate();

        printf("[Cleanup]\t Finished\n");
    }

    virtual Renderer::RendererLayerCreateInfo createRenderLayerInfo() = 0;
    
    virtual ApplicationInfo createApplicationInfo() {
        ApplicationInfo info;
        return info;
    }

    virtual std::vector<const char*> getValidationLayers() {
        return std::vector<const char*> { "VK_LAYER_KHRONOS_validation" };
    }

    virtual std::vector<const char*> getDeviceExtensions() {
        return std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    }


};
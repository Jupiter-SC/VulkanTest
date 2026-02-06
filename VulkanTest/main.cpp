// Vulkan Test
// By: Jupiter Sinclair Chong

// Applications
#include "helloTriangleArch.h"
//#include "helloTriangleDefault.h"

int main() {
    Application* app = new HelloArchitecture();

    try {
        app->run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
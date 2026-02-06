// Vulkan Test
// By: Jupiter Sinclair Chong

#pragma once 

#include "baseApplication.h"

class HelloArchitecture : public Application {
    // Inherited via Application
    Renderer::RendererLayerCreateInfo createRenderLayerInfo() override;
};
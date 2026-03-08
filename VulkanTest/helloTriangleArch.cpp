#include "helloTriangleArch.h"

Renderer::RendererLayerCreateInfo HelloArchitecture::createRenderLayerInfo()
{
    // Application specific setup
    Renderer::RendererLayerCreateInfo RL_CreateInfo;
    RL_CreateInfo.window = window;

    RL_CreateInfo.LI_CreateInfo.deviceExtensions = getDeviceExtensions();
    RL_CreateInfo.LI_CreateInfo.validationLayers = getValidationLayers();

    // What's the deal with copy contructors ?

    return Renderer::RendererLayerCreateInfo(RL_CreateInfo);
}

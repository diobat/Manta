#pragma once

// Read this before you start:
// https://github.com/ocornut/imgui/wiki/Getting-Started

// Include the main ImGUI header
#include "imgui.h"
// Include the GLFW and Vulkan ImGUI wrappers
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

// std::string support for ImGUI
#include "misc/cpp/imgui_stdlib.h"

class rendering_system;

class imGUI_handler {

public:
    imGUI_handler(rendering_system* core);
    
    void init(VkInstance& instance, VkQueue& graphicsQueue, VkRenderPass& renderPass);

    void onFrameStart();
    void onFrameEnd(uint32_t frameIndex);

    void cleanup();

private:

    rendering_system* _core;
    ImGuiContext* _context;
};

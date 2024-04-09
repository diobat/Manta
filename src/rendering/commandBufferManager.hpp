#pragma once

class rendering_system;

// Third party includes
#include <entt/entt.hpp>
// GLFW
#include "wrapper/glfw.hpp"

struct FrameData
{
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
};



class command_buffer_system
{
public:
    command_buffer_system(rendering_system* core, VkCommandPool& commandPool, VkQueue& graphicsQueue);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkCommandBuffer& getCommandBuffer(uint32_t index) { return _commandBuffers[index]; }

private:
    rendering_system* _core;

    const unsigned int& _framesInFlight;
    VkCommandPool& _commandPool;
    VkQueue& _graphicsQueue;

    std::vector<VkCommandBuffer> _commandBuffers;           // command buffers, one for each frame
};
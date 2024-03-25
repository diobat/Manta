#pragma once

// Third party includes
#include <entt/entt.hpp>
// GLFW
#include "wrapper/glfw.hpp"


class command_buffer_system
{
public:
    command_buffer_system(VkDevice& logicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
    VkDevice& _logicalDevice;
    VkCommandPool& _commandPool;
    VkQueue& _graphicsQueue;
};
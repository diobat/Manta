#pragma once

// Third party includes
#include <entt/entt.hpp>
// GLFW
#include "wrapper/glfw.hpp"

// First party includes
#include "rendering/resources/vertex.hpp"

class rendering_system;

struct memoryBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkBufferUsageFlagBits usage;
    VkMemoryPropertyFlags properties;
    void* mappedTo;
};



class memory_system
{
public:
    memory_system(rendering_system* core, entt::registry& registry, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice);

    // Resource creation
    memoryBuffer createVertexBuffer(std::vector<Vertex> vertices);
    memoryBuffer createIndexBuffer(std::vector<uint32_t> indices);

    // In order to make it work for any type of object, we need to use templates
    template<typename T>
    std::vector<memoryBuffer> createUniformBuffers(uint32_t count = 1)
    {
        std::vector<memoryBuffer> buffers(count);

        for (uint32_t i = 0; i < count; i++)
        {
            buffers[i] = createBuffer(sizeof(T), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            vkMapMemory(_logicalDevice, buffers[i].memory, 0, sizeof(T), 0, &buffers[i].mappedTo);
        }
        return buffers;
    }

    void deleteBuffer(VkBuffer buffer, VkDeviceMemory memory);
private:

    memoryBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    rendering_system* _core;
    entt::registry& _registry;  
    VkDevice& _logicalDevice;
    VkPhysicalDevice& _physicalDevice;

};
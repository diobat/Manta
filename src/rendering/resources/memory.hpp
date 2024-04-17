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
    VkDescriptorBufferInfo descriptorInfo;
    void* mappedTo;
};

struct memoryBuffers{
    std::vector<memoryBuffer> buffers;
};

class memory_system
{
public:
    memory_system(rendering_system* core, entt::registry& registry, VkDevice& logicalDevice);

    // Buffer creation
    memoryBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    // Buffer resource creation
    memoryBuffer createVertexBuffer(std::vector<Vertex> vertices);
    memoryBuffer createIndexBuffer(std::vector<uint32_t> indices);

    // In order to make it work for any type of object, we need to use templates
    // Maybe this function cannot be templated and it should take the size as a parameter
    template<typename T>
    std::vector<memoryBuffer> createUniformBuffers(uint32_t count = 1)
    {
        std::vector<memoryBuffer> buffers(count);

        for (uint32_t i = 0; i < count; i++)
        {
            buffers[i] = createBuffer(sizeof(T), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            buffers[i].descriptorInfo = { buffers[i].buffer, 0, sizeof(T) };
            vkMapMemory(_logicalDevice, buffers[i].memory, 0, sizeof(T), 0, &buffers[i].mappedTo);
        }
        return buffers;
    }
    std::vector<memoryBuffer> createUniformBuffers(uint32_t size, uint32_t count = 1);



    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void freeBuffer(VkBuffer buffer, VkDeviceMemory memory);
    void freeBuffer(memoryBuffer buffer);
    void freeBuffer(memoryBuffers buffers);
private:

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    rendering_system* _core;
    entt::registry& _registry;  
    VkDevice& _logicalDevice;
    // VkPhysicalDevice& _physicalDevice;
};
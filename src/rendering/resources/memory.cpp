#include "rendering/resources/memory.hpp"

#include "core/settings.hpp"
#include "rendering/rendering.hpp"


memory_system::memory_system(rendering_system* core, entt::registry& registry, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice) : 
    _core(core),
    _registry(registry),
    _logicalDevice(logicalDevice),
    _physicalDevice(physicalDevice)
{
    ;
}

memoryBuffer memory_system::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    memoryBuffer buffer;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(_logicalDevice, &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_logicalDevice, buffer.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if(vkAllocateMemory(_logicalDevice, &allocInfo, nullptr, &buffer.memory) != VK_SUCCESS)
    
    {
        throw std::runtime_error("Failed to allocate buffer memory");
    }

    vkBindBufferMemory(_logicalDevice, buffer.buffer, buffer.memory, 0);

    return buffer;
}

void memory_system::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = _core->getCommandBuffer().beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    _core->getCommandBuffer().endSingleTimeCommands(commandBuffer);

}

void memory_system::deleteBuffer(VkBuffer buffer, VkDeviceMemory memory)
{
    vkDestroyBuffer(_logicalDevice, buffer, nullptr);
    vkFreeMemory(_logicalDevice, memory, nullptr);
}

    // There are two types of memory at play here: device local memory and host visible memory.
    // Host visible memory is memory that is accessible by the CPU. We will first create a buffer in host visible memory and then copy the data to a buffer in device local memory.
    // Device local memory is memory that is local to the GPU and is the fastest memory to access. We want the vertex buffer to be in device local memory. Device local memory is not accesible by the CPU.
memoryBuffer memory_system::createVertexBuffer(std::vector<Vertex> vertices)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    memoryBuffer stagingBuffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(_logicalDevice, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(_logicalDevice, stagingBuffer.memory);

    memoryBuffer vertexBuffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

    deleteBuffer(stagingBuffer.buffer, stagingBuffer.memory);

    return vertexBuffer;
}

memoryBuffer memory_system::createIndexBuffer(std::vector<uint32_t> indices)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    memoryBuffer stagingBuffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(_logicalDevice, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(_logicalDevice, stagingBuffer.memory);

    memoryBuffer indexBuffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(stagingBuffer.buffer, indexBuffer.buffer, bufferSize);

    deleteBuffer(stagingBuffer.buffer, stagingBuffer.memory);

    return indexBuffer;
}



uint32_t memory_system::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

    for(uint32_t i(0); i < memProperties.memoryTypeCount; ++i){
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");   
}
#include "rendering/commandBufferManager.hpp"
#include "rendering/rendering.hpp"

#include "core/settings.hpp"

command_buffer_system::command_buffer_system(rendering_system* core, VkCommandPool& commandPool, VkQueue& graphicsQueue) :
    _core(core),
    _commandPool(commandPool),
    _graphicsQueue(graphicsQueue), 
    _framesInFlight(getSettingsData(core->getRegistry()).framesInFlight)
{
    _commandBuffers.resize(3);
}

VkCommandBuffer command_buffer_system::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_core->getLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void command_buffer_system::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkFreeCommandBuffers(_core->getLogicalDevice(), _commandPool, 1, &commandBuffer);
}


void command_buffer_system::recordCommandBuffer(uint32_t frameIndex, uint32_t swapChainImageIndex)
{
    ;
}

void command_buffer_system::resetCommandBuffer(uint32_t frameIndex)
{
    vkResetCommandBuffer(_commandBuffers[frameIndex], 0);
}

void command_buffer_system::submitCommandBuffer(uint32_t frameIndex, VkSemaphore imageAvailableSemaphore, VkSemaphore renderFinishedSemaphore, VkFence fence)
{
    ;
}
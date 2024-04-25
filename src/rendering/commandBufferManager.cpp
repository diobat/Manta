#include "rendering/commandBufferManager.hpp"
#include "rendering/rendering.hpp"

#include "core/settings.hpp"
#include "util/physicalDeviceHelper.hpp"

command_buffer_system::command_buffer_system(rendering_system* core, VkQueue& graphicsQueue) :
    _core(core),
    _graphicsQueue(graphicsQueue), 
    _framesInFlight(getSettingsData(core->getRegistry()).framesInFlight)
{
    _commandBuffers.resize(3);
}

void command_buffer_system::createCommandPools()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_core->getPhysicalDevice(), _core->getSurface());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if(vkCreateCommandPool(_core->getLogicalDevice(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }

    poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

    if(vkCreateCommandPool(_core->getLogicalDevice(), &poolInfo, nullptr, &_transferCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create transfer command pool!");
    }
}

void command_buffer_system::createCommandBuffers()
{
    _commandBuffers.resize(_framesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) _commandBuffers.size();

    if(vkAllocateCommandBuffers(_core->getLogicalDevice(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
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
    VkCommandBuffer& commandBuffer = _commandBuffers[frameIndex];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _core->getRenderPass();
    renderPassInfo.framebuffer = _core->getSwapChainSystem().getSwapChain().Framebuffers[swapChainImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _core->getSwapChainSystem().getSwapChain().Extent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _core->getPipelineSystem().getPipeline("basic").pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_core->getSwapChainSystem().getSwapChain().Extent.width);
    viewport.height = static_cast<float>(_core->getSwapChainSystem().getSwapChain().Extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _core->getSwapChainSystem().getSwapChain().Extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    _core->getFrameManager().updateUniformBuffers(frameIndex);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _core->getPipelineSystem().getPipeline("basic").layout, 0, 1, & _core->getFrameManager().getDescriptorSet(descriptorSetType::MVP_MATRICES, frameIndex), 0, nullptr);

    auto renderModels = _core->getScene()->getRegistry().view<Model>();

    unsigned int i{0};
    for(auto entity : renderModels)
    {
        auto& model = renderModels.get<Model>(entity);

        // Push constants
        vkCmdPushConstants(commandBuffer, _core->getPipelineSystem().getPipeline("basic").layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(int), &i);


        for(auto& mesh : *model.meshes)
        {
            // Attribute data
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

            unsigned int textureIndex = mesh.textureIndices[static_cast<unsigned int>(E_TextureType::DIFFUSE)];

            //mesh.textureIndices[static_cast<unsigned int>(E_TextureType::DIFFUSE)]
            vkCmdPushConstants(commandBuffer, _core->getPipelineSystem().getPipeline("basic").layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(int), 2*sizeof(int), &i);

            // Draw call
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indexData.size()), 1, 0, 0, 0);
        }
        i++;
    }

    vkCmdEndRenderPass(commandBuffer);

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }        
}

void command_buffer_system::resetCommandBuffer(uint32_t frameIndex)
{
    vkResetCommandBuffer(_commandBuffers[frameIndex], 0);
}

void command_buffer_system::submitCommandBuffer(uint32_t frameIndex, VkSemaphore imageAvailableSemaphore, VkSemaphore renderFinishedSemaphore, VkFence fence)
{
    ;
}

void command_buffer_system::cleanup()
{
    for(auto& commandBuffer : _commandBuffers)
    {
        vkFreeCommandBuffers(_core->getLogicalDevice(), _commandPool, 1, &commandBuffer);
    }

    vkDestroyCommandPool(_core->getLogicalDevice(), _commandPool, nullptr);
    vkDestroyCommandPool(_core->getLogicalDevice(), _transferCommandPool, nullptr);
}
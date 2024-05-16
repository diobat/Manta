#include "rendering/commandBufferManager.hpp"
#include "rendering/rendering.hpp"
#include "core/settings.hpp"
#include "util/physicalDeviceHelper.hpp"
#include "util/VertexShapes.hpp"

command_buffer_system::command_buffer_system(rendering_system* core, VkQueue& graphicsQueue, VkQueue& presentationQueue) :
    _core(core),
    _graphicsQueue(graphicsQueue), 
    _presentationQueue(presentationQueue),
    _framesInFlight(getSettingsData(core->getRegistry()).framesInFlight)
{
    // Set the default clear values
    _clearValues[0].color = {0.2f, 0.2f, 0.2f, 1.0f};
    _clearValues[1].depthStencil = {1.0f, 0};
}

void command_buffer_system::createCommandPools()
{

    // Set the default viewport values
    _viewport.x = 0.0f;
    _viewport.y = 0.0f;
    _viewport.width = static_cast<float>(_core->getSwapChainSystem().getSwapChain().Extent.width);
    _viewport.height = static_cast<float>(_core->getSwapChainSystem().getSwapChain().Extent.height);
    _viewport.minDepth = 0.0f;
    _viewport.maxDepth = 1.0f;

    // Set the default scissor values
    _scissor.offset = {0, 0};
    _scissor.extent = _core->getSwapChainSystem().getSwapChain().Extent;


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

std::vector<VkCommandBuffer> command_buffer_system::createCommandBuffers(short unsigned int count)
{
    std::vector<VkCommandBuffer> commandBuffers(count, VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if(vkAllocateCommandBuffers(_core->getLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    // // Create the cube map baking command buffer
    // VkCommandBufferAllocateInfo cubeMapBakingAllocInfo{};
    // cubeMapBakingAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // cubeMapBakingAllocInfo.commandPool = _commandPool;
    // cubeMapBakingAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    // cubeMapBakingAllocInfo.commandBufferCount = 1;

    // if(vkAllocateCommandBuffers(_core->getLogicalDevice(), &cubeMapBakingAllocInfo, &_cubeMapBakingCommandBuffer) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to allocate cube map baking command buffer!");
    // }

    return commandBuffers;
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

VkCommandBuffer command_buffer_system::generateCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;

    if(vkAllocateCommandBuffers(_core->getLogicalDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffer!");
    }

    return commandBuffer;
}

VkResult command_buffer_system::beginRecordingCommandBuffer(VkCommandBuffer& commandBuffer, E_RenderPassType renderPassType, VkFramebuffer framebuffer, VkExtent2D extent)
{
    // Begin command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // Begin Render Pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _core->getPipelineSystem().getRenderPass(renderPassType);
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    const std::array<VkClearValue, 2>& _clearValues = _core->getCommandBufferSystem().getClearValues();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(_clearValues.size());
    renderPassInfo.pClearValues = _clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    return result;
}

VkResult command_buffer_system::endRecordingCommandBuffer(VkCommandBuffer& commandBuffer)
{
    // End Render Pass
    vkCmdEndRenderPass(commandBuffer);

    return vkEndCommandBuffer(commandBuffer);
}

void command_buffer_system::recordCommandBuffer(const renderRequest& request)
{

    // Verify that the number of per model push constants match the number of models
    if(request.perModelPC.size() != 0 && request.perModelPC.size() != request.models.size())
    {
        throw std::runtime_error("The number of per model push constants does not match the number of models.");
    }

    // Begin the command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    // Bind pipeline, set viewpoet and scissor
    vkCmdBindPipeline(request.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, request.pipeline.pipeline);
    vkCmdSetViewport(request.commandBuffer, 0, 1, &_viewport);
    vkCmdSetScissor(request.commandBuffer, 0, 1, &_scissor);

    // Descriptor set
    vkCmdBindDescriptorSets(request.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, request.pipeline.layout, 
        0, 
        request.descriptorSets.size(), 
        &request.descriptorSets[0], 
        0, 
        nullptr);

    // General push constant
    if(request.generalPC.size > 0)
    {
        vkCmdPushConstants(request.commandBuffer, request.pipeline.layout, request.generalPC.stageFlags, request.generalPC.offset, request.generalPC.size, request.generalPC.data);
    }

    // Draw calls
    for(size_t i{0}; i < request.models.size() ; ++i)
    {
        // Model specific push constants
        if(request.perModelPC.size() > 0)
        {
            unsigned int modelID = * (unsigned int*) &request.perModelPC[i].data;
            vkCmdPushConstants(request.commandBuffer, request.pipeline.layout, request.perModelPC[i].stageFlags , request.perModelPC[i].offset, request.perModelPC[i].size, request.perModelPC[i].data);
        }


        for(auto& mesh : *request.models[i].meshes)
        {
            // Attribute data
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(request.commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(request.commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

            // Texture Index push constants
            if(request.useTextureLibraryBinds)
            {
                vkCmdPushConstants(request.commandBuffer, request.pipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(unsigned int), &mesh.textureIndices[static_cast<unsigned int>(E_TextureType::DIFFUSE)]);
            }
            
            // Draw call
            vkCmdDrawIndexed(request.commandBuffer, static_cast<uint32_t>(mesh.indexData.size()), 1, 0, 0, 0);
        }
    }

}

void command_buffer_system::resetCommandBuffer(VkCommandBuffer& commandBuffer)
{
    vkResetCommandBuffer(commandBuffer, 0);
}

void command_buffer_system::submitCommandBuffer(VkCommandBuffer& cmdBuffer, VkFence fence)
{
    std::vector<VkSemaphore> emptyVector;
    submitCommandBuffer(cmdBuffer, emptyVector, emptyVector, fence);
}

void command_buffer_system::submitCommandBuffer(VkCommandBuffer& cmdBuffer,  std::vector<VkSemaphore>& imageAvailableSemaphore, std::vector<VkSemaphore>& renderFinishedSemaphore, VkFence fence)
{
    // Wait until signals are ready
    if(fence != VK_NULL_HANDLE)
    {
        vkWaitForFences(_core->getLogicalDevice(), 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(_core->getLogicalDevice(), 1, &fence);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    if(imageAvailableSemaphore.size() > 0)
    {
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(imageAvailableSemaphore.size());
        submitInfo.pWaitSemaphores = imageAvailableSemaphore.data();
    }
    if(renderFinishedSemaphore.size() > 0)
    {
        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(renderFinishedSemaphore.size());
        submitInfo.pSignalSemaphores = renderFinishedSemaphore.data();
    }

    if(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void command_buffer_system::freeCommandBuffers(VkCommandBuffer& commandBuffer)
{
    vkFreeCommandBuffers(_core->getLogicalDevice(), _commandPool, 1, &commandBuffer);
}

void command_buffer_system::freeCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers)
{
    vkFreeCommandBuffers(_core->getLogicalDevice(), _commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}

void command_buffer_system::cleanup()
{
    vkDestroyCommandPool(_core->getLogicalDevice(), _commandPool, nullptr);
    vkDestroyCommandPool(_core->getLogicalDevice(), _transferCommandPool, nullptr);
}

const std::array<VkClearValue, 2>& command_buffer_system::getClearValues() const
{
    return _clearValues;
}
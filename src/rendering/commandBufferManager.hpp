#pragma once

// STD includes
#include <vector>

// Third party includes
#include <entt/entt.hpp>
// GLFW
#include "wrapper/glfw.hpp"

// First party includes
#include "rendering/pipelineManager.hpp"

// Forward declarations
class rendering_system;
struct Model;

struct renderRequest
{
    VkCommandBuffer commandBuffer;

    E_RenderPassType renderPass;
    VkFramebuffer framebuffer;
    VkExtent2D extent;
    shaderPipeline pipeline;
    std::vector<VkDescriptorSet> descriptorSets;

    VkFence fence;

    // Push constants
    const void* pushConstants;
    size_t pushConstantsSize;
    VkShaderStageFlagBits pushConstantsStage;
};

struct FrameData
{
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
};

class command_buffer_system
{
public:
    command_buffer_system(rendering_system* core, VkQueue& graphicsQueue, VkQueue& presentationQueue);

    void createCommandPools();
    std::vector<VkCommandBuffer> createCommandBuffers(short unsigned int count);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkCommandBuffer generateCommandBuffer();
    VkResult beginRecordingCommandBuffer(VkCommandBuffer& commandBuffer, E_RenderPassType renderPassType, VkFramebuffer framebuffer, VkExtent2D extent);
    VkResult endRecordingCommandBuffer(VkCommandBuffer& commandBuffer);

    void recordCommandBuffer(uint32_t frameIndex, uint32_t swapChainImageIndex);
    void recordCommandBuffer(uint32_t frameIndex);
    void recordCommandBuffer(renderRequest& request, std::vector<Model>& models);
    
    void resetCommandBuffer(VkCommandBuffer& commandBuffer);
    void submitCommandBuffer(VkCommandBuffer& cmdBuffer, VkFence fence = VK_NULL_HANDLE);
    void submitCommandBuffer(VkCommandBuffer& cmdBuffer, std::vector<VkSemaphore>& imageAvailableSemaphore, std::vector<VkSemaphore>& renderFinishedSemaphore, VkFence fence = VK_NULL_HANDLE);

    void freeCommandBuffers(VkCommandBuffer& commandBuffer);
    void freeCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers);

    const std::array<VkClearValue, 2>& getClearValues() const;

    void cleanup();

private:
    rendering_system* _core;

    const unsigned int& _framesInFlight;

    VkCommandPool _commandPool;                             // command pool
    VkCommandPool _transferCommandPool;                     // transfer command pool

    VkQueue& _graphicsQueue;                                // graphics queue
    VkQueue& _presentationQueue;                            // presentation queue

    std::array<VkClearValue, 2> _clearValues;               // clear values for the render pass
    VkViewport _viewport;                                   // viewport
    VkRect2D _scissor;                                      // scissor

};
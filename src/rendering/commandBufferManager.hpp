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


struct PushConstant
{
    VkShaderStageFlagBits stageFlags = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    void* data = nullptr;
    uint32_t offset = 0;
    uint32_t size = 0;
};


struct renderRequest
{
    VkCommandBuffer commandBuffer;

    E_RenderPassType renderPass = E_RenderPassType::COLOR_DEPTH;
    VkFramebuffer framebuffer;
    VkExtent2D extent;
    shaderPipeline pipeline;

    VkViewport viewport = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    VkRect2D scissor = {{0, 0}, {0, 0}};

    VkFence fence;

    // Models
    std::vector<Model> models;

    // Descriptor sets
    std::vector<VkDescriptorSet> descriptorSets;

    // Push constants
    PushConstant generalPC;
    std::vector<PushConstant> perModelPC;
    bool useTextureLibraryBinds = false;
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

    void recordCommandBuffer(const renderRequest& request);
    
    void resetCommandBuffer(VkCommandBuffer& commandBuffer);
    void submitCommandBuffer(VkCommandBuffer& cmdBuffer, VkFence fence = VK_NULL_HANDLE);
    void submitCommandBuffer(VkCommandBuffer& cmdBuffer, std::vector<VkSemaphore>& imageAvailableSemaphore, std::vector<VkSemaphore>& renderFinishedSemaphore, VkFence fence = VK_NULL_HANDLE);

    void freeCommandBuffers(VkCommandBuffer& commandBuffer);
    void freeCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers);

    const std::array<VkClearValue, 2>& getClearValues() const;

    void cleanup();

private:

    VkViewport validateViewport(const VkViewport& viewport);
    VkRect2D validateScissor(const VkRect2D& scissor);

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
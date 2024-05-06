#pragma once 

// GLFW
#include "wrapper/glfw.hpp"
#include "rendering/descriptors/descriptorBuilder.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <memory>

class rendering_system;
struct shaderProgram;

enum class E_RenderPassType : unsigned int
{
    COLOR_DEPTH,                // 1 color 1 depth no stencil
    CUBE_MAP,                   // 1 color no depth no stencil
    SIZE
};

struct shaderPipeline
{
    VkPipeline pipeline;
    VkPipelineLayout layout;
};

struct bindingSlot
{
    uint32_t binding;
    VkDescriptorType type;
    VkShaderStageFlags stageFlags;
};

class pipeline_system
{
public:
    pipeline_system(rendering_system* core);

    void init();
    void createPipeline(std::string shaderProgramName, E_RenderPassType renderPassType = E_RenderPassType::COLOR_DEPTH);

    void cleanup();

    shaderPipeline& getPipeline(std::string name);
    VkRenderPass& getRenderPass(E_RenderPassType type) { return _renderPass[type]; }

private:
    // Generate the pipeline layout from reflection on the SPIR-V code
    VkPipelineLayout generatePipelineLayout(const shaderProgram& program);

    // Creates the shader stages info for the pipeline (how many shaders, which shaders, etc.)
    std::vector<VkPipelineShaderStageCreateInfo> createShaderStagesInfo(const shaderProgram& program);
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;

    // Defines the layout of the vertex data that will be passed to the vertex shader
    VkPipelineVertexInputStateCreateInfo createVertexInputInfo();
    VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
    VkVertexInputBindingDescription _bindingDescription;
    std::array<VkVertexInputAttributeDescription, 5> _attributeDescriptions;

    // Defines which primitive type the input assembly stage will use (point, line, triangle, etc.)
    VkPipelineInputAssemblyStateCreateInfo createInputAssemblyInfo();
    VkPipelineInputAssemblyStateCreateInfo _inputAssembly;

    // Defines the viewport and scissor rectangle
    VkPipelineDynamicStateCreateInfo createDynamicStateInfo();
    VkPipelineDynamicStateCreateInfo _dynamicState;
    std::vector<VkDynamicState> _dynamicStates;

    // Defines the viewport and scissor rectangle
    VkPipelineViewportStateCreateInfo createViewportStateInfo();
    VkPipelineViewportStateCreateInfo _viewportState;

    // Defines how the rasterizer will behave
    VkPipelineRasterizationStateCreateInfo createRasterizerInfo();
    VkPipelineRasterizationStateCreateInfo _rasterizer;

    // Defines how multisampling will be performed
    VkPipelineMultisampleStateCreateInfo createMultisamplingInfo();
    VkPipelineMultisampleStateCreateInfo _multisampling;

    // Defines how depth and stencil testing will be performed
    VkPipelineDepthStencilStateCreateInfo createDepthStencilInfo();
    VkPipelineDepthStencilStateCreateInfo _depthStencil;

    // Defines how color blending will be performed
    VkPipelineColorBlendStateCreateInfo createColorBlendingInfo();
    VkPipelineColorBlendStateCreateInfo _colorBlending;
    VkPipelineColorBlendAttachmentState _colorBlendAttachment;

    // Defines the layout of the descriptor sets that will be used in the pipeline
    VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayout, const std::vector<VkPushConstantRange>& pushConstantRanges);
    VkPipelineLayout _pipelineLayout;

    // Defines the render passes that the pipeline will be used with
    void initializeRenderPasses();

    std::unordered_map<E_RenderPassType, VkRenderPass> _renderPass;

    rendering_system* _core;
    std::unordered_map<std::string, shaderPipeline> _pipelines;
};

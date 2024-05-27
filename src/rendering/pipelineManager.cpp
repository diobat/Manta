#include "rendering/pipelineManager.hpp"

#include "rendering/rendering.hpp"
#include "rendering/shaderManager.hpp"
#include "rendering/resources/vertex.hpp"
#include "util/physicalDeviceHelper.hpp"

#include "spirv_cross.hpp"

#include <iostream>


pipeline_system::pipeline_system(rendering_system* core) :
    _core(core)
{
    ;
}

void pipeline_system::init()
{
    initializeRenderPasses();
}

void pipeline_system::createPipeline(std::string shaderProgramName, E_RenderPassType renderPassType)
{
    if(renderPassType == E_RenderPassType::SIZE)
    {
        throw std::runtime_error("Invalid render pass type");
    }

    shader_system& shaderManager = _core->getShaderSystem();
    auto shaderProgram = shaderManager.getShaderProgram(shaderProgramName);

    createShaderStagesInfo(shaderProgram);

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = _shaderStages.size();
    pipelineInfo.pStages = _shaderStages.data();
    pipelineInfo.pVertexInputState = &createVertexInputInfo();
    pipelineInfo.pInputAssemblyState = &createInputAssemblyInfo();
    pipelineInfo.pViewportState = &createViewportStateInfo();
    pipelineInfo.pRasterizationState = &createRasterizerInfo();
    pipelineInfo.pMultisampleState = &createMultisamplingInfo();
    pipelineInfo.pDepthStencilState = &createDepthStencilInfo(); // Optional
    pipelineInfo.pColorBlendState = &createColorBlendingInfo();
    pipelineInfo.pDynamicState = &createDynamicStateInfo(); // Optional
    pipelineInfo.layout = generatePipelineLayout(shaderProgram);
    pipelineInfo.renderPass = _renderPass[renderPassType];
    pipelineInfo.subpass = 0;

    shaderPipeline shaderPipeline;

    VkResult result = vkCreateGraphicsPipelines(_core->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &shaderPipeline.pipeline);
    shaderPipeline.layout = _pipelineLayout;

    if(result != VK_SUCCESS)
    {
        std::cerr << "Failed to create pipeline" << std::endl;
    }
    else
    {
        _pipelines[shaderProgramName] = shaderPipeline;
    }

    
}

void pipeline_system::cleanup()
{
    for (auto& pipeline : _pipelines)
    {
        vkDestroyPipeline(_core->getLogicalDevice(), pipeline.second.pipeline, nullptr);
        vkDestroyPipelineLayout(_core->getLogicalDevice(), pipeline.second.layout, nullptr);
    }


    for(auto& renderPass : _renderPass)
    {
        vkDestroyRenderPass(_core->getLogicalDevice(), renderPass.second, nullptr);
    }
}

shaderPipeline& pipeline_system::getPipeline(std::string name)
{

    if(_pipelines.find(name) == _pipelines.end())
    {
        throw std::runtime_error("Pipeline not found");
    }

    return _pipelines[name];
}

namespace
{
    // Struct to hold information about a shader resource
    struct ShaderResource {
        uint32_t set;
        uint32_t binding;
        spirv_cross::SPIRType::BaseType type;
        // Add more information as needed
    };
}

VkPipelineLayout pipeline_system::generatePipelineLayout(const shaderProgram& program)
{
    // Get max number of descriptor sets from the physical device
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(_core->getPhysicalDevice(), &properties);
    uint32_t maxSets = properties.limits.maxBoundDescriptorSets;

    // Create a vector of descriptor set layouts for each set
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindings;
    descriptorSetLayoutBindings.resize(maxSets);
    // Same for push constant ranges
    std::vector<VkPushConstantRange> pushConstantRanges_VERTEX;
    std::vector<VkPushConstantRange> pushConstantRanges_FRAGMENT;

    std::set<uint32_t> descriptorSetsUsed;

    // Iterate over all shaders in the program (vertex, fragment, etc.)
    for (auto& shader : program.shaders)
    {
        if(shader.VKmodule == VK_NULL_HANDLE)
        {
            continue;
        }

        // Load the SPIR-V code for this shader
        std::vector<uint32_t> spirv = shader.code;
        spirv_cross::Compiler comp(std::move(spirv)); 

        spirv_cross::ShaderResources resources = comp.get_shader_resources();

        // Push constants for vertex shader
        for (auto& resource : resources.push_constant_buffers) 
        {
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = _core->getShaderSystem().getVkShaderStageFlagBits(shader.type);

            if(pushConstantRange.stageFlags != VK_SHADER_STAGE_VERTEX_BIT)
            {
                continue;
            }

            pushConstantRange.offset = pushConstantRanges_VERTEX.empty() ? PUSH_CONSTANT_VERTEX_OFFSET : pushConstantRanges_VERTEX.back().offset + pushConstantRanges_VERTEX.back().size;
            pushConstantRange.size = comp.get_declared_struct_size(comp.get_type(resource.base_type_id));

            pushConstantRanges_VERTEX.push_back(pushConstantRange);
        }

        // Push constants for the fragment shader
        for(auto& resource : resources.push_constant_buffers)
        {
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = _core->getShaderSystem().getVkShaderStageFlagBits(shader.type);

            if(pushConstantRange.stageFlags != VK_SHADER_STAGE_FRAGMENT_BIT)
            {
                continue;
            }

            pushConstantRange.offset = pushConstantRanges_FRAGMENT.empty() ? PUSH_CONSTANT_FRAGMENT_OFFSET : pushConstantRanges_FRAGMENT.back().offset + pushConstantRanges_FRAGMENT.back().size;
            pushConstantRange.size = comp.get_declared_struct_size(comp.get_type(resource.base_type_id));
            pushConstantRange.size = pushConstantRange.size % PUSH_CONSTANT_FRAGMENT_OFFSET;
            pushConstantRanges_FRAGMENT.push_back(pushConstantRange);
        }

        // Uniform buffers
        for (auto& resource : resources.uniform_buffers) 
        {
            uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = _core->getShaderSystem().getVkShaderStageFlagBits(shader.type);

            descriptorSetsUsed.insert(set);
            descriptorSetLayoutBindings[set].push_back(layoutBinding);
        }

        // Sampled images
        for (auto& resource : resources.sampled_images) 
        {
            uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = _core->getShaderSystem().getVkShaderStageFlagBits(shader.type);
            
            descriptorSetsUsed.insert(set);
            descriptorSetLayoutBindings[set].push_back(layoutBinding);
        }

        // Texture images
        for (auto& resource : resources.separate_images)
        {
            uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);

            const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);

            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            layoutBinding.descriptorCount = type.array[0];
            layoutBinding.stageFlags = _core->getShaderSystem().getVkShaderStageFlagBits(shader.type);

            descriptorSetsUsed.insert(set);
            descriptorSetLayoutBindings[set].push_back(layoutBinding);
        }

        // Sampler
        for (auto& resource : resources.separate_samplers)
        {
            uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);

            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = _core->getShaderSystem().getVkShaderStageFlagBits(shader.type);

            descriptorSetsUsed.insert(set);
            descriptorSetLayoutBindings[set].push_back(layoutBinding);
        }

    }

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.resize(descriptorSetsUsed.size());

    for(auto& set : descriptorSetsUsed)
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings[set].size());
        layoutInfo.pBindings = descriptorSetLayoutBindings[set].data();
        if(vkCreateDescriptorSetLayout(_core->getLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[set]) != VK_SUCCESS)
        {
            std::cerr << "Failed to create descriptor set layout" << std::endl;
        }
    }

    std::vector<VkPushConstantRange> allPushConstantRanges;
    allPushConstantRanges.reserve(pushConstantRanges_VERTEX.size() + pushConstantRanges_FRAGMENT.size());
    allPushConstantRanges.insert(allPushConstantRanges.end(), pushConstantRanges_VERTEX.begin(), pushConstantRanges_VERTEX.end());
    allPushConstantRanges.insert(allPushConstantRanges.end(), pushConstantRanges_FRAGMENT.begin(), pushConstantRanges_FRAGMENT.end());

    VkPipelineLayout pipelineLayout = createPipelineLayout(descriptorSetLayouts, allPushConstantRanges);

    vkDestroyDescriptorSetLayout(_core->getLogicalDevice(), descriptorSetLayouts[0], nullptr);

    return pipelineLayout;
    
}

std::vector<VkPipelineShaderStageCreateInfo> pipeline_system::createShaderStagesInfo(const shaderProgram& program)
{
    shader_system& shaderManager = _core->getShaderSystem();

    _shaderStages.clear();
    for (auto& shader : program.shaders)
    {
        if (shader.name.empty())
        {
            continue;
        }

        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = shaderManager.getVkShaderStageFlagBits(shader.type);
        shaderStageInfo.module = shader.VKmodule;
        shaderStageInfo.pName = "main"; // entry point function for the shader

        _shaderStages.push_back(shaderStageInfo);
    }

    return _shaderStages;
}

VkPipelineVertexInputStateCreateInfo pipeline_system::createVertexInputInfo()
{
    // VkPipelineVertexInputStateCreateInfo temp{};

    // temp.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // temp.vertexBindingDescriptionCount = 1;
    // temp.pVertexBindingDescriptions = &Vertex::getBindingDescription();
    // temp.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::getAttributeDescriptions().size());;
    // temp.pVertexAttributeDescriptions = Vertex::getAttributeDescriptions().data();


    _bindingDescription = Vertex::getBindingDescription();
    _attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo temp{};
    temp.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    temp.vertexBindingDescriptionCount = 1;
    temp.pVertexBindingDescriptions = &_bindingDescription; 
    temp.vertexAttributeDescriptionCount = static_cast<uint32_t>(_attributeDescriptions.size());
    temp.pVertexAttributeDescriptions = _attributeDescriptions.data(); 

    _vertexInputInfo = temp;

    return _vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo pipeline_system::createInputAssemblyInfo()
{
    VkPipelineInputAssemblyStateCreateInfo temp{};

    temp.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    temp.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    temp.primitiveRestartEnable = VK_FALSE;

    _inputAssembly = temp;

    return _inputAssembly;
}

VkPipelineDynamicStateCreateInfo pipeline_system::createDynamicStateInfo()
{
    _dynamicStates.clear();
    _dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo temp{};

    temp.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    temp.dynamicStateCount = static_cast<uint32_t>(_dynamicStates.size());;
    temp.pDynamicStates = _dynamicStates.data();

    _dynamicState = temp;

    return _dynamicState;
}

VkPipelineViewportStateCreateInfo pipeline_system::createViewportStateInfo()
{
    _viewportState = {};

    _viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    _viewportState.viewportCount = 1;
    _viewportState.pViewports = nullptr;
    _viewportState.scissorCount = 1;
    _viewportState.pScissors = nullptr;
    _viewportState.pNext = nullptr;

    return _viewportState;
}

VkPipelineRasterizationStateCreateInfo pipeline_system::createRasterizerInfo()
{
    _rasterizer = {};

    _rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    _rasterizer.depthClampEnable = VK_FALSE;
    _rasterizer.rasterizerDiscardEnable = VK_FALSE;
    _rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    _rasterizer.lineWidth = 1.0f;
    _rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    _rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    _rasterizer.depthBiasEnable = VK_FALSE;
    _rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    _rasterizer.depthBiasClamp = 0.0f; // Optional
    _rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    return _rasterizer;
}

VkPipelineMultisampleStateCreateInfo pipeline_system::createMultisamplingInfo()
{
    _multisampling = {};

    _multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    _multisampling.sampleShadingEnable = VK_FALSE;
    _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    _multisampling.minSampleShading = 1.0f; // Optional
    _multisampling.pSampleMask = nullptr; // Optional
    _multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    _multisampling.alphaToOneEnable = VK_FALSE; // Optional

    return _multisampling;
}

VkPipelineDepthStencilStateCreateInfo pipeline_system::createDepthStencilInfo()
{
    _depthStencil = {};

    _depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    _depthStencil.depthTestEnable = VK_TRUE;
    _depthStencil.depthWriteEnable = VK_TRUE;
    _depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    _depthStencil.depthBoundsTestEnable = VK_FALSE;
    _depthStencil.minDepthBounds = 0.0f; // Optional
    _depthStencil.maxDepthBounds = 1.0f; // Optional
    _depthStencil.stencilTestEnable = VK_FALSE;
    _depthStencil.front = {}; // Optional
    _depthStencil.back = {}; // Optional

    return _depthStencil;
}

VkPipelineColorBlendStateCreateInfo pipeline_system::createColorBlendingInfo()
{
    _colorBlendAttachment = {};

    _colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    _colorBlendAttachment.blendEnable = VK_FALSE;
    _colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    _colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    _colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    _colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    _colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    _colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    _colorBlending = {};

    _colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    _colorBlending.logicOpEnable = VK_FALSE;
    _colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    _colorBlending.attachmentCount = 1;
    _colorBlending.pAttachments = &_colorBlendAttachment;
    _colorBlending.blendConstants[0] = 0.0f; // Optional
    _colorBlending.blendConstants[1] = 0.0f; // Optional
    _colorBlending.blendConstants[2] = 0.0f; // Optional
    _colorBlending.blendConstants[3] = 0.0f; // Optional

    return _colorBlending;
}

VkPipelineLayout pipeline_system::createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayout, const std::vector<VkPushConstantRange>& pushConstantRanges)
{
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayout.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    if(vkCreatePipelineLayout(_core->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
    {
        std::cerr << "Failed to create pipeline layout" << std::endl;
    }

    return _pipelineLayout;
}

void pipeline_system::initializeRenderPasses()
{
    // Standard renderpass
    	
    // Color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _core->getSwapChainSystem().getSwapChain().ImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat(_core->getPhysicalDevice());
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass;

    if(vkCreateRenderPass(_core->getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }

    _renderPass[E_RenderPassType::COLOR_DEPTH] = renderPass;

    // Cube map renderpass

    VkAttachmentDescription colorAttachmentCube{};
    colorAttachmentCube.format = VK_FORMAT_R8G8B8A8_SRGB;
    colorAttachmentCube.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentCube.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentCube.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentCube.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentCube.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorAttachmentRefCube{};
    colorAttachmentRefCube.attachment = 0;
    colorAttachmentRefCube.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassCube{};
    subpassCube.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassCube.colorAttachmentCount = 1;
    subpassCube.pColorAttachments = &colorAttachmentRefCube;

    VkSubpassDependency dependencyCube{};
    dependencyCube.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencyCube.dstSubpass = 0;
    dependencyCube.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencyCube.srcAccessMask = 0;
    dependencyCube.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencyCube.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachmentsCube = {colorAttachmentCube};
    VkRenderPassCreateInfo renderPassInfoCube{};
    renderPassInfoCube.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfoCube.attachmentCount = static_cast<uint32_t>(attachmentsCube.size());
    renderPassInfoCube.pAttachments = attachmentsCube.data();
    renderPassInfoCube.subpassCount = 1;
    renderPassInfoCube.pSubpasses = &subpassCube;
    renderPassInfoCube.dependencyCount = 1;
    renderPassInfoCube.pDependencies = &dependencyCube;

    VkRenderPass _renderPassCube;

    if(vkCreateRenderPass(_core->getLogicalDevice(), &renderPassInfoCube, nullptr, &_renderPassCube) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }

    _renderPass[E_RenderPassType::CUBE_MAP] = _renderPassCube;
}
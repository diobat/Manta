#include "rendering/strategy/SNode.hpp"

#include "rendering/strategy/SChain.hpp"
#include "rendering/rendering.hpp"
#include "core/settings.hpp"
#include "rendering/resources/memory.hpp"

#include "ECS/components/skybox.hpp"

StrategyNode::StrategyNode(const StrategyChain* chain) : _chain(chain)
{
    ;
}

//// Skybox Node

RenderSkyboxNode::RenderSkyboxNode(const StrategyChain* chain) : StrategyNode(chain)
{
    _chain->core()->getPipelineSystem().createPipeline("skybox");


}

void RenderSkyboxNode::run()
{
    uint32_t currentFrame = _chain->currentFrame();

    // Render request struct populating
    renderRequest request;
    request.commandBuffer = _chain->core()->getSwapChainSystem().getCommandBuffer(currentFrame);
    request.renderPass = E_RenderPassType::COLOR_DEPTH;
    request.framebuffer = _chain->core()->getSwapChainSystem().getFramebuffer(currentFrame);
    request.extent = _chain->core()->getSwapChainSystem().getSwapChain().Extent;
    request.pipeline = _chain->core()->getPipelineSystem().getPipeline("skybox");
    _chain->core()->getFrameManager().updateUniformBuffers(currentFrame);
    request.useTextureLibraryBinds = false;

    // Descriptor sets
    // MVP matrices
    request.descriptorSets.push_back(_chain->core()->getFrameManager().getDescriptorSet(descriptorSetType::MVP_MATRICES, currentFrame));

    // Cube model for skybox
    Model& cube = _chain->core()->getModelMeshLibrary().createModelFromMesh("cube", shapes::cube::mesh(glm::vec3(1.0f)));
    request.models = std::vector<Model>(6, cube);

    // Record request
    _chain->core()->getCommandBufferSystem().recordCommandBuffer(request);
}

void RenderSkyboxNode::prepare()
{
    auto& skyboxEntities = _chain->core()->getRegistry().view<Skybox>();

    Skybox& skybox = _chain->core()->getRegistry().get<Skybox>(*skyboxEntities.begin());


}

//// Opaque Node

RenderOpaqueNode::RenderOpaqueNode(const StrategyChain* chain) : StrategyNode(chain)
{
    ;
}

void RenderOpaqueNode::run()
{
    uint32_t currentFrame = _chain->currentFrame();

    // Render request struct populating
    renderRequest request;
    request.commandBuffer = _chain->core()->getSwapChainSystem().getCommandBuffer(currentFrame);
    request.renderPass = E_RenderPassType::COLOR_DEPTH;
    request.framebuffer = _chain->core()->getSwapChainSystem().getFramebuffer(currentFrame);
    request.extent = _chain->core()->getSwapChainSystem().getSwapChain().Extent;
    request.pipeline = _chain->core()->getPipelineSystem().getPipeline("basic");
    _chain->core()->getFrameManager().updateUniformBuffers(currentFrame);
    request.useTextureLibraryBinds = true;

    // Descriptor sets
    request.descriptorSets.push_back(_chain->core()->getFrameManager().getDescriptorSet(_ds)[currentFrame]); 

    // Gather models
    auto& allModelsView = _chain->core()->getRegistry().view<Model>();

    // Reserve space for models and per model push constants so that pointers don't get invalidated later
    request.models.reserve(allModelsView.size());
    request.perModelPC.reserve(allModelsView.size());

    // Gather models
    for(auto& entity : allModelsView)
    {
        request.models.push_back(_chain->core()->getRegistry().get<Model>(entity));
    }

    // Push constants
    for(int i = 0; i < request.models.size(); ++i)
    {
        PushConstant perModel_pc;
        perModel_pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        perModel_pc.data = &request.models[i].id;
        perModel_pc.size = sizeof(request.models[i].id);
        perModel_pc.offset = PUSH_CONSTANT_VERTEX_OFFSET;
        request.perModelPC.push_back(perModel_pc);
    }

    _chain->core()->getCommandBufferSystem().recordCommandBuffer(request);
}

void RenderOpaqueNode::prepare()
{
   _chain->core()->getPipelineSystem().createPipeline("basic");

    unsigned int framesinFlight = getSettingsData(_chain->core()->getScene()->getRegistry()).framesInFlight;

    std::vector<descriptorSetBindings> allFramesBindings;

    for(size_t i = 0; i < framesinFlight; i++)
    {
        descriptorSetBindings singleFrameBindings;

        descriptorBindingData mvpMatricesBinding;
        mvpMatricesBinding.binding = 0;
        mvpMatricesBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpMatricesBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        mvpMatricesBinding.data = &_chain->core()->getScene()->getRegistry().get<memoryBuffers>( _chain->core()->getScene()->getActiveCamera() ).buffers[i].descriptorInfo;
        singleFrameBindings.push_back(mvpMatricesBinding);

        descriptorBindingData modelMatrices;
        modelMatrices.binding = 1;
        modelMatrices.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        modelMatrices.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        modelMatrices.data = &_chain->core()->getFrameManager().getMemoryBuffer(descriptorSetType::MVP_MATRICES).buffers[i].descriptorInfo;
        singleFrameBindings.push_back(modelMatrices);

        descriptorBindingData sampler;
        sampler.binding = 2;
        sampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        sampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        sampler.data = &_chain->core()->getTextureSystem().getTextureSamplerDescriptor();
        singleFrameBindings.push_back(sampler);

        descriptorBindingData textureDiffuse;
        textureDiffuse.binding = 3;
        textureDiffuse.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        textureDiffuse.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        textureDiffuse.data = &_chain->core()->getTextureSystem().aggregateDescriptorTextureInfos(E_TextureType::DIFFUSE , kTextureArraySize);
        textureDiffuse.count = kTextureArraySize;
        singleFrameBindings.push_back(textureDiffuse);

        // descriptorBindingData textureCubemap;
        // textureCubemap.binding = 4;
        // textureCubemap.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        // textureCubemap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        // textureCubemap.data = &_chain->core()->getTextureSystem().aggregateDescriptorTextureInfos(E_TextureType::CUBEMAP, kCubemapArraySize);
        // textureCubemap.count = kCubemapArraySize;
        // singleFrameBindings.push_back(textureCubemap);

        allFramesBindings.push_back(singleFrameBindings);
    }

    _ds = _chain->core()->getFrameManager().compileDescriptorSet(allFramesBindings);
}


//// GUI Node OnFrameStart
renderGUIOnFrameStartNode::renderGUIOnFrameStartNode(const StrategyChain* chain) : StrategyNode(chain)
{
    ;
}

void renderGUIOnFrameStartNode::run()
{
    // Render GUI
    
}

void renderGUIOnFrameStartNode::prepare()
{
    // Prepare GUI
}

//// GUI Node OnFrameEnd

renderGUIOnFrameEndNode::renderGUIOnFrameEndNode(const StrategyChain* chain) : StrategyNode(chain)
{
    ;
}

void renderGUIOnFrameEndNode::run()
{
    // Render GUI
}

void renderGUIOnFrameEndNode::prepare()
{
    // Prepare GUI
}
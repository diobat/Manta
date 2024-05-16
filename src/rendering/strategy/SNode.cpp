#include "rendering/strategy/SNode.hpp"

#include "rendering/strategy/SChain.hpp"
#include "rendering/rendering.hpp"

#include "ECS/components/skybox.hpp"

StrategyNode::StrategyNode(const StrategyChain* chain) : _chain(chain)
{
    ;
}

//// Skybox Node

RenderSkyboxNode::RenderSkyboxNode(const StrategyChain* chain) : StrategyNode(chain)
{
    ;
}

void RenderSkyboxNode::run()
{
    // Render skybox
}

void RenderSkyboxNode::prepare()
{
    // Prepare skybox
    for(auto& skybox : _chain->core()->getRegistry().view<Skybox>())
    {
        // Prepare skybox
    }
    
    
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
    request.descriptorSets.push_back(_chain->core()->getFrameManager().getDescriptorSet(descriptorSetType::MVP_MATRICES, currentFrame));
    request.useTextureLibraryBinds = true;

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
    // Prepare opaque objects
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
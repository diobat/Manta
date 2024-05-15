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

    // Gather all models in a vector
    std::vector<Model> models;
    for(auto& entity : _chain->core()->getRegistry().view<Model>())
    {
        Model model = _chain->core()->getRegistry().get<Model>(entity);
        models.push_back(model);
    }

    _chain->core()->getCommandBufferSystem().recordCommandBuffer(request, models);

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
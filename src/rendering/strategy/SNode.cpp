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
    _chain->core()->getCommandBufferSystem().recordCommandBuffer(_chain->currentFrame());
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
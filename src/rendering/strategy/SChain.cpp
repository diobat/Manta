#include "rendering/strategy/SChain.hpp"

#include "rendering/strategy/SNode.hpp"
#include "rendering/rendering.hpp" 
#include "ECS/components/skybox.hpp"


StrategyChain::StrategyChain(rendering_system* core) : 
    _core(core),
    _firstRun(true)
{
    ;
}

bool StrategyChain::add(std::shared_ptr<StrategyNode> node)
{
    _nodes.push_back(node);
    return true;
}

void StrategyChain::clear()
{
    _nodes.clear();
}

void StrategyChain::run()
{

    if(_firstRun)
    {    
        reserveResources();
        reserveNodeResources();
        _firstRun = false;
    }

    beginRenderPass();

    for (auto& node : _nodes)
    {
        node->run();
    }

    endRenderPass();
}

rendering_system* StrategyChain::core() const
{
    return _core;
}

void StrategyChain::reserveNodeResources()
{
    for(auto& node : _nodes)
    {
        node->prepare();
    }
}   

void StrategyChain::beginRenderPass()
{   
    // Get the current frame
    _currentFrame = _core->getSwapChainSystem().getNextImageIndex();

    // Signal frame Start to imGUI
    _core->getImGUIHandler().onFrameStart();

    // Start recording the command buffer
    VkCommandBuffer commandBuffer = _core->getSwapChainSystem().getCommandBuffer(_currentFrame);
    E_RenderPassType renderPassType = E_RenderPassType::COLOR_DEPTH;
    VkFramebuffer framebuffer = _core->getSwapChainSystem().getSwapChain().Framebuffers[_currentFrame];
    VkExtent2D extent = _core->getSwapChainSystem().getSwapChain().Extent;

    _core->getCommandBufferSystem().beginRecordingCommandBuffer(commandBuffer, renderPassType, framebuffer, extent);

}

void StrategyChain::endRenderPass()
{
    // Signal frame End to imGUI
    _core->getImGUIHandler().onFrameEnd(_currentFrame);

    // End the command buffer
    VkCommandBuffer commandBuffer = _core->getSwapChainSystem().getCommandBuffer(_currentFrame);
    _core->getCommandBufferSystem().endRecordingCommandBuffer(commandBuffer);

    std::vector<VkSemaphore> imageAvailableSemaphores;
    imageAvailableSemaphores.push_back(_core->getSwapChainSystem().getImageAvailableSemaphore(_currentFrame));
    std::vector<VkSemaphore> renderFinishedSemaphores;
    renderFinishedSemaphores.push_back(_core->getSwapChainSystem().getRenderFinishedSemaphore(_currentFrame));

    // Submit the command buffer
    _core->getCommandBufferSystem().submitCommandBuffer(
        commandBuffer, 
        imageAvailableSemaphores, 
        renderFinishedSemaphores, 
        _core->getSwapChainSystem().getInFlightFence(_currentFrame));

    _core->getSwapChainSystem().presentImage(_currentFrame);
}

// PBSShadingStrategyChain
PBSShadingStrategyChain::PBSShadingStrategyChain(rendering_system* engine) : StrategyChain(engine)
{
    add(std::make_shared<RenderSkyboxNode>(this));
    add(std::make_shared<RenderOpaqueNode>(this));
}

bool PBSShadingStrategyChain::reserveResources()
{
    // Create diffuse irradiance map

    auto skyboxViews = _core->getRegistry().view<Skybox>();

    Skybox& skybox = skyboxViews.get<Skybox>(*skyboxViews.begin());

    //skybox.irradianceMap = _core->getTextureSystem().bakeIrradianceDiffuseLightmap(skybox.texture, true);

    // Create pre-filtered specular map
    //skybox.prefilteredMap = _core->getTextureSystem().bakeIrradianceSpecularLightmap(skybox.texture, true);

    // Create BRDF LUT
    
    return true;
}

void PBSShadingStrategyChain::bakeDiffuseIrradiance()
{
    // Create a cube map for the diffuse irradiance
}

void PBSShadingStrategyChain::bakeSpecularIrradiance()
{
    // Create a cube map for the specular irradiance
}

void PBSShadingStrategyChain::bakeBRDFLUT()
{
    // Create a 2D texture for the BRDF LUT
}
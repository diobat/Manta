#include "rendering/swapChainManager.hpp"

#include "rendering/rendering.hpp"
#include "util/physicalDeviceHelper.hpp"
#include "core/settings.hpp"

#include <set>

swap_chain_system::swap_chain_system(rendering_system* core, VkSurfaceKHR& surface, VkQueue& presentationQueue) :
    _core(core), 
    _surface(surface),
    _presentationQueue(presentationQueue)
{
    ;
}

uint32_t swap_chain_system::getNextImageIndex()
{
    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    VkResult initialState = vkWaitForFences(_core->getLogicalDevice(), 1 , &_swapChain.inFlightFences[_swapChain.currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    // Reset the signalled fence
    vkResetFences(_core->getLogicalDevice(), 1, &_swapChain.inFlightFences[_swapChain.currentFrame]);

    // Aquire the next image from the swap chain extension
    VkResult result = vkAcquireNextImageKHR(
        _core->getLogicalDevice(),
        _swapChain.swapChain,
        UINT64_MAX,
        _swapChain.imageAvailableSemaphores[_swapChain.currentFrame],
        _swapChain.inFlightFences[_swapChain.currentFrame],
        &_swapChain.ImageIndices[_swapChain.currentFrame]);

    // Reset Command Buffer
    _core->getCommandBufferSystem().resetCommandBuffer(_swapChain.commandBuffers[_swapChain.currentFrame]);

    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate();
        return getNextImageIndex();
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    // Set aside return value
    uint32_t fetchedFrameNumber = _swapChain.currentFrame;

    // Increate current frame counter
    _swapChain.currentFrame = (_swapChain.currentFrame + 1) % getSettingsData(_core->getRegistry()).framesInFlight;

    return fetchedFrameNumber;
}

void swap_chain_system::presentImage(uint32_t frame)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &_swapChain.renderFinishedSemaphores[frame];

    VkSwapchainKHR swapChains[] = {_swapChain.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &_swapChain.ImageIndices[frame];
    presentInfo.pResults = nullptr; // Optional

    VkResult result = vkQueuePresentKHR(_presentationQueue, &presentInfo);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreate();
    }
    else if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }
}

bool swap_chain_system::createSwapChain()
{
    
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_core->getPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // determine the number of images in the swap chain
    uint32_t imageCount = getSettingsData(_core->getRegistry()).framesInFlight;
    if(swapChainSupport.capabilities.maxImageCount < imageCount && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        std::runtime_error("Requested SwapChain image count is greater than the maximum image count supported by the swap chain!");
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(_core->getPhysicalDevice(), _surface);

    // Piggyback on std::set guarantee they are unique
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value()};
    // Back to vector so we can use .data()
    std::vector<uint32_t> queueFamilyIndices(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());

    // Sharing mode is exclusive if the graphics and present queues are the same
    if(queueFamilyIndices.size() > 1)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if(vkCreateSwapchainKHR(_core->getLogicalDevice(), &createInfo, nullptr, &_swapChain.swapChain) != VK_SUCCESS)
    {
        return false;
    }

    vkGetSwapchainImagesKHR(_core->getLogicalDevice(), _swapChain.swapChain, &imageCount, nullptr);
    _swapChain.Images.resize(imageCount);
    vkGetSwapchainImagesKHR(_core->getLogicalDevice(), _swapChain.swapChain, &imageCount, _swapChain.Images.data());
    _swapChain.ImageFormat = surfaceFormat.format;
    _swapChain.Extent = extent;

    _swapChain.ImageIndices.resize(imageCount);
    _swapChain.commandBuffers = std::vector<VkCommandBuffer>(imageCount, VK_NULL_HANDLE) ;       // Need to initialize these buffers

    return true;
}

void swap_chain_system::createImageViews()
{
    _swapChain.ImageViews.resize(_swapChain.Images.size());

    for(size_t i(0); i < _swapChain.Images.size(); ++i)
    {
        _swapChain.ImageViews[i] = _core->getTextureSystem().createImageView(_swapChain.Images[i], _swapChain.ImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1 , VK_IMAGE_VIEW_TYPE_2D);
    }
}

void swap_chain_system::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat(_core->getPhysicalDevice());

    _swapChain.depthImage = _core->getTextureSystem().createImage(_swapChain.Extent.width, _swapChain.Extent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    _swapChain.depthImage.imageView = _core->getTextureSystem().createImageView(_swapChain.depthImage.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 0,1, VK_IMAGE_VIEW_TYPE_2D);

    _core->getTextureSystem().transitionImageLayout(_swapChain.depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void swap_chain_system::createFramebuffers()
{
    _swapChain.Framebuffers.resize(_swapChain.ImageViews.size());

    for(size_t i(0); i < _swapChain.ImageViews.size(); ++i)
    {
        std::array<VkImageView, 2> attachments = {
            _swapChain.ImageViews[i],
            _swapChain.depthImage.imageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _core->getPipelineSystem().getRenderPass(E_RenderPassType::COLOR_DEPTH);
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _swapChain.Extent.width;
        framebufferInfo.height = _swapChain.Extent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(_core->getLogicalDevice(), &framebufferInfo, nullptr, &_swapChain.Framebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void swap_chain_system::createCommandBuffers()
{
    _swapChain.commandBuffers = _core->getCommandBufferSystem().createCommandBuffers(_swapChain.Images.size());
}

void swap_chain_system::createSyncObjects()
{
    unsigned int framesinFlight = getSettingsData(_core->getScene()->getRegistry()).framesInFlight;

    _swapChain.imageAvailableSemaphores.resize(framesinFlight);
    _swapChain.renderFinishedSemaphores.resize(framesinFlight);
    _swapChain.inFlightFences.resize(framesinFlight);
    
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice device = _core->getLogicalDevice();

    for(size_t i(0); i < framesinFlight; ++i)
    {
        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_swapChain.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_swapChain.renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &_swapChain.inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void swap_chain_system::recreate()
{
    vkDeviceWaitIdle(_core->getLogicalDevice());

    cleanup();

    createSwapChain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
    createCommandBuffers();
    createSyncObjects();
}

void swap_chain_system::cleanup()
{
    // Image Views
    for(auto imageView : _swapChain.ImageViews)
    {
        vkDestroyImageView(_core->getLogicalDevice(), imageView, nullptr);
    }

    // Swap Chain extension
    vkDestroySwapchainKHR(_core->getLogicalDevice(), _swapChain.swapChain, nullptr);

    // Framebuffers
    for(auto framebuffer : _swapChain.Framebuffers)
    {
        vkDestroyFramebuffer(_core->getLogicalDevice(), framebuffer, nullptr);
    }

    // Depth Image
    _core->getTextureSystem().cleanupImage(_swapChain.depthImage);

    // Fences
    for(size_t i(0); i < _swapChain.inFlightFences.size(); ++i) {
    vkDestroyFence(_core->getLogicalDevice(), _swapChain.inFlightFences[i], nullptr);
    }

    // Image Available Semaphores
    for(size_t i(0); i < _swapChain.imageAvailableSemaphores.size(); ++i) {
    vkDestroySemaphore(_core->getLogicalDevice(), _swapChain.imageAvailableSemaphores[i], nullptr);
    }

    // Render Finished Semaphores
    for(size_t i(0); i < _swapChain.renderFinishedSemaphores.size(); ++i) {
    vkDestroySemaphore(_core->getLogicalDevice(), _swapChain.renderFinishedSemaphores[i], nullptr);
    }

}

swapChain& swap_chain_system::getSwapChain()
{
    return _swapChain;
}

uint32_t swap_chain_system::getSwapChainImageIndex(uint32_t index) const
{
    return _swapChain.ImageIndices[index];
}

const VkCommandBuffer& swap_chain_system::getCommandBuffer(uint32_t index) const
{
    return _swapChain.commandBuffers[index];
}

const VkImageView& swap_chain_system::getImageView(uint32_t index) const
{
    return _swapChain.ImageViews[index];
}

const VkFramebuffer& swap_chain_system::getFramebuffer(uint32_t index) const
{
    return _swapChain.Framebuffers[index];
}

const VkFence& swap_chain_system::getInFlightFence(uint32_t index) const
{
    return _swapChain.inFlightFences[index];
}

const VkSemaphore& swap_chain_system::getImageAvailableSemaphore(uint32_t index) const
{
    return _swapChain.imageAvailableSemaphores[index];
}

const VkSemaphore& swap_chain_system::getRenderFinishedSemaphore(uint32_t index) const
{
    return _swapChain.renderFinishedSemaphores[index];
}

SwapChainSupportDetails swap_chain_system::querySwapChainSupport(VkPhysicalDevice device) const
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

    if(formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

    if(presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR swap_chain_system::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR swap_chain_system::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D swap_chain_system::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
    if(capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(_core->getWindow(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

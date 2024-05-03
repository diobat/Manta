#include "rendering/swapChainManager.hpp"

#include "rendering/rendering.hpp"
#include "util/physicalDeviceHelper.hpp"

#include <set>

swap_chain_system::swap_chain_system(rendering_system* core, VkSurfaceKHR& surface) :
    _core(core), 
    _surface(surface)
{
    ;
}

uint32_t swap_chain_system::getNextImageIndex(VkSemaphore semaphore, VkFence fence)
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(_core->getLogicalDevice(), _swapChain.swapChain, UINT64_MAX, semaphore, fence, &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate();
        return getNextImageIndex(semaphore, fence);
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    return imageIndex;
}

bool swap_chain_system::createSwapChain()
{
    
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_core->getPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // determine the number of images in the swap chain
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
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

    return true;
}

void swap_chain_system::createImageViews()
{
    _swapChain.ImageViews.resize(_swapChain.Images.size());

    for(size_t i(0); i < _swapChain.Images.size(); ++i)
    {
        _swapChain.ImageViews[i] = _core->getTextureSystem().createImageView(_swapChain.Images[i], _swapChain.ImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void swap_chain_system::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat(_core->getPhysicalDevice());

    _swapChain.depthImage = _core->getTextureSystem().createImage(_swapChain.Extent.width, _swapChain.Extent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    _swapChain.depthImage.imageView = _core->getTextureSystem().createImageView(_swapChain.depthImage.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    _core->getTextureSystem().transitionImageLayout(_swapChain.depthImage.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
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
        framebufferInfo.renderPass = _core->getPipelineSystem().getRenderPass();
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

void swap_chain_system::recreate()
{
    vkDeviceWaitIdle(_core->getLogicalDevice());

    cleanup();

    createSwapChain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

void swap_chain_system::cleanup()
{
    for(auto imageView : _swapChain.ImageViews)
    {
        vkDestroyImageView(_core->getLogicalDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(_core->getLogicalDevice(), _swapChain.swapChain, nullptr);

    for(auto framebuffer : _swapChain.Framebuffers)
    {
        vkDestroyFramebuffer(_core->getLogicalDevice(), framebuffer, nullptr);
    }

    _core->getTextureSystem().cleanupImage(_swapChain.depthImage);

}

swapChain& swap_chain_system::getSwapChain()
{
    return _swapChain;
}

SwapChainSupportDetails swap_chain_system::querySwapChainSupport(VkPhysicalDevice device)
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

VkSurfaceFormatKHR swap_chain_system::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR swap_chain_system::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D swap_chain_system::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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
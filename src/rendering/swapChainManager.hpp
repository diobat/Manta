#pragma once

// GLFW
#include "wrapper/glfw.hpp"

#include "rendering/resources/texture.hpp"

#include <vector>

class rendering_system;

struct swapChain
{
    VkSwapchainKHR swapChain;
    std::vector<VkImage> Images;
    image depthImage;
    VkFormat ImageFormat;
    VkExtent2D Extent;
    std::vector<VkImageView> ImageViews;
    std::vector<VkFramebuffer> Framebuffers;
};

struct SwapChainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


class swap_chain_system
{
public:
    swap_chain_system(rendering_system* core, VkSurfaceKHR& surface);

    uint32_t getNextImageIndex(VkSemaphore semaphore, VkFence fence);

    bool createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createFramebuffers();

    void recreate();

    void cleanup();

    swapChain& getSwapChain();

private:
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    swapChain _swapChain;

    rendering_system* _core;
    VkSurfaceKHR& _surface;
};
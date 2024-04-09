#pragma once

// GLFW
#include "wrapper/glfw.hpp"

#include <vector>

class rendering_system;

struct swapChain
{
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
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

    bool createSwapChain();

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
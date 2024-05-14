#pragma once

// GLFW
#include "wrapper/glfw.hpp"

#include "rendering/resources/texture.hpp"

#include <vector>

class rendering_system;

struct SwapChain_FrameData
{
    VkImage image;
    
    VkImageView imageView;
    VkFramebuffer framebuffer;

    // Sync objects
    VkFence inFlightFence;                   // in flight fence
    VkSemaphore imageAvailableSemaphore;     // image available semaphore
    VkSemaphore renderFinishedSemaphore;     // render finished semaphore
};

struct swapChain
{
    VkSwapchainKHR swapChain;

    std::vector<VkImage> Images;
    std::vector<uint32_t> ImageIndices;                    // Indices of each swapchain image insiode the extension
    uint32_t currentFrame = 0;                             // current frame

    // Command buffers
    std::vector<VkCommandBuffer> commandBuffers;           // command buffers, one for each frame

    image depthImage;
    VkFormat ImageFormat;
    VkExtent2D Extent;
    std::vector<VkImageView> ImageViews;
    std::vector<VkFramebuffer> Framebuffers;

    // Sync objects
    std::vector<VkFence> inFlightFences;                   // in flight fence
    std::vector<VkSemaphore> imageAvailableSemaphores;     // image available semaphore
    std::vector<VkSemaphore> renderFinishedSemaphores;     // render finished semaphore

    // std::vector<SwapChain_FrameData> frames;
};

struct SwapChainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


class swap_chain_system
{
public:
    swap_chain_system(rendering_system* core, VkSurfaceKHR& surface, VkQueue& presentationQueue);

    uint32_t getNextImageIndex();
    void presentImage(uint32_t frame);

    bool createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createFramebuffers();
    void createCommandBuffers();
    void createSyncObjects();

    void recreate();
    void cleanup();

    swapChain& getSwapChain();
    uint32_t getSwapChainImageIndex(uint32_t index) const;
    const VkCommandBuffer& getCommandBuffer(uint32_t index) const;
    const VkImageView& getImageView(uint32_t index) const;
    const VkFramebuffer& getFramebuffer(uint32_t index) const;
    const VkFence& getInFlightFence(uint32_t index) const;
    const VkSemaphore& getImageAvailableSemaphore(uint32_t index) const;
    const VkSemaphore& getRenderFinishedSemaphore(uint32_t index) const;

private:
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

    swapChain _swapChain;

    VkQueue& _presentationQueue;

    rendering_system* _core;
    VkSurfaceKHR& _surface;
};
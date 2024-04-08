#pragma once

// GLFW
#include "wrapper/glfw.hpp"

#include <string>

class rendering_system;

struct image
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView imageView;
    VkFormat format;
    VkImageLayout layout;
    uint32_t mipLevels;
    VkDescriptorImageInfo descriptor;
};

class texture_system
{
public:
    texture_system(rendering_system* rendering);

    image createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

    image createTextureFromImageFile(const std::string& path);


    VkImageView createImageView(image& img, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    VkImageView createTextureImageView(image& img, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

    // Exists temporarily as some older code depends on overloading with a different signature
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);


    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
private:
    bool hasStencilComponent(VkFormat format);
    void generateMipMaps(VkImage& image, VkFormat format, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels);

    rendering_system* _core;
};
#pragma once

// GLFW
#include "wrapper/glfw.hpp"

#include <string>
#include <unordered_map>
#include <memory>

#include "util/imageData.hpp"

class rendering_system;
struct externalTextureLoaderHelper
{
    std::vector<std::string> diffusePaths;
    std::vector<std::string> specularPaths;
    std::vector<std::string> normalPaths;
    std::vector<std::string> roughnessPaths;
    std::vector<std::string> lightmapPaths;
};

enum class E_TextureType : unsigned int
{
    DIFFUSE,
    SPECULAR,
    NORMAL,
    HEIGHT,
    LIGHTMAP,
    ROUGHNESS,
    CUBEMAP,
    SIZE
};

struct image
{
    E_TextureType type;
    unsigned int id;

    VkImage image;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkFormat format;
    VkImageLayout layout;
    uint32_t mipLevels;
    VkDescriptorImageInfo descriptor;
};

class texture_system
{
public:
    texture_system(rendering_system* rendering);

    void init();
    void initTextureSampler();
    VkSampler& getTextureSampler();
    VkDescriptorImageInfo& getTextureSamplerDescriptor();

    uint32_t getMipLevels() const { return _mipLevels; }

    image createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool isCubeMap = false);

    // Equirectangular images
    image createTexture(const std::string& filePath, E_TextureType type = E_TextureType::DIFFUSE, bool addToCache = true);
    image createTexture(const loadedImageDataRGB, VkFormat format, E_TextureType type = E_TextureType::DIFFUSE,  bool addToCache = true);
    image createTexture(const loadedImageDataHDR, VkFormat format, E_TextureType type = E_TextureType::DIFFUSE,  bool addToCache = true);

    // Cubemaps
    image bakeCubemap(const std::string& filePath, bool addToCache = true);
    image bakeCubemapFromFlat(image img, bool addToCache = true);


    VkImageView createImageView(image& img, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);

    VkImageView createTextureImageView(image& img, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

    std::vector<VkDescriptorImageInfo>& aggregateDescriptorTextureInfos(E_TextureType type,  size_t returnVectorSize);

    // Exists temporarily as some older code depends on overloading with a different signature
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    // Resource release functions
    void cleanup();
    void cleanupImage(image& img);

private:

    void addTextureToCache(const E_TextureType type, image& img);
    bool hasStencilComponent(VkFormat format) const;
    void generateMipMaps(VkImage& image, VkFormat format, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels);

    image _defaultTexture; 

    std::unordered_map<E_TextureType, std::shared_ptr<std::vector<image>>> _textures;
    std::unordered_map<E_TextureType, std::vector<VkDescriptorImageInfo>> _textureDescriptors;

    uint32_t _mipLevels = 1;                                // mip levels
    VkSampler _textureSampler;
    VkDescriptorImageInfo _textureSamplerDescriptor;

    rendering_system* _core;
};  
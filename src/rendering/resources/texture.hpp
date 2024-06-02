#pragma once

// GLFW
#include "wrapper/glfw.hpp"

#include <vector>
#include <string>

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

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    uint32_t layers = 1;
    uint32_t mipLevels = 1;

    VkImage image;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkFormat format;
    VkImageLayout layout;
    VkDescriptorImageInfo descriptor;
};

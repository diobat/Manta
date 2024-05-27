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

    VkImage image;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkFormat format;
    VkImageLayout layout;
    uint32_t mipLevels;
    VkDescriptorImageInfo descriptor;
};

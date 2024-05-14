#include "util/imageData.hpp"

#include "helpers/RootDir.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

loadedImageDataRGB STB_load_image(const std::string& path)
{
    loadedImageDataRGB img;
    img.data = stbi_load(path.c_str(), &img.width, &img.height, &img.channels, STBI_rgb_alpha);
    return img;
};

loadedImageDataHDR STB_load_image_HDR(const std::string &path)
{
    loadedImageDataHDR img;
    std::string fullPath = ROOT_DIR + path;
    img.data = stbi_loadf(fullPath.c_str(), &img.width, &img.height, &img.channels, STBI_rgb_alpha);
    return img;
}

loadedImageDataRGB ASSIMP_load_image(const aiTexture* texture)
{
    loadedImageDataRGB img;
    img.data = stbi_load_from_memory(
        reinterpret_cast<const unsigned char*>(texture->pcData),
        texture->mWidth,
        &img.width,
        &img.height,
        &img.channels,
        STBI_rgb_alpha);
    return img;
};

void free_image(unsigned char* data)
{
    stbi_image_free(data);
}

void free_image(float* data)
{
    stbi_image_free(data);
}
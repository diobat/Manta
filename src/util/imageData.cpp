#include "util/imageData.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

loadedImageData STB_load_image(const std::string& path)
{
    loadedImageData img;
    img.data = stbi_load(path.c_str(), &img.width, &img.height, &img.channels, STBI_rgb_alpha);
    return img;
};

loadedImageData ASSIMP_load_image(const aiTexture* texture)
{
    loadedImageData img;
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
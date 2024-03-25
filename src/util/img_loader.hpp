#pragma once

#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct loadedImageData
{
    stbi_uc* data;
    int width;
    int height;
    int channels;
};

loadedImageData load_image(const std::string& path)
{
    loadedImageData img;
    img.data = stbi_load(path.c_str(), &img.width, &img.height, &img.channels, STBI_rgb_alpha);
    return img;
};

void free_image(unsigned char* data)
{
    stbi_image_free(data);
}
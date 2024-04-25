#pragma once

#include <string>
#include <assimp/scene.h>

struct loadedImageData
{
    unsigned char* data;
    int width;
    int height;
    int channels;
};

loadedImageData STB_load_image(const std::string& path);

loadedImageData ASSIMP_load_image(const aiTexture* texture);

void free_image(unsigned char* data);

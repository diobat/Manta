#pragma once

#include <string>
#include <assimp/scene.h>

template<typename T>
struct loadedImageData
{
    T* data;
    int width;
    int height;
    int channels;
};

using loadedImageDataRGB = loadedImageData<unsigned char>;
using loadedImageDataHDR = loadedImageData<float>;

loadedImageDataRGB STB_load_image(const std::string& path);

loadedImageDataHDR STB_load_image_HDR(const std::string &path);

loadedImageDataRGB ASSIMP_load_image(const aiTexture* texture);

void free_image(unsigned char* data);

void free_image(float* data);
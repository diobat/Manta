#pragma once

#include "rendering/resources/model.hpp"
#include "rendering/resources/texture.hpp"
#include "util/VertexShapes.hpp"

struct Skybox
{
    Model cube;
    image skyboxTexture;
    bool enabled = false;
};

Skybox createSkybox(std::shared_ptr<rendering_system> core, const std::string& path);